#include "met/metagameutil.h"

bool TrimSpaces(String *pText) {
    if (pText->mLength == 0) {
        return false;
    }

    bool bTrimmed = false;
    int nLeading = 0;
    while (static_cast<unsigned>(nLeading) < static_cast<unsigned>(pText->mLength) &&
           (*pText)[nLeading] == ' ') {
        ++nLeading;
    }
    if (nLeading != 0) {
        pText->Erase(0, nLeading);
        bTrimmed = true;
    }

    int nLast = pText->mLength - 1;
    while (nLast > 0 && (*pText)[nLast] == ' ') {
        --nLast;
    }
    if (static_cast<unsigned>(nLast) < static_cast<unsigned>(pText->mLength - 1)) {
        // The count is one more than the characters after nLast, as in the binary.
        pText->Erase(nLast + 1, pText->mLength - nLast);
        bTrimmed = true;
    }
    return bTrimmed;
}
