#include "os/arkfile.h"

#include <cstdio>

#include "os/archive.h"
#include "os/blockmgr.h"
#include "os/debug.h"
#include "os/fileutil.h"

namespace {

// Bytes of the buffer ArchivePath() joins a relative path in.
constexpr int kPathSize = 256;

// Seek origins.
enum SeekOrigin { kSeekBegin = 0, kSeekCurrent = 1, kSeekEnd = 2 };

} // namespace

ArkFile::ArkFile(const char *pszFile, int nMode)
    : mNumOutstanding(0), mBytesRead(0), mTell(0), mFail(0) {
    if (!Archive::Locate(ArchivePath(pszFile), &mArkOffset, &mSize, &mUncompressedSize) ||
        (nMode & kModeWrite) != 0) {
        mFail = 1;
    }
}

ArkFile::~ArkFile() {
    if (mNumOutstanding > 0) {
        TheBlockMgr.KillBlockRequests(this);
    }
}

const char *ArkFile::ArchivePath(const char *pszPath) {
    const char *pszNormalized = FileNormalizePath(pszPath);
    if (!FileIsAbsolute(pszNormalized)) {
        char szPath[kPathSize];
        sprintf(szPath, "%s/%s", FileRoot(), pszNormalized);
        pszNormalized = FileNormalizePath(szPath);
    }
    return FileRelativePath(pszNormalized, FileRoot());
}

int ArkFile::Read(void *pBuffer, int nBytes) {
    if (!ReadAsync(pBuffer, nBytes)) {
        return 0;
    }
    int nRead;
    while (!ReadDone(&nRead)) {
    }
    return nRead;
}

bool ArkFile::ReadAsync(void *pBuffer, int nBytes) {
    if (mTell == mSize || mNumOutstanding != 0) {
        return false;
    }
    mBytesRead = 0;
    if (nBytes == 0) {
        return true;
    }
    if (mTell + nBytes > mSize) {
        nBytes = mSize - mTell;
    }
    const long long nStart = mArkOffset + mTell;
    int nFirstBlock = 0;
    int nBlockCount = 0;
    int nBlockSize = 0;
    TheBlockMgr.GetBlockSpan(nStart, nBytes, &nFirstBlock, &nBlockCount, &nBlockSize);
    const int nLastBlock = nFirstBlock + nBlockCount - 1;
    const long long nEnd = nStart + nBytes;
    char *pOut = static_cast<char *>(pBuffer);
    for (int nBlock = nFirstBlock; nBlock <= nLastBlock; ++nBlock) {
        int nFrom = 0;
        int nTo = nBlockSize;
        if (nBlock == nFirstBlock) {
            nFrom = static_cast<int>(nStart % nBlockSize);
        }
        if (nBlock == nLastBlock && nBytes != 0) {
            const int nLastEnd = static_cast<int>(nEnd % nBlockSize);
            if (nLastEnd != 0) {
                nTo = nLastEnd;
            }
        }
        AsyncTask task(this, pOut, nBlock, nFrom, nTo);
        pOut += nTo - nFrom;
        ++mNumOutstanding;
        if (!task.TryComplete()) {
            TheBlockMgr.AddRequest(task);
        }
    }
    TheBlockMgr.Poll();
    return true;
}

int ArkFile::Write(const void *, int) {
    DebugWarn("ERROR: Cannot write to a file in an archive!");
    return 0;
}

int ArkFile::Seek(int nOffset, int nOrigin) {
    switch (nOrigin) {
    case kSeekBegin:
        mTell = nOffset;
        break;
    case kSeekCurrent:
        mTell += nOffset;
        break;
    case kSeekEnd:
        mTell = mSize + nOffset;
        break;
    default:
        break;
    }
    return mTell;
}

void ArkFile::TaskDone(int nBytes) {
    --mNumOutstanding;
    mBytesRead += nBytes;
    mTell += nBytes;
}

bool ArkFile::ReadDone(int *pnBytes) {
    TheBlockMgr.Poll();
    if (mNumOutstanding != 0) {
        return false;
    }
    *pnBytes = mBytesRead;
    return true;
}
