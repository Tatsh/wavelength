#include "met/fullrankedpanel.h"

namespace {

constexpr char kFlashKind[] = "sub";

} // namespace

void FullRankedPanel::FinishLoad() {
    FreqPanel::FinishLoad();
    InitFlash(mName, kFlashKind);
    mHilite = 0;
}

void FullRankedPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
    PollFlash(fTime);
}
