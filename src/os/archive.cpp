#include "os/archive.h"

#include <libcdvd.h>

#include "os/blockmgr.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/filestream.h"
#include "os/system.h"

namespace {

// The archive version ReadHeader() reads.
constexpr int kArchiveVersion = 2;

// The ArkHash slot of an absent name, and the directory slot of a file at the root.
constexpr int kNoSlot = -1;

// sceCdSearchFile() result of a found file.
constexpr int kSearchFound = 1;

} // namespace

Archive *TheArchive;

void Archive::Init() {
    if (UsingCD()) {
        TheArchive = new Archive("gen/main.ark");
    }
    TheBlockMgr.Init();
}

bool Archive::Locate(const char *pszFile,
                     long long *pnOffset,
                     int *pnSize,
                     int *pnUncompressedSize) {
    if (TheArchive == nullptr) {
        return false;
    }
    return TheArchive->GetFileInfo(pszFile, pnOffset, pnSize, pnUncompressedSize);
}

Archive::Archive(const char *pszBasename) : mBasename(pszBasename), mReserved40(0) {
    String path;
    MakeDevicePath(path, pszBasename);
    path = path.Substring(path.Find('\\'));
    sceCdlFILE file;
    if (sceCdSearchFile(&file, path.c_str()) == kSearchFound) {
        const int nStart = TheBlockMgr.SectorToByte(static_cast<int>(file.lsn));
        mArkfileStart = nStart;
        DebugPrint("Arkfile(%s) start byte = %d\n", pszBasename, nStart);
    } else {
        DebugWarn(" Archive NOT FOUND!!!");
    }
    ReadHeader();
}

bool Archive::GetFileInfo(const char *pszFile,
                          long long *pnOffset,
                          int *pnSize,
                          int *pnUncompressedSize) {
    if (pszFile == nullptr || *pszFile == '\0') {
        return false;
    }
    const String file(pszFile);
    int nName;
    int nPath = kNoSlot;
    const int nSlash = file.RFind('/');
    if (nSlash != String::npos) {
        const String name = file.Substring(nSlash + 1);
        const String directory = file.Substring(0, nSlash);
        nName = mHashTable.GetHashValue(name.c_str());
        nPath = mHashTable.GetHashValue(directory.c_str());
        if (nName == kNoSlot || nPath == kNoSlot) {
            return false;
        }
    } else {
        nName = mHashTable.GetHashValue(file.c_str());
        if (nName == kNoSlot) {
            return false;
        }
    }
    for (const auto &entry : mFileEntries) {
        if (entry.mHashedName == nName && entry.mHashedPath == nPath) {
            *pnOffset = mArkfileStart + entry.mOffset;
            *pnSize = entry.mSize;
            *pnUncompressedSize = entry.mUncompressedSize;
            return true;
        }
    }
    *pnOffset = 0;
    *pnSize = 0;
    *pnUncompressedSize = 0;
    return false;
}

void Archive::ReadHeader() {
    FileStream stream(mBasename.c_str(), false, true, File::kFlagNoArchive);
    (void)stream.Fail(); // Yes, the binary discards this call's result.
    int nVersion;
    stream.ReadEndian(&nVersion, sizeof(nVersion));
    if (nVersion != kArchiveVersion) {
        DebugWarn(" ERROR: %s  unsupported archive version %d", mBasename.c_str(), nVersion);
        return;
    }
    int nCount = 0;
    stream.ReadEndian(&nCount, sizeof(nCount));
    mFileEntries.reserve(nCount);
    mFileEntries.resize(nCount);
    stream.Read(&mFileEntries[0], nCount * sizeof(FileEntry));
    mHashTable.Read(stream);
}
