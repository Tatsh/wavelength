#include "utl/AllocInfo.h"

#include <cstring>

#include "os/Debug.h"

void AllocInfo::Print(PrnStream &stream) const {
    if (mPoolName) {
        stream.Printf("p \"%s\" %d ", mPoolName, mPoolSize);
    } else if (mName) {
        stream.Printf("m \"%s\" %d ", mName, mSizeReq);
    } else {
        ASSERT(false);
    }
}

bool AllocInfo::operator<(const AllocInfo &other) const {
    if (mSizeReq != other.mSizeReq) {
        return mSizeReq < other.mSizeReq;
    }
    if (mPoolSize != other.mPoolSize) {
        return mPoolSize < other.mPoolSize;
    }
    if (mName && other.mName) {
        const int order = strcmp(mName, other.mName);
        if (order) {
            // Equal sizes sort by name in reverse order.
            return order > 0;
        }
    }
    if (mPoolName && other.mPoolName) {
        const int order = strcmp(mPoolName, other.mPoolName);
        if (order) {
            return order > 0;
        }
    }
    return false;
}
