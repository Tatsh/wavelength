#pragma once

#include <memory>

#include "os/file.h"
#include "os/streambuffer.h"

/**
 * StreamBuffer that a file fills with asynchronous reads.
 *
 * The RTTI includes the class name and records StreamBuffer as the base. The object is 0x38
 * bytes.
 */
class FileStreamBuffer : public StreamBuffer {
public:
    /**
     * Open a file and start the first read.
     *
     * @param pszPath The path of the file.
     * @param nFlags The open flags passed to File::New().
     * @ghidraAddress NTSC-U/C: 0x00299970
     * @ghidraAddress PAL: 0x002a3570
     */
    FileStreamBuffer(const char *pszPath, int nFlags);

    /**
     * Close the file and free the read buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x00299a40
     * @ghidraAddress PAL: 0x002a3640
     */
    ~FileStreamBuffer() override;

    /**
     * Append the bytes of a finished read, then read as many bytes as the buffer has room for.
     *
     * @ghidraAddress NTSC-U/C: 0x00299838
     * @ghidraAddress PAL: 0x002a3438
     */
    void Poll() override;

    /**
     * Seek the file back to its start and clear mInputDone.
     *
     * The heads stay where they are.
     *
     * @ghidraAddress NTSC-U/C: 0x00299918
     * @ghidraAddress PAL: 0x002a3518
     */
    void Rewind() override;

    /**
     * Report whether the file failed to open.
     *
     * @return Whether no file is open.
     * @ghidraAddress NTSC-U/C: 0x00299960
     * @ghidraAddress PAL: 0x002a3560
     */
    bool IsClosed() const;

    /** @return The file, or null when it did not open. */
    File *GetFile() const {
        return mFile.get();
    }

private:
    int mDelivered; /*!< Non-zero once the bytes of the last read went to the buffer. */
    std::unique_ptr<File> mFile; /*!< The file, or null when it did not open. */
    char *mReadBuffer;           /*!< The buffer each read fills, freed with PoolMemFree. */
    int mReserved30;             // +0x30, cleared by the constructor and not yet identified.
    int mReserved34;             // +0x34, cleared by the constructor and not yet identified.
};
