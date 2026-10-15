#include "game/avatar.h"

#include <cmath>
#include <utility>

#include "game/armpart.h"
#include "game/avatarpartset.h"
#include "game/gameoptions.h"
#include "game/instrpart.h"
#include "game/torsopart.h"
#include "math/quaternion.h"
#include "math/sine.h"
#include "math/transformops.h"
#include "os/debug.h"
#include "os/system.h"
#include "rnd/rndenviron.h"
#include "rnd/rndmanager.h"
#include "rnd/rndrenderer.h"
#include "script/symbol.h"

namespace {

// The keys of the frame ranges of the `anims` entries.
constexpr int kPoseKey = 4;
constexpr int kBodyKey = 5;

// The type of a frame range that loops.
constexpr int kLoopType = 3;

// The fields of a frame range.
enum AnimRangeField {
    kAnimRangeType = 0,
    kAnimRangeStart = 1,
    kAnimRangeLength = 2,
};

// The range Anim::Set() takes when the `anims` entry does not list the key.
constexpr int kDefaultAnimRange = 1;

// The number of avatars when `num_avatars` is missing.
constexpr int kDefaultAvatarCount = 4;

// The loader of the base model reports the objects that exist and deletes its objects with it.
constexpr int kBaseLoaderFlags = RndLoader::kReportExisting | RndLoader::kDeleteObjects;

// The camera Init() starts at, and the cameras of the avatar display sizes.
constexpr char kStartCam[] = "bust";
constexpr char kFarCam[] = "game_far";
constexpr char kCloseCam[] = "game_close";

constexpr float kDegreesToRadians = 0.017453292f;
constexpr float kHalfPi = 1.5707964f;

// The parts the body and the pose animations move.
constexpr int kFirstBodyPart = AvatarPartSet::kPartHead;
constexpr int kFirstPosePart = AvatarPartSet::kPartLeftArm;
constexpr int kEndPosePart = AvatarPartSet::kPartHead;

// The parent AvatarPart takes for a part that hangs from no other.
constexpr int kNoParentPart = AvatarPartSet::kNumParts;

// Transform one row of a rotation by another rotation, leaving the fourth word of the row.
inline void MultiplyRow(Vector3 &row, const Vector3 *pBasis) {
    const float fX = (pBasis[0].x * row.x) + (pBasis[1].x * row.y) + (pBasis[2].x * row.z);
    const float fY = (pBasis[0].y * row.x) + (pBasis[1].y * row.y) + (pBasis[2].y * row.z);
    const float fZ = (pBasis[0].z * row.x) + (pBasis[1].z * row.y) + (pBasis[2].z * row.z);
    row.x = fX;
    row.y = fY;
    row.z = fZ;
}

} // namespace

float Avatar::sAnimBlend;
float Avatar::sTimeToIdle;
float Avatar::sFov;
float Avatar::sCamBlend;
RndLoader *Avatar::sLoader = nullptr;
RndTex *Avatar::sTargetTex = nullptr;
RndView *Avatar::sView = nullptr;
const char *Avatar::sNone = nullptr;
const char *Avatar::sIdle = nullptr;
DataArray *Avatar::sCams = nullptr;
RndCam *Avatar::sCam = nullptr;
const char *Avatar::sCurrentCam = nullptr;
const char *Avatar::sPreviousCam = nullptr;
float Avatar::sCamBlendStart = Avatar::kNever;
int Avatar::sFreqSize = GameOptions::kFreqSizeSmall;
float Avatar::sTime = 0.0f;
DataArray *Avatar::sAnims = nullptr;

// The unit's static initialiser at NTSC-U/C: 0x002715a8, PAL: 0x0027b158, and its global
// constructor and destructor at NTSC-U/C: 0x00271650, PAL: 0x0027b200, and NTSC-U/C: 0x00271670,
// PAL: 0x0027b220, construct and destroy the list.
std::vector<Avatar *> Avatar::sAvatars;

