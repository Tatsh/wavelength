#include "gfx/tnlconnectorbeam.h"

#include <iterator>
#include <utility>
#include <vector>

#include "game/gamedb.h"
#include "gfx/floatlist.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/interpolator.h"
#include "math/rand.h"
#include "math/transform.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

constexpr float kNoTick = -1e9f;
constexpr char kNoTrack = -1;
constexpr char kNoType = -1;
constexpr float kPi = 3.14159274f;
constexpr float kHalfPi = 1.57079637f;
constexpr float kTwoPi = 6.28318548f;

// The points of the line, by the community of the game.
constexpr int kSoloPoints = 146;
constexpr int kSharedPoints = 64;

// The beam is cut into spans of at most one beat, and kPhasePerTick maps a tick onto the phase of
// the waves.
constexpr float kTicksPerSpan = 480.0f;
constexpr float kPhasePerTick = 1.0f / 3840.0f;

// The wave in time that a pulse starts.
constexpr float kBeatFrequency = 1.0f / 240.0f;
constexpr float kBeatAmplitude = 0.85f;

constexpr float kMinDecayRate = 1e-4f;
constexpr float kGlowBoost = 0.75f;
constexpr float kGlowSizeScale = 1.1f;
constexpr float kFullEnergy = 1.0f;

// A new glow particle waits off the tunnel until RebuildSpans() moves it onto its gem.
constexpr float kParkedOffset = -99.0f;

// An arch spans one half cycle between two gems.
constexpr float kArchCycle = 0.5f;

const char *const kSectionName = "connector_beam";

// NTSC-U/C: 0x003afc14
int sNumPoints = 146;
// NTSC-U/C: 0x003afc18
float sGlowBoostDecay = 0.0012f;
// NTSC-U/C: 0x003afc1c
float sAmplitude = 0.3f;
// NTSC-U/C: 0x003afc20
Interpolator *sEnergyModel = nullptr;
// NTSC-U/C: 0x003afc24
float sHeight = 0.04f;
// NTSC-U/C: 0x003afc28
float sParticleBaseSize = 0.4f;
// NTSC-U/C: 0x003afc2c
float sParticleMultiplierBaseSize = 0.7f;
// NTSC-U/C: 0x0043b6e0
std::vector<float> sLineEnergyFalloff;
// Alternate spans round the number of points to the nearest and down, which spreads the rounding.
// NTSC-U/C: 0x00400f10
const float kPointBias[] = {0.0f, 0.4999f};

} // namespace

TnlConnectorBeam::TnlConnectorBeam(GfxTunnel *pTunnel,
                                   int nPlayer,
                                   const char *pszColor,
                                   char nExcludedType,
                                   char nOtherExcludedType)
    : mSway(nullptr), mBeat(nullptr) {
    mDecayRate = 1.0f;
    mTunnel = pTunnel;
    mNumSpans = 0;
    auto *pLine =
        dynamic_cast<Rnd::Line *>(Rnd::TheManager.Find(FormatString("sabre_%c.str", pszColor[0])));
    mGlowSize = 0.0f;
    mGlowBoost = 0.0f;
    mEnergyLevel = 0.0f;
    mEnergy = 0.0f;
    mLine = pLine;
    mInvWindowLength = 1.0f;
    mLineEndTick = kNoTick;
    mStartTick = kNoTick;
    mEndTick = kNoTick;
    mPulseTick = kNoTick;
    mTrack = kNoTrack;
    auto *pGlow = dynamic_cast<Rnd::ParticleSys *>(
        Rnd::TheManager.Find(FormatString("gem_glow%d.ps", nPlayer)));
    mMultiplier = 0;
    mGlow = pGlow;
    mLineMat = mLine->GetMat();
    mMultiplierMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("mult fx gem.mat"));
    mExcludedType = nExcludedType;
    mOtherExcludedType = nOtherExcludedType;
    mLine->SetShowing(0);
    sNumPoints = (TheGameDb->mCommunity == GameDb::kCommunitySolo) ? kSoloPoints : kSharedPoints;
    mLine->SetNumPoints(sNumPoints);
}

TnlConnectorBeam::~TnlConnectorBeam() {
}

