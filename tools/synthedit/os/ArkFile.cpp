#include "os/ArkFile.h"

#include <cstdio>

#include "os/Archive.h"
#include "os/AsyncTask.h"
#include "os/BlockMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "utl/BinStream.h"

namespace {

const int kPathChars = 256;

} // namespace

const char *FileLocalize(const char *path) {
    char absolute[kPathChars];
    const char *normalized = FileNormalizePath(path);
    if (FileIsAbsolute(normalized)) {
        normalized = FileNormalizePath(normalized);
    } else {
        sprintf(absolute, "%s/%s", FileRoot(), normalized);
        normalized = FileNormalizePath(absolute);
    }
    return FileRelativePath(normalized, FileRoot());
}

ArkFile::ArkFile(const char *name, int mode)
    : mNumOutstanding(0), mBytesRead(0), mTell(0), mFail(0) {
    if (!ArchiveGetFileInfo(FileLocalize(name), mArcOffset, mSize, mUCSize) ||
        (mode & FILE_OPEN_WRITE)) {
        mFail = 1;
    }
}

ArkFile::~ArkFile() {
    if (mNumOutstanding > 0) {
        TheBlockMgr.KillBlockRequests(this);
    }
}

int ArkFile::Read(void *data, int bytes) {
    if (!ReadAsync(data, bytes)) {
        return 0;
    }
    int bytesRead = -1;
    while (!ReadDone(&bytesRead)) {
    }
    return bytesRead;
}

bool ArkFile::ReadAsync(void *data, int bytes) {
    if (mTell == mSize || mNumOutstanding != 0) {
        return false;
    }
    mBytesRead = 0;
    if (bytes == 0) {
        return true;
    }
    if (mTell + bytes > mSize) {
        bytes = mSize - mTell;
    }
    const __int64 offset = mArcOffset + mTell;
    int firstBlock;
    int numBlocks;
    int blockSize;
    TheBlockMgr.GetAssociatedBlocks(offset, bytes, firstBlock, numBlocks, blockSize);
    char *dest = static_cast<char *>(data);
    const int lastBlock = firstBlock + numBlocks - 1;
    for (int block = firstBlock; block <= lastBlock; ++block) {
        const int start = block == firstBlock ? static_cast<int>(offset % blockSize) : 0;
        int end = blockSize;
        if (block == lastBlock && bytes != 0) {
            const int remainder = static_cast<int>((offset + bytes) % blockSize);
            if (remainder != 0) {
                end = remainder;
            }
        }
        AsyncTask task(this, dest, block, start, end);
        dest += end - start;
        ++mNumOutstanding;
        if (!task.FillData()) {
            TheBlockMgr.AddRequest(task);
        }
    }
    TheBlockMgr.Poll();
    return true;
}

int ArkFile::Write(const void *data, int bytes) {
    TheDebug.Fail("ERROR: Cannot write to a file in an archive!");
    return 0;
}

int ArkFile::Seek(int offset, int origin) {
    switch (origin) {
    case BinStream::kSeekBegin:
        mTell = offset;
        break;
    case BinStream::kSeekCurrent:
        mTell += offset;
        break;
    case BinStream::kSeekEnd:
        mTell = mSize + offset;
        break;
    }
    return mTell;
}

int ArkFile::Tell() {
    return mTell;
}

void ArkFile::Flush() {
}

bool ArkFile::Eof() {
    return mTell == mSize;
}

bool ArkFile::Fail() {
    return mFail != 0;
}

int ArkFile::Size() {
    return mSize;
}

int ArkFile::UncompressedSize() {
    return mUCSize;
}

void ArkFile::TaskDone(int bytes) {
    --mNumOutstanding;
    mBytesRead += bytes;
    mTell += bytes;
}

bool ArkFile::ReadDone(int *bytes) {
    TheBlockMgr.Poll();
    if (mNumOutstanding != 0) {
        return false;
    }
    *bytes = mBytesRead;
    return true;
}
