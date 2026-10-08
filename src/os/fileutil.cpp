#include "os/fileutil.h"

#include <cctype>
#include <cstring>
#include <list>

namespace {

// Bytes of the shared path buffers.
constexpr int kPathSize = 256;
constexpr int kBaseSize = 64;

// Components FileNormalizePath() can hold.
constexpr int kMaxComponents = 16;

// NTSC-U/C: 0x0047fa98
char gNormalizedPath[kPathSize];

// NTSC-U/C: 0x0047fb98
char gRelativePath[kPathSize];

// NTSC-U/C: 0x0047fc98
char gDirectory[kPathSize];

// NTSC-U/C: 0x0047fd98
char gBase[kBaseSize];

// Break a path at its slashes, adding each component to a list. The path is modified.
inline void SplitPath(char *pszPath, std::list<char *> &components) {
    for (char *pszToken = strtok(pszPath, "/"); pszToken != nullptr;
         pszToken = strtok(nullptr, "/")) {
        components.push_back(pszToken);
    }
}

} // namespace

const char *FileRoot() {
    return ".";
}

const char *FileNormalizePath(const char *pszPath) {
    strcpy(gNormalizedPath, pszPath);
    for (char *p = gNormalizedPath; *p != '\0'; ++p) {
        if (isupper(*p)) {
            *p = static_cast<char>(tolower(*p));
        }
    }
    for (char *p = gNormalizedPath; *p != '\0'; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }

    const char *apszComponents[kMaxComponents];
    const char **ppszEnd = apszComponents;
    bool bCancelled = false;
    if (gNormalizedPath[0] == '/') {
        *ppszEnd++ = "";
    }
    for (char *pszToken = strtok(gNormalizedPath, "/"); pszToken != nullptr;
         pszToken = strtok(nullptr, "/")) {
        if (ppszEnd == apszComponents || pszToken[0] != '.') {
            *ppszEnd++ = pszToken;
            bCancelled = false;
            continue;
        }
        if (pszToken[1] != '.') {
            continue;
        }
        --ppszEnd;
        bCancelled = true;
        const char *pszPrevious = *ppszEnd;
        if (pszPrevious[0] != '.') {
            continue;
        }
        // A `..` cannot cancel a `..`. Any other dot component is replaced.
        if (pszPrevious[1] == '.') {
            ++ppszEnd;
        }
        *ppszEnd++ = pszToken;
    }
    if (bCancelled) {
        ppszEnd = apszComponents; // Yes, the binary discards every component here.
    }

    char *pOut = gNormalizedPath;
    for (const char **ppsz = apszComponents; ppsz != ppszEnd; ++ppsz) {
        if (ppsz != apszComponents) {
            *pOut++ = '/';
        }
        for (const char *p = *ppsz; *p != '\0'; ++p) {
            *pOut++ = *p;
        }
    }
    *pOut = '\0';
    return gNormalizedPath;
}

const char *FileRelativePath(const char *pszPath, const char *pszBase) {
    if (*pszPath == '\0') {
        gRelativePath[0] = '\0';
        return gRelativePath;
    }
    char szBase[kPathSize];
    strcpy(szBase, pszBase);
    char szPath[kPathSize];
    strcpy(szPath, pszPath);

    std::list<char *> baseComponents;
    std::list<char *> pathComponents;
    SplitPath(szBase, baseComponents);
    SplitPath(szPath, pathComponents);
    while (baseComponents.size() != 0 && pathComponents.size() != 0 &&
           strcmp(baseComponents.front(), pathComponents.front()) == 0) {
        baseComponents.pop_front();
        pathComponents.pop_front();
    }

    char *pOut = gRelativePath;
    while (baseComponents.size() != 0) {
        if (pOut != gRelativePath) {
            *pOut++ = '/';
        }
        *pOut++ = '.';
        *pOut++ = '.';
        baseComponents.pop_front();
    }
    while (pathComponents.size() != 0) {
        if (pOut != gRelativePath) {
            *pOut++ = '/';
        }
        for (const char *p = pathComponents.front(); *p != '\0'; ++p) {
            *pOut++ = *p;
        }
        pathComponents.pop_front();
    }
    *pOut = '\0';
    return gRelativePath;
}

bool FileIsAbsolute(const char *pszPath) {
    if (pszPath == nullptr || *pszPath == '\0') {
        return true;
    }
    return pszPath[0] == '/' || pszPath[0] == '\\' || pszPath[1] == ':';
}

const char *FileGetPath(const char *pszPath) {
    if (pszPath != nullptr) {
        strcpy(gDirectory, pszPath);
        char *pSlash = strrchr(gDirectory, '/');
        if (pSlash == nullptr) {
            pSlash = strrchr(gDirectory, '\\');
        }
        if (pSlash != nullptr) {
            *pSlash = '\0';
            return gDirectory;
        }
    }
    gDirectory[0] = '.';
    gDirectory[1] = '\0';
    return gDirectory;
}

const char *FileGetExt(const char *pszPath) {
    const char *pDot = strrchr(pszPath, '.');
    if (pDot != nullptr) {
        return pDot + 1;
    }
    return pszPath + strlen(pszPath);
}

const char *FileGetBaseName(const char *pszPath) {
    const char *pSlash = strrchr(pszPath, '/');
    if (pSlash == nullptr) {
        pSlash = strrchr(pszPath, '\\');
    }
    strcpy(gBase, pSlash != nullptr ? pSlash + 1 : pszPath);
    char *pDot = strrchr(gBase, '.');
    if (pDot != nullptr) {
        *pDot = '\0';
    }
    return gBase;
}
