#include "os/Archive.h"

#include "os/BlockMgr.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/FileStream.h"

namespace {

const int kArchiveVersion = 2;
const int kNoHash = -1;
const int kNumReservedWords = 3;

// 0x10040cc0
int gArchiveReserved[kNumReservedWords];

} // namespace

Archive *gArchive;

void ArchivePreInit() {
    for (int i = 0; i < kNumReservedWords; ++i) {
        gArchiveReserved[i] = 0;
    }
}

void ArchiveInit() {
    if (UsingCD()) {
        gArchive = new Archive("gen/main.ark");
    }
    TheBlockMgr.Init();
}

bool ArchiveGetFileInfo(const char *name, __int64 &offset, int &size, int &ucSize) {
    if (gArchive == NULL) {
        return false;
    }
    return gArchive->GetFileInfo(name, offset, size, ucSize);
}

Archive::Archive(const char *basename) : mBasename(basename), mArcBase(0), mReserved40(0) {
    Read();
}

bool Archive::GetFileInfo(const char *name, __int64 &offset, int &size, int &ucSize) {
    if (name == NULL || *name == '\0') {
        return false;
    }
    String path(name);
    const int slash = path.FindLast('/');
    int fileHash;
    int dirHash = kNoHash;
    if (slash != String::npos) {
        String file = path.Substring(slash + 1);
        String dir = path.Substring(0, slash);
        fileHash = mHashTable.GetHashValue(file.c_str());
        dirHash = mHashTable.GetHashValue(dir.c_str());
        if (fileHash == kNoHash || dirHash == kNoHash) {
            return false;
        }
    } else {
        fileHash = mHashTable.GetHashValue(path.c_str());
        if (fileHash == kNoHash) {
            return false;
        }
    }
    std::vector<FileEntry>::const_iterator it;
    for (it = mFileEntries.begin(); it != mFileEntries.end(); ++it) {
        const FileEntry &entry = *it;
        if (entry.mHashedName == fileHash && entry.mHashedPath == dirHash) {
            offset = mArcBase + entry.mOffset;
            size = entry.mSize;
            ucSize = entry.mUCSize;
            return true;
        }
    }
    offset = 0;
    size = 0;
    ucSize = 0;
    return false;
}

void Archive::Read() {
    FileStream file(mBasename.c_str(), FileStream::kRead, true, 1);
    ASSERT(!file.Fail());
    int version;
    file.ReadEndian(&version, sizeof(version));
    if (version != kArchiveVersion) {
        TheDebug.Fail(" ERROR: %s  unsupported archive version %d", mBasename.c_str(), version);
        return;
    }
    int numEntries = 0;
    file.ReadEndian(&numEntries, sizeof(numEntries));
    mFileEntries.reserve(numEntries);
    mFileEntries.resize(numEntries);
    file.Read(&mFileEntries[0], numEntries * static_cast<int>(sizeof(FileEntry)));
    mHashTable.Read(file);
}
