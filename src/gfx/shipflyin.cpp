#include "gfx/shipflyin.h"

#include "game/gamedb.h"
#include "math/transformops.h"
#include "rnd/manager.h"

namespace {

// The ticks the flight lasts, one bar.
constexpr float kFlightTicks = 1920.0f;

} // namespace

ShipFlyIn::ShipFlyIn(Ship *pShip) : mShip(pShip), mStarted(0), mPath(0.0f, 0.0f, 0.0f, 1.0f) {
    mAnim = dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find("ship tutorial fly-in.tnm"));
    mShip->SetShown(false);
}

bool ShipFlyIn::Poll(float fTick,
                     [[maybe_unused]] float fTime,
                     float (*pXfm)[Rnd::kXfmRowFloatCount]) {
    if (mStarted == 0) {
        return false;
    }
    if (mPath.mX1 <= fTick) {
        return true;
    }
    float aflFlight[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    mAnim->EvalFrame(mPath.Interp(fTick), &aflFlight[0][0], 1);
    sceVu0MulAffineMatrixXyz(&pXfm[0][0], &pXfm[0][0], &aflFlight[0][0]);
    return false;
}

bool ShipFlyIn::Start(bool bShow) {
    if (!bShow || mStarted != 0) {
        return bShow;
    }
    mStarted = 1;
    const float fTick = TheGameDb->mSongTick;
    mPath.Reset(0.0f, kFlightTicks, fTick, fTick + kFlightTicks);
    return bShow;
}
