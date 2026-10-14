#include "os/stringoption.h"

StringOption::StringOption(const char *pszName, String *pTarget) : Option(pszName) {
    mTarget = pTarget;
}

void StringOption::Parse(int *pnIndex) {
    *mTarget = g_pCommandLineArgv[(*pnIndex)++];
}
