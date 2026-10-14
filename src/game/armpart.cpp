#include "game/armpart.h"

#include <cmath>

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "math/quaternion.h"
#include "os/string.h"

namespace {

// The animations of an arm, in the order of the `anims` array.
constexpr int kForearmAnim = 0;
constexpr int kHandAnim = 1;

// The names of the forearm and the hand in the `anims` array, after its tag.
constexpr int kForearmBone = 1;
constexpr int kHandBone = 2;

// Report the length of a vector, as the vector unit takes it.
inline float Length(const Vector3 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Scale a vector to unit length by the reciprocal square root of the vector unit.
inline void Normalize(Vector3 &v) {
    const float fScale = 1.0f / Length(v);
    v.x *= fScale;
    v.y *= fScale;
    v.z *= fScale;
}

// Report the distance between the world positions of two meshes.
inline float Distance(const RndMesh *pFrom, const RndMesh *pTo) {
    Vector3 offset;
    offset.x = pFrom->mWorldXfm.mTranslation.x - pTo->mWorldXfm.mTranslation.x;
    offset.y = pFrom->mWorldXfm.mTranslation.y - pTo->mWorldXfm.mTranslation.y;
    offset.z = pFrom->mWorldXfm.mTranslation.z - pTo->mWorldXfm.mTranslation.z;
    return Length(offset);
}

// The bend axis of an arm, which the right arm reverses.
inline void BendAxis(bool bLeft, const Vector3 &axis, Vector3 &out) {
    if (bLeft) {
        out = axis;
    } else {
        out.x = -axis.x;
        out.y = -axis.y;
        out.z = -axis.z;
    }
}

} // namespace

ArmPart::ArmPart(int nPart, int nParent, Avatar *pAvatar) : AvatarPart(nPart, nParent, pAvatar) {
    mForearm = nullptr;
    mHand = nullptr;
}

void ArmPart::SetFrame(float fFrame) {
    if (mTransRoot == nullptr) {
        return;
    }
    AvatarPart *pInstrument = mAvatar->GetPart(AvatarPartSet::kPartInstrument);
    Transform toJoint;
    Invert(toJoint, mJoint->mWorldXfm);
    Multiply(toJoint, toJoint, pInstrument->mTransRoot->mWorldXfm);

    Transform hand;
    mAnims[kHandAnim]->MakeTransform(fFrame, hand, 0);
    Multiply(hand, toJoint, hand);

    Vector3 pole;
    InterpKeys(mAnims[kForearmAnim]->mFramesOwner->mTransKeys, fFrame, pole);
    MultiplyPoint(toJoint, pole, pole);
    SolveIK(hand, pole);
}

void ArmPart::SetFrames(float fFrom, float fTo, float fBlend) {
    if (mTransRoot == nullptr) {
        return;
    }
    AvatarPart *pInstrument = mAvatar->GetPart(AvatarPartSet::kPartInstrument);
    Transform toJoint;
    Invert(toJoint, mJoint->mWorldXfm);
    Multiply(toJoint, toJoint, pInstrument->mTransRoot->mWorldXfm);

    Transform hand;
    BlendTranslation(mAnims[kHandAnim], fFrom, fTo, fBlend, hand.mTranslation);
    Quat rotation;
    BlendRotation(mAnims[kHandAnim], fFrom, fTo, fBlend, rotation);
    Transform basis;
    Rnd::MakeRotMatrix(rotation, &basis.mBasisX.x);
    hand.mBasisX = basis.mBasisX;
    hand.mBasisY = basis.mBasisY;
    hand.mBasisZ = basis.mBasisZ;
    Multiply(hand, toJoint, hand);

    Vector3 pole;
    BlendTranslation(mAnims[kForearmAnim], fFrom, fTo, fBlend, pole);
    MultiplyPoint(toJoint, pole, pole);
    SolveIK(hand, pole);
}

void ArmPart::SolveIK(const Transform &hand, const Vector3 &pole) {
    const Vector3 &wrist = hand.mTranslation;
    const float fReach = Length(wrist);
    const float fInverseReach = -1.0f / fReach;
    Vector3 toShoulder;
    toShoulder.x = wrist.x * fInverseReach;
    toShoulder.z = wrist.z * fInverseReach;
    toShoulder.y = wrist.y * fInverseReach;
    Vector3 toPole;
    toPole.x = pole.x - wrist.x;
    toPole.y = pole.y - wrist.y;
    toPole.z = pole.z - wrist.z;
    Vector3 axis;
    CrossVec3(&toPole.x, &toShoulder.x, &axis.x);
    Normalize(axis);
    Vector3 bend;
    CrossVec3(&toShoulder.x, &axis.x, &bend.x);

    // The law of cosines places the elbow along the line to the shoulder and out along the bend.
    const float fForearmSquared = mForearmLength * mForearmLength;
    float fAlong =
        (fForearmSquared - mUpperLength * mUpperLength + fReach * fReach) / (fReach + fReach);
    if (mForearmLength < fAlong) {
        fAlong = mForearmLength;
    }
    const float fOut = std::sqrt(fForearmSquared - fAlong * fAlong);
    Vector3 elbow;
    elbow.x = toShoulder.x * fAlong + bend.x * fOut + wrist.x;
    elbow.y = toShoulder.y * fAlong + bend.y * fOut + wrist.y;
    elbow.z = toShoulder.z * fAlong + bend.z * fOut + wrist.z;

    Vector3 forearmY;
    forearmY.x = wrist.x - elbow.x;
    forearmY.y = wrist.y - elbow.y;
    forearmY.z = wrist.z - elbow.z;
    Normalize(forearmY);
    Vector3 forearmZ;
    BendAxis(mAvatar->GetPart(AvatarPartSet::kPartLeftArm) == this, axis, forearmZ);
    forearmZ.x += hand.mBasisZ.x;
    forearmZ.y += hand.mBasisZ.y;
    forearmZ.z += hand.mBasisZ.z;
    Vector3 forearmX;
    CrossVec3(&forearmY.x, &forearmZ.x, &forearmX.x);
    Normalize(forearmX);
    CrossVec3(&forearmX.x, &forearmY.x, &forearmZ.x);

    Vector3 upperY = elbow;
    Normalize(upperY);
    Vector3 upperZ;
    BendAxis(mAvatar->GetPart(AvatarPartSet::kPartLeftArm) == this, axis, upperZ);
    Vector3 upperX;
    CrossVec3(&upperY.x, &upperZ.x, &upperX.x);

    mTransRoot->mLocalXfm.mBasisX = upperX;
    mTransRoot->mLocalXfm.mBasisY = upperY;
    mTransRoot->mDirty = 1;
    mTransRoot->mLocalXfm.mBasisZ = upperZ;

    mForearm->mLocalXfm.mBasisX = forearmX;
    mForearm->mLocalXfm.mBasisY = forearmY;
    mForearm->mLocalXfm.mBasisZ = forearmZ;
    mForearm->mDirty = 1;
    mForearm->mLocalXfm.mTranslation = elbow;

    mHand->mLocalXfm = hand;
    mHand->mDirty = 1;
}

void ArmPart::SetType(const char *pszType) {
    if (mAvatar->GetPart(AvatarPartSet::kPartRightArm) != this) {
        AvatarPart::SetType(pszType);
    }
}

bool ArmPart::Rebuild() {
    if (mType == mPendingType) {
        return true;
    }
    if (!AvatarPart::Rebuild()) {
        return false;
    }
    DataArray *pAnims = mData->FindArray("anims", true);
    mForearm =
        FindMesh(FormatString("avatar%d_%s.mesh", mAvatar->mIndex, pAnims->Sym(kForearmBone)));
    mHand = FindMesh(FormatString("avatar%d_%s.mesh", mAvatar->mIndex, pAnims->Sym(kHandBone)));
    mTransRoot->RemoveTrans(mForearm);
    mForearm->RemoveTrans(mHand);
    mUpperLength = Distance(mTransRoot, mForearm);
    mForearmLength = Distance(mForearm, mHand);
    if (mAvatar->GetPart(AvatarPartSet::kPartLeftArm) == this) {
        AvatarPart *pRightArm = mAvatar->GetPart(AvatarPartSet::kPartRightArm);
        pRightArm->mPendingType = mType;
        pRightArm->mMat = mMat;
        pRightArm->Rebuild(); // The binary discards the result.
    }
    return true;
}

void ArmPart::Update() {
    if (mTransRoot == nullptr) {
        return;
    }
    mTransRoot->UpdateWorldXfm(mJoint, 1);
    mForearm->UpdateWorldXfm(mJoint, 1);
    mHand->UpdateWorldXfm(mJoint, 1);
}
