#include "os/booloption.h"

BoolOption::BoolOption(const char *pszName, int *pnTarget, int nValue) : Option(pszName) {
    mTarget = pnTarget;
    mValue = nValue;
}

void BoolOption::Parse(int * /* pnIndex */) {
    *mTarget = mValue;
}
