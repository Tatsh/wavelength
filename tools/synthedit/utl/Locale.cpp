#include "utl/Locale.h"

Locale TheLocale;

Locale::~Locale() {
    Terminate();
}

void Locale::Terminate() {
    if (mTable != NULL) {
        mTable->Release();
        mTable = NULL;
    }
}
