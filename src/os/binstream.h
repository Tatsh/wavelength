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

private:
    bool mLittleEndian; /*!< Whether values are stored in the console's byte order. */
};