void TnlConnectorBeam::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel) {
    DataArray *pSection = pConfig->FindArray(kSectionName, false);
    DataArray *pDefaultSection = pDefaults->FindArray(kSectionName, true);
    delete sEnergyModel;
    sEnergyModel = nullptr;

    DataArray *pValue;
    FindConfigArray(pSection, pDefaultSection, "gem_energy_model", &pValue, true);
    sEnergyModel = ObjectToInterpolator(pValue->Array(1));
    FindConfigArray(pSection, pDefaultSection, "line_energy_falloff", &pValue, true);
    LoadFloatList(sLineEnergyFalloff, pValue);

    float flWidth = 0.04f;
    FindConfigFloat(pSection, pDefaultSection, "particle_base_size", &sParticleBaseSize, true);
    FindConfigFloat(pSection,
                    pDefaultSection,
                    "particle_multiplier_base_size",
                    &sParticleMultiplierBaseSize,
                    true);
    FindConfigFloat(pSection, pDefaultSection, "height", &sHeight, true);
    FindConfigFloat(pSection, pDefaultSection, "width", &flWidth, true);
    sHeight = 1.0f - (TheGfxManager.mScrollSpeed * sHeight);
    mLine->mWidth = flWidth * pTunnel->PanelWidth();
    sParticleBaseSize *= TheGfxManager.mScrollSpeed;
    sParticleMultiplierBaseSize *= TheGfxManager.mScrollSpeed;

    FindConfigFloat(pSection, pDefaultSection, "gem_energy_falloff", &sGlowBoostDecay, true);
    sGlowBoostDecay /= 100.0f;
    FindConfigFloat(pSection, pDefaultSection, "min_frequency", &mMinFrequency, true);
    FindConfigFloat(pSection, pDefaultSection, "max_frequency", &mMaxFrequency, true);
    FindConfigFloat(pSection, pDefaultSection, "amplitude", &sAmplitude, true);
    sAmplitude *= TheGfxManager.mScrollSpeed;

    mSway.mAmplitude = 1.0f;
    mBeat.mAmplitude = 1.0f;
    mSway.mFrequency = 1.0f;
    mBeat.mFrequency = 0.0f;
    mSway.mPhase = 0.0f;
    mBeat.mPhase = 0.0f;
}

void TnlConnectorBeam::Reset() {
    mLineEndTick = kNoTick;
    mPulseTick = kNoTick;
    if (mMultiplier) {
        StopMultiplier();
    }
}

void TnlConnectorBeam::SetWindow(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick) {
    if (flEndTick <= flStartTick || pGems == nullptr || nTrack < 0) {
        mLine->SetShowing(0);
        return;
    }
    mLine->SetShowing(1);
    mStartTick = flStartTick;
    const float flFirstTick = (mPulseTick < flStartTick) ? flStartTick : mPulseTick;
    const float flPulseTick = mPulseTick;
    mEndTick = flEndTick;
    mInvWindowLength = 1.0f / (flEndTick - flStartTick);
    mTrack = nTrack;
    mLineEndTick = flEndTick;
    if (flEndTick < flPulseTick) {
        mLine->SetShowing(0);
        return;
    }

    mGlow->FreeAllParticles();
    mGems.clear();
    int nLinked = 0;
    float flLastTick = kNoTick;
    for (auto it = pGems->LowerBound(flFirstTick); it != pGems->mGems.end(); ++it) {
        const float flTick = it->mTick;
        if (mEndTick < flTick) {
            break;
        }
        if (flTick < mPulseTick || it->mTrack != nTrack) {
            continue;
        }
        if (it->mType == mExcludedType || it->mType == mOtherExcludedType) {
            continue;
        }
        if (flTick == flLastTick || it->mRemoveTick != TnlGem::kNever) {
            continue;
        }
        if (nLinked != 0 && (static_cast<unsigned char>(it->mType) & TnlGem::kTypeSprite) != 0) {
            continue;
        }
        ++nLinked;
        AddGem(flTick, it->Lateral(), it->mParticle);
        flLastTick = flTick;
    }
    RebuildSpans();
}

