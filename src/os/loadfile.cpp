#include "os/loadfile.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "os/async.h"
#include "os/hostmode.h"
#include "os/log.h"
#include "os/mem.h"
#include "os/openarkobject.h"
#include "os/zone.h"

namespace {

// The mode every open in this file passes to FileOpen().
constexpr int kFileOpenRead = 0;

// The gzip trailer ends with the inflated size, one little-endian word.
constexpr int kGzTrailerSizeOffset = -4;

// Allocates a destination the way both loaders do, from the selected zone when there is one.
void *AllocateLoadBuffer(unsigned nSize, const char *pszFile, int nLine) {
    if (ZoneGetCurrent() == kNoZone) {
        return MemAllocTagged(nSize, pszFile, nLine);
    }
    return ZoneAlloc(nSize);
}

} // namespace

void *LoadWholeFile(const char *pszPath, void *pBuffer, unsigned nBufferSize, unsigned *pnSize) {
    const char *pszExtension = pszPath + strlen(pszPath) - (sizeof(kGzExtension) - 1);
    if (strcasecmp(pszExtension, kGzExtension) == 0) {
        return LoadGzFile(pszPath, pBuffer, nBufferSize, pnSize);
    }

    int nFile = FileOpen(pszPath, kFileOpenRead);
    if (nFile < 0) {
        return nullptr;
    }

    unsigned nSize = FileSeek(nFile, 0, kFileSeekEnd);
    FileSeek(nFile, 0, kFileSeekSet);

    if (pBuffer == nullptr) {
        pBuffer = AllocateLoadBuffer(nSize, __FILE__, __LINE__);
    } else if (nBufferSize < nSize) {
        close(nFile);
        return nullptr;
    }

    if (pBuffer != nullptr) {
        read(nFile, pBuffer, nSize);
    }
    close(nFile);
    // The size is reported even when the allocation failed and nothing was read.
    *pnSize = nSize;
    return pBuffer;
}

void *LoadGzFile(const char *pszPath, void *pBuffer, unsigned nBufferSize, unsigned *pnSize) {
    int nFile = FileOpen(pszPath, kFileOpenRead);
    if (nFile < 0) {
        return nullptr;
    }

    unsigned nSize;
    if (UsingArkFiles() != 0) {
        nSize = FindOpenFileInArkTrueSize(nFile & ~kFileHandleArkStream);
    } else {
        nSize = GetGzFileSize(nFile);
    }

    if (pBuffer == nullptr) {
        pBuffer = AllocateLoadBuffer(nSize, __FILE__, __LINE__);
    } else if (nBufferSize < nSize) {
        close(nFile);
        Fatal("  Not enough room to load: %s!\n", pszPath);
        return nullptr;
    }

    if (pBuffer != nullptr) {
        GzipFdToRam(nFile, pBuffer, 0);
        close(nFile);
    }
    *pnSize = nSize;
    return pBuffer;
}

int FileSize(const char *pszPath) {
    const int nFile = FileOpen(pszPath, kFileOpenRead);
    if (nFile < 0) {
        return 0;
    }
    const int nLength = FileSeek(nFile, 0, kFileSeekEnd);
    FileSeek(nFile, 0, kFileSeekSet);
    close(nFile);
    return nLength;
}

unsigned GetGzFileSize(int nFile) {
    FileSeek(nFile, kGzTrailerSizeOffset, kFileSeekEnd);
    unsigned nSize;
    read(nFile, &nSize, sizeof(nSize));
    FileSeek(nFile, 0, kFileSeekSet);
    return nSize;
}

int FileTrueSize(const char *pszPath) {
    const int nFile = FileOpen(pszPath, kFileOpenRead);
    if (nFile < 0) {
        return 0;
    }

    int nStored;
    int nSize;
    if ((nFile & kFileHandleArkStream) != 0) {
        const ArkFileEntry *pEntry = FindOpenFileInArk(nFile & ~kFileHandleArkStream);
        nSize = pEntry->mSize;
        nStored = pEntry->mLength;
    } else {
        nStored = FileSeek(nFile, 0, kFileSeekEnd);
        // Unlike LoadWholeFile(), the extension test here is case-sensitive.
        if (strcmp(pszPath + strlen(pszPath) - (sizeof(kGzExtension) - 1), kGzExtension) == 0) {
            FileSeek(nFile, kGzTrailerSizeOffset, kFileSeekEnd);
            read(nFile, &nSize, sizeof(nSize));
        } else {
            nSize = nStored;
        }
        FileSeek(nFile, 0, kFileSeekSet);
    }
    close(nFile);

    // The larger of the two is reported. A file that compressed badly reports its stored size.
    return (nSize < nStored) ? nStored : nSize;
}
