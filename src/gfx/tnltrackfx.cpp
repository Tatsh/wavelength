#include "gfx/tnltrackfx.h"

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "gfx/playercamfx.h"
#include "gfx/tnlcripfx.h"
#include "gfx/tnlgems.h"
#include "gfx/tnlstreakgemfx.h"
#include "math/color.h"
#include "math/vector3.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The time a track has started its effect at before any effect runs.
constexpr float kNever = 1e9f;

// The power of the ease of the drop and of the fall of a capture.
constexpr float kCurvePower = 2.0f;

// The gem ticks a track reserves room for.
constexpr int kReservedGemTicks = 12;

// The frames of the spew each capture advances it by.
constexpr float kSpewFrames = 100.0f;

// The position across the track a capture spews from.
constexpr float kSpewLateral = 0.5f;

// The values of the mode of TnlGeom::SetTrackOverlay().
constexpr int kOverlayNormal = 0;
constexpr int kOverlayEnabling = 1;

// The steps of each effect, the values of mPhase.
constexpr char kPhaseNone = 0;
constexpr char kPhaseRise = 1;
constexpr char kPhaseHold = 2;
constexpr char kPhaseFade = 3;
constexpr char kPhaseDrop = 1;
constexpr char kPhaseFall = 2;

// Conversions of the crippler settings.
constexpr float kCyclesPerBarToRadiansPerTick = 0.00327249235f;
constexpr float kHertzToRadiansPerMillisecond = 0.00628318544f;
constexpr float kRadiansPerDegree = 0.0174532924f;

// The fire of a capture burns white.
constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};

// Report that the player of a fire is not known.
constexpr int kNoPlayer = -1;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

inline void ScaleBasis(Transform *pXfm, float flScale) {
    Vector3 *rows[] = {&pXfm->mBasisX, &pXfm->mBasisY, &pXfm->mBasisZ};
    for (Vector3 *pRow : rows) {
        pRow->x *= flScale;
        pRow->y *= flScale;
        pRow->z *= flScale;
    }
}

} // namespace

float TnlTrackFX::sEnableTimes[3] = {200.0f, 400.0f, 200.0f};
float TnlTrackFX::sCaptureTimes[3] = {400.0f, 200.0f, 1000.0f};
float TnlTrackFX::sCaptureHeight = 0.5f;
float TnlTrackFX::sEnableHeight = -3.0f;
float TnlTrackFX::sCaptureSpewTicks = 120.0f;
float TnlTrackFX::sCaptureCamJiggle = 0.1f;
float TnlTrackFX::sCaptureBrightness = 0.6f;
Vector2 TnlTrackFX::sFlareSize{0.1f, 0.3f};
Rnd::ParticleSys *TnlTrackFX::sSpew = nullptr;
float TnlTrackFX::sSpewFrame = 0.0f;

TnlTrackFX::TnlTrackFX(char nTrack)
    : mState(kStateIdle), mTrack(nTrack), mPhase(kPhaseNone), mStartTime(kNever),
      mEnableCurve(0.0f, 0.0f, 0.0f, 1.0f), mDropCurve(0.0f, 0.0f, 0.0f, 1.0f, kCurvePower),
      mFallCurve(0.0f, 0.0f, 0.0f, 1.0f, kCurvePower) {
    if (nTrack == 0) {
        sSpew = FindObject<Rnd::ParticleSys>("autocapture spew.part");
        ScaleParticles(sSpew, TheGfxManager.mScrollSpeed);
        sSpewFrame = 0.0f;
        sSpew->Rnd::ParticleSys::SetFrameSelf(sSpewFrame);
        sSpew->FreeAllParticles();
    }
    Rnd::Mat *pBase = FindObject<Rnd::Mat>("panel enable.mat");
    if (nTrack == 0) {
        mMat = pBase;
    } else {
        mMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Create(
            Rnd::Mat::sClassName.mStr, FormatString("panel enable trk %d.mat", nTrack)));
        mMat->Copy(pBase, 0);
    }
    mGemTicks.reserve(kReservedGemTicks);
}

TnlTrackFX::~TnlTrackFX() {
    sSpew = nullptr;
    sSpewFrame = 0.0f;
    if ((mTrack != 0) && (mMat != nullptr)) {
        delete mMat;
    }
}

