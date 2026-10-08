#pragma once

#include "os/File.h"
#include "utl/BinStream.h"

/**
 * Stream over a file.
 *
 * The RTTI records the class as deriving from BinStream. The object is 0x10 bytes. Every transfer
 * asserts that no earlier one failed.
 */
class FileStream : public BinStream {
public:
    /** How the file is opened. */
    enum FileType {
        kRead = 0,  /*!< Open an existing file for reading. */
        kWrite = 1, /*!< Create or truncate the file for writing. */
    };

    /**
     * Open a file. The stream fails when the file does not open.
     *
     * @param file The path.
     * @param type How the file is opened.
     * @param littleEndian Whether values are stored least significant byte first.
     * @param flags Passed to NewFile(); 1 bypasses the archive.
     * @ghidraAddress 0x10011db0
     */
    FileStream(const char *file, FileType type, bool littleEndian, int flags);

    /**
     * Close the file.
     *
     * @ghidraAddress 0x10011e80
     */
    virtual ~FileStream();

    /**
     * Read bytes. A short read fails the stream.
     *
     * @param data Receives the bytes.
     * @param bytes The number of bytes.
     * @ghidraAddress 0x10011ed0
     */
    virtual void Read(void *data, int bytes);

    /**
     * Write bytes. A short write fails the stream.
     *
     * @param data The bytes.
     * @param bytes The number of bytes.
     * @ghidraAddress 0x10011f20
     */
    virtual void Write(const void *data, int bytes);

    /**
     * Write out the file's buffered data.
     *
     * @ghidraAddress 0x10011f70
     */
    virtual void Flush();

    /**
     * Move the position. A negative result fails the stream.
     *
     * @param offset The offset in bytes.
     * @param type The origin of the offset.
     * @ghidraAddress 0x10011fb0
     */
    virtual void Seek(int offset, SeekType type);

    /**
     * @return The position in bytes.
     * @ghidraAddress 0x10012020
     */
    virtual int Tell();

    /**
     * @return Whether the position is at the end of the file.
     * @ghidraAddress 0x10012060
     */
    virtual bool Eof();

    /**
     * @return Whether the file did not open or a transfer failed.
     * @ghidraAddress 0x100120a0
     */
    virtual bool Fail();

private:
    File *mFile; /*!< The file, or null when it did not open. */
    bool mFail;  /*!< Whether the file did not open or a transfer failed. */
};
