#pragma once

/**
 * Stream of raw bytes, the base of the memory, file, and network streams.
 *
 * The RTTI includes the class name. One data word precedes the vptr at `+0x04`. Read() and Write()
 * are pure in this class. Only the members its callers here use are declared.
 */
class BinStream {
public:
    /** Release the stream. */
    virtual ~BinStream();

    /**
     * Read bytes.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     */
    virtual void Read(void *pData, int nBytes) = 0;

    /**
     * Write bytes.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     */
    virtual void Write(const void *pData, int nBytes) = 0;

    /**
     * Read a value of 2, 4, 8, or 16 bytes, reversing its bytes unless the stream is little endian.
     *
     * @param pData The destination.
     * @param nBytes The size of the value.
     * @ghidraAddress NTSC-U/C: 0x00293f28
     * @ghidraAddress PAL: 0x0029db90
     */
    void ReadEndian(void *pData, int nBytes);

    /**
     * Write a value of 2, 4, 8, or 16 bytes, reversing its bytes unless the stream is little
     * endian.
     *
     * @param pData The source.
     * @param nBytes The size of the value.
     * @ghidraAddress NTSC-U/C: 0x00294158
     * @ghidraAddress PAL: 0x0029ddb0
     */
    void WriteEndian(const void *pData, int nBytes);

private:
    bool mLittleEndian; /*!< Whether values are stored in the console's byte order. */
};
