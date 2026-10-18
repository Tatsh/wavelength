#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <memory>

#include <libezctl.h>
#include <sifrpc.h>

#include "os/system.h"
#include "script/dataarray.h"

int main(int argc, char **argv);

/**
 * Check of the executable's code against the CRC-32 the `xs` block of the configuration records.
 *
 * The block lists the address of main() at build time (`main`), the offset of the checked code
 * from that address (`offset`), its size in bytes (`size`), and the expected CRC (`crc`). Each
 * Poll() hashes up to a mebibyte of the code. Once the whole range is hashed and the CRC differs,
 * the network is marked as checked through the eznetctl server, and its interface reports a
 * disconnected state from then on. The check restarts from the beginning of the range either way.
 *
 * Every member is inline, as the European executable inlines the check into each caller. The
 * class name is inferred.
 */
class CodeCheck {
public:
    /**
     * Allocate the cells of the check's state.
     *
     * The cells are separate allocations rather than static storage.
     */
    static void Create() {
        sStart = std::make_unique<const unsigned char *>();
        sEnd = std::make_unique<const unsigned char *>();
        sExpected = std::make_unique<int>();
        sCursor = std::make_unique<const unsigned char *>();
        sCrc = std::make_unique<unsigned int>();
    }

    /** Release the cells and the CRC table. */
    static void Destroy() {
        sStart.reset();
        sEnd.reset();
        sExpected.reset();
        sCursor.reset();
        sCrc.reset();
        sTable.reset();
    }

    /**
     * Read the range from the configuration, and hash the next mebibyte of it.
     *
     * The first call builds the CRC table and starts at the beginning of the range.
     */
    static void Poll() {
        DataArray *pXs = SystemConfig()->FindArray("xs", false);
        const int nMain = pXs->FindArray("main", false)->Int(1);
        const int nOffset = pXs->FindArray("offset", false)->Int(1);
        const int nSize = pXs->FindArray("size", false)->Int(1);
        const int nCrc = pXs->FindArray("crc", false)->Int(1);
        // Taking main()'s address is what the check is for.
        const auto *pStart = reinterpret_cast<const unsigned char *>(&main) + (nOffset - nMain);
        *sStart = pStart;
        *sEnd = pStart + nSize;
        *sExpected = nCrc;
        if (sTableBuilt == 0) {
            BuildTable();
            sTableBuilt = 1;
            *sCursor = pStart;
            *sCrc = kCrcInitial;
        }

        const unsigned char *pCursor = *sCursor;
        const unsigned char *pLimit = pCursor + kBytesPerPoll;
        const unsigned char *pStop = *sEnd < pLimit ? *sEnd : pLimit;
        unsigned int crc = *sCrc;
        for (int nLeft = pStop - pCursor; nLeft-- != 0;) {
            crc = sTable[(crc ^ *pCursor++) & kByteMask] ^ (crc >> kBitsPerByte);
        }
        *sCrc = crc;
        *sCursor = pCursor;
        if (pCursor != *sEnd) {
            return;
        }
        if (~crc != static_cast<unsigned int>(*sExpected)) {
            sceSifCallRpc(ezNetCtlGetClient(),
                          EZ_NETCTL_FUNCTION_SET_NET_CHECKED,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          nullptr);
        }
        *sCursor = *sStart;
        *sCrc = kCrcInitial;
    }

private:
    /** The first byte of the checked code. @ghidraAddress PAL: 0x004d2b2c */
    static std::unique_ptr<const unsigned char *> sStart;

    /** The byte after the checked code. @ghidraAddress PAL: 0x004d2b30 */
    static std::unique_ptr<const unsigned char *> sEnd;

    /** The next byte Poll() hashes. @ghidraAddress PAL: 0x004d2b34 */
    static std::unique_ptr<const unsigned char *> sCursor;

    /** The CRC of the bytes hashed so far, not yet inverted. @ghidraAddress PAL: 0x004d2b38 */
    static std::unique_ptr<unsigned int> sCrc;

    /** The CRC the configuration expects. @ghidraAddress PAL: 0x004d2b3c */
    static std::unique_ptr<int> sExpected;

    /** The CRC of each byte value. @ghidraAddress PAL: 0x004d2b40 */
    static std::unique_ptr<unsigned int[]> sTable;

    /** Nonzero once Poll() has built the table. @ghidraAddress PAL: 0x0042e580 */
    static int sTableBuilt;

    static constexpr int kTableSize = 256;
    static constexpr unsigned int kPolynomial = 0x04c11db7;
    static constexpr unsigned int kTopBit = 0x80000000;
    static constexpr unsigned int kCrcInitial = 0xffffffff;
    static constexpr unsigned int kByteMask = 0xff;
    static constexpr int kBitsPerByte = 8;
    static constexpr int kBitsPerWord = 32;
    static constexpr int kBytesPerPoll = 0x100000;

    // Reverse the order of the low nBits bits of value.
    static unsigned int Reflect(unsigned int value, int nBits) {
        unsigned int reflected = 0;
        for (int bit = 1; bit < nBits + 1; ++bit) {
            if ((value & 1) != 0) {
                reflected |= 1 << (nBits - bit);
            }
            value >>= 1;
        }
        return reflected;
    }

    // Build the table of the reflected CRC-32 one bit at a time.
    static void BuildTable() {
        sTable = std::make_unique<unsigned int[]>(kTableSize);
        for (int i = 0; i < kTableSize; ++i) {
            sTable[i] = Reflect(i, kBitsPerByte) << (kBitsPerWord - kBitsPerByte);
            for (int bit = 0; bit < kBitsPerByte; ++bit) {
                sTable[i] =
                    (sTable[i] & kTopBit) != 0 ? (sTable[i] << 1) ^ kPolynomial : sTable[i] << 1;
            }
            sTable[i] = Reflect(sTable[i], kBitsPerWord);
        }
    }
};

#endif
