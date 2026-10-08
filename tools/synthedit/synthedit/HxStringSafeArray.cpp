#include "synthedit/HxStringSafeArray.h"

#include <stdlib.h>
#include <string.h>

#include "os/Debug.h"

namespace {

// A created array starts empty, indexed from zero.
const LONG kCreateLowerBound = 0;
const ULONG kCreateCount = 1;

// The longest element Get() converts, with its terminator.
const int kElementBufferSize = 500;

// The most wide characters Set() converts.
const size_t kUnlimitedCount = 0x7fffffff;

} // namespace

HxStringSafeArray::HxStringSafeArray(SAFEARRAY *array)
    : HxSafeArray(array == NULL ? SafeArrayCreateVector(VT_BSTR, kCreateLowerBound, kCreateCount) :
                                  array) {
    mOwned = false;
    if (array == NULL) {
        mOwned = true;
    }
}

HxStringSafeArray::HxStringSafeArray(std::vector<String> &values)
    : HxSafeArray(SafeArrayCreateVector(VT_BSTR, kCreateLowerBound, kCreateCount)) {
    mOwned = true;
    SetFromVector(values);
}

HxStringSafeArray::~HxStringSafeArray() {
    if (mOwned) {
        SafeArrayDestroy(mArray);
    }
}

String HxStringSafeArray::Get(long iIndex) {
    ASSERT(iIndex < mNumItems);
    BSTR element;
    SafeArrayLock(mArray);
    SafeArrayGetElement(mArray, &iIndex, &element);
    SafeArrayUnlock(mArray);
    char text[kElementBufferSize];
    int i = wcstombs(text, element, kElementBufferSize);
    ASSERT(i >= 0);
    String value(text);
    return value;
}

void HxStringSafeArray::Set(long iIndex, const char *value) {
    ASSERT(iIndex < mNumItems);
    BSTR *old = NULL;
    // The pointer is passed instead of its address, so the old element is never freed.
    SafeArrayPtrOfIndex(mArray, &iIndex, reinterpret_cast<void **>(old));
    if (old != NULL) {
        SysFreeString(*old);
    }
    BSTR element = SysAllocStringLen(NULL, strlen(value));
    mbstowcs(element, value, kUnlimitedCount);
    SafeArrayLock(mArray);
    SafeArrayPutElement(mArray, &iIndex, element);
    SafeArrayUnlock(mArray);
}

void HxStringSafeArray::GetVariant(VARIANT *variant) {
    VariantInit(variant);
    variant->vt = VT_ARRAY | VT_BSTR;
    variant->parray = mArray;
}

void HxStringSafeArray::SetFromVector(std::vector<String> &values) {
    int numItems = values.size();
    Resize(numItems);
    for (int i = 0; i < numItems; ++i) {
        Set(i, values[i]);
    }
}
