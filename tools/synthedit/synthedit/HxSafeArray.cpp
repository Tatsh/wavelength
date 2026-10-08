#include "synthedit/HxSafeArray.h"

#include "os/Debug.h"

namespace {

// Every array the control handles has one dimension.
const UINT kDimension = 1;

} // namespace

HxSafeArray::HxSafeArray(SAFEARRAY *array) {
    mArray = array;
    LONG lower;
    if (FAILED(SafeArrayGetLBound(mArray, kDimension, &lower))) {
        ASSERT(false);
    }
    LONG upper;
    if (FAILED(SafeArrayGetUBound(mArray, kDimension, &upper))) {
        ASSERT(false);
    }
    mNumItems = upper - lower + 1;
}

int HxSafeArray::NumItems() {
    return mNumItems;
}

void HxSafeArray::Resize(int numItems) {
    SAFEARRAYBOUND bound;
    bound.cElements = numItems;
    bound.lLbound = 0;
    SafeArrayRedim(mArray, &bound);
    mNumItems = numItems;
}
