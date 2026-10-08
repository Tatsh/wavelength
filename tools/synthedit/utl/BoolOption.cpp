#include "utl/BoolOption.h"

#include "os/Debug.h"

BoolOption::BoolOption(const char *name, bool *value, bool setValue) : Option(name) {
    mValue = value;
    mSetValue = setValue;
}

void BoolOption::Set(int & /*curArg*/) {
    *mValue = mSetValue;
}

void BoolOption::PrintUsage() {
    TheDebug.Printf("\t-%s\n", mName);
}
