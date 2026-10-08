#include "gfx/ship.h"

#include <math.h>
#include <string.h>

#include "game/gamedb.h"
#include "gfx/beam.h"
#include "gfx/beampool.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxtunnel.h"
#include "gfx/shipflyin.h"
#include "math/rand.h"
#include "math/transformops.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/cam.h"
#include "rnd/manager.h"
#include "rnd/rndrenderer.h"

namespace {

// The components of a point or of a row of a basis.
constexpr int kNumComponents = 3;

// The frame and the time of a ship that has not started a measurement.
constexpr float kUnset = -1.0e9f;
constexpr float kFarFuture = 1.0e9f;

// The colour letter of the scene objects of a solo ship.
constexpr char kSoloColor = 's';

// The names of the arms in the names of their scene objects.
const char *const kArmNames[] = {"left", "middle", "right"};

// The extra beams in each pool beyond one for each player.
constexpr int kSpareBeams = 2;

// The time a beam lasts, in milliseconds.
constexpr float kBeamTime = 250.0f;

// The position across a track of the gem of each button.
constexpr float kSlotSpacing = 0.295f;
constexpr float kSlotOffset = 0.205f;

// The sideways tilt of the beams of the outer buttons.
constexpr float kBeamSpread = 0.4f;

// The gem buttons.
constexpr int kSlotLeft = 0;
constexpr int kSlotRight = 2;

// The energy zones SetEnergy() maps to the colours of the arms.
constexpr int kZoneMiddle = 1;
constexpr int kZoneHigh = 2;
constexpr int kArmOfHighZone = 0;
constexpr int kArmOfMiddleZone = 1;
constexpr int kArmOfLowZone = 2;

// The particles of an arm whose energy has run out.
const Color kSputterStartLow{1.0f, 1.0f, 1.0f, 0.5f};
const Color kSputterStartHigh{0.0f, 0.0f, 0.0f, 0.25f};
const Color kSputterEndLow{0.0f, 0.0f, 0.0f, 0.2f};
const Color kSputterEndHigh{0.0f, 0.0f, 0.0f, 0.2f};

// The particles of the crippler and of the arms of a ship of another console.
constexpr int kRemoteCripplerParticles = 35;
constexpr float kRemoteCripplerRate = 0.2f;
constexpr float kRemoteCripplerLife = 150.0f;
constexpr float kRemoteCripplerSpeedLow = -0.002f;
constexpr float kRemoteCripplerSpeedHigh = -0.003f;
constexpr float kRemoteCripplerSize = 0.07f;
constexpr int kRemoteArmParticles = 8;
constexpr float kRemoteArmRate = 0.02f;
constexpr float kRemoteArmLifeLow = 300.0f;
constexpr float kRemoteArmLifeHigh = 400.0f;
constexpr float kRemoteArmSpeed = 0.0001f;
constexpr float kRemoteArmSizeLow = 0.1f;
constexpr float kRemoteArmSizeHigh = 0.12f;

// The speed across the tracks beyond which the ship banks.
constexpr float kBankThreshold = 0.003f;

// The tick before which the ship does not read the speed of its player.
constexpr float kBankStartTick = -7300.0f;

// The tick the intro of the play field starts at.
constexpr float kIntroStartTick = -7680.0f;

// The blend across the cell of the tracks the ship rides at.
constexpr float kCellCentre = 0.5f;

// The growth of the vertical offset of a spread beyond 1.
constexpr float kSpreadLift = 0.25f;

// The frame rate of the arm animations of a firing arm, in frames per millisecond.
constexpr float kArmFrameRate = 0.06f;

// The share of a move across the tracks a knock takes.
constexpr float kBumpDurationScale = 0.75f;

// The frames of the knock and of the attack of a bumper.
constexpr float kBumpFrames = 1000.0f;

// The screen area an off-screen ship's arrow appears outside of, and the edge the arrow sits on.
constexpr float kScreenMin = -0.05f;
constexpr float kScreenMax = 1.05f;
constexpr float kArrowEdgeMin = 0.02f;
constexpr float kArrowEdgeMax = 0.98f;
constexpr float kArrowCentreX = 0.5f;
constexpr float kArrowCentreY = 0.25f;
constexpr float kArrowSlopeLeft = -0.48f;
constexpr float kArrowSlopeRight = 0.48f;
constexpr float kArrowSlopeTop = -0.23f;
constexpr float kArrowSlopeBottom = 0.73f;

// The pulse of an arrow that has just appeared.
constexpr float kArrowPulseTime = 250.0f;
constexpr float kArrowPulseRate = 0.004f;
constexpr float kArrowPulseGrowth = 1.5f;
constexpr float kArrowPulseBrighten = 2.0f;

// Transform a point by a transform.
void XfmPoint(const float (*pXfm)[Rnd::kXfmRowFloatCount], const Vector3 &point, float *pOut) {
    for (int i = 0; i < kNumComponents; ++i) {
        pOut[i] =
            (point.x * pXfm[0][i]) + (point.y * pXfm[1][i]) + (point.z * pXfm[2][i]) + pXfm[3][i];
    }
}

// Blend two points, pTo at a blend of 1.
void LerpPoint(const float *pTo, const float *pFrom, float fBlend, float *pOut) {
    for (int i = 0; i < kNumComponents; ++i) {
        pOut[i] = (pTo[i] * fBlend) + (pFrom[i] * (1.0f - fBlend));
    }
}

// Copy a transform into the local transform of an object and mark it for recomposing.
void SetLocalXfm(Rnd::Transformable *pTrans, const float (*pXfm)[Rnd::kXfmRowFloatCount]) {
    memcpy(pTrans->mLocalXfm, pXfm, sizeof(pTrans->mLocalXfm));
    pTrans->mDirty = 1;
}

// Give the basis of an object's local transform a uniform scale and no rotation.
void SetLocalScale(Rnd::Transformable *pTrans, float fScale) {
    for (int i = 0; i < kNumComponents; ++i) {
        for (int j = 0; j < kNumComponents; ++j) {
            pTrans->mLocalXfm[i][j] = i == j ? fScale : 0.0f;
        }
    }
    pTrans->mDirty = 1;
}

// Evaluate a curve of keys at a frame, clamped to its ends, or 1 for an empty curve.
float KeyValue(const std::vector<FloatKey> &keys, float fFrame) {
    if (keys.empty()) {
        return 1.0f;
    }
    if (fFrame <= keys.front().mFrame) {
        return keys.front().mValue;
    }
    if (keys.back().mFrame <= fFrame) {
        return keys.back().mValue;
    }
    int nLow = 0;
    int nHigh = static_cast<int>(keys.size()) - 1;
    while (nLow + 1 < nHigh) {
        const int nMiddle = (nLow + nHigh) / 2;
        if (fFrame == keys[nMiddle].mFrame) {
            return keys[nMiddle].mValue;
        }
        if (keys[nMiddle].mFrame < fFrame) {
            nLow = nMiddle;
        } else {
            nHigh = nMiddle;
        }
    }
    const FloatKey &from = keys[nLow];
    const FloatKey &to = keys[nHigh];
    const float fBlend = (fFrame - from.mFrame) / (to.mFrame - from.mFrame);
    return from.mValue + ((to.mValue - from.mValue) * fBlend);
}

// Find a scene object of a class by name.
template <class T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

// Copy a transform into the rows of a matrix.
void ToRows(const Transform &xfm, float (*pRows)[Rnd::kXfmRowFloatCount]) {
    const Vector3 *rows[] = {&xfm.mBasisX, &xfm.mBasisY, &xfm.mBasisZ, &xfm.mTranslation};
    for (int i = 0; i < Rnd::kXfmRowCount; ++i) {
        pRows[i][0] = rows[i]->x;
        pRows[i][1] = rows[i]->y;
        pRows[i][2] = rows[i]->z;
        pRows[i][kVec3PaddingFloat] = rows[i]->w;
    }
}

// The geometry of the tracks of the song.
TnlGeom *Geom() {
    return GfxTunnel::sCurrent->mGeom;
}

} // namespace