void Avatar::Init() {
    DataArray *pAvatar = SystemConfig()->FindArray("db", true)->FindArray("avatar", true);
    pAvatar->FindFloat("anim_blend", &sAnimBlend, false);
    pAvatar->FindFloat("time_to_idle", &sTimeToIdle, false);
    sAnims = pAvatar->FindArray("anims", true);
    const char *pszBase;
    pAvatar->FindSymbol("base_rnd", &pszBase, true);
    sLoader = TheManager.AddLoader(pszBase, kBaseLoaderFlags, nullptr, nullptr);

    sCam = dynamic_cast<RndCam *>(TheManager.Find("base_freq.cam"));
    const Vector3 position{0.0f, 0.0f, 0.0f};
    sCam->mDirty = 1;
    sCam->mLocalXfm.mTranslation = position;
    sView = dynamic_cast<RndView *>(TheManager.Find("base_freq.view"));
    sView->ClearTrans();
    sFov = sCam->mFov;
    sTargetTex = sCam->mTargetTex;
    sNone = LookupSymbol("fm_none");
    sIdle = LookupSymbol("idle");
    sCams = pAvatar->FindArray("cams", true);
    pAvatar->FindFloat("cam_blend", &sCamBlend, false);

    int nAvatars = kDefaultAvatarCount;
    pAvatar->FindInt("num_avatars", &nAvatars, true);
    sAvatars.resize(nAvatars, nullptr);
    for (int i = 0; i < nAvatars; ++i) {
        sAvatars[i] = new Avatar(i);
    }
    SnapCamera(kStartCam);
}

void Avatar::Terminate() {
    for (size_t i = 0; i < sAvatars.size(); ++i) {
        if (sAvatars[i] != nullptr) {
            delete sAvatars[i];
        }
    }
    if (sLoader != nullptr) {
        delete sLoader;
    }
}

int Avatar::NumAvatars() {
    return static_cast<int>(sAvatars.size());
}

Avatar *Avatar::GetAvatar(int nIndex) {
    return sAvatars[nIndex];
}

Avatar::Avatar(int nIndex)
    : mParts(AvatarPartSet::kNumParts, nullptr), mIndex(nIndex), mUser(nullptr) {
    mBody.mAnim = nullptr;
    mPrevBody.mAnim = nullptr;
    mPoseAnim.mAnim = nullptr;
    mPrevPoseAnim.mAnim = nullptr;
    // The binary discards the lookup.
    (void)SystemConfig()->FindArray("db", true)->FindArray("avatar", true);
    mParts[AvatarPartSet::kPartLowerBody] =
        new AvatarPart(AvatarPartSet::kPartLowerBody, kNoParentPart, this);
    mParts[AvatarPartSet::kPartTorso] = new TorsoPart(AvatarPartSet::kPartLowerBody, this);
    mParts[AvatarPartSet::kPartInstrument] = new InstrPart(AvatarPartSet::kPartTorso, this);
    mParts[AvatarPartSet::kPartLeftArm] =
        new ArmPart(AvatarPartSet::kPartLeftArm, AvatarPartSet::kPartTorso, this);
    mParts[AvatarPartSet::kPartRightArm] =
        new ArmPart(AvatarPartSet::kPartRightArm, AvatarPartSet::kPartTorso, this);
    mParts[AvatarPartSet::kPartHead] =
        new AvatarPart(AvatarPartSet::kPartHead, AvatarPartSet::kPartTorso, this);
    mParts[AvatarPartSet::kPartFaceGear] =
        new AvatarPart(AvatarPartSet::kPartFaceGear, AvatarPartSet::kPartHead, this);
    mParts[AvatarPartSet::kPartHeadGear] =
        new AvatarPart(AvatarPartSet::kPartHeadGear, AvatarPartSet::kPartHead, this);
    Reset();
}

Avatar::~Avatar() {
    for (int i = 0; i < AvatarPartSet::kNumParts; ++i) {
        if (mParts[i] != nullptr) {
            delete mParts[i];
        }
    }
}

void Avatar::SetPose(const char *pszPose, bool bImmediate) {
    const char *pszSymbol = LookupSymbol(pszPose);
    const char *pszOldPose = mPose;
    if (pszSymbol == pszOldPose) {
        return;
    }
    if (bImmediate) {
        mPose = pszSymbol;
        mPrevPose = pszOldPose;
        mPoseChangeTime = kNever;
        mPrevPoseAnim = mPoseAnim;
        mPoseBlendStart = kNever;
    } else {
        if (mOneShot != 0) {
            mPrevPose = pszSymbol;
            mPrevPoseAnim.Set(pszSymbol, kPoseKey);
            return;
        }
        mPose = pszSymbol;
        mPrevPose = pszOldPose;
        mPoseChangeTime = sTime;
        mPrevPoseAnim = mPoseAnim;
        mPoseBlendStart = sTime;
    }
    if (mPose == sNone) {
        mPoseAnim = mBody;
    } else {
        mPoseAnim.Set(mPose, kPoseKey);
    }
}

