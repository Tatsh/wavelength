#include "os/filepath.h"

#include <stdio.h>

#include "os/fileutil.h"

namespace {

constexpr int kPathSize = 256;

} // namespace

// NTSC-U/C: 0x002994f8, PAL: 0x002a3108
// NTSC-U/C: 0x00299548, PAL: 0x002a3158
// NTSC-U/C: 0x00299568, PAL: 0x002a3178
// NTSC-U/C: 0x00515318
String FilePath::sRoot(".");

void FilePath::SetRoot(const char *pszRoot) {
    if (FileIsAbsolute(pszRoot)) {
        sRoot = FileNormalizePath(pszRoot);
    } else {
        char szPath[kPathSize];
        sprintf(szPath, "%s/%s", FileRoot(), pszRoot);
        sRoot = FileNormalizePath(szPath);
    }
}

void FilePath::Set(const char *pszPath) {
    if (FileIsAbsolute(pszPath)) {
        String::operator=(FileNormalizePath(pszPath));
    } else {
        char szPath[kPathSize];
        sprintf(szPath, "%s/%s", sRoot.c_str(), pszPath);
        String::operator=(FileNormalizePath(szPath));
    }
}

const char *FilePath::RelativePath() const {
    return FileRelativePath(c_str(), sRoot.c_str());
}