// The static initialiser of the unit, and the two routines that run it to construct and to destroy
// the statics of the ships.
// NTSC-U/C: 0x001cb3c0, PAL: 0x001d4160
// NTSC-U/C: 0x001cb5c8, PAL: 0x001d4368
// NTSC-U/C: 0x001cb5e8, PAL: 0x001d4388

std::vector<FloatKey> Ship::sFireTrodeSizeMult;
Vector3 Ship::sOffset{0.0f, 0.0f, 0.0f};
Vector3 Ship::sMultiOffsets[kNumMultiOffsets];
Vector3 Ship::sPivot{0.0f, 0.0f, 0.0f};
Vector3 Ship::sOffsetPivot{0.0f, 0.0f, 0.0f};
Vector3 Ship::sMultiOffsetPivots[kNumMultiOffsets];
Color Ship::sTrackLabelPitchColor{1.0f, 1.0f, 1.0f, 1.0f};
Vector3 Ship::sIntroPos{0.0f, 5.0f, -0.5f};
std::vector<FloatKey> Ship::sIntroTrodeSizeMult;
float Ship::sViewDistance;
float Ship::sFireOffset = 0.0f;
float Ship::sFireScale = 1.0f;
float Ship::sFireTrodeEnd = 0.0f;
float Ship::sTilt = 0.0f;
float Ship::sScale = 1.0f;
float Ship::sBankSpring = 0.1f;
float Ship::sBankDamper = 0.5f;
float Ship::sBankMagnitude = 100.0f;
float Ship::sBankForce = 1e-05f;
int Ship::sMoveMissBeams[kNumBeamEnds] = {1, 1};
int Ship::sMoveHitBeams[kNumBeamEnds] = {1, 1};
int Ship::sMoveHit2Beams[kNumBeamEnds] = {1, 1};
float Ship::sSputterForce = -5e-06f;
float Ship::sBumpHeightAccel = 0.0f;
float Ship::sBumpAttackTime = 480.0f;
float Ship::sBumpShipMoveSlowdown = 0.5f;
float Ship::sCrippledCamJiggleProbability = 0.1f;
float Ship::sCrippledCamJiggle = 0.1f;
float Ship::sIntroEndTick = -5760.0f;
float Ship::sIntroBlendEndTick = -6800.0f;
float Ship::sIntroTrodeEnd = -1920.0f;
BeamPool<false> *Ship::sMissBeams;
BeamPool<false> *Ship::sHitBeams;
BeamPool<false> *Ship::sHit2Beams;
Rnd::View *Ship::sBeamView;
Rnd::Environ *Ship::sEnviron;
Rnd::View *Ship::sLoadingView;
Rnd::TransAnim *Ship::sWinFlyOff;
Rnd::Animatable *Ship::sAllAnim;
Rnd::Animatable *Ship::sIntroAnim;
Rnd::Mesh *Ship::sArrowMesh;

#pragma mark - Configuration

void Ship::ConfigureCrippler(Rnd::ParticleSys *pSys, int nPlayer) {
    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity != GameDb::kCommunityLocal) {
        if (nCommunity != GameDb::kCommunityOnline || TheGameDb->IsLocalPlayer(nPlayer)) {
            return;
        }
    }
    pSys->mEmitRateHigh = kRemoteCripplerRate;
    pSys->mEmitRateLow = kRemoteCripplerRate;
    pSys->SetNumParticles(kRemoteCripplerParticles);
    const Vector3 zero{0.0f, 0.0f, 0.0f};
    pSys->mLife.y = kRemoteCripplerLife;
    pSys->mPosHigh = zero;
    pSys->mSpeed.x = kRemoteCripplerSpeedLow;
    pSys->mSpeed.y = kRemoteCripplerSpeedHigh;
    pSys->mSizeHigh = kRemoteCripplerSize;
    pSys->mPosLow = zero;
    pSys->mLife.x = kRemoteCripplerLife;
    pSys->mSizeLow = kRemoteCripplerSize;
}

void Ship::ConfigureArm(Rnd::ParticleSys *pSys, int nPlayer) {
    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity != GameDb::kCommunityLocal) {
        if (nCommunity != GameDb::kCommunityOnline || TheGameDb->IsLocalPlayer(nPlayer)) {
            return;
        }
    }
    pSys->mEmitRateHigh = kRemoteArmRate;
    pSys->mEmitRateLow = kRemoteArmRate;
    pSys->SetNumParticles(kRemoteArmParticles);
    const Vector3 zero{0.0f, 0.0f, 0.0f};
    pSys->mLife.x = kRemoteArmLifeLow;
    pSys->mPosHigh = zero;
    pSys->mLife.y = kRemoteArmLifeHigh;
    pSys->mSpeed.y = kRemoteArmSpeed;
    pSys->mSizeLow = kRemoteArmSizeLow;
    pSys->mSizeHigh = kRemoteArmSizeHigh;
    pSys->mPosLow = zero;
    pSys->mSpeed.x = kRemoteArmSpeed;
}