void Avatar::SetBaseAnim(const char *pszAnim, bool bOneShot, bool bImmediate) {
    const char *pszSymbol = LookupSymbol(pszAnim);
    const char *pszOldAnim = mBody.mAnim;
    if (pszSymbol == pszOldAnim) {
        return;
    }
    if (bOneShot) {
        if (mOneShot == 0) {
            mPrevBody = mBody;
            mPrevPoseAnim = mPoseAnim;
            mPrevPose = mPose;
            SetCamera(kFarCam);
            const float fStart = bImmediate ? kNever : sTime;
            mBodyBlendStart = fStart;
            mPoseChangeTime = fStart;
            mPoseBlendStart = fStart;
        }
        mBody.Set(pszSymbol, kBodyKey);
        mPoseAnim.Set(pszSymbol, kBodyKey);
        mOneShot = 1;
        mPose = sNone;
        return;
    }
    if (bImmediate) {
        mPrevBody = mBody;
        mBody.Set(pszSymbol, kBodyKey);
        mBodyBlendStart = kNever;
        if (mPose == sNone) {
            mPrevPoseAnim = mPoseAnim;
            mPoseAnim = mBody;
            mPoseBlendStart = kNever;
        }
        return;
    }
    if (mOneShot != 0) {
        mPrevBody.Set(pszSymbol, kBodyKey);
        return;
    }
    if (pszOldAnim == sIdle) {
        mPrevBody.Set(pszSymbol, kBodyKey);
        if (mPose == sNone) {
            mPrevPoseAnim = mPoseAnim;
            mPoseAnim = mPrevBody;
            mPoseBlendStart = mBodyBlendStart;
        }
        return;
    }
    mPrevBody = mBody;
    mBody.Set(pszSymbol, kBodyKey);
    mBodyBlendStart = sTime;
    if (mPose == sNone) {
        mPrevPoseAnim = mPoseAnim;
        mPoseAnim = mBody;
        mPoseBlendStart = mBodyBlendStart;
    }
}

void Avatar::Reset() {
    mOneShot = 0;
    mPrevPose = sNone;
    mPoseChangeTime = kNever;
    mActiveTime = sTime;
    mPose = sNone;
    mBodyBlendStart = kNever;
    mPoseBlendStart = kNever;
    mPrevBody.Set(sIdle, kBodyKey);
    mBody.Set(sIdle, kBodyKey);
    mPrevPoseAnim.Set(sIdle, kBodyKey);
    mPoseAnim.Set(sIdle, kBodyKey);
}

AvatarPart *Avatar::GetPart(int nPart) {
    return mParts[nPart];
}