void TnlTrackFX::LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *, int) {
    Vector3 times;
    FindConfigVector3(pConfig, pDefaults, "track_activate_lengths", &times, true);
    sEnableTimes[0] = times.x;
    sEnableTimes[2] = times.z;
    sEnableTimes[1] = times.y;

    FindConfigFloat(pConfig, pDefaults, "streak_arrow_fade_ms", &TnlStreakGemFX::sBlinkRate, true);
    float flRate = 1.0f / TnlStreakGemFX::sBlinkRate;
    if (1.0f < flRate) {
        flRate = 1.0f;
    } else if (flRate < 0.0f) {
        flRate = 0.0f;
    }
    TnlStreakGemFX::sBlinkRate = flRate;

    FindConfigVector2(pConfig, pDefaults, "autocapture_flare_size", &sFlareSize, true);
    const float flScrollSpeed = TheGfxManager.mScrollSpeed;
    sFlareSize.y *= flScrollSpeed;
    sFlareSize.x *= flScrollSpeed;

    FindConfigVector3(pConfig, pDefaults, "autocapture_anim_times", &times, true);
    sCaptureTimes[0] = times.x;
    sCaptureTimes[2] = times.z;
    sCaptureTimes[1] = times.y;
    FindConfigFloat(pConfig, pDefaults, "autocapture_offset_height", &sCaptureHeight, true);
    FindConfigFloat(pConfig, pDefaults, "track_enable_offset_height", &sEnableHeight, true);
    sCaptureHeight *= TheGfxManager.mScrollSpeed;
    sEnableHeight *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "autocapture_burst_tick", &sCaptureSpewTicks, true);
    FindConfigFloat(pConfig, pDefaults, "autocapture_cam_jiggle", &sCaptureCamJiggle, true);
    FindConfigFloat(pConfig, pDefaults, "autocapture_track_brightness", &sCaptureBrightness, true);

    FindConfigFloat(pConfig, pDefaults, "crippler_fly_time", &TnlCripFX::sFlyTime, true);
    FindConfigFloat(pConfig, pDefaults, "crippler_fade_time", &TnlCripFX::sFadeTime, true);
    FindConfigFloat(pConfig, pDefaults, "crippler_wave_time", &TnlCripFX::sWaveTime, true);
    Vector2 pair;
    FindConfigVector2(pConfig, pDefaults, "crippler_spatial_freqs", &pair, true);
    pair.x *= kCyclesPerBarToRadiansPerTick;
    pair.y *= kCyclesPerBarToRadiansPerTick;
    TnlCripFX::sSpatialFreqs[1] = pair.y;
    TnlCripFX::sSpatialFreqs[0] = pair.x;
    FindConfigVector2(pConfig, pDefaults, "crippler_time_freqs", &pair, true);
    pair.y *= kHertzToRadiansPerMillisecond;
    pair.x *= kHertzToRadiansPerMillisecond;
    TnlCripFX::sTimeFreqs[0] = pair.x;
    TnlCripFX::sTimeFreqs[1] = pair.y;
    FindConfigFloat(
        pConfig, pDefaults, "crippler_wave_death_length", &TnlCripFX::sWaveDeathLength, true);
    FindConfigVector2(pConfig, pDefaults, "crippler_wave_magnitudes", &pair, true);
    TnlCripFX::sWaveMagnitudes[0] = pair.x * TheGfxManager.mScrollSpeed;
    TnlCripFX::sWaveMagnitudes[1] = pair.y * TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "crippler_tick_vel", &TnlCripFX::sTickVelocity, true);
    FindConfigFloat(pConfig, pDefaults, "crippler_start_height", &TnlCripFX::sStartHeight, true);
    FindConfigFloat(
        pConfig, pDefaults, "crippler_start_height_vel", &TnlCripFX::sStartHeightVelocity, true);
    FindConfigFloat(
        pConfig, pDefaults, "crippler_height_accel", &TnlCripFX::sHeightAcceleration, true);
    FindConfigFloat(
        pConfig, pDefaults, "crippler_impact_cam_jiggle", &TnlCripFX::sImpactCamJiggle, true);
    FindConfigFloat(pConfig, pDefaults, "crippler_max_twist_angle", &TnlCripFX::sMaxTwist, true);
    TnlCripFX::sMaxTwist *= kRadiansPerDegree;
    FindConfigFloat(pConfig, pDefaults, "crippler_twist_frequency", &TnlCripFX::sTwistFreq, true);
    TnlCripFX::sTwistFreq *= kCyclesPerBarToRadiansPerTick;
}

