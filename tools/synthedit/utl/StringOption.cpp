#include "utl/StringOption.h"

#include "os/Debug.h"

StringOption::StringOption(const char *name, String *value) : Option(name) {
    mValue = value;
}

void StringOption::Set(int &curArg) {
    *mValue = gOptionArgv[curArg++];
}

void StringOption::PrintUsage() {
    TheDebug.Printf("\t-%s <string>\n", mName);
}