void Avatar::Draw(const Rect *pScreenRect) {
    if (sTime < mActiveTime) {
        mActiveTime = sTime;
    }
    if (mOneShot != 0 && mBody.IsDone()) {
        std::swap(mBody, mPrevBody);
        mBodyBlendStart = sTime;
        std::swap(mPoseAnim, mPrevPoseAnim);
        mPoseChangeTime = sTime;
        mPoseBlendStart = sTime;
        std::swap(mPose, mPrevPose);
        SetCamera(sPreviousCam);
        mOneShot = 0;
        mActiveTime = sTime;
    }
    if (mOneShot == 0 && mBody.mAnim != sIdle && mActiveTime + sTimeToIdle < sTime) {
        mPrevBody = mBody;
        mBody.Set(sIdle, kBodyKey);
        mBodyBlendStart = sTime;
    }

    UpdateInstrument();
    SetPartFrames(&mPrevBody, &mBody, &mBodyBlendStart, kFirstBodyPart, AvatarPartSet::kNumParts);
    sCam->UpdateWorldXfm(nullptr, 0);
    mParts[AvatarPartSet::kPartLowerBody]->mTransRoot->UpdateWorldXfm(sView, 1);
    mParts[AvatarPartSet::kPartTorso]->Update();
    mParts[AvatarPartSet::kPartInstrument]->Update();
    mParts[AvatarPartSet::kPartHead]->Update();
    mParts[AvatarPartSet::kPartFaceGear]->Update();
    mParts[AvatarPartSet::kPartHeadGear]->Update();
    SetPartFrames(&mPrevPoseAnim, &mPoseAnim, &mPoseBlendStart, kFirstPosePart, kEndPosePart);
    mParts[AvatarPartSet::kPartLeftArm]->Update();
    mParts[AvatarPartSet::kPartRightArm]->Update();

    if (pScreenRect != nullptr) {
        sCam->SetTargetTex(nullptr);
        const Rect &current = sCam->mScreenRect;
        const bool bSame = pScreenRect->x == current.x && pScreenRect->y == current.y &&
                           pScreenRect->w == current.w && pScreenRect->h == current.h;
        if (!bSame) {
            const float fFov = RndCam::ConvertFov(
                1.0f, (pScreenRect->h * TheRnd->AspectRatio()) / pScreenRect->w, sFov);
            sCam->mScreenRect = *pScreenRect;
            sCam->UpdateProjection();
            sCam->SetFrustum(sCam->mNearPlane, sCam->mFarPlane, fFov);
        }
    } else if (sCam->mTargetTex == nullptr) {
        sCam->SetTargetTex(sTargetTex);
        const Rect full{0.0f, 0.0f, 1.0f, 1.0f};
        sCam->mScreenRect = full;
        sCam->UpdateProjection();
        sCam->SetFrustum(sCam->mNearPlane, sCam->mFarPlane, sFov);
    }

    RndEnviron *pEnviron = RndEnviron::sCurrent;
    RndCam *pCam = RndCam::sCurrent;
    sCam->Draw();
    mParts[AvatarPartSet::kPartLowerBody]->Draw();
    mParts[AvatarPartSet::kPartTorso]->Draw();
    mParts[AvatarPartSet::kPartLeftArm]->Draw();
    mParts[AvatarPartSet::kPartRightArm]->Draw();
    mParts[AvatarPartSet::kPartHead]->Draw();
    mParts[AvatarPartSet::kPartFaceGear]->Draw();
    mParts[AvatarPartSet::kPartHeadGear]->Draw();
    mParts[AvatarPartSet::kPartInstrument]->Draw();
    if (pCam != nullptr) {
        pCam->Draw();
    }
    if (pEnviron != nullptr) {
        pEnviron->Draw();
    }
}

void Avatar::DrawCamera() {
    RndEnviron *pEnviron = RndEnviron::sCurrent;
    RndCam *pCam = RndCam::sCurrent;
    sCam->Draw();
    if (pCam != nullptr) {
        pCam->Draw();
    }
    if (pEnviron != nullptr) {
        pEnviron->Draw();
    }
}

void Avatar::GetDepthRange(float *pfNear, float *pfFar) {
    *pfNear = sCam->mZRange.x;
    *pfFar = sCam->mZRange.y;
}

void Avatar::SetDepthRange(float fNear, float fFar) {
    const Vector2 range{fNear, fFar};
    sCam->mZRange = range;
}

void Avatar::SetViewAngles(float fTilt, float fTurn) {
    const Vector3 position{0.0f, 0.0f, sCam->mLocalXfm.mTranslation.z};
    sView->SetOrigin(position);
    sView->mLocalXfm.mTranslation = position;
    sView->mDirty = 1;

    const float fTurnCos = SinApprox(fTurn + kHalfPi);
    const float fTurnSin = SinApprox(fTurn);
    Vector3 rows[] = {
        {fTurnCos, fTurnSin, 0.0f},
        {-fTurnSin, fTurnCos, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };
    const float fTiltCos = SinApprox(fTilt + kHalfPi);
    const float fTiltSin = SinApprox(fTilt);
    const Vector3 tilt[] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, fTiltCos, fTiltSin},
        {0.0f, -fTiltSin, fTiltCos},
    };
    for (Vector3 &row : rows) {
        MultiplyRow(row, tilt);
    }
    sView->mLocalXfm.mBasisX = rows[0];
    sView->mLocalXfm.mBasisY = rows[1];
    sView->mDirty = 1;
    sView->mLocalXfm.mBasisZ = rows[2];
    sView->UpdateWorldXfm(nullptr, 0);
}

