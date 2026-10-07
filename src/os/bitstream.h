#pragma once

/**
 * Packer of values into a buffer bit by bit, lowest bit first.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The packets of an online
 * session pack their fields with it before they write the buffer to a BinStream.
 */
class BitStream {
public:
    /**
     * Construct a stream over a buffer, positioned at the first bit.
     *
     * @param pBuffer The buffer, which the caller retains.
     * @param nSize The buffer size in bytes.
     * @ghidraAddress NTSC-U/C: 0x00296e78
     * @ghidraAddress PAL: 0x002a0a88
     */
    BitStream(void *pBuffer, int nSize);

    /**
     * Append one bit.
     *
     * @param bValue The bit.
     * @ghidraAddress NTSC-U/C: 0x00296e90
     * @ghidraAddress PAL: 0x002a0aa0
     */
    void PackBool(bool bValue);

    /**
     * Append the low bits of a value.
     *
     * The first bit appended to an empty stream clears the whole buffer.
     *
     * @param nValue The value.
     * @param nBits The number of bits to append.
     * @ghidraAddress NTSC-U/C: 0x00296ed0
     * @ghidraAddress PAL: 0x002a0ae0
     */
    void Pack(unsigned int nValue, int nBits);

    /**
     * Read the next bit.
     *
     * @return The bit.
     * @ghidraAddress NTSC-U/C: 0x00296f90
     * @ghidraAddress PAL: 0x002a0ba0
     */
    bool UnpackBool();

    /**
     * Read the next bits as an unsigned value.
     *
     * @param nBits The number of bits to read.
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x00296ff8
     * @ghidraAddress PAL: 0x002a0c08
     */
    unsigned int Unpack(int nBits);

    void *mBuffer; /*!< The buffer. */
    int mBitPos;   /*!< The position of the next bit. */
    int mSize;     /*!< The buffer size in bytes. */
};
