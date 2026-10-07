#include "app/attachment.h"

Attachment::~Attachment() {
}

void Attachment::AddRef() {
    ++mRefs;
}

void Attachment::Release() {
    --mRefs;
    // The binary really tests the receiver against null before dispatching the deleting
    // destructor. The standard makes `this` non-null, so a current compiler both warns and is free
    // to discard the test. The diagnostic is suppressed here rather than over the file.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull-compare"
    if (mRefs == 0 && this != nullptr) {
        delete this;
    }
#pragma GCC diagnostic pop
}
