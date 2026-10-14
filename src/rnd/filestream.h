#pragma once

#include <stdio.h>

#include "rnd/stream.h"

class HxStr;

namespace Rnd {

/**
 * Stream over a C library `FILE`.
 *
 * Its RTTI descriptor is at `0x008eefd8`. It has single inheritance from `Rnd::Stream` at offset 0.
 * The object is 8 bytes and its vtable is at `0x008261d8`.
 *
 * Every transfer goes straight to `fread` or `fwrite` with a one-byte element size, and the end
 * and failure tests read bits 0x20 and 0x40 of the `FILE` flags rather than calling `feof` and
 * `ferror`. A path that fails to open leaves the `FILE` null, which Fail() then reports.
 */
class FileStream : public Stream {
public:
    /**
     * Open a file.
     *
     * An empty path arrives at `fopen` as the program-wide empty string rather than as null.
     *
     * @param path The file to open.
     * @param nWrite Non-zero to open for writing, which uses mode "wb" rather than "rb".
     */
    FileStream(const HxStr &path, int nWrite);

    /**
     * Close the file.
     */
    virtual ~FileStream();

    virtual Stream &Read(void *pDest, int nSize);

    virtual Stream &Write(const void *pSrc, int nSize);

    virtual Stream &Flush();

    virtual Stream &Seek(int nOffset, int nWhence);

    virtual int Tell();

    virtual int Eof();

    virtual int Fail();

private:
    FILE *mFile; // +0x04
};

} // namespace Rnd
