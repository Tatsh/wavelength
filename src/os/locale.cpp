#include "os/locale.h"

#include <vector>

#include "os/system.h"

// NTSC-U/C: 0x00299d60, PAL: 0x002a3960
// NTSC-U/C: 0x00299da0, PAL: 0x002a39a0
// NTSC-U/C: 0x00299dc0, PAL: 0x002a39c0
Locale TheLocale;

void LocalePrepare() {
}

void Locale::Init(const char *pszLanguage) {
    DataArray *pFiles = SystemConfig()->FindArray("locale", true)->FindArray(pszLanguage, true);
    std::vector<DataArray *> files(pFiles->Size() - 1, nullptr);
    mFileCount = static_cast<int>(files.size());
    for (int i = 1; i < pFiles->Size(); ++i) {
        files[i - 1] = DataArray::Read(pFiles->Sym(i), nullptr);
    }
    mStrings = DataArray::Concat(files, true);
    mStrings->SetFileLine(pFiles->Sym(1), 0);
    mStrings->SortByKey();
}

Locale::Locale() {
    mStrings = nullptr;
    mFileCount = 0;
}

void Locale::Terminate() {
    if (mStrings != nullptr) {
        mStrings->Release();
        mStrings = nullptr;
    }
}

Locale::~Locale() {
    Terminate();
}

const char *Locale::Localize(const char *pszToken, bool bFail) {
    const char *pszText = pszToken;
    mStrings->FindSymbol(pszToken, &pszText, bFail);
    return pszText;
}
