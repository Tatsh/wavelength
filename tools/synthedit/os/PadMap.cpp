#include "os/System.h"

namespace {

// Controller a player's input comes from.
struct PadMapEntry {
    int mPort; // Controller port, or -1 when the player has no controller.
    int mSlot; // Multitap slot of the port, or -1 when the player has no controller.
};

const int kNumPlayers = 4;
const int kNoController = -1;
const int kSecondPort = 1;

// 0x1003fee0
PadMapEntry gPadMap[kNumPlayers];

} // namespace

void PadMapInit() {
    EmptyRoutine();
    PadMapSet(false, false);
}

void PadMapTerminate() {
    EmptyRoutine();
}

void PadMapSet(bool multitap, bool secondPort) {
    if (multitap) {
        for (int i = 0; i < kNumPlayers; ++i) {
            gPadMap[i].mPort = 0;
            gPadMap[i].mSlot = i;
        }
        return;
    }
    gPadMap[0].mPort = 0;
    gPadMap[0].mSlot = 0;
    if (secondPort) {
        gPadMap[1].mPort = kSecondPort;
        gPadMap[1].mSlot = 0;
    } else {
        gPadMap[1].mPort = kNoController;
        gPadMap[1].mSlot = kNoController;
    }
    gPadMap[2].mPort = kNoController;
    gPadMap[2].mSlot = kNoController;
    gPadMap[3].mPort = kNoController;
    gPadMap[3].mSlot = kNoController;
}
