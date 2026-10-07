#pragma once

#include "os/binstream.h"
#include "os/file.h"

/**
 * BinStream that reads or writes a file.
 *
 * The RTTI includes the class name and records BinStream as the base. Only the members its callers
 * here use are declared.
 */
class FileStream : public BinStream {
public:
    /**
     * Open a file.
     *
     * @param pszPath The path of the file.
     * @param bWrite Open the file for writing, creating or truncating it.
     * @param bLittleEndian Store values in the console's byte order.
     * @param nFlags Passed to the routine that opens the file.
     * @ghidraAddress NTSC-U/C: 0x00299588
     * @ghidraAddress PAL: 0x002a3198
     */
    FileStream(const char *pszPath, bool bWrite, bool bLittleEndian, int nFlags);

    /**
     * Close the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00299610
     * @ghidraAddress PAL: 0x002a3220
     */
    ~FileStream() override;

    /**
     * Read bytes, recording a failure when fewer arrive.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00299688
     * @ghidraAddress PAL: 0x002a3288
     */
    void Read(void *pData, int nBytes) override;

    /**
     * Write bytes, recording a failure when fewer are written.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x002996d8
     * @ghidraAddress PAL: 0x002a32d8
     */
    void Write(const void *pData, int nBytes) override;

    File *mFile; /*!< The file, or null when it did not open. */
    int mFail;   /*!< Non-zero once an operation failed. */
};