void TnlConnectorBeam::AddGem(float flTick, float flLateral, Rnd::Particle *pParticle) {
    int nOwnParticle;
    if (mMultiplier) {
        if (pParticle != nullptr) {
            pParticle->mSize = 0.0f;
        }
        nOwnParticle = 1;
    } else {
        nOwnParticle = (pParticle == nullptr);
    }
    if (nOwnParticle) {
        pParticle = mGlow->AllocParticle();
        if (pParticle == nullptr) {
            nOwnParticle = 0;
        } else {
            // The original leaves the fourth word of the position as stack garbage.
            pParticle->mPos = Vector3{kParkedOffset, kNoTick, kParkedOffset};
            pParticle->mSize = 0.0f;
            pParticle->mCol = Color{1.0f, 1.0f, 1.0f, 1.0f};
        }
    }
    mGems.push_back(Gem{pParticle, nOwnParticle, flTick, flLateral});
}

void TnlConnectorBeam::RebuildSpans() {
    mNumSpans = 0;
    mSpans.clear();
    if (mGems.empty()) {
        return;
    }

    Transform xfms[2];
    Transform *pFrom = &xfms[0];
    Transform *pTo = &xfms[1];
    float flArchPhase = 0.0f;
    const bool bCaptured = mTunnel->IsTrackCaptured(mTrack);
    TnlGeom *pGeom = mTunnel->mGeom;
    auto it = mGems.begin();
    pGeom->PlaceCell(mTrack, pFrom, false, bCaptured, it->mTick, it->mLateral, sHeight);
    for (;;) {
        if (it->mOwnParticle && it->mParticle != nullptr) {
            it->mParticle->mPos = pFrom->mTranslation;
        }
        const auto next = std::next(it);
        if (next == mGems.end()) {
            return;
        }

        float flTick = it->mTick;
        const int nSteps = static_cast<int>(((next->mTick - flTick) - 1.0f) / kTicksPerSpan) + 1;
        float flPhase = (flTick - mStartTick) * kPhasePerTick;
        const float flGemPhaseGap = ((next->mTick - mStartTick) * kPhasePerTick) - flPhase;
        const float flGemPhase = flPhase;
        const float flStep = 1.0f / static_cast<float>(nSteps);
        float flFraction = 0.0f;
        for (int i = 0; i < nSteps; ++i) {
            const float flNextFraction = flFraction + flStep;
            mSpans.push_back(Span());
            ++mNumSpans;
            const float flSpanTick = ((next->mTick - it->mTick) * flNextFraction) + it->mTick;
            const float flLateral =
                ((next->mLateral - it->mLateral) * flNextFraction) + it->mLateral;
            const float flSpanPhase = (flSpanTick - mStartTick) * kPhasePerTick;
            pGeom->PlaceCell(mTrack, pTo, false, bCaptured, flSpanTick, flLateral, sHeight);

            Span &span = mSpans.back();
            span.mTick = flTick;
            span.mPhase = flPhase;
            span.mGemPhase = flGemPhase;
            span.mTickLength = flSpanTick - flTick;
            span.mPhaseLength = flSpanPhase - flPhase;
            span.mGemPhaseGap = flGemPhaseGap;
            span.mStart = pFrom->mTranslation;
            span.mArchPhase = flArchPhase;
            span.mDirection.x = pTo->mTranslation.x - pFrom->mTranslation.x;
            span.mDirection.z = pTo->mTranslation.z - pFrom->mTranslation.z;
            span.mDirection.y = pTo->mTranslation.y - pFrom->mTranslation.y;
            CrossVec3(&span.mDirection.x, &pTo->mBasisZ.x, &span.mSwayAxis.x);
            Vec3Normalize(&span.mSwayAxis.x, &span.mSwayAxis.x);

            std::swap(pFrom, pTo);
            flFraction = flNextFraction;
            flTick = flSpanTick;
            flPhase = flSpanPhase;
        }
        flArchPhase = (flArchPhase == 0.0f) ? kPi : 0.0f;
        it = next;
    }
}

void TnlConnectorBeam::OnRangeChanged(const TnlTrackRange *pRange) {
    if (pRange->Overlaps(mTrack, mStartTick, mEndTick)) {
        RebuildSpans();
    }
}

