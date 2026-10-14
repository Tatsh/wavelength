#pragma once

#include "os/file.h"
#include "os/streambuffer.h"

/**
 * StreamBuffer that a file fills with asynchronous reads.
 *
 * The RTTI includes the class name and records StreamBuffer as the base. The object is 0x38
 * bytes. Only the members its callers here use are declared.
 */
class FileStreamBuffer : public StreamBuffer {
public:
    /**
     * Open a file and start filling the buffer.
     *
     * @param pszPath The path of the file.
     * @param nFlags The open flags passed to File::New().
     * @ghidraAddress NTSC-U/C: 0x00299970
     * @ghidraAddress PAL: 0x002a3570
     */
    FileStreamBuffer(const char *pszPath, int nFlags);

    /**
     * Close the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00299a40
     * @ghidraAddress PAL: 0x002a3640
     */
    ~FileStreamBuffer() override;

    /**
     * Report whether the file failed to open.
     *
     * @return Whether no file is open.
     * @ghidraAddress NTSC-U/C: 0x00299960
     * @ghidraAddress PAL: 0x002a3560
     */
    bool IsClosed() const;

    int mReserved24;   // +0x24, not yet identified.
    File *mFile;       /*!< The file, or null when it did not open. */
    char *mReadBuffer; /*!< The buffer each read fills. */
    int mReserved30;   // +0x30, not yet identified.
    int mReserved34;   // +0x34, not yet identified.
};
