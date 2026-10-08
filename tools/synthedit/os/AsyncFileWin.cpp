#include "os/AsyncFile.h"

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "os/Debug.h"
#include "os/File.h"

int AsyncFile::_Open() {
    mHandle = _open(mFilename.c_str(), (mMode & ~FILE_OPEN_READ) | _O_BINARY, _S_IREAD | _S_IWRITE);
    mFail = mHandle < 0;
    const int size = _lseek(mHandle, 0, SEEK_END);
    _lseek(mHandle, 0, SEEK_SET);
    return size;
}

int AsyncFile::_Write(const void *data, int bytes) {
    if (_write(mHandle, data, bytes) < bytes) {
        mFail = true;
    }
    return bytes;
}

void AsyncFile::_SeekToTell() {
    if (_lseek(mHandle, mTell, SEEK_SET) < 0) {
        mFail = true;
    }
}

void AsyncFile::_ReadAsync(void *data, int bytes) {
    const int bytesRead = _read(mHandle, data, bytes);
    if (bytesRead < bytes) {
        mFail = true;
        TheDebug.Fail(
            "Read failed on %s: read = %d, size = %d", mFilename.c_str(), bytesRead, bytes);
    }
}

bool AsyncFile::_ReadDone() {
    return true;
}

void AsyncFile::_Close() {
    _close(mHandle);
}
