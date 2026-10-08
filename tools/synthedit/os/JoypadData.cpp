#include "os/JoypadData.h"

#include "utl/ArraySize.h"

namespace {

const int kNoPlayer = -1;

} // namespace

JoypadData::JoypadData() {
    mButtons = 0;
    mPlayerNum = kNoPlayer;
    mReserved78 = false;
    int i;
    for (i = 0; i < ARRAY_SIZE(mReserved14); ++i) {
        mReserved14[i] = 0;
    }
    for (i = 0; i < ARRAY_SIZE(mReserved04); ++i) {
        mReserved04[i] = 0;
    }
}