void TnlTrackFX::Enable(GfxTunnel *pTunnel) {
    if (mState != kStateIdle) {
        Stop(pTunnel);
    }
    mState = kStateEnable;
    mStartTime = TheGameDb->mSongTime;
    mEnableCurve.Reset(0.0f, 1.0f, mStartTime, mStartTime + sEnableTimes[0]);
    mMat->mBlend = Rnd::Mat::kBlendModeSrcAlphaAdd;
    pTunnel->mGeom->SetTrackOverlay(mTrack, mMat, kOverlayEnabling);
    mPhase = kPhaseRise;
}

void TnlTrackFX::Capture(GfxTunnel *pTunnel, float flTick) {
    if (mState != kStateIdle) {
        Stop(pTunnel);
    }
    mState = kStateCapture;
    mCaptureTick = flTick;
    mStartTime = TheGameDb->mSongTime;
    mDropCurve.Reset(0.0f, 1.0f, mStartTime, mStartTime + sCaptureTimes[0]);
    mMat->mBlend = Rnd::Mat::kBlendModeSrcAlphaAdd;
    mMat->mStages[0].mBlend = Rnd::Mat::kBlendModeSrcAlphaAdd;
    pTunnel->mGeom->SetTrackOverlay(mTrack, mMat, kOverlayNormal);

    Transform xfm;
    pTunnel->mGeom->CellXfm(
        mTrack, &xfm, false, true, TheGameDb->mSongTick + sCaptureSpewTicks, kSpewLateral);
    ScaleBasis(&xfm, TheGfxManager.mScrollSpeed);
    sSpew->SetLocalXfm(xfm);
    sSpew->UpdateWorldXfm(nullptr, 0);
    sSpewFrame += kSpewFrames;
    sSpew->Rnd::ParticleSys::SetFrameSelf(sSpewFrame);

    mGemTicks.clear();
    pTunnel->CollectGemTicks(&mGemTicks, mTrack, TheGameDb->mSongTick, flTick);
    mPhase = kPhaseDrop;
}

void TnlTrackFX::Stop(GfxTunnel *pTunnel) {
    const Vector3 rest{0.0f, 0.0f, 0.0f};
    pTunnel->mGeom->SetTrackOffset(mTrack, &rest);
    pTunnel->mGeom->SetTrackOverlay(mTrack, nullptr, kOverlayNormal);
    mPhase = kPhaseNone;
    mMat->SetAlpha(0.0f);
    mState = kStateIdle;
}

