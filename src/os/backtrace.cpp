#include "os/backtrace.h"

#include <cstdint>

// The entry point, whose call ends the walk. The label avoids naming main() in C++.
extern "C" const char kMainEntry[] __asm__("main");

namespace {

// Entries of the scan cache.
constexpr int kCacheSize = 256;

constexpr unsigned int kInstructionShift = 2;
constexpr unsigned int kWordAlignMask = ~3U;

// Instruction encodings the scan recognises. The masked forms keep the 16-bit immediate clear.
constexpr unsigned int kUpperMask = 0xffff0000;
constexpr unsigned int kImmediateMask = 0xffff;
constexpr unsigned int kImmediateShift = 16;
constexpr unsigned int kLwRaSp = 0x8fbf0000;     // lw ra, imm(sp)
constexpr unsigned int kLdRaSp = 0xdfbf0000;     // ld ra, imm(sp)
constexpr unsigned int kLqRaSp = 0x7bbf0000;     // lq ra, imm(sp)
constexpr unsigned int kAddiuSpSp = 0x27bd0000;  // addiu sp, sp, imm
constexpr unsigned int kAdduSpSpAt = 0x03a1e821; // addu sp, sp, at
constexpr unsigned int kLuiAt = 0x3c010000;      // lui at, imm
constexpr unsigned int kOriAtAt = 0x34210000;    // ori at, at, imm
constexpr unsigned int kOriAtZero = 0x34010000;  // ori at, zero, imm
constexpr unsigned int kJrRa = 0x03e00008;       // jr ra
constexpr unsigned int kJal = 0x0c000000;        // jal target

// The scan does not pass this address before it finds a `jr ra`.
constexpr std::uintptr_t kScanLimit = 0x10000000;

// Words from a `jr ra` to the end of its delay slot.
constexpr int kJrRaLength = 2;

// Words from a return address back to its call instruction.
constexpr int kCallLength = 2;

// The frame of one function, found by scanning its epilogue.
struct FrameInfo {
    const unsigned int *mPc; // The return address the scan started from.
    int mRaOffset;           // The slot of the saved return address in the frame.
    int mFrameSize;          // The frame size, or 0 or less when the scan did not find it.
};

// NTSC-U/C: 0x00491a28
FrameInfo gFrameCache[kCacheSize];

// Find the epilogue of the function that includes pPc.
inline void ScanFrame(FrameInfo &info, const unsigned int *pPc) {
    bool bFoundRa = false;
    bool bFoundSp = false;
    bool bFoundHigh = false;
    bool bFoundLow = false;
    int nRaOffset = 0;
    int nFrameSize = -1;
    unsigned int nHigh = 0;
    unsigned int nLow = 0;
    const unsigned int *pLimit = reinterpret_cast<const unsigned int *>(kScanLimit);
    for (const unsigned int *p = pPc; !(bFoundRa && bFoundSp) && p < pLimit; ++p) {
        const unsigned int nInstruction = *p;
        const unsigned int nUpper = nInstruction & kUpperMask;
        if (nUpper == kLwRaSp || nUpper == kLdRaSp || nUpper == kLqRaSp) {
            nRaOffset = static_cast<int>(nInstruction & kImmediateMask);
            bFoundRa = true;
        } else if (nUpper == kAddiuSpSp) {
            nFrameSize = static_cast<short>(nInstruction & kImmediateMask);
            bFoundSp = true;
        } else if (nInstruction == kAdduSpSpAt) {
            nFrameSize = 0;
            bFoundSp = true;
        } else if (nUpper == kLuiAt) {
            nHigh = nInstruction & kImmediateMask;
            nLow = 0;
            bFoundHigh = true;
        } else if (nUpper == kOriAtAt) {
            nLow = nInstruction & kImmediateMask;
            bFoundLow = true;
        } else if (nUpper == kOriAtZero) {
            nLow = nInstruction & kImmediateMask;
            nHigh = 0;
            bFoundLow = true;
        } else if (nInstruction == kJrRa) {
            pLimit = p + kJrRaLength;
        }
    }
    if (nFrameSize == 0 && (bFoundHigh || bFoundLow)) {
        nFrameSize = static_cast<int>((nHigh << kImmediateShift) | nLow);
    }
    info.mRaOffset = nRaOffset;
    info.mFrameSize = nFrameSize;
}

} // namespace

// The walk reads code and stack words at computed addresses, which only integer addresses
// express.
void CaptureStackFrames(unsigned int *pFrames, int nMaxFrames) {
    int nLeft = nMaxFrames - 1;
    const unsigned int *pPc = reinterpret_cast<const unsigned int *>(BacktraceReturnAddress());
    std::uintptr_t nStack = BacktraceStackPointer();
    const unsigned int nMainCall =
        kJal | (static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(kMainEntry)) >>
                kInstructionShift);
    if (pPc == nullptr || nLeft == 0) {
        return;
    }
    for (;;) {
        FrameInfo &info =
            gFrameCache[(reinterpret_cast<std::uintptr_t>(pPc) >> kInstructionShift) % kCacheSize];
        if (info.mPc != pPc) {
            info.mPc = pPc;
            ScanFrame(info, pPc);
        }
        if (info.mFrameSize <= 0) {
            *pFrames = 0;
            return;
        }
        const unsigned int *pReturn = *reinterpret_cast<const unsigned int *const *>(
            nStack + (static_cast<unsigned int>(info.mRaOffset) & kWordAlignMask));
        nStack += static_cast<unsigned int>(info.mFrameSize) & kWordAlignMask;
        const unsigned int *pCall = pReturn - kCallLength;
        *pFrames = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pCall));
        ++pFrames;
        if (*pCall == nMainCall) {
            *pFrames = 0;
            return;
        }
        if (pReturn == nullptr) {
            return;
        }
        if (--nLeft == 0) {
            return;
        }
        pPc = pReturn;
    }
}

unsigned int BacktraceReturnAddress() {
    return static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0)));
}

unsigned int BacktraceStackPointer() {
    unsigned int nStack;
    asm volatile("move %0, $sp" : "=r"(nStack));
    return nStack;
}
