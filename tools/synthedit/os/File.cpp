#include "os/File.h"

#include <cctype>
#include <cstring>
#include <list>
#include <vector>

#include <windows.h>

#include "os/ArkFile.h"
#include "os/AsyncFile.h"
#include "os/Debug.h"
#include "os/System.h"

namespace {

const int kMaxOpenFiles = 128;
const int kPathChars = 256;
const int kMaxDirs = 16;
const char kSeparators[] = "/";

// 0x1003fc90
std::vector<File *> gOpenFiles(kMaxOpenFiles);

// 0x1003fca0
File *gFileLog;

// 0x1003fca4
bool gFileLogEnabled;

// 0x1003fca8
char gRootDir[kPathChars];

// 0x1003f978
char gNormalizedPath[kPathChars];

// 0x1003fa78
char gRelativePath[kPathChars];

// 0x1003fb8c
char gPathBuffer[kPathChars];

// 0x1003f938
char gBaseBuffer[64];

} // namespace

void FileLogInit(const char *file) {
    if (file != NULL && *file != '\0') {
        gFileLog = NewFile(file, FILE_OPEN_WRITE | FILE_OPEN_CREATE | FILE_OPEN_TRUNCATE, 0);
        gFileLogEnabled = true;
    }
}

void FileInit() {
    AsyncFileInit();
}

void FileTerminate() {
    delete gFileLog;
    EmptyRoutine();
}

File::~File() {
}

File *NewFile(const char *name, int mode, int flags) {
    File *file;
    if (UsingCD() && mode == FILE_OPEN_READ && flags != 1) {
        file = new ArkFile(name, mode);
    } else {
        file = new AsyncFile(name, mode);
    }
    if (file->Fail()) {
        delete file;
        return NULL;
    }
    if (gFileLogEnabled && gFileLog != NULL && mode == FILE_OPEN_READ) {
        const char *line = FormatString("\"%s\"\n", FileLocalize(name));
        gFileLog->Write(line, static_cast<int>(strlen(line)));
        gFileLog->Flush();
    }
    return file;
}

void FileMakeLocalPath(String &out, const char *path) {
    char local[kPathChars];
    FileCopyPath(local, sizeof(local), path);
    out = local;
}

void FileCopyPath(char *out, int size, const char *path) {
    if (path == NULL) {
        *out = '\0';
        return;
    }
    strcpy(out, path);
}

const char *FileRoot() {
    if (gRootDir[0] == '\0') {
        GetCurrentDirectoryA(sizeof(gRootDir), gRootDir);
        strcpy(gRootDir, FileNormalizePath(gRootDir));
    }
    return gRootDir;
}

const char *FileNormalizePath(const char *path) {
    strcpy(gNormalizedPath, path);
    char *c;
    for (c = gNormalizedPath; *c != '\0'; ++c) {
        if (isupper(*c)) {
            *c += 'a' - 'A';
        }
    }
    for (c = gNormalizedPath; *c != '\0'; ++c) {
        if (*c == '\\') {
            *c = '/';
        }
    }

    const char *dirs[kMaxDirs];
    const char **endDir = dirs;
    bool backedUp = false;
    if (gNormalizedPath[0] == '/') {
        *endDir++ = "";
    }
    for (char *dir = strtok(gNormalizedPath, kSeparators); dir != NULL;
         dir = strtok(NULL, kSeparators)) {
        if (endDir == dirs || dir[0] != '.') {
            *endDir++ = dir;
            backedUp = false;
        } else if (dir[1] == '.') {
            --endDir;
            backedUp = true;
            if ((*endDir)[0] == '.') {
                if ((*endDir)[1] == '.') {
                    ++endDir;
                }
                *endDir++ = dir;
            }
        }
    }
    if (backedUp) {
        endDir = dirs; // Yes, a path that ends in ".." loses every component.
    }
    ASSERT(endDir - dirs <= 16);

    c = gNormalizedPath;
    for (const char **component = dirs; component != endDir; ++component) {
        if (component != dirs) {
            *c++ = '/';
        }
        for (const char *s = *component; *s != '\0'; ++s) {
            *c++ = *s;
        }
    }
    char *buffer = gNormalizedPath;
    ASSERT(c - buffer < 256);
    *c = '\0';
    return gNormalizedPath;
}

const char *FileRelativePath(const char *path, const char *root) {
    ASSERT(path);
    ASSERT(root);
    if (*path == '\0') {
        gRelativePath[0] = '\0';
        return gRelativePath;
    }

    char rootBuffer[kPathChars];
    char pathBuffer[kPathChars];
    strcpy(rootBuffer, root);
    strcpy(pathBuffer, path);
    std::list<char *> rootDirs;
    std::list<char *> pathDirs;
    char *dir;
    for (dir = strtok(rootBuffer, kSeparators); dir != NULL; dir = strtok(NULL, kSeparators)) {
        rootDirs.insert(rootDirs.end(), dir);
    }
    for (dir = strtok(pathBuffer, kSeparators); dir != NULL; dir = strtok(NULL, kSeparators)) {
        pathDirs.insert(pathDirs.end(), dir);
    }

    while (rootDirs.size() != 0 && pathDirs.size() != 0 &&
           strcmp(rootDirs.front(), pathDirs.front()) == 0) {
        rootDirs.erase(rootDirs.begin());
        pathDirs.erase(pathDirs.begin());
    }

    char *p = gRelativePath;
    while (rootDirs.size() != 0) {
        if (p != gRelativePath) {
            *p++ = '/';
        }
        *p++ = '.';
        *p++ = '.';
        rootDirs.erase(rootDirs.begin());
    }
    while (pathDirs.size() != 0) {
        if (p != gRelativePath) {
            *p++ = '/';
        }
        for (const char *s = pathDirs.front(); *s != '\0'; ++s) {
            *p++ = *s;
        }
        pathDirs.erase(pathDirs.begin());
    }
    char *relative = gRelativePath;
    ASSERT(p - relative < 256);
    *p = '\0';
    return gRelativePath;
}

bool FileIsAbsolute(const char *path) {
    return path == NULL || path[0] == '\0' || path[0] == '/' || path[0] == '\\' || path[1] == ':';
}

const char *FileGetPath(const char *path) {
    if (path != NULL) {
        strcpy(gPathBuffer, path);
        char *end = strrchr(gPathBuffer, '/');
        if (end == NULL) {
            end = strrchr(gPathBuffer, '\\');
        }
        if (end != NULL) {
            *end = '\0';
            return gPathBuffer;
        }
    }
    gPathBuffer[0] = '.';
    gPathBuffer[1] = '\0';
    return gPathBuffer;
}

const char *FileGetExt(const char *path) {
    const char *dot = strrchr(path, '.');
    if (dot != NULL) {
        return dot + 1;
    }
    return path + strlen(path);
}

const char *FileGetBase(const char *path) {
    const char *start = strrchr(path, '/');
    if (start == NULL) {
        start = strrchr(path, '\\');
    }
    strcpy(gBaseBuffer, start != NULL ? start + 1 : path);
    char *dot = strrchr(gBaseBuffer, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    return gBaseBuffer;
}
