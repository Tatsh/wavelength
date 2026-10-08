#include "utl/FreeBlock.h"

bool FreeBlock::Coalesce(FreeBlock *next) {
    if (reinterpret_cast<FreeBlock *>(EndAddr()) != next) {
        return false;
    }
    mNext = next->mNext;
    mSizeWords += next->mSizeWords;
    return true;
}
