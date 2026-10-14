#include "os/option.h"

#include <string.h>

char **g_pCommandLineArgv = nullptr;

Option::Option(const char *pszName) {
    mName = pszName;
}

bool Option::Match(int *pnIndex) const {
    // The argument's first character is the leading `-`.
    if (strcmp(&g_pCommandLineArgv[*pnIndex][1], mName) != 0) {
        return false;
    }
    ++*pnIndex;
    return true;
}
