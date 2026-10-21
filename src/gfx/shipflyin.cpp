#include "gfx/shipflyin.h"

#include "game/gamedb.h"
#include "math/transformops.h"
#include "rnd/rndmanager.h"

namespace {

// The ticks the flight lasts, one bar.
constexpr float kFlightTicks = 1920.0f;

} // namespace

ShipFlyIn::ShipFlyIn(Ship *pShip) : mShip(pShip), mStarted(0), mPath(0.0f, 0.0f, 0.0f, 1.0f) {
    mAnim = dynamic_cast<RndTransAnim *>(TheManager.Find("ship tutorial fly-in.tnm"));
    mShip->SetShown(false);
}

bool ShipFlyIn::Poll(float fTick,
                     [[maybe_unused]] float fTime,
                     float (*pXfm)[kTransformRowFloatCount]) {
    if (mStarted == 0) {
        return false;
    }
    if (mPath.GetX1() <= fTick) {
        return true;
    }
    Transform flight;
    mAnim->MakeTransform(mPath.Interp(fTick), flight, 1);
    sceVu0MulAffineMatrixXyz(&pXfm[0][0], &pXfm[0][0], &flight.mBasisX.x);
    return false;
}

bool ShipFlyIn::Start(bool bShow) {
    if (!bShow || mStarted != 0) {
        return bShow;
    }
    mStarted = 1;
    const float fTick = TheGameDb->GetSongTick();
    mPath.Reset(0.0f, kFlightTicks, fTick, fTick + kFlightTicks);
    return bShow;
}