void Ship::LoadConfig(DataArray *pConfig, DataArray *pDefaults, bool bReload) {
    FindConfigVector3(pConfig, pDefaults, "ship_multi_offset_start", &sMultiOffsets[0], true);
    FindConfigVector3(pConfig, pDefaults, "ship_multi_offset_end", &sMultiOffsets[1], true);
    FindConfigVector3(pConfig, pDefaults, "ship_offset", &sOffset, true);
    FindConfigVector3(pConfig, pDefaults, "ship_pivot", &sPivot, true);
    const float fScale = TheGfxManager.mScrollSpeed;
    sOffsetPivot.x = (sOffset.x + sPivot.x) * fScale;
    sOffsetPivot.y = (sOffset.y + sPivot.y) * fScale;
    sOffsetPivot.z = (sOffset.z + sPivot.z) * fScale;
    sOffset.x *= fScale;
    sOffset.y *= fScale;
    sOffset.z *= fScale;
    for (int i = 0; i < kNumMultiOffsets; ++i) {
        Vector3 &offset = sMultiOffsets[i];
        sMultiOffsetPivots[i].x = (offset.x + sPivot.x) * fScale;
        sMultiOffsetPivots[i].y = (offset.y + sPivot.y) * fScale;
        sMultiOffsetPivots[i].z = (offset.z + sPivot.z) * fScale;
        offset.x *= fScale;
        offset.y *= fScale;
        offset.z *= fScale;
    }
    FindConfigFloat(pConfig, pDefaults, "ship_tilt", &sTilt, true);
    FindConfigFloat(pConfig, pDefaults, "ship_scale", &sScale, true);
    sScale *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "view_distance", &sViewDistance, true);
    sViewDistance *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "ship_bank_spring", &sBankSpring, true);
    FindConfigFloat(pConfig, pDefaults, "ship_bank_damper", &sBankDamper, true);
    FindConfigFloat(pConfig, pDefaults, "ship_bank_magnitude", &sBankMagnitude, true);
    FindConfigFloat(pConfig, pDefaults, "ship_bank_force", &sBankForce, true);
    sBankForce *= TheGfxManager.mScrollSpeed;
    FindConfigFloat(pConfig, pDefaults, "ship_fire_offset", &sFireOffset, true);
    FindConfigFloat(pConfig, pDefaults, "ship_fire_scale", &sFireScale, true);
    FindConfigFloat(pConfig, pDefaults, "ship_sputter_force", &sSputterForce, true);
    sSputterForce *= TheGfxManager.mScrollSpeed;

    DataArray *pArray;
    FindConfigArray(pConfig, pDefaults, "ship_move_miss_beams", &pArray, true);
    sMoveMissBeams[0] = pArray->Int(1) != 0;
    sMoveMissBeams[1] = pArray->Int(2) != 0;
    FindConfigArray(pConfig, pDefaults, "ship_move_hit_beams", &pArray, true);
    sMoveHitBeams[0] = pArray->Int(1) != 0;
    sMoveHitBeams[1] = pArray->Int(2) != 0;
    FindConfigArray(pConfig, pDefaults, "ship_move_hit2_beams", &pArray, true);
    sMoveHit2Beams[0] = pArray->Int(1) != 0;
    sMoveHit2Beams[1] = pArray->Int(2) != 0;

    FindConfigFloat(pConfig,
                    pDefaults,
                    "ship_crippled_cam_jiggle_probability",
                    &sCrippledCamJiggleProbability,
                    true);
    FindConfigFloat(pConfig, pDefaults, "ship_crippled_cam_jiggle", &sCrippledCamJiggle, true);
    FindConfigFloat(pConfig, pDefaults, "ship_bump_height_accel", &sBumpHeightAccel, true);
    FindConfigFloat(pConfig, pDefaults, "ship_bump_attack_time", &sBumpAttackTime, true);
    FindConfigFloat(
        pConfig, pDefaults, "ship_bump_ship_move_slowdown", &sBumpShipMoveSlowdown, true);
    FindConfigColor(
        pConfig, pDefaults, "ship_track_label_pitch_color", &sTrackLabelPitchColor, true);
    FindConfigFloat(pConfig, pDefaults, "ship_intro_end_tick", &sIntroEndTick, true);
    FindConfigFloat(pConfig, pDefaults, "ship_intro_blend_end_tick", &sIntroBlendEndTick, true);
    FindConfigVector3(pConfig, pDefaults, "ship_intro_pos", &sIntroPos, true);
    const float fPosScale = TheGfxManager.mScrollSpeed;
    sIntroPos.x *= fPosScale;
    sIntroPos.y *= fPosScale;
    sIntroPos.z *= fPosScale;
    FindConfigArray(pConfig, pDefaults, "ship_intro_trode_size_mult", &pArray, true);
    LoadFloatKeys(pArray, &sIntroTrodeSizeMult);
    sIntroTrodeEnd = sIntroTrodeSizeMult.back().mFrame;
    FindConfigArray(pConfig, pDefaults, "ship_fire_trode_size_mult", &pArray, true);
    LoadFloatKeys(pArray, &sFireTrodeSizeMult);
    const float fBeamScale = TheGfxManager.mScrollSpeed;
    sFireTrodeEnd = sFireTrodeSizeMult.back().mFrame;
    if (sMissBeams != nullptr) {
        sMissBeams->LoadConfig(pConfig, pDefaults, bReload, fBeamScale);
    }
    if (sHitBeams != nullptr) {
        sHitBeams->LoadConfig(pConfig, pDefaults, bReload, fBeamScale);
    }
    if (sHit2Beams != nullptr) {
        sHit2Beams->LoadConfig(pConfig, pDefaults, bReload, fBeamScale);
    }
    if (sBeamView == nullptr) {
        sBeamView =
            dynamic_cast<Rnd::View *>(Rnd::TheManager.Create("View", "ship beam draw.view"));
    }
}

#pragma mark - Construction