void Avatar::SetFreqSize(int nFreqSize, bool bSnap) {
    sFreqSize = nFreqSize;
    const char *pszCam = nullptr;
    switch (nFreqSize) {
    case GameOptions::kFreqSizeSmall:
        pszCam = kFarCam;
        break;
    case GameOptions::kFreqSizeLarge:
    case GameOptions::kFreqSizeHidden:
        pszCam = kCloseCam;
        break;
    default:
        DebugWarn(" Unrecognized toggle size\n");
        break;
    }
    if (bSnap) {
        SnapCamera(pszCam);
    } else {
        SetCamera(pszCam);
    }
}

bool Avatar::IsShown() {
    return sFreqSize != GameOptions::kFreqSizeHidden;
}

float Avatar::Anim::Frame() const {
    if (mLoop != 0) {
        const float fLength = mEnd - mStart;
        float fPhase = fmodf((sTime + mOffset) - mStart, fLength);
        if (fPhase < 0.0f) {
            fPhase += fLength;
        }
        return mStart + fPhase;
    }
    const float fFrame = sTime + mOffset;
    if (mEnd < fFrame) {
        return mEnd;
    }
    if (fFrame < mStart) {
        return mStart;
    }
    return fFrame;
}

bool Avatar::Anim::IsDone() const {
    return mLoop == 0 && mEnd < sTime + mOffset;
}

void Avatar::Anim::Set(const char *pszAnim, int nKey) {
    mAnim = pszAnim;
    if (pszAnim == nullptr) {
        return;
    }
    DataArray *pAnim = sAnims->FindArray(pszAnim, true);
    DataArray *pRange = pAnim->FindArray(nKey);
    if (pRange == nullptr) {
        pRange = pAnim->Array(kDefaultAnimRange);
    }
    mLoop = pRange->Int(kAnimRangeType) == kLoopType;
    mStart = pRange->Float(kAnimRangeStart);
    mEnd = mStart + pRange->Float(kAnimRangeLength);
    mOffset = mStart - (mLoop != 0 ? 0.0f : sTime);
}

void Avatar::UpdateInstrument() {
    AvatarPart *pInstrument = mParts[AvatarPartSet::kPartInstrument];
    if (mPoseChangeTime == kNever) {
        pInstrument->SetType(mPose);
        mParts[AvatarPartSet::kPartInstrument]->SetBlend(1.0f);
        return;
    }
    float fBlend = fabsf(sTime - mPoseChangeTime) / sAnimBlend;
    if (fBlend < 0.5) {
        fBlend = 1.0f - (fBlend + fBlend);
        if (fBlend < 0.0) {
            fBlend = 0.0f;
        }
        pInstrument->SetType(mPrevPose);
    } else {
        fBlend = (fBlend - 0.5f) * 2.0f;
        if (!(fBlend < 1.0)) {
            mPoseChangeTime = kNever;
            fBlend = 1.0f;
        }
        mParts[AvatarPartSet::kPartInstrument]->SetType(mPose);
    }
    mParts[AvatarPartSet::kPartInstrument]->SetBlend(fBlend);
}

void Avatar::SetPartFrames(
    const Anim *pFrom, const Anim *pTo, float *pfBlendStart, int nFirst, int nEnd) {
    float fBlend = 1.0f;
    if (*pfBlendStart != kNever) {
        fBlend = fabsf(sTime - *pfBlendStart) / sAnimBlend;
        if (1.0f < fBlend) {
            *pfBlendStart = kNever;
            fBlend = 1.0f;
        }
    }
    if (fBlend < 1.0f) {
        const float fFrom = pFrom->Frame();
        const float fTo = pTo->Frame();
        for (int i = nFirst; i < nEnd; ++i) {
            mParts[i]->SetFrames(fFrom, fTo, fBlend);
        }
    } else {
        const float fFrame = pTo->Frame();
        for (int i = nFirst; i < nEnd; ++i) {
            mParts[i]->SetFrame(fFrame);
        }
    }
}

