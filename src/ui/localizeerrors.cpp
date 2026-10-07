#include "ui/localizeerrors.h"

#include "os/debug.h"
#include "os/locale.h"
#include "ui/uimanager.h"

std::list<LocalizeErrors> *LocalizeErrors::sRecords = nullptr;
std::map<String, int> *LocalizeErrors::sTokenCounts = nullptr;

void LocalizeErrors::Store() {
    if (sRecords == nullptr) {
        sRecords = new std::list<LocalizeErrors>;
    }
    sRecords->push_back(*this);
}

void LocalizeErrors::CountToken(const char *pszToken, int nCount) {
    if (sTokenCounts == nullptr) {
        sTokenCounts = new std::map<String, int>;
    }
    (*sTokenCounts)[String(pszToken)] += nCount;
}

void LocalizeErrors::CountLocaleTokens() {
    DataArray *pStrings = TheLocale.mStrings;
    for (int i = 0; i < pStrings->Size(); ++i) {
        CountToken(pStrings->Array(i)->Sym(0), 0);
    }
}

void UIManager::ReportLocalizeErrors() {
    if (mEditMode) {
        LocalizeErrors::CountLocaleTokens();
    }
    if (LocalizeErrors::sTokenCounts == nullptr && LocalizeErrors::sRecords == nullptr) {
        return;
    }

    if (mEditMode && LocalizeErrors::sTokenCounts != nullptr) {
        LocalizeErrors unused(LocalizeErrors::kTypeUnused);
        DebugPrint("error size is %d\n", static_cast<int>(unused.mErrors.size()));
        for (const auto &count : *LocalizeErrors::sTokenCounts) {
            if (count.second == 0) {
                unused.mErrors.push_back(
                    LocalizeErrors::SingleError{String(nullptr), String(count.first.c_str())});
            }
        }
        DebugPrint("error size is %d\n", static_cast<int>(unused.mErrors.size()));
        if (unused.mErrors.size() != 0) {
            unused.Store();
        }
    }

    if (LocalizeErrors::sRecords != nullptr) {
        for (const auto &record : *LocalizeErrors::sRecords) {
            String listing;
            String header;
            if (record.mType == LocalizeErrors::kTypeMissing) {
                header = "The following text tokens were referenced in the file ";
                header += record.mFile;
                header += "\nand were not found in the localization file:\n\n";
            } else {
                header = "The following text tokens were not used by any loaded RND or UI "
                         "objects:\n\n";
            }
            for (const auto &error : record.mErrors) {
                if (record.mType == LocalizeErrors::kTypeMissing) {
                    listing += "   ";
                    listing += "object: ";
                    listing += error.mObject;
                    listing += " (\"";
                    listing += error.mText;
                    listing += "\")\n";
                } else if (record.mType == LocalizeErrors::kTypeUnused) {
                    listing += "   ";
                    listing += error.mText;
                    listing += " (localized: ";
                    listing += TheLocale.Localize(error.mText.c_str(), true);
                    listing += ")\n";
                }
            }
            DebugNotify("%s%s", header.c_str(), listing.c_str());
            listing = static_cast<const char *>(nullptr);
        }
        delete LocalizeErrors::sRecords;
        LocalizeErrors::sRecords = nullptr;
    }

    if (LocalizeErrors::sTokenCounts != nullptr) {
        delete LocalizeErrors::sTokenCounts;
        LocalizeErrors::sTokenCounts = nullptr;
    }
}