Ship::Ship(const char *pszColor, int nPlayer) {
    mView = nullptr;
    mStartMs = kUnset;
    mLeaderMat = nullptr;
    mPlayer = nPlayer;
    const Color *pColor = TheGfxManager.GetPlayerColor(nPlayer);
    mShown = 1;
    mColor = *pColor;
    mSpread = 0.0f;
    mHasOverride = 0;
    mIntroActive = 0;
    mHidden = 0;
    mFlags = 0;
    mBumpHeight = 0.0f;
    mBumpVel = 0.0f;
    mCripplerTime = 0.0f;
    mBumperTime = 0.0f;
    mBumpStart = kFarFuture;
    mArrowStart = kUnset;
    mAttackTime = kFarFuture;
    mMulti = TheGameDb->mCommunity != GameDb::kCommunitySolo;
    mBank = new Rnd::Animatable::SecondOrder(0.0f, sBankSpring, sBankDamper);
    mFlyIn = nullptr;

    const float fScrollSpeed = TheGfxManager.mScrollSpeed;
    const int nBeams = TheGameDb->GetNumPlayers() + kSpareBeams;
    Rnd::Drawable *pBeamParent = sBeamView;
    if (sMissBeams == nullptr) {
        sMissBeams = new BeamPool<false>(nBeams, pBeamParent, "ship_miss");
        sMissBeams->LoadConfig(GetModeGfxConfig(), GetGfxConfig(), false, fScrollSpeed);
    }
    if (sHitBeams == nullptr) {
        sHitBeams = new BeamPool<false>(nBeams, pBeamParent, "ship_hit");
        sHitBeams->LoadConfig(GetModeGfxConfig(), GetGfxConfig(), false, fScrollSpeed);
    }
    if (sHit2Beams == nullptr) {
        sHit2Beams = new BeamPool<false>(nBeams, pBeamParent, "ship_hit2");
        sHit2Beams->LoadConfig(GetModeGfxConfig(), GetGfxConfig(), false, fScrollSpeed);
    }

    const int nColor = mMulti ? pszColor[0] : kSoloColor;
    mView = FindObject<Rnd::View>(FormatString("ship_%c.view", nColor));
    mMesh = FindObject<Rnd::Mesh>(FormatString("ship_%c.mesh", nColor));
    mPsAnim = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Create("View", FormatString("ship ps anim %c.view", nColor)));
    mBackPsAnim = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Create("View", FormatString("ship backps anim %c.view", nColor)));
    mFxView = FindObject<Rnd::View>(FormatString("ship fx_%c.view", nColor));
    mMat = FindObject<Rnd::Mat>(FormatString("ship_%c.mat", nColor));
    if (mMulti != 0) {
        mLeaderMat = FindObject<Rnd::Mat>(FormatString("ship leader_%c.mat", nColor));
    }
    mBumperAttack = FindObject<Rnd::View>("bumper attack.view");
    mBumperFly = FindObject<Rnd::TransAnim>("bumper fly.tnm");
    mScratchRibbon = FindObject<Rnd::String>(FormatString("scratch ribbon_%c.line", nColor));
    mIntroFlyIn = FindObject<Rnd::TransAnim>(FormatString("ship intro fly-in_%c.tnm", nColor));
    mIntro = FindObject<Rnd::TransAnim>(FormatString("ship intro_%c.tnm", nColor));
    mBumperBlur = FindObject<Rnd::Blur>(FormatString("ship bumper_%c.blur", nColor));

    Rnd::Drawable *pViewDraw = mView;
    if (mBumperBlur != nullptr) {
        pViewDraw->RemoveDraw(mBumperBlur);
    }
    pViewDraw->RemoveDraw(mMesh);
    static_cast<Rnd::Transformable *>(mFxView)->RemoveTrans(mScratchRibbon);
    const float aflIdentity[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    SetLocalXfm(mFxView, aflIdentity);
    static_cast<Rnd::Drawable *>(mScratchRibbon)->SetShowing(0);

    for (int i = 0; i < kNumArms; ++i) {
        const char *pszArm = kArmNames[i];
        mArmAnims[i] =
            FindObject<Rnd::Animatable>(FormatString("ship arm %s_%c.tnm", pszArm, nColor));
        mArmXfms[i] = FindObject<Rnd::Transformable>(FormatString("ship %s_%c", pszArm, nColor));
        Rnd::ParticleSys *pSys =
            FindObject<Rnd::ParticleSys>(FormatString("ship %s_%c.ps", pszArm, nColor));
        ConfigureArm(pSys, mPlayer);
        mArms[i].Init(pSys, mArmXfms[i], sScale);
        pViewDraw->RemoveDraw(mArms[i].mSys);
        mTrodes[i] =
            FindObject<Rnd::Mesh>(FormatString("ship trode core %s_%c.mesh", pszArm, nColor));
        if (mTrodes[i] != nullptr) {
            mTrodeMats[i] = mTrodes[i]->mMat;
            pViewDraw->RemoveDraw(mTrodes[i]);
        } else {
            mTrodeMats[i] = nullptr;
        }
        Rnd::ParticleSys *pArmSys = mArms[i].mSys;
        mArmColors[i][kArmColorStartLow] = pArmSys->mStartColorLow;
        mArmColors[i][kArmColorStartHigh] = pArmSys->mStartColorHigh;
        mArmColors[i][kArmColorEndLow] = pArmSys->mEndColorLow;
        mArmColors[i][kArmColorEndHigh] = pArmSys->mEndColorHigh;
    }
    mArmSizeLow = mArms[0].mSys->mSizeLow;
    mArmSizeHigh = mArms[0].mSys->mSizeHigh;
    if (mTrodes[0] != nullptr) {
        float aflScale[kNumComponents];
        Mat33Scale(&static_cast<Rnd::Transformable *>(mTrodes[0])->mLocalXfm[0][0], aflScale);
        mTrodeSize = aflScale[0];
    } else {
        mTrodeSize = 1.0f;
    }
    SetEnergy(1.0f, kZoneHigh, 1.0f);

    Rnd::ParticleSys *pThrusterSys =
        FindObject<Rnd::ParticleSys>(FormatString("ship ass_%c.part", nColor));
    Rnd::Transformable *pThrusterXfm =
        FindObject<Rnd::Transformable>(FormatString("ship_%c.mesh", nColor));
    ConfigureCrippler(pThrusterSys, mPlayer);
    mThruster.Init(pThrusterSys, pThrusterXfm, sScale);
    pViewDraw->RemoveDraw(mThruster.mSys);
    const float fSpeedOffset = TheGfxManager.mCamPathScale / fScrollSpeed;
    mCripplerPs = FindObject<Rnd::ParticleSys>(FormatString("ship crippler_%c.part", nColor));
    if (mCripplerPs != nullptr) {
        mCripplerPs->SetFrameSelf(mCripplerTime);
        static_cast<Rnd::Drawable *>(mCripplerPs)->SetShowing(1);
        mCripplerPs->FreeAllParticles();
        mView->RemoveAnim(mCripplerPs);
        mThruster.Scale(mCripplerPs, sScale);
    }
    mThruster2 = FindObject<Rnd::ParticleSys>(FormatString("ship ass_%c2.part", nColor));
    if (mThruster2 != nullptr) {
        mThruster.Scale(mThruster2, sScale);
    }
    mThruster3 = FindObject<Rnd::ParticleSys>(FormatString("ship ass_%c3.part", nColor));
    if (mThruster3 != nullptr) {
        mThruster.Scale(mThruster3, sScale);
    }
    mBumperPs = FindObject<Rnd::ParticleSys>(FormatString("ship bumper_%c.ps", nColor));
    if (mBumperPs != nullptr) {
        mBumperPs->SetFrameSelf(mBumperTime);
        static_cast<Rnd::Drawable *>(mBumperPs)->SetShowing(1);
        mBumperPs->FreeAllParticles();
        ScaleParticles(mBumperPs, sScale);
        mBumperPs->mSpeed.x += fSpeedOffset;
        mBumperPs->mSpeed.y += fSpeedOffset;
        mView->RemoveAnim(mBumperPs);
    }
    ClearMeshScroll(mView);

    mName = FindObject<Rnd::Text>(FormatString("ship name_%c.txt", nColor));
    if (mName != nullptr) {
        Rnd::Transformable *pNameXfm = mName;
        // Yes, the binary flattens the basis of the name to nothing.
        for (int i = 0; i < kNumComponents; ++i) {
            for (int j = 0; j < kNumComponents; ++j) {
                pNameXfm->mLocalXfm[i][j] = 0.0f;
            }
        }
        pNameXfm->mDirty = 1;
        mName->SetText(TheGameDb->GetPlayerName(mPlayer));
    }

    if (mPlayer == 0) {
        sAllAnim = FindObject<Rnd::Animatable>("ship all.anim");
        sIntroAnim = FindObject<Rnd::Animatable>("ship intro.anim");
        sEnviron = FindObject<Rnd::Environ>("ship.env");
        sLoadingView = FindObject<Rnd::View>("ship loading.view");
        if (mMulti != 0) {
            sArrowMesh = FindObject<Rnd::Mesh>("ship arrow.mesh");
        }
        sWinFlyOff = FindObject<Rnd::TransAnim>("ship win fly-off.tnm");
        GfxTunnel::ScaleLights(sEnviron);
    }
    static_cast<Rnd::Transformable *>(mView)->SetOrigin(&sPivot.x);

    // Move the particle systems the view animates to the two views that animate them on the
    // system clock, the thrusters behind the ship and the arms in front of it.
    bool bMoved;
    do {
        bMoved = false;
        Rnd::Animatable *pViewAnim = mView;
        for (Rnd::Animatable *pAnim : pViewAnim->mAnims) {
            Rnd::ParticleSys *pSys = dynamic_cast<Rnd::ParticleSys *>(pAnim);
            if (pSys == nullptr) {
                continue;
            }
            pViewAnim->RemoveAnim(pAnim);
            bMoved = true;
            if (pSys == mThruster.mSys || pSys == mThruster2 || pSys == mThruster3) {
                mBackPsAnim->AddAnim(pAnim);
            } else {
                mPsAnim->AddAnim(pAnim);
            }
            break;
        }
    } while (bMoved);
    Reset();
}

Ship::~Ship() {
    delete mFlyIn;
    delete sBeamView;
    sBeamView = nullptr;
    delete sMissBeams;
    sMissBeams = nullptr;
    delete sHitBeams;
    sHitBeams = nullptr;
    delete sHit2Beams;
    sHit2Beams = nullptr;
    // The spring declares no destructor, and the binary releases it with a bare delete.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
    delete mBank;
#pragma GCC diagnostic pop
    delete mBackPsAnim;
    mBackPsAnim = nullptr;
    delete mPsAnim;
    mPsAnim = nullptr;
    sAllAnim = nullptr;
    sIntroAnim = nullptr;
    sEnviron = nullptr;
    sLoadingView = nullptr;
    sWinFlyOff = nullptr;
    sArrowMesh = nullptr;
}

#pragma mark - State

void Ship::SetMultiplier(int nMultiplier) {
    if (mLeaderMat == nullptr) {
        return;
    }
    mMesh->SetMat(nMultiplier < 2 ? mMat : mLeaderMat);
}