void Avatar::Poll(float fTime) {
    sTime = fTime;
    for (size_t i = 0; i < sAvatars.size(); ++i) {
        AvatarPart *pInstrument = sAvatars[i]->GetPart(AvatarPartSet::kPartInstrument);
        if (pInstrument->mType != nullptr) {
            continue;
        }
        if (pInstrument->mPendingType != nullptr) {
            pInstrument->Rebuild();
        } else {
            pInstrument->SetType(sNone);
        }
        break;
    }
    if (sCamBlendStart == kNever) {
        return;
    }

    DataArray *pFrom = sCams->FindArray(sPreviousCam, true);
    DataArray *pTo = sCams->FindArray(sCurrentCam, true);
    Vector3 fromOrigin;
    pFrom->FindVector("origin", &fromOrigin, true);
    Vector3 origin;
    pTo->FindVector("origin", &origin, true);
    Vector3 angles;
    pFrom->FindVector("rot", &angles, true);
    angles.x *= kDegreesToRadians;
    angles.y *= kDegreesToRadians;
    angles.z *= kDegreesToRadians;
    const Quat fromRotation = EulerAnglesToQuat(&angles.x);
    pTo->FindVector("rot", &angles, true);
    angles.x *= kDegreesToRadians;
    angles.y *= kDegreesToRadians;
    angles.z *= kDegreesToRadians;
    Quat rotation = EulerAnglesToQuat(&angles.x);

    const float fBlend = fabsf(sTime - sCamBlendStart) / sCamBlend;
    if (fBlend >= 1.0) {
        sCamBlendStart = kNever;
    } else {
        QuatSlerp(fromRotation, rotation, rotation, fBlend);
        if (fBlend == 0.0f) {
            origin = fromOrigin;
        } else if (fBlend != 1.0f) {
            const float fFromWeight = 1.0f - fBlend;
            origin.x = (origin.x * fBlend) + (fromOrigin.x * fFromWeight);
            origin.y = (origin.y * fBlend) + (fromOrigin.y * fFromWeight);
            origin.z = (origin.z * fBlend) + (fromOrigin.z * fFromWeight);
        }
    }
    Vector3 rows[3];
    Rnd::MakeRotMatrix(rotation, &rows[0].x);
    sCam->mLocalXfm.mBasisX = rows[0];
    sCam->mLocalXfm.mBasisY = rows[1];
    sCam->mDirty = 1;
    sCam->mLocalXfm.mBasisZ = rows[2];
    sCam->SetOrigin(origin);
}

void Avatar::CatchGem(int nKey, int bRestartPose) {
    if (mOneShot != 0) {
        return;
    }
    if (bRestartPose != 0 && mPose != sNone) {
        mPoseAnim.Set(mPoseAnim.mAnim, nKey);
    }
    if (mBody.mAnim == sIdle && mPrevBody.mLoop != 0) {
        std::swap(mBody, mPrevBody);
        mBodyBlendStart = sTime;
    }
    mActiveTime = sTime;
}

void Avatar::SetCamera(const char *pszCam) {
    const char *pszSymbol = LookupSymbol(pszCam);
    if (pszSymbol != sCurrentCam) {
        sPreviousCam = sCurrentCam;
        sCurrentCam = pszSymbol;
        sCamBlendStart = sTime;
#ifdef VIDEO_STANDARD_PAL
    } else {
        sPreviousCam = pszSymbol;
#endif
    }
}

void Avatar::SnapCamera(const char *pszCam) {
    const char *pszSymbol = LookupSymbol(pszCam);
    sCamBlendStart = kNever;
    sPreviousCam = pszSymbol;
    sCurrentCam = pszSymbol;
    DataArray *pCam = sCams->FindArray(pszSymbol, true);
    Vector3 origin;
    pCam->FindVector("origin", &origin, true);
    sCam->SetOrigin(origin);
    Vector3 angles;
    pCam->FindVector("rot", &angles, true);
    angles.x *= kDegreesToRadians;
    angles.y *= kDegreesToRadians;
    angles.z *= kDegreesToRadians;
    Vector3 rows[3];
    Rnd::MakeRotMatrix(&angles.x, &rows[0].x);
    sCam->mLocalXfm.mBasisX = rows[0];
    sCam->mLocalXfm.mBasisY = rows[1];
    sCam->mDirty = 1;
    sCam->mLocalXfm.mBasisZ = rows[2];
}

void Avatar::SetUser(AvatarPartSet *pUser) {
    mUser = pUser;
}