void TnlConnectorBeam::OnGemAdded(char nTrack, char nType, TnlGems *pGems, float flTick) {
    if (nTrack != mTrack) {
        return;
    }
    if (nType != kNoType) {
        if (nType == mExcludedType || nType == mOtherExcludedType) {
            return;
        }
        if ((nType & TnlGem::kTypeSprite) != 0) {
            return;
        }
    }
    if (mStartTick <= flTick && flTick < mEndTick) {
        SetWindow(nTrack, pGems, mStartTick, mEndTick);
    }
}

void TnlConnectorBeam::OnTicksChanged(char nTrack,
                                      TnlGems *pGems,
                                      float flStartTick,
                                      float flEndTick) {
    if (nTrack == mTrack && flStartTick < mEndTick && mStartTick < flEndTick) {
        SetWindow(nTrack, pGems, mStartTick, mEndTick);
    }
}

void TnlConnectorBeam::OnGemRemoved(const TnlGem *pGem) {
    if (pGem->mTrack != mTrack) {
        return;
    }
    if (!(pGem->mTick < mEndTick) || !(mStartTick <= pGem->mTick)) {
        return;
    }
    if (pGem->mType == mExcludedType || pGem->mType == mOtherExcludedType) {
        return;
    }
    if (pGem->mMeshGroup == nullptr || pGem->mParticle == nullptr) {
        return;
    }
    for (auto &gem : mGems) {
        if (gem.mParticle == pGem->mParticle) {
            gem.mParticle = nullptr;
        }
    }
}

void TnlConnectorBeam::Pulse(float flTick) {
    if (!(mStartTick <= flTick) || !(flTick < mEndTick)) {
        return;
    }
    if (mEnergy == 0.0f) {
        mSway.mFrequency = RandomFloat(mMinFrequency, mMaxFrequency);
        mBeat.Sync(flTick, kHalfPi);
        mBeat.mFrequency = kBeatFrequency;
        mBeat.mAmplitude = kBeatAmplitude;
    }
    mPulseTick = flTick;
    mEnergy = mEnergyLevel;
    const float flRate = EvalPolynomial(sLineEnergyFalloff, mEnergyLevel);
    mDecayRate = (kMinDecayRate < flRate) ? flRate : kMinDecayRate;
}

void TnlConnectorBeam::SetEnergy(float flEnergy) {
    const float flLevel = sEnergyModel->Eval(flEnergy);
    if (mEnergyLevel < flLevel || flLevel == kFullEnergy) {
        mGlowBoost = kGlowBoost;
    } else {
        mGlowBoost = 0.0f;
    }
    mEnergyLevel = flLevel;
    mGlowSize = flLevel * kGlowSizeScale;
}

void TnlConnectorBeam::StartMultiplier(TnlGems *pGems, float flEndTick) {
    mMultiplier = 1;
    SetWindow(mTrack, pGems, mStartTick, mEndTick);
    mMultiplierEndTick = flEndTick;
}

void TnlConnectorBeam::StopMultiplier() {
    mMultiplier = 0;
    const Color white{1.0f, 1.0f, 1.0f, 1.0f};
    const float flSize = mGlowSize + mGlowBoost;
    for (auto &gem : mGems) {
        if (gem.mParticle != nullptr) {
            gem.mParticle->mSize = flSize * sParticleBaseSize;
            gem.mParticle->mCol = white;
        }
    }
}

