#include "os/Joypad.h"

#include "os/Debug.h"
#include "os/System.h"

namespace {

const int kNoPlayer = -1;
const float kDefaultThreshold = 0.75f;

} // namespace

JoypadData gJoypadData[kNumJoypads];

JoypadMsgSource gJoypadMsgSource;

int gPlayerNumToPadNum[kNumPlayers];

void JoypadConfigInit(DataArray *config) {
    float threshold = kDefaultThreshold;
    config->FindFloat("threshold", &threshold, true);
    for (int i = 0; i < kNumJoypads; ++i) {
        gJoypadData[i].mThreshold = threshold;
    }
    gJoypadMsgSource.mInitialized = true;
}

JoypadData *JoypadGetPlayerData(int iPlayerNum) {
    ASSERT_RANGE(iPlayerNum, 0, kNumPlayers);
    ASSERT_RANGE(gPlayerNumToPadNum[iPlayerNum], 0, kNumJoypads);
    return &gJoypadData[gPlayerNumToPadNum[iPlayerNum]];
}

void JoypadSubscribe(MsgSink *sink) {
    gJoypadMsgSource.AddSink(sink);
}

void JoypadUnsubscribe(MsgSink *sink) {
    gJoypadMsgSource.RemoveSink(sink);
}

void JoypadAssignPadToPlayer(int iPlayerNum, int iPadNum) {
    ASSERT_RANGE(iPadNum, 0, kNumJoypads);
    if (iPlayerNum == kNoPlayer) {
        gJoypadData[iPadNum].mPlayerNum = iPlayerNum;
        return;
    }
    ASSERT_RANGE(iPlayerNum, 0, kNumPlayers);
    gPlayerNumToPadNum[iPlayerNum] = iPadNum;
    gJoypadData[iPadNum].mPlayerNum = iPlayerNum;
}

void EmptyRoutine() {
}
