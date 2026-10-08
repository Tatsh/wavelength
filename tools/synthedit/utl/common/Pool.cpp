#include "utl/common/Pool.h"

#include "os/Debug.h"

void *Pool::Alloc() {
    FreeElem *elem = mFree;
    if (elem == NULL) {
        return NULL;
    }
    mFree = elem->mNext;
    return elem;
}

void Pool::Free(void *elem) {
    ASSERT(reinterpret_cast<int>(mElems) <= reinterpret_cast<int>(elem) &&
           reinterpret_cast<int>(elem) < mSize * mNum + reinterpret_cast<int>(mElems));
    FreeElem *freed = static_cast<FreeElem *>(elem);
    freed->mNext = mFree;
    mFree = freed;
}
