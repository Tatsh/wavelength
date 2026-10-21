#include "gfx/tnlactivator.h"

#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "gfx/ship.h"
#include "gfx/tnlgeom.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// A slide to a new place takes this many ticks.
constexpr float kMoveTicks = 240.0f;

// The scale an activator loses for each place it is ahead of the last.
constexpr float kScalePerPlace = 0.04f;

// The catcher of a blinking activator spends the later part of each period at half opacity.
constexpr int kBlinkPeriod = 240;
constexpr int kBlinkBright = 121;
constexpr float kBlinkAlpha = 0.5f;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

TnlActivator::TnlActivator(GfxTunnel *pTunnel, int nPlayer, int, const char *pszColor)
    : mPlayer(nPlayer), mRot(FindObject<RndView>(FormatString("activator rot%d", nPlayer))),
      mView(FindObject<RndView>(FormatString("activator%d.view", nPlayer))),
      mFx(FindObject<RndView>(FormatString("activator fx%d", nPlayer))),
      mCatcher(pszColor, pTunnel), mBlinking(0), mMoving(0), mPlace(0.0f, 0.0f, 0.0f, 1.0f),
      mScale(1.0f, 1.0f, 0.0f, 1.0f) {
    mTunnel = pTunnel;
    mPosition = 0.0f;
    SetRotShowing(1);
    mTunnel->GetGeom()->GetPlayer(mPlayer)->SetActivator(mView);
    mFx->ClearTrans();
    mFx->ClearDraws();
    mCatcher.AttachTo(mFx);
    mTunnel->GetGeom()->GetPlayer(mPlayer)->SetActivatorPosition(0.0f);
}

TnlActivator::~TnlActivator() = default;

void TnlActivator::LoadConfig(DataArray *, DataArray *, GfxTunnel *, int) {
}

void TnlActivator::SetRotShowing(int bShowing) {
    mRot->SetShowing(bShowing);
}

void TnlActivator::SetSuppressed(int bSuppressed) {
    mCatcher.SetEnabled(bSuppressed ^ 1);
}

void TnlActivator::SetPlace(signed char nPlace, signed char nNumPlaces) {
    const float flPlace = static_cast<float>(nPlace);
    const float flTick = TheGameDb->GetSongTick();
    const float flEndTick = flTick + kMoveTicks;
    (void)mTunnel->GetGeom()->GetPlayer(mPlayer); // Yes, the binary discards this call's result.
    if (mPlace.GetY1() != flPlace) {
        mMoving = 1;
        mPlace.Reset(mPlace.Eval(flTick), flPlace, flTick, flEndTick);
    }
    const signed char nBehind = static_cast<signed char>(nNumPlaces - nPlace - 1);
    const float flScale = 1.0f - (static_cast<float>(nBehind) * kScalePerPlace);
    if (mScale.GetY1() != flScale) {
        mScale.Reset(mScale.Eval(flTick), flScale, flTick, flEndTick);
    }
    mCatcher.SetRaised(nPlace != 0);
}

void TnlActivator::Hit(int nTarget, [[maybe_unused]] bool bHit) {
    mCatcher.Hit(nTarget);
}

void TnlActivator::Bump(bool bBumped, int, const TnlActivator *pAttacker) {
    if (bBumped && (mPlace.GetY1() == 0.0f)) {
        mCatcher.StartBump(pAttacker, mTunnel->GetGeom(), mPlayer);
    }
}

void TnlActivator::Reset() {
    mCatcher.StopBump(mTunnel->GetGeom(), mPlayer);
    mScale.Reset(1.0f, 1.0f, 0.0f, 1.0f);
    mPlace.Reset(0.0f, 0.0f, 0.0f, 1.0f);
    mPosition = 0.0f;
}

void TnlActivator::Update(float flDelta) {
    const float flTick = TheGameDb->GetSongTick();
    TnlGeom::Player *pPlayer = mTunnel->GetGeom()->GetPlayer(mPlayer);
    if (mMoving) {
        float flPosition;
        if (mPlace.GetX1() <= flTick) {
            mMoving = 0;
            flPosition = mPlace.GetY1();
        } else {
            flPosition = mPlace.Interp(flTick);
        }
        mPosition = flPosition;
        SetRotShowing(1);
        pPlayer->SetActivatorPosition(0.0f);
    }
    bool bDim = false;
    if (mBlinking) {
        bDim = (static_cast<int>(flTick) % kBlinkPeriod) >= kBlinkBright;
    }
    mCatcher.SetAlpha(bDim ? kBlinkAlpha : 1.0f);
    Ship *pShip = TheGfxManager.GetShip(mPlayer);
    pShip->SetArrowAspect(mScale.Eval(flTick));
    pShip->SetSpread(mPosition);
    mCatcher.Update(mTunnel->GetGeom(),
                    mPlayer,
                    (pShip->GetFlags() & Ship::kFlagBumped) != 0,
                    flDelta,
                    pShip->GetBumpStart());
}
