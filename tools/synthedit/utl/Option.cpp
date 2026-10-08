#include "utl/Option.h"

#include <cstring>

// 0x100c38e4
char **gOptionArgv;

Option::Option(const char *name) : mName(name) {
}

Option::~Option() {
}

bool Option::Matches(int &curArg) {
    if (strcmp(gOptionArgv[curArg] + 1, mName) != 0) {
        return false;
    }
    ++curArg;
    return true;
}
