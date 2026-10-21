#include "gfx/tnldivider.h"

#include <algorithm>
#include <iterator>

#include "game/gamedb.h"
#include "gfx/gfxutil.h"
#include "gfx/playercamfx.h"
#include "gfx/unreadvector.h"
#include "os/locale.h"
#include "rnd/rndcollideable.h"
#include "rnd/rndenviron.h"
#include "rnd/rndmanager.h"
#include "rnd/rndtransanim.h"

namespace {

// The frame of the flash fade and of the condition view at rest, and the flash at its peak.
constexpr float kFlashFrames = 1000.0f;

// A boundary further behind the song than this many ticks is dropped.
constexpr float kBarTicks = 1920.0f;

// The ticks since a boundary its particles fade over.
constexpr float kParticleFadeStart = -5760.0f;
constexpr float kParticleFadeEnd = -1920.0f;

// TicksSinceSection() reports this before the first boundary.
constexpr float kNoSection = 1e9f;

// The text of a boundary with no label.
constexpr const char *kNoText = "";

inline bool IsBehind(const TnlDivider::Section &section, float flTick) {
    return (section.mTick - flTick) < -kBarTicks;
}

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

float TnlDivider::sFlashTime = 500.0f;

// The static initialiser of the unit, and the routines that run it to construct and to destroy the
// statics of the boundaries. The same initialiser constructs TnlTrackFX::sFlareSize and
// TnlStreakGemFX::sDistanceFade.
// NTSC-U/C: 0x001f1f10, PAL: 0x001facb0
// NTSC-U/C: 0x001f1ff8, PAL: 0x001fad98
// NTSC-U/C: 0x001f2018, PAL: 0x001fadb8

Vector3 TnlDivider::sScale{1.0f, 1.0f, 1.0f};
Vector3 TnlDivider::sOffset{0.0f, 0.0f, 0.0f};

TnlDivider::TnlDivider()
    : mView(nullptr), mText(nullptr), mFlash(kFlashFrames,
                                             FindObject<RndAnimatable>("boundary flash fade.tnm"),
                                             nullptr,
                                             kFlashFrames / sFlashTime),
      mConditionFrame(kFlashFrames),
      mParticleFade(1.0f, 0.0f, kParticleFadeStart, kParticleFadeEnd), mFlashing(0), mEnabled(1) {
    mStarted = 1;
    mTopView = FindObject<RndView>("boundary top.view");
    mTextScaleView = FindObject<RndView>("boundary text scale.view");
    mConditionView = FindObject<RndView>("boundary_condition.view");
    mMessageAnim = FindObject<RndAnimatable>("boundary msg.tnm");
    mText = FindObject<RndText>("boundary msg");
    mFlashMesh = FindObject<RndMesh>("boundary flash.mesh");
    mFlashMesh->SetShowing(0);
    mParticles[0] = FindObject<RndParticleSys>("boundary.part");
    mParticles[1] = FindObject<RndParticleSys>("boundary add.part");
    mView = FindObject<RndView>("boundary.view");
    mTransparentView = FindObject<RndView>("boundary transparent.view");
    mView->RemoveDraw(mTransparentView);
    mTransparentView->AddDraw(mText, nullptr);
    for (int i = 0; i < 2; ++i) {
        mEmitRates[i] = Vector2{mParticles[i]->GetEmitRate().x, mParticles[i]->GetEmitRate().y};
        mSizes[i] = Vector2{mParticles[i]->GetSize().x, mParticles[i]->GetSize().y};
    }
    mText->SetShowing(0);
    mView->SetShowing(0);
    mTransparentView->SetShowing(0);
    UpdateText(mSections.begin());

    mTopView->AddTrans(mView);
    // Yes, the binary scales every axis by the X of sScale.
    const float flScale = sScale.x;
    Transform layout;
    layout.mBasisX = Vector3{flScale, 0.0f, 0.0f};
    layout.mBasisY = Vector3{0.0f, flScale, 0.0f};
    layout.mBasisZ = Vector3{0.0f, 0.0f, flScale};
    layout.mTranslation = sOffset;
    mView->SetLocalXfm(layout);
    mView->SetDirty(1);
    ScaleParticleTree(mView, sScale.x);
    RndTransAnim *pMessage = FindObject<RndTransAnim>("boundary msg.tnm");
    ScaleKeys(pMessage->GetFramesOwner()->GetTransKeys(), sScale.x);
    ScaleKeys(pMessage->GetFramesOwner()->GetScaleKeys(), sScale.x);
    RndTransAnim *pFlash = FindObject<RndTransAnim>("boundary flash.tnm");
    ScaleKeys(pFlash->GetFramesOwner()->GetScaleKeys(), sScale.x);
    ScaleParticles(0.0f);
    mCurrent = mSections.begin();
}

void TnlDivider::ScaleParticles(float flScale) {
    for (int i = 0; i < 2; ++i) {
        mParticles[i]->GetEmitRate().x = mEmitRates[i].x * flScale;
        mParticles[i]->GetEmitRate().y = mEmitRates[i].y * flScale;
    }
}

void TnlDivider::Reset() {
    ClearSections();
}

void TnlDivider::AddSection(float flTick, const char *pszLabel, float flTextScale) {
    const float flNow = TheGameDb->GetSongTick();
    while (!mSections.empty() && IsBehind(mSections.front(), flNow)) {
        mSections.pop_front();
    }
    const Section section{flTick, flTextScale, pszLabel};
    const auto it = std::lower_bound(mSections.begin(), mSections.end(), section);
    if ((it == mSections.end()) || (it->mTick != flTick)) {
        mSections.insert(it, section);
    }
}

void TnlDivider::ClearSections() {
    mSections.clear();
    mCurrent = mSections.begin();
}

float TnlDivider::TicksSinceSection(float flTick) const {
    float flSince = kNoSection;
    for (auto it = mSections.begin(); (it != mSections.end()) && !(flTick < it->mTick); ++it) {
        flSince = flTick - it->mTick;
    }
    return flSince;
}

void TnlDivider::Poll(float flDelta, GfxTunnel *pTunnel) {
    if (!mEnabled) {
        mText->SetShowing(0);
        mView->SetShowing(0);
        mTransparentView->SetShowing(0);
        return;
    }
    const float flNow = TheGameDb->GetSongTick();
    if (!mStarted) {
        mCurrent = mSections.begin();
    }
    auto it = mCurrent;
    if ((it != mSections.end()) && IsBehind(*it, flNow)) {
        ++it;
    }
    if (it == mSections.end()) {
        if (!mSections.empty()) {
            const auto last = std::prev(mSections.end());
            if (!IsBehind(*last, flNow)) {
                it = last;
            }
        }
    } else if (!IsBehind(*it, flNow) && (it != mSections.begin())) {
        const auto previous = std::prev(it);
        if (!IsBehind(*previous, flNow)) {
            it = previous;
        }
    }

    if (it == mSections.end()) {
        mText->SetShowing(0);
        mView->SetShowing(0);
        mTransparentView->SetShowing(0);
    } else {
        mText->SetShowing(1);
        mView->SetShowing(1);
        mTransparentView->SetShowing(1);
        if (mCurrent != it) {
            PlaceOnPath(mTopView, pTunnel->GetGeom(), it->mTick);
            const float flScale = it->mTextScale;
            Transform xfm = Transform{Vector3{flScale, 0.0f, 0.0f},
                                      Vector3{0.0f, flScale, 0.0f},
                                      Vector3{0.0f, 0.0f, flScale},
                                      Vector3{}};
            mTextScaleView->GetLocalXfm().mBasisX = xfm.mBasisX;
            mTextScaleView->GetLocalXfm().mBasisY = xfm.mBasisY;
            mTextScaleView->GetLocalXfm().mBasisZ = xfm.mBasisZ;
            mTextScaleView->SetDirty(1);
            UpdateText(it);
        }
        const float flSince = flNow - it->mTick;
        mMessageAnim->SetFrame(flSince);
        mView->SetFrame(flNow);
        mConditionView->SetFrame(mConditionFrame);
        ScaleParticles(mParticleFade.Eval(flSince));
        if (mFlashing) {
            Segment sight;
            sight.mEnds[0] = pTunnel->GetCamFX()->GetView().mTranslation;
            sight.mEnds[1] = mTopView->GetLocalXfm().mTranslation;
            int nTrack;
            int nBar;
            const float flTarget =
                pTunnel->GetGeom()->Pick(&sight, &nTrack, &nBar) ? kFlashFrames : 0.0f;
            if (flTarget != mFlash.GetInterp()->GetY1()) {
                mFlash.SetTarget(flTarget);
            }
            mFlash.Update(flDelta, false);
            mFlashMesh->SetShowing((mFlash.GetValue() == kFlashFrames) ? 0 : 1);
        }
    }
    mStarted = 1;
    mCurrent = it;
}

void TnlDivider::Draw() {
    if (!mEnabled) {
        return;
    }
    RndEnviron *pEnviron = RndEnviron::sCurrent;
    mView->Draw();
    if (pEnviron != nullptr) {
        pEnviron->Draw();
    }
}

void TnlDivider::SetLayout(const Vector3 *pScale, const Vector3 *pOffset) {
    sScale = *pScale;
    sOffset = *pOffset;
}

void TnlDivider::UpdateText(std::list<Section>::iterator section) {
    int bShowParticles = 1;
    int bFlash = 0;
    const char *pszText = kNoText;
    if (section != mSections.end()) {
        if (section->mLabel != nullptr) {
            pszText = section->mLabel;
        } else if (section == mSections.begin()) {
            pszText = TheLocale.Localize("GAME_DIVIDER_START", true);
        } else if (TheGameDb->GetRuleSet() != GameDb::kRuleSetRemix) {
            ++section;
            if (section == mSections.end()) {
                pszText = TheLocale.Localize("GAME_DIVIDER_FINISH", true);
                bShowParticles = 0;
                bFlash = 1;
            } else {
                ++section;
                if (section == mSections.end()) {
                    pszText = TheLocale.Localize("GAME_DIVIDER_FINAL_SECTION", true);
                }
            }
        }
    }
    mText->SetText(pszText);
    mParticles[0]->SetShowing(bShowParticles);
    mParticles[1]->SetShowing(bShowParticles);
    mFlashing = bFlash;
    mFlashMesh->SetShowing(bFlash);
}

void TnlDivider::PlaceOnPath(RndTransformable *pTrans, TnlGeom *pGeom, float flTick) {
    Transform xfm;
    pGeom->PathXfm(&xfm, flTick);
    pTrans->SetLocalXfm(xfm);
    pTrans->SetDirty(1);
    pTrans->UpdateWorldXfm(nullptr, 0);
}
