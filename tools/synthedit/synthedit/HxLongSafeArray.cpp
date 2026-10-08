#include "synthedit/HxLongSafeArray.h"

#include "os/Debug.h"

namespace {

// A created array starts empty, indexed from zero.
const LONG kCreateLowerBound = 0;
const ULONG kCreateCount = 1;

} // namespace

HxLongSafeArray::HxLongSafeArray(SAFEARRAY *array)
    : HxSafeArray(array == NULL ? SafeArrayCreateVector(VT_I4, kCreateLowerBound, kCreateCount) :
                                  array) {
    mOwned = false;
    if (array == NULL) {
        mOwned = true;
    }
}

HxLongSafeArray::HxLongSafeArray(std::vector<int> &values)
    : HxSafeArray(SafeArrayCreateVector(VT_I4, kCreateLowerBound, kCreateCount)) {
    mOwned = true;
    SetFromVector(values);
}

HxLongSafeArray::~HxLongSafeArray() {
    if (mOwned) {
        SafeArrayDestroy(mArray);
    }
}

long HxLongSafeArray::Get(long iIndex) {
    ASSERT(iIndex < mNumItems);
    long value;
    SafeArrayLock(mArray);
    SafeArrayGetElement(mArray, &iIndex, &value);
    SafeArrayUnlock(mArray);
    return value;
}

void HxLongSafeArray::Set(long iIndex, long value) {
    ASSERT(iIndex < mNumItems);
    SafeArrayLock(mArray);
    SafeArrayPutElement(mArray, &iIndex, &value);
    SafeArrayUnlock(mArray);
}

void HxLongSafeArray::GetVariant(VARIANT *variant) {
    VariantInit(variant);
    variant->vt = VT_ARRAY | VT_I4;
    variant->parray = mArray;
}

void HxLongSafeArray::SetFromVector(std::vector<int> &values) {
    int numItems = values.size();
    Resize(numItems);
    for (int i = 0; i < numItems; ++i) {
        Set(i, values[i]);
    }
}
