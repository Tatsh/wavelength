#include "game/worldlogic.h"

#include "game/gamedb.h"
#include "os/joypad.h"
#include "os/memfun3command.h"
#include "os/scheduler.h"
#include "synth/synth.h"

namespace {

constexpr unsigned int kStatusControlChange = 0xb0;
constexpr unsigned int kAllNotesOffController = 123;
constexpr unsigned int kNumChannels = 16;
constexpr int kControllerShift = 8;

constexpr int kNoPad = -1;
constexpr int kDisconnectedReason = 1;
constexpr int kControllerCheckDelayTicks = 1;

} // namespace

Scheduler TheSongScheduler;

WorldLogic::WorldLogic() {
    JoypadAddSink(this);
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        JoypadSetMenuControl(i, false);
    }
}

WorldLogic::~WorldLogic() {
    JoypadRemoveSink(this);
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        JoypadSetMenuControl(i, true);
    }
}

void WorldLogic::AllNotesOff() {
    for (unsigned int nChannel = 0; nChannel < kNumChannels; ++nChannel) {
        TheSynth->SendPackedMessage((kStatusControlChange | nChannel) |
                                    (kAllNotesOffController << kControllerShift));
    }
}

void WorldLogic::SetPaused(bool bPaused, [[maybe_unused]] int nPad, [[maybe_unused]] int nReason) {
    if (!bPaused) {
        ScheduleControllerCheck();
    }
}

void WorldLogic::ScheduleControllerCheck() {
    const int nPad = FindDisconnectedPad();
    if (nPad == kNoPad) {
        return;
    }
    TheSongScheduler.PostIn(
        NewMemFun3Command(this, &WorldLogic::SetPaused, true, nPad, kDisconnectedReason),
        kControllerCheckDelayTicks,
        false);
}

int WorldLogic::FindDisconnectedPad() {
    if (TheGameDb->GetDemo() != nullptr) {
        return kNoPad;
    }
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        const int nPad = TheGameDb->GetPlayerPad(i);
        if (nPad != kNoPad && JoypadGetState(nPad)->mConnected == 0) {
            return nPad;
        }
    }
    return kNoPad;
}

int WorldLogic::OnJoypadConnection(JoypadConnectionMsg *pMsg) {
    if (TheGameDb->GetDemo() != nullptr || !IsPlaying()) {
        return 0;
    }
    if (pMsg->mPad < TheGameDb->GetNumPads() && pMsg->mConnected == 0) {
        SetPaused(true, pMsg->mPad, kDisconnectedReason);
    }
    return 0;
}

void WorldLogic::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadConnectionMsgType) {
        (void)OnJoypadConnection(static_cast<JoypadConnectionMsg *>(pMsg)); // The result is 0.
    }
}