bool TnlTrackFX::Poll(GfxTunnel *pTunnel, TnlGeom *pGeom) {
    switch (mState) {
    case kStateIdle:
        return false;
    case kStateEnable: {
        const float flNow = TheGameDb->mSongTime;
        float flAlpha;
        Rnd::Mat::BlendMode stageBlend = Rnd::Mat::kBlendModeSrcAlpha;
        if (mPhase == kPhaseRise) {
            const char nTrack = mTrack;
            Vector3 offset{0.0f, 0.0f, 0.0f};
            if (mEnableCurve.mX1 <= flNow) {
                mPhase = kPhaseHold;
                flAlpha = 1.0f;
                mEnableCurve.Reset(
                    flAlpha, 0.0f, mEnableCurve.mX1, mEnableCurve.mX1 + sEnableTimes[1]);
                pTunnel->mGeom->SetTrackOverlay(mTrack, mMat, kOverlayNormal);
            } else {
                flAlpha = mEnableCurve.Interp(flNow);
                offset.x = 0.0f;
                offset.y = 0.0f;
                offset.z = (1.0f - flAlpha) * sEnableHeight;
            }
            pGeom->SetTrackOffset(nTrack, &offset);
        } else if (mPhase == kPhaseHold) {
            if (mEnableCurve.mX1 <= flNow) {
                mPhase = kPhaseFade;
                flAlpha = 0.0f;
                mEnableCurve.Reset(
                    1.0f, flAlpha, mEnableCurve.mX1, mEnableCurve.mX1 + sEnableTimes[2]);
                mMat->mBlend = Rnd::Mat::kBlendModeSrcAlpha;
            } else {
                flAlpha = mEnableCurve.Interp(flNow);
            }
            stageBlend = Rnd::Mat::kBlendModeSrcAlphaAdd;
        } else if (mPhase == kPhaseFade) {
            if (mEnableCurve.mX1 <= flNow) {
                mPhase = kPhaseNone;
                mState = kStateIdle;
                pGeom->SetTrackOverlay(mTrack, nullptr, kOverlayNormal);
                flAlpha = 0.0f;
            } else {
                flAlpha = mEnableCurve.Interp(flNow);
            }
        } else {
            return false;
        }
        mMat->SetAlpha(flAlpha);
        mMat->mStages[0].mBlend = stageBlend;
        return false;
    }
    case kStateCapture: {
        const float flNow = TheGameDb->mSongTime;
        if (mPhase == kPhaseDrop) {
            float flSize;
            float flAlpha;
            float flHeight;
            if (mDropCurve.mX1 <= flNow) {
                mPhase = kPhaseFall;
                flSize = sFlareSize.y;
                flHeight = sCaptureHeight;
                flAlpha = sCaptureBrightness;
                mFallCurve.Reset(1.0f, 0.0f, mDropCurve.mX1, mDropCurve.mX1 + sCaptureTimes[1]);
            } else {
                const float flDrop = mDropCurve.Interp(flNow);
                flHeight = flDrop * sCaptureHeight;
                flAlpha = flDrop * sCaptureBrightness;
                flSize = ((sFlareSize.y - sFlareSize.x) * flDrop) + sFlareSize.x;
            }
            SetGlowSizes(&pTunnel->mGems->mGems, mTrack, flSize);
            mMat->SetAlpha(flAlpha);
            const Vector3 offset{0.0f, 0.0f, flHeight};
            pTunnel->mGeom->SetTrackOffset(mTrack, &offset);
            return true;
        }
        if (mPhase != kPhaseFall) {
            return true;
        }
        float flHeight;
        if (mFallCurve.mX1 <= flNow) {
            pGeom->SetTrackOverlay(mTrack, nullptr, kOverlayNormal);
            flHeight = 0.0f;
            pTunnel->mCamFX->Kick(sCaptureCamJiggle);
            pTunnel->StartFire(kNoPlayer,
                               mTrack,
                               1,
                               TheGameDb->mSongTick,
                               TheGameDb->mSongTick,
                               mCaptureTick,
                               &kWhite,
                               &kWhite,
                               &mGemTicks);
            RestoreGlowSizes(&pTunnel->mGems->mGems, mTrack);
            mState = kStateIdle;
            mPhase = kPhaseNone;
        } else {
            flHeight = mFallCurve.Interp(flNow) * sCaptureHeight;
        }
        const Vector3 offset{0.0f, 0.0f, flHeight};
        pTunnel->mGeom->SetTrackOffset(mTrack, &offset);
        return true;
    }
    default:
        return true;
    }
}

float TnlTrackFX::CaptureEndTime() const {
    return (mStartTime + sCaptureTimes[0]) + sCaptureTimes[1];
}

void TnlTrackFX::SetGlowSizes(std::list<TnlGem> *pGems, char nTrack, float flSize) {
    for (TnlGem &gem : *pGems) {
        if ((gem.mTrack != nTrack) || ((gem.mType & TnlGem::kTypeSprite) != 0)) {
            continue;
        }
        if (gem.mParticle != nullptr) {
            gem.mParticle->mSize = flSize;
        }
    }
}

void TnlTrackFX::RestoreGlowSizes(std::list<TnlGem> *pGems, char nTrack) {
    for (TnlGem &gem : *pGems) {
        if ((gem.mTrack != nTrack) || ((gem.mType & TnlGem::kTypeSprite) != 0)) {
            continue;
        }
        if (gem.mParticle != nullptr) {
            gem.mParticle->mSize = gem.mMeshGroup->mParticles->mSizeLow;
        }
    }
}
