#include "gs/notemuse.h"

Muse *NoteMuse::Clone() {
    return new NoteMuse(mNote, mVelocity, mDuration, mChannel);
}