void TnlConnectorBeam::Poll(bool bCaptured, float flDelta, float flAlpha) {
    const float flNow = TheGameDb->mSongTick;
    mLineMat->SetAlpha(flAlpha);
    if (!bCaptured) {
        if (mMultiplier) {
            const float flSize = (sEnergyModel->mY1 + mGlowBoost) * sParticleMultiplierBaseSize;
            if (mMultiplierEndTick <= flNow) {
                StopMultiplier();
            } else {
                for (auto &gem : mGems) {
                    if (gem.mParticle != nullptr) {
                        gem.mParticle->mSize = flSize;
                        gem.mParticle->mCol = mMultiplierMat->mAmbient;
                    }
                }
            }
        } else {
            const float flSize = (mGlowSize + mGlowBoost) * sParticleBaseSize;
            for (auto &gem : mGems) {
                if (gem.mParticle != nullptr) {
                    gem.mParticle->mSize = flSize;
                }
            }
        }
        const float flBoost = mGlowBoost - (flDelta * sGlowBoostDecay);
        mGlowBoost = (0.0f < flBoost) ? flBoost : 0.0f;
    }

    const float flEnergy = mEnergy;
    const float flDecayed = flEnergy - (flDelta * mDecayRate);
    mEnergy = (0.0f < flDecayed) ? flDecayed : 0.0f;
    const float flFirstTick = (mStartTick < mPulseTick) ? mPulseTick : mStartTick;
    if (mSpans.empty()) {
        mLine->SetShowing(0);
        return;
    }

    int nPoint = sNumPoints - 1;
    int nRemaining = sNumPoints - 1;
    const float flLength = mLineEndTick - flFirstTick;
    float flInvLength = 1.0f;
    if (flLength != 0.0f) {
        flInvLength = 1.0f / flLength;
    }
    const float flSwing = (mBeat.Sine(flNow) + (1.0f - mBeat.mAmplitude)) * flEnergy * sAmplitude;
    int nBias = 1;
    if (nRemaining != 0) {
        for (auto it = mSpans.begin(); it != mSpans.end();) {
            const Span &span = *it;
            ++it;
            if (!(span.mTick < mPulseTick)) {
                int nCount = static_cast<int>(
                    (span.mTickLength * flInvLength * static_cast<float>(sNumPoints)) +
                    kPointBias[nBias]);
                nBias ^= 1;
                if (nCount < 1) {
                    nCount = 1;
                }
                if (nRemaining < nCount) {
                    nCount = nRemaining;
                }
                nRemaining -= nCount;

                const float flInvCount = 1.0f / static_cast<float>(nCount);
                Vector3 pos = span.mStart;
                Vector3 step = span.mDirection;
                step.x *= flInvCount;
                step.y *= flInvCount;
                step.z *= flInvCount;
                float flPhase = span.mPhase;
                const float flPhaseStep = span.mPhaseLength * flInvCount;
                TnlWave arch(1.0f, kArchCycle / span.mGemPhaseGap, 0.0f);
                arch.Sync(span.mGemPhase, 0.0f);
                for (int i = nCount; i > 0; --i) {
                    const float flArch = arch.Sine(flPhase);
                    const float flSway = mSway.Sine(flPhase);
                    flPhase += flPhaseStep;
                    const float flOffset = flArch * flSwing * flSway;
                    const Vector3 point{pos.x + (span.mSwayAxis.x * flOffset),
                                        pos.y + (span.mSwayAxis.y * flOffset),
                                        pos.z + (span.mSwayAxis.z * flOffset)};
                    mLine->SetPoint(nPoint--, point);
                    pos.x += step.x;
                    pos.y += step.y;
                    pos.z += step.z;
                }
            }
            if (nRemaining == 0) {
                break;
            }
        }
    }

    if (!mSpans.empty()) {
        const Span &last = mSpans.back();
        const Vector3 end{last.mStart.x + last.mDirection.x,
                          last.mStart.y + last.mDirection.y,
                          last.mStart.z + last.mDirection.z};
        mLine->SetPoint(nPoint--, end);
    } else if (!mGems.empty()) {
        // Unreachable, the spans having been tested above, but present in the original.
        Transform xfm;
        const Gem &last = mGems.back();
        mTunnel->mGeom->PlaceCell(mTrack,
                                  &xfm,
                                  false,
                                  mTunnel->IsTrackCaptured(mTrack),
                                  last.mTick,
                                  last.mLateral,
                                  sHeight);
        mLine->SetPoint(nPoint--, xfm.mTranslation);
    }
    if (nPoint == sNumPoints - 1) {
        mLine->SetShowing(0);
        return;
    }
    const Vector3 &lastPoint = mLine->mPoints[nPoint + 1].mPos;
    for (; nPoint >= 0; --nPoint) {
        mLine->SetPoint(nPoint, lastPoint);
    }
}