void Ship::Reset() {
    mSpread = 0.0f;
    ScaleArms(mSpread);
    for (int i = kNumArms - 1; i >= 0; --i) {
        mFireTimes[i] = kUnset;
    }
    if (TheGameDb->mTutorial != 0) {
        delete mFlyIn;
        // The constructor of the flight hides the ship through SetShown(), which reads mFlyIn.
        mFlyIn = nullptr;
        mFlyIn = new ShipFlyIn(this);
    } else {
        SetShown(true);
    }
    ClearBeams();
    mArrowStart = kUnset;
    SetMultiplier(1);
    mBumpVel = 0.0f;
    mBumpHeight = 0.0f;
    if (GfxTunnel::sCurrent != nullptr) {
        Geom()->GetPlayer(mPlayer)->mMoveTime = 1.0f;
    }
    mFlags &= ~kFlagBumped;
    const float aflIdentity[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    SetLocalXfm(mFxView, aflIdentity);
}

void Ship::SetEnergy(float fEnergy, int nZone, [[maybe_unused]] float fZoneFraction) {
    Vector3 force{0.0f, 0.0f, 0.0f};
    Rnd::Mat *pTrodeMat = nullptr;
    const Color *pColors;
    Color sputter[kNumArmColors];
    if (fEnergy <= 0.0f) {
        sputter[kArmColorStartLow] = kSputterStartLow;
        sputter[kArmColorStartHigh] = kSputterStartHigh;
        sputter[kArmColorEndLow] = kSputterEndLow;
        sputter[kArmColorEndHigh] = kSputterEndHigh;
        force.y = sSputterForce;
        pColors = sputter;
    } else if (nZone == kZoneHigh) {
        pColors = mArmColors[kArmOfHighZone];
        pTrodeMat = mTrodeMats[kArmOfHighZone];
    } else if (nZone == kZoneMiddle) {
        pColors = mArmColors[kArmOfMiddleZone];
        pTrodeMat = mTrodeMats[kArmOfMiddleZone];
    } else {
        pColors = mArmColors[kArmOfLowZone];
        pTrodeMat = mTrodeMats[kArmOfLowZone];
    }
    for (ParticleArm &arm : mArms) {
        arm.mSys->mStartColorLow = pColors[kArmColorStartLow];
        arm.mSys->mStartColorHigh = pColors[kArmColorStartHigh];
    }
    for (ParticleArm &arm : mArms) {
        arm.mSys->mEndColorLow = pColors[kArmColorEndLow];
        arm.mSys->mEndColorHigh = pColors[kArmColorEndHigh];
    }
    for (ParticleArm &arm : mArms) {
        arm.mSys->mForce = force;
    }
    if (mTrodes[0] == nullptr) {
        return;
    }
    for (Rnd::Mesh *pTrode : mTrodes) {
        if (pTrodeMat != nullptr) {
            pTrode->SetMat(pTrodeMat);
            static_cast<Rnd::Drawable *>(pTrode)->SetShowing(1);
        } else {
            static_cast<Rnd::Drawable *>(pTrode)->SetShowing(0);
        }
    }
}

void Ship::ScaleArms(float fScale) {
    const float fSizeLow = fScale * mArmSizeLow;
    const float fSizeHigh = fScale * mArmSizeHigh;
    for (ParticleArm &arm : mArms) {
        arm.mSys->mSizeLow = fSizeLow;
        arm.mSys->mSizeHigh = fSizeHigh;
    }
    if (mTrodes[0] == nullptr) {
        return;
    }
    for (Rnd::Mesh *pTrode : mTrodes) {
        SetLocalScale(pTrode, fScale * mTrodeSize);
    }
}

void Ship::ScaleArm(int nArm, float fScale) {
    mArms[nArm].mSys->mSizeLow = fScale * mArmSizeLow;
    mArms[nArm].mSys->mSizeHigh = fScale * mArmSizeHigh;
    if (mTrodes[0] == nullptr) {
        return;
    }
    SetLocalScale(mTrodes[nArm], fScale * mTrodeSize);
}

void Ship::SetTrack([[maybe_unused]] int nTrack,
                    [[maybe_unused]] int nTrackType,
                    [[maybe_unused]] int nInstrument) {
    ClearBeams();
}

void Ship::SetShown(bool bShown) {
    if (mFlyIn != nullptr) {
        bShown = mFlyIn->Start(bShown);
    }
    mShown = bShown;
    static_cast<Rnd::Drawable *>(mView)->SetShowing(bShown && mHidden == 0);
}

void Ship::SetHidden(bool bHidden) {
    mHidden = bHidden;
    static_cast<Rnd::Drawable *>(mView)->SetShowing(mShown != 0 && !bHidden);
    ClearBeams();
}

void Ship::SetOverride(const float (*pXfm)[Rnd::kXfmRowFloatCount], float fBlend) {
    if (pXfm == nullptr) {
        mHasOverride = 0;
        return;
    }
    memcpy(mXfm, pXfm, sizeof(mXfm));
    mOverrideBlend = fBlend;
    mHasOverride = 1;
    mIntroActive = 0;
}

void Ship::StartBump(bool bVictim, int nTrack) {
    TnlGeom *pGeom = Geom();
    TnlGeom::Player *pPlayer = pGeom->GetPlayer(mPlayer);
    const float fTime = TheGameDb->mSongTime;
    if (!bVictim) {
        mFlags |= kFlagAttacking;
        mAttackTime = fTime;
        mAttackTrack = pPlayer->mTrack;
        return;
    }
    const float fTick = TheGameDb->mSongTick;
    const float fSlowdown = sBumpShipMoveSlowdown;
    pPlayer->mMoveTime = fSlowdown;
    const float fMoveLength = pGeom->MoveLength(pPlayer->mPosition, static_cast<float>(nTrack));
    const float fTicks = fSlowdown * kBumpDurationScale * fMoveLength;
    mBumpStart = fTime;
    mFlags |= kFlagBumped;
    mBumpHeight = 0.0f;
    const float fEnd = (fTick + fTicks) * *TheGfxManager.mMsPerTick;
    mBumpEnd = fEnd;
    mBumpVel = sBumpHeightAccel * -0.5f * (fEnd - fTime);
    if (nTrack < pPlayer->mTrack) {
        mFlags |= kFlagBumpReverse;
    } else {
        mFlags &= ~kFlagBumpReverse;
    }
    if (mBumperBlur != nullptr) {
        mBumperBlur->mXfms.clear();
    }
}

#pragma mark - Beams

void Ship::AddBeam(std::list<BeamData> *pList,
                   BeamPool<false> *pPool,
                   signed char nSlot,
                   signed char nTrack,
                   float fTick,
                   float fStart,
                   float fEnd) {
    Beam *pBeam = pPool->Start(this, pList, fStart, fEnd);
    if (pBeam != nullptr) {
        pList->push_back(BeamData{pBeam, nSlot, nTrack});
    } else {
        // Steal the beam that started first from whichever ship holds it.
        pBeam = pPool->FindOldest();
        std::list<BeamData> *pOwnerList = pBeam->mOwnerList;
        std::list<BeamData>::iterator it = pOwnerList->begin();
        while (it != pOwnerList->end() && it->mBeam != pBeam) {
            ++it;
        }
        pList->splice(pList->end(), *pOwnerList, it);
        pList->back().mSlot = nSlot;
        pList->back().mTrack = nTrack;
        pBeam = pList->back().mBeam;
        pBeam->mOwner = this;
        pBeam->mOwnerList = pList;
    }
    pBeam->SetEnd(0, mArmXfms[nSlot]->mWorldXfm[Rnd::kXfmRowCount - 1]);
    float aflCell[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    const float fOffset = (static_cast<float>(nSlot) * kSlotSpacing) + kSlotOffset;
    Transform cell;
    Geom()->CellXfm(nTrack, &cell, false, true, fTick, fOffset);
    ToRows(cell, aflCell);
    pBeam->SetEnd(1, aflCell[Rnd::kXfmRowCount - 1]);
}

void Ship::ClearBeams() {
    for (BeamData &data : mMissBeams) {
        sMissBeams->Free(data.mBeam);
    }
    mMissBeams.clear();
    for (BeamData &data : mHitBeams) {
        sHitBeams->Free(data.mBeam);
    }
    mHitBeams.clear();
    for (BeamData &data : mHit2Beams) {
        sHit2Beams->Free(data.mBeam);
    }
    mHit2Beams.clear();
}

void Ship::Fire(int nSlot, signed char nTrack, bool bHit) {
    const float fTime = TheGameDb->mSongTime;
    const float fTick = TheGameDb->mSongTick;
    mFireTimes[nSlot] = fTime;
    const signed char nBeamSlot = static_cast<signed char>(nSlot);
    const float fEnd = fTime + kBeamTime;
    if (bHit) {
        AddBeam(&mHitBeams, sHitBeams, nBeamSlot, nTrack, fTick, fTime, fEnd);
        AddBeam(&mHit2Beams, sHit2Beams, nBeamSlot, nTrack, fTick, fTime, fEnd);
    } else {
        AddBeam(&mMissBeams, sMissBeams, nBeamSlot, nTrack, fTick, fTime, fEnd);
    }
}

void Ship::UpdateBeams(std::list<BeamData> *pList,
                       BeamPool<false> *pPool,
                       const float (*pXfm)[Rnd::kXfmRowFloatCount],
                       const Color *pColor,
                       bool bMoveStart,
                       bool bMoveEnd) {
    const float fTick = TheGameDb->mSongTick;
    int nCellSlot = -1;
    float aflCell[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    std::list<BeamData>::iterator it = pList->begin();
    while (it != pList->end()) {
        Beam *pBeam = it->mBeam;
        if (bMoveStart) {
            pBeam->SetEnd(0, mArmXfms[it->mSlot]->mWorldXfm[Rnd::kXfmRowCount - 1]);
        }
        if (bMoveEnd) {
            // Yes, the binary rebuilds the cell only when the button changes, not the track.
            if (it->mSlot != nCellSlot) {
                nCellSlot = it->mSlot;
                const float fOffset = (static_cast<float>(nCellSlot) * kSlotSpacing) + kSlotOffset;
                Transform cell;
                Geom()->CellXfm(it->mTrack, &cell, false, true, fTick, fOffset);
                ToRows(cell, aflCell);
            }
            pBeam->SetEnd(1, aflCell[Rnd::kXfmRowCount - 1]);
        }
        Vector3 direction{0.0f, 0.0f, 0.0f};
        if (it->mSlot == kSlotLeft) {
            direction.x = pXfm[2][0] * -kBeamSpread;
            direction.y = pXfm[2][1] * -kBeamSpread;
            direction.z = pXfm[2][2] * -kBeamSpread;
        } else if (it->mSlot == kSlotRight) {
            direction.x = pXfm[2][0] * kBeamSpread;
            direction.y = pXfm[2][1] * kBeamSpread;
            direction.z = pXfm[2][2] * kBeamSpread;
        }
        direction.x += pXfm[0][0];
        direction.y += pXfm[0][1];
        direction.z += pXfm[0][2];
        Vec3Normalize(&direction.x, &direction.x);
        Vector3 side;
        CrossVec3(&direction.x, pXfm[1], &side.x);
        Vec3Normalize(&side.x, &side.x);
        if (pBeam->Update(direction, side, *pColor)) {
            pPool->Free(pBeam);
            it = pList->erase(it);
        } else {
            ++it;
        }
    }
}

#pragma mark - Polling

void Ship::PollBump(float fTimeDelta) {
    if (mBumperPs != nullptr) {
        mBumperPs->SetFrame(TheGameDb->mSongTick);
    }
    if ((mFlags & kFlagBumped) == 0) {
        return;
    }
    float fFrame = 0.0f;
    const float fTime = TheGameDb->mSongTime;
    if (mBumpEnd <= fTime) {
        mBumpVel = 0.0f;
        mBumpHeight = 0.0f;
        Geom()->GetPlayer(mPlayer)->mMoveTime = 1.0f;
        mFlags &= ~kFlagBumped;
    } else {
        const float fDuration = mBumpEnd - mBumpStart;
        mBumpHeight += fTimeDelta * mBumpVel;
        mBumpVel += fTimeDelta * sBumpHeightAccel;
        const float fProgress = fDuration == 0.0f ? 1.0f : (fTime - mBumpStart) / fDuration;
        fFrame = fProgress * kBumpFrames;
        if ((mFlags & kFlagBumpReverse) != 0) {
            fFrame = kBumpFrames - fFrame;
        }
    }
    Rnd::Transformable *pFxXfm = mFxView;
    float *pTranslation = pFxXfm->mLocalXfm[Rnd::kXfmRowCount - 1];
    pTranslation[0] = 0.0f;
    pTranslation[1] = 0.0f;
    pTranslation[2] = mBumpHeight;
    pFxXfm->mDirty = 1;
    mBumperFly->SetTrans(mFxView);
    mBumperFly->SetFrame(fFrame);
    mBumperFly->SetTrans(nullptr);
    if (mBumperPs == nullptr) {
        return;
    }
    if (TheGameDb->mCommunity == GameDb::kCommunityLocal || !TheGameDb->IsLocalPlayer(mPlayer)) {
        mBumperTime += fTimeDelta;
        mBumperPs->SetFrameSelf(mBumperTime);
    }
}

void Ship::PollCrippler(float fTimeDelta, bool bCrippled) {
    if (mCripplerPs != nullptr) {
        mCripplerPs->SetFrame(TheGameDb->mSongTick);
    }
    PlayerCamFX *pCamFX = GfxTunnel::sCurrent->mCamFX;
    if (!bCrippled) {
        return;
    }
    if (TheGameDb->IsLocalPlayer(mPlayer) && TheGameDb->mCommunity != GameDb::kCommunityLocal &&
        fTimeDelta != 0.0f) {
        // Yes, the binary shakes the camera of a crippled player here and leaves its particles be.
        if (RandomFraction() <= sCrippledCamJiggleProbability) {
            pCamFX->Kick(sCrippledCamJiggle);
        }
        return;
    }
    if (mCripplerPs != nullptr) {
        mCripplerTime += fTimeDelta;
        mCripplerPs->SetFrameSelf(mCripplerTime);
    }
}

void Ship::Poll([[maybe_unused]] float fTicks, float fTimeDelta) {
    TnlGeom::Player *pPlayer = Geom()->GetPlayer(mPlayer);
    const float fTick = TheGameDb->mSongTick;
    const float fTime = TheGameDb->mSongTime;
    const float fVelocity = fTick < kBankStartTick ? 0.0f : pPlayer->mVelocity;
    PlayerCamFX *pCamFX = GfxTunnel::sCurrent->mCamFX;
    bool bShowThrusters = mMulti != 0 || pCamFX->mZoomOut != 0.0f;

    float fBankTarget = 0.0f;
    if ((mFlags & kFlagBumped) == 0) {
        if (fVelocity <= -kBankThreshold) {
            fBankTarget = -sBankMagnitude;
        } else if (kBankThreshold <= fVelocity) {
            fBankTarget = sBankMagnitude;
        }
    }
    const float fBank = mBank->Apply(fBankTarget);
    const Vector3 force{sBankForce * fBank, 0.0f, 0.0f};
    mThruster.mSys->mForce = force;
    if (mThruster2 != nullptr) {
        mThruster2->mForce = force;
        mThruster3->mForce = force;
    }

    float aflBasis[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    float *pPosition = aflBasis[Rnd::kXfmRowCount - 1];
    if (mMulti != 0) {
        const float fSpread = mSpread < 1.0f ? mSpread : 1.0f;
        Vector3 offset;
        if (fSpread == 0.0f) {
            offset = sMultiOffsetPivots[0];
        } else if (fSpread == 1.0f) {
            offset = sMultiOffsetPivots[1];
        } else {
            LerpPoint(&sMultiOffsetPivots[1].x, &sMultiOffsetPivots[0].x, fSpread, &offset.x);
        }
        if (TheGameDb->mCommunity != GameDb::kCommunityLocal && 1.0f < mSpread) {
            offset.y *= ((mSpread - 1.0f) * kSpreadLift) + 1.0f;
        }
        float aflCell[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
        Transform cell;
        Geom()->BlendCell(&cell, true, false, pPlayer->mPosition, fTick, kCellCentre);
        ToRows(cell, aflCell);
        XfmPoint(aflCell, offset, pPosition);
        float aflUp[Rnd::kXfmRowFloatCount] = {};
        float aflForward[Rnd::kXfmRowFloatCount] = {};
        for (int i = 0; i < kNumComponents; ++i) {
            aflUp[i] = aflCell[1][i] + (aflCell[2][i] * sTilt);
            aflForward[i] = aflCell[2][i] + (aflCell[0][i] * fBank);
        }
        Mat33BuildOrthonormal(aflUp, aflForward, &aflBasis[0][0]);
    } else {
        const float (*pCamXfm)[Rnd::kXfmRowFloatCount] = pCamFX->mXfm;
        XfmPoint(pCamXfm, sOffsetPivot, pPosition);
        float aflUp[Rnd::kXfmRowFloatCount] = {};
        float aflForward[Rnd::kXfmRowFloatCount] = {};
        for (int i = 0; i < kNumComponents; ++i) {
            const float fEye = pCamXfm[3][i] + (pCamXfm[1][i] * sViewDistance);
            aflUp[i] = fEye - pPosition[i];
            aflForward[i] = pCamXfm[2][i] + (pCamXfm[0][i] * fBank);
        }
        Mat33BuildOrthonormal(aflUp, aflForward, &aflBasis[0][0]);
    }

    if (mHasOverride != 0) {
        const float fBlend = mOverrideBlend;
        if (fBlend == 1.0f) {
            memcpy(pPosition, mXfm[Rnd::kXfmRowCount - 1], sizeof(mXfm[0]));
        } else if (fBlend != 0.0f) {
            LerpPoint(mXfm[Rnd::kXfmRowCount - 1], pPosition, fBlend, pPosition);
        }
        InterpBasis(&aflBasis[0][0], &mXfm[0][0], &aflBasis[0][0], fBlend);
    } else if (mIntroActive != 0) {
        bShowThrusters = true;
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            PollSoloIntro(fTick, pCamFX, aflBasis);
        } else {
            PollIntro(fTick, pCamFX, aflBasis);
        }
    }

    if (mFlyIn != nullptr && mFlyIn->Poll(fTick, fTime, aflBasis)) {
        delete mFlyIn;
        mFlyIn = nullptr;
    }
    PollBump(fTimeDelta);
    PollCrippler(fTimeDelta, ((GfxTunnel::sCurrent->mCrippledTracks >> pPlayer->mTrack) & 1) != 0);

    for (int i = 0; i < kNumComponents; ++i) {
        for (int j = 0; j < kNumComponents; ++j) {
            aflBasis[i][j] *= sScale;
        }
    }
    SetLocalXfm(mView, aflBasis);

    const bool bIntroTrodes = fTick < sIntroTrodeEnd;
    if (bIntroTrodes) {
        ScaleArms(KeyValue(sIntroTrodeSizeMult, fTick));
    }
    for (int i = 0; i < kNumArms; ++i) {
        if (mFireTimes[i] == kUnset) {
            continue;
        }
        const float fSinceFire = fTime - mFireTimes[i];
        if (mArmAnims[i] != nullptr) {
            mArmAnims[i]->SetFrame((sFireScale * fSinceFire * kArmFrameRate) + sFireOffset);
        }
        if (!bIntroTrodes) {
            ScaleArm(i, KeyValue(sFireTrodeSizeMult, fSinceFire));
        }
        if (sFireTrodeEnd <= fSinceFire) {
            mFireTimes[i] = kUnset;
        }
    }
    mView->SetFrame(fTime);

    const float fNow = SystemMs();
    const int nShowThrusters = bShowThrusters;
    if (bShowThrusters) {
        mBackPsAnim->SetFrame(fNow);
    }
    static_cast<Rnd::Drawable *>(mThruster.mSys)->SetShowing(nShowThrusters);
    if (mMulti == 0) {
        static_cast<Rnd::Drawable *>(mThruster2)->SetShowing(nShowThrusters);
        static_cast<Rnd::Drawable *>(mThruster3)->SetShowing(nShowThrusters);
    }
    mPsAnim->SetFrame(fNow);
    static_cast<Rnd::Transformable *>(mView)->UpdateWorldXfm(nullptr, 0);

    UpdateBeams(
        &mMissBeams, sMissBeams, aflBasis, &mColor, sMoveMissBeams[0] != 0, sMoveMissBeams[1] != 0);
    UpdateBeams(
        &mHitBeams, sHitBeams, aflBasis, &mColor, sMoveHitBeams[0] != 0, sMoveHitBeams[1] != 0);
    UpdateBeams(
        &mHit2Beams, sHit2Beams, aflBasis, &mColor, sMoveHit2Beams[0] != 0, sMoveHit2Beams[1] != 0);
    if (mPlayer == 0) {
        sAllAnim->SetFrame(fTick);
        if (fTick < 0.0f) {
            sIntroAnim->SetFrame(fTick);
        }
    }
}

void Ship::PollSoloIntro(float fTick, PlayerCamFX *pCamFX, float (*pXfm)[Rnd::kXfmRowFloatCount]) {
    float *pPosition = pXfm[Rnd::kXfmRowCount - 1];
    const float (*pSlideXfm)[Rnd::kXfmRowFloatCount] = pCamFX->mSlideXfm;
    Rnd::Transformable *pViewXfm = mView;
    if (sIntroBlendEndTick <= fTick) {
        float fBlend = (sIntroEndTick - fTick) / (sIntroEndTick - sIntroBlendEndTick);
        fBlend = fBlend > 1.0f ? 1.0f : (fBlend < 0.0f ? 0.0f : fBlend);
        pViewXfm->SetOrigin(&sPivot.x);
        if (fBlend <= 0.0f) {
            mIntroActive = 0;
            return;
        }
        float aflIntroPos[Rnd::kXfmRowFloatCount];
        XfmPoint(pSlideXfm, sIntroPos, aflIntroPos);
        if (fBlend == 1.0f) {
            memcpy(pPosition, aflIntroPos, sizeof(aflIntroPos));
        } else {
            LerpPoint(aflIntroPos, pPosition, fBlend, pPosition);
        }
        return;
    }
    float fBlend = (sIntroBlendEndTick - fTick) / (sIntroBlendEndTick - kIntroStartTick);
    fBlend = fBlend > 1.0f ? 1.0f : (fBlend < 0.0f ? 0.0f : fBlend);
    float aflIntroXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    sceVu0MulAffineMatrixXyz(&aflIntroXfm[0][0], &pSlideXfm[0][0], &mXfm[0][0]);
    float aflIntroPos[Rnd::kXfmRowFloatCount];
    XfmPoint(pSlideXfm, sIntroPos, aflIntroPos);
    float *pIntroTranslation = aflIntroXfm[Rnd::kXfmRowCount - 1];
    if (fBlend == 0.0f) {
        memcpy(pPosition, aflIntroPos, sizeof(aflIntroPos));
    } else if (fBlend == 1.0f) {
        memcpy(pPosition, pIntroTranslation, sizeof(aflIntroPos));
    } else {
        LerpPoint(pIntroTranslation, aflIntroPos, fBlend, pPosition);
    }
    InterpBasis(&pXfm[0][0], &aflIntroXfm[0][0], &pXfm[0][0], fBlend);
    float aflOrigin[Rnd::kXfmRowFloatCount];
    for (int i = 0; i < kNumComponents; ++i) {
        aflOrigin[i] = (&sPivot.x)[i] * (1.0f - fBlend);
    }
    pViewXfm->SetOrigin(aflOrigin);
}

void Ship::PollIntro(float fTick, PlayerCamFX *pCamFX, float (*pXfm)[Rnd::kXfmRowFloatCount]) {
    float *pPosition = pXfm[Rnd::kXfmRowCount - 1];
    Rnd::Transformable *pViewXfm = mView;
    // Yes, the binary leaves this blend unclamped above 1.
    const float fBlend = (sIntroBlendEndTick - fTick) / (sIntroBlendEndTick - kIntroStartTick);
    if (fBlend <= 0.0f) {
        mIntroActive = 0;
        pViewXfm->SetOrigin(&sPivot.x);
        return;
    }
    float aflIntroXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    sceVu0MulAffineMatrixXyz(&aflIntroXfm[0][0], &pCamFX->mSlideXfm[0][0], &mXfm[0][0]);
    float *pIntroTranslation = aflIntroXfm[Rnd::kXfmRowCount - 1];
    if (fBlend == 1.0f) {
        memcpy(pPosition, pIntroTranslation, sizeof(aflIntroXfm[0]));
    } else {
        LerpPoint(pIntroTranslation, pPosition, fBlend, pPosition);
    }
    InterpBasis(&pXfm[0][0], &aflIntroXfm[0][0], &pXfm[0][0], fBlend);
    float aflOrigin[Rnd::kXfmRowFloatCount];
    for (int i = 0; i < kNumComponents; ++i) {
        aflOrigin[i] = (&sPivot.x)[i] * (1.0f - fBlend);
    }
    pViewXfm->SetOrigin(aflOrigin);
}

void Ship::UpdateTransform() {
    const float fNow = SystemMs();
    if (mStartMs == kUnset) {
        mStartMs = fNow;
    }
    const float fElapsed = fNow - mStartMs;
    mIntroActive = 1;
    mIntroFlyIn->EvalFrame(mIntroFlyIn->FilterFrame(fElapsed), &mXfm[0][0], 1);
    float aflIntro[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    mIntro->EvalFrame(mIntro->FilterFrame(fElapsed), &aflIntro[0][0], 1);
    sceVu0MulAffineMatrixXyz(&mXfm[0][0], &mXfm[0][0], &aflIntro[0][0]);

    float aflLocal[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    for (int i = 0; i < kNumComponents; ++i) {
        for (int j = 0; j < kNumComponents; ++j) {
            aflLocal[i][j] = mXfm[i][j] * sScale;
        }
        aflLocal[i][kNumComponents] = mXfm[i][kNumComponents];
    }
    // Yes, the binary scales the translation of mXfm in place, unlike its basis.
    float *pTranslation = mXfm[Rnd::kXfmRowCount - 1];
    for (int j = 0; j < kNumComponents; ++j) {
        pTranslation[j] *= sScale;
    }
    memcpy(aflLocal[Rnd::kXfmRowCount - 1], pTranslation, sizeof(aflLocal[0]));
    SetLocalXfm(mView, aflLocal);
    const float aflOrigin[Rnd::kXfmRowFloatCount] = {0.0f, 0.0f, 0.0f, 0.0f};
    static_cast<Rnd::Transformable *>(mView)->SetOrigin(aflOrigin);
    mView->SetFrame(fNow);
    mPsAnim->SetFrame(fNow);
    mBackPsAnim->SetFrame(fNow);
    static_cast<Rnd::Transformable *>(mView)->UpdateWorldXfm(nullptr, 0);
    if (mPlayer != 0) {
        return;
    }
    sAllAnim->SetFrame(fNow);
    sLoadingView->SetFrame(fNow);
    static_cast<Rnd::Transformable *>(sLoadingView)->UpdateWorldXfm(nullptr, 0);
}

#pragma mark - Drawing

void Ship::DrawShared() {
    static_cast<Rnd::Drawable *>(sEnviron)->Draw();
}

void Ship::DrawTrack() {
    Rnd::Drawable *pViewDraw = mView;
    if (pViewDraw->mShowing == 0) {
        return;
    }
    const float *pTranslation =
        static_cast<Rnd::Transformable *>(mView)->mWorldXfm[Rnd::kXfmRowCount - 1];
    const Vector3 position{pTranslation[0], pTranslation[1], pTranslation[2]};
    Rnd::Cam::sCurrent->WorldToScreen(position, mScreenPos);
    if ((mFlags & kFlagBumped) != 0) {
        static_cast<Rnd::Drawable *>(mBumperBlur)->Draw();
    } else {
        static_cast<Rnd::Drawable *>(mMesh)->Draw();
    }
    pViewDraw->Draw();
}

void Ship::DrawEffects() {
    if (static_cast<Rnd::Drawable *>(mView)->mShowing == 0) {
        return;
    }
    if (mTrodes[0] != nullptr) {
        for (Rnd::Mesh *pTrode : mTrodes) {
            static_cast<Rnd::Drawable *>(pTrode)->Draw();
        }
    }
    for (ParticleArm &arm : mArms) {
        arm.Draw();
    }
    mThruster.Draw();
    if ((mFlags & kFlagAttacking) == 0) {
        return;
    }
    const float fSinceAttack = TheGameDb->mSongTime - mAttackTime;
    if (sBumpAttackTime <= fSinceAttack) {
        mFlags &= ~kFlagAttacking;
        return;
    }
    float aflCell[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    Transform cell;
    Geom()->CellXfm(mAttackTrack, &cell, true, false, TheGameDb->mSongTick, kCellCentre);
    ToRows(cell, aflCell);
    mBumperAttack->SetFrame((fSinceAttack * kBumpFrames) / sBumpAttackTime);
    SetLocalXfm(mBumperAttack, aflCell);
    static_cast<Rnd::Transformable *>(mBumperAttack)->UpdateWorldXfm(nullptr, 0);
    static_cast<Rnd::Drawable *>(mBumperAttack)->Draw();
}

void Ship::DrawArrow() {
    if (sArrowMesh == nullptr || mShown == 0) {
        return;
    }
    float fX = mScreenPos.x;
    float fY = mScreenPos.y;
    if (kScreenMin <= fX && fX <= kScreenMax && kScreenMin <= fY && fY <= kScreenMax) {
        mArrowStart = kUnset;
        return;
    }
    if (mArrowStart == kUnset) {
        mArrowStart = TheGameDb->mSongTime;
    }
    // Slide the arrow along the line from the centre of the screen to the ship until it lies on
    // the edge the ship left by.
    if (fX < kArrowEdgeMin || kArrowEdgeMax < fX) {
        const float fSlope = fX < kArrowEdgeMin ? kArrowSlopeLeft : kArrowSlopeRight;
        const float fDeltaX = fX - kArrowCentreX;
        const float fScale = (mArrowAspect * fSlope) / fDeltaX;
        fY = ((fY - kArrowCentreY) * fScale) + kArrowCentreY;
        fX = (fDeltaX * fScale) + kArrowCentreX;
    }
    if (fY < kArrowEdgeMin || kArrowEdgeMax < fY) {
        const float fSlope = fY < kArrowEdgeMin ? kArrowSlopeTop : kArrowSlopeBottom;
        const float fDeltaY = fY - kArrowCentreY;
        const float fScale = (mArrowAspect * fSlope) / fDeltaY;
        fY = (fDeltaY * fScale) + kArrowCentreY;
        fX = ((fX - kArrowCentreX) * fScale) + kArrowCentreX;
    }
    const float fAspect = TheRnd->AspectRatio();
    float aflXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount];
    float *pRight = aflXfm[0];
    float *pUp = aflXfm[1];
    float *pDirection = aflXfm[2];
    float *pPosition = aflXfm[Rnd::kXfmRowCount - 1];
    pDirection[0] = -(fX - kArrowCentreX);
    pDirection[1] = 0.0f;
    pDirection[2] = fY - kArrowCentreY;
    pPosition[0] = fX;
    pPosition[1] = 0.0f;
    pPosition[2] = -fY * fAspect;
    Vec3Normalize(pDirection, pDirection);
    pUp[0] = 0.0f;
    pUp[1] = 1.0f;
    pUp[2] = 0.0f;
    CrossVec3(pUp, pDirection, pRight);

    Rnd::Mat *pMat = sArrowMesh->mMat;
    const float fSinceArrow = TheGameDb->mSongTime - mArrowStart;
    if (kArrowPulseTime < fSinceArrow || fSinceArrow < 0.0f) {
        pMat->SetAmbient(mColor);
    } else {
        float fPulse = 1.0f - (fSinceArrow * kArrowPulseRate);
        fPulse *= fPulse;
        const float fBrighten = fPulse * kArrowPulseBrighten;
        const Color pulse{mColor.r + fBrighten, mColor.g + fBrighten, mColor.b + fBrighten, 1.0f};
        pMat->SetAmbient(pulse);
        const float fScale = (fPulse * kArrowPulseGrowth) + 1.0f;
        for (int i = 0; i < kNumComponents; ++i) {
            for (int j = 0; j < kNumComponents; ++j) {
                aflXfm[i][j] *= fScale;
            }
        }
    }
    SetLocalXfm(sArrowMesh, aflXfm);
    static_cast<Rnd::Transformable *>(sArrowMesh)->UpdateWorldXfm(nullptr, 0);
    static_cast<Rnd::Drawable *>(sArrowMesh)->Draw();
}
