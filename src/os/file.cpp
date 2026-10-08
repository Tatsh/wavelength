#include "os/file.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include <sifdev.h>

#include "os/arkfile.h"
#include "os/asyncfile.h"
#include "os/system.h"

namespace {

// Device prefixes FileSetBootPath() removes.
const char kCdromPrefix[] = "cdrom0:\\";
const char kHostPrefix[] = "host0:";

// The separator of the version suffix of a disc path.
constexpr char kVersionSeparator = ';';

// Characters of the disc device name, which FormatDevicePath() leaves as they are.
constexpr int kCdromDeviceLength = 6;

// Bytes of the buffer MakeDevicePath() formats into.
constexpr int kDevicePathSize = 256;

// Entries of gFileTable.
constexpr int kFileTableSize = 128;

// Open mode of the trace log.
constexpr int kTraceLogMode = SCE_WRONLY | SCE_CREAT | SCE_TRUNC;

// Access bits of a directory FileMkDir() creates.
constexpr int kDirectoryMode = 0777;

// FileSourceLetter() letters.
constexpr char kToolSourceLetter = 'r';
constexpr char kRetailSourceLetter = 's';

// NTSC-U/C: 0x0028a020, PAL: 0x00293818 (static initialiser)
// NTSC-U/C: 0x0028a138, PAL: 0x00293930 (constructor call)
// NTSC-U/C: 0x0028a158, PAL: 0x00293950 (destructor call)
// NTSC-U/C: 0x0047fdd8
String gBootPath;

// NTSC-U/C: 0x003b20d0
File *gBootFile;

// NTSC-U/C: 0x003b20d4
File *gTraceLog;

// NTSC-U/C: 0x003b20d8
int gTraceEnabled;

} // namespace

std::vector<File *> gFileTable(kFileTableSize, nullptr);

void FileSetBootPath(const char *pszPath) {
    gBootPath = pszPath;
    int nPos = gBootPath.Find(kCdromPrefix);
    if (nPos != String::npos) {
        gBootPath.Erase(nPos, strlen(kCdromPrefix));
    } else {
        nPos = gBootPath.Find(kHostPrefix);
        if (nPos != String::npos) {
            gBootPath.Erase(nPos, strlen(kHostPrefix));
        }
    }
    nPos = gBootPath.RFind(kVersionSeparator);
    if (nPos != String::npos) {
        gBootPath.Truncate(nPos);
    }
    gBootFile = new AsyncFile("", File::kModeRead);
}

const char *FileBootExecutable() {
    return gBootPath.c_str();
}

void FileOpenTraceLog(const char *pszPath) {
    if (pszPath == nullptr || *pszPath == '\0') {
        return;
    }
    gTraceLog = File::New(pszPath, kTraceLogMode, 0);
    gTraceEnabled = 1;
}

void FileSetTraceEnabled(int nEnabled) {
    gTraceEnabled = nEnabled;
}

int FileIsTraceEnabled() {
    return gTraceEnabled;
}

void FileInit() {
    AsyncFile::Init();
}

void FileTerminate() {
    if (gTraceLog != nullptr) {
        delete gTraceLog;
    }
    AsyncFile::Terminate();
}

File *File::New(const char *pszPath, int nMode, int nFlags) {
    File *pFile;
    if (UsingCD() && nMode == kModeRead && nFlags != kFlagNoArchive) {
        pFile = new ArkFile(pszPath, kModeRead);
    } else {
        pFile = new AsyncFile(pszPath, nMode);
    }
    if (pFile->Fail()) {
        delete pFile;
        return nullptr;
    }
    if (gTraceEnabled != 0 && gTraceLog != nullptr && nMode == kModeRead) {
        const char *pszLine = FormatString("\"%s\"\n", ArkFile::ArchivePath(pszPath));
        gTraceLog->Write(pszLine, strlen(pszLine));
        gTraceLog->Flush();
    }
    return pFile;
}

void FileNoteRead(const char *) {
}

void MakeDevicePath(String &path, const char *pszFile) {
    char szPath[kDevicePathSize];
    FormatDevicePath(szPath, kDevicePathSize, pszFile);
    path = szPath;
}

void FormatDevicePath(char *pszDest, int, const char *pszFile) {
    if (pszFile == nullptr) {
        *pszDest = '\0';
        return;
    }
    if (!UsingCD()) {
        sprintf(pszDest, "host0:%s", pszFile);
        return;
    }
    sprintf(pszDest, "cdrom0:\\%s;1", pszFile);
    for (char *p = pszDest + kCdromDeviceLength; *p != '\0'; ++p) {
        *p = static_cast<char>(toupper(*p));
        if (*p == '/') {
            *p = '\\';
        }
    }
}

int FileTableRead(int nDescriptor, void *pBuffer, int nBytes) {
    return gFileTable[nDescriptor]->Read(pBuffer, nBytes);
}

int FileGetStat(const char *pszPath, FileStat *pStat) {
    String path;
    MakeDevicePath(path, pszPath);
    if (kImageFirstWord != 0) {
        memset(pStat, 0, sizeof(*pStat));
        return 0;
    }
    sce_stat stat;
    const int nResult = sceGetstat(path.c_str(), &stat);
    pStat->mMode = static_cast<int>(stat.st_mode);
    pStat->mSize = static_cast<int>(stat.st_size);
    memcpy(pStat->mCreated, stat.st_ctime, sizeof(pStat->mCreated));
    memcpy(pStat->mAccessed, stat.st_atime, sizeof(pStat->mAccessed));
    memcpy(pStat->mModified, stat.st_mtime, sizeof(pStat->mModified));
    return nResult;
}

int FileMkDir(const char *pszPath) {
    if (UsingCD()) {
        return -1;
    }
    String path;
    MakeDevicePath(path, pszPath);
    return sceMkdir(path.c_str(), kDirectoryMode);
}

char FileSourceLetter() {
    return kImageFirstWord != 0 ? kToolSourceLetter : kRetailSourceLetter;
}
