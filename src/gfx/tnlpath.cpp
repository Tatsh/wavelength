#include "gfx/tnlpath.h"

#include <cmath>
#include <vector>

#include "gfx/gfxutil.h"
#include "math/key.h"
#include "math/vector3.h"

namespace {

// The blend of a path that is not blending holds the segment in use from the start.
constexpr float kBlendDone = 1.0f;
constexpr float kBlendStart = 0.0f;

} // namespace

TnlPath::TnlPath() : mBlend(kBlendDone, kBlendDone, kBlendStart, kBlendDone) {
}

void TnlPath::Set(RndTransAnim *pAnim,
                  int bLoop,
                  int bUseTime,
                  float flStart,
                  float flFrame,
                  float flLoopStart,
                  float flLoopEnd) {
    mSegments[0].Reset();
    Segment &segment = mSegments[1];
    segment.mAnim = pAnim;
    segment.mLoop = bLoop;
    segment.mUseTime = bUseTime;
    segment.mLoopStart = flLoopStart;
    segment.mLoopEnd = flLoopEnd;
    segment.mOffset = flFrame - flStart;
    mBlend.Reset(kBlendDone, kBlendDone, kBlendStart, kBlendDone);
}

void TnlPath::BlendTo(RndTransAnim *pAnim,
                      int bLoop,
                      int bUseTime,
                      float flLength,
                      float flStart,
                      float flFrame,
                      float flLoopStart,
                      float flLoopEnd) {
    mSegments[0] = mSegments[1];
    Segment &segment = mSegments[1];
    segment.mLoopStart = flLoopStart;
    segment.mAnim = pAnim;
    segment.mLoop = bLoop;
    segment.mUseTime = bUseTime;
    segment.mLoopEnd = flLoopEnd;
    segment.mOffset = flFrame - flStart;
    mBlend.Reset(kBlendStart, kBlendDone, flStart, flStart + flLength);
}

void TnlPath::Xfm(float flTick, float flTime, Transform *pXfm) {
    const float flBlend = mBlend.Eval(mSegments[1].mUseTime ? flTime : flTick);
    mSegments[1].Xfm(flTick, flTime, pXfm);
    if (!(flBlend < kBlendDone)) {
        return;
    }
    Transform from;
    mSegments[0].Xfm(flTick, flTime, &from);
    if (flBlend == kBlendStart) {
        pXfm->mTranslation = from.mTranslation;
    } else if (flBlend != kBlendDone) {
        const float flRest = kBlendDone - flBlend;
        pXfm->mTranslation.x = (pXfm->mTranslation.x * flBlend) + (from.mTranslation.x * flRest);
        pXfm->mTranslation.y = (pXfm->mTranslation.y * flBlend) + (from.mTranslation.y * flRest);
        pXfm->mTranslation.z = (pXfm->mTranslation.z * flBlend) + (from.mTranslation.z * flRest);
    }
    InterpBasis(&from.mBasisX.x, &pXfm->mBasisX.x, &pXfm->mBasisX.x, flBlend);
}

void TnlPath::Segment::Reset() {
    mOffset = 0.0f;
    mAnim = nullptr;
    mLoopStart = mOffset;
    mLoop = 0;
    mLoopEnd = mOffset;
    mUseTime = 0;
}

void TnlPath::Segment::Xfm(float flTick, float flTime, Transform *pXfm) const {
    float flFrame = (mUseTime ? flTime : flTick) + mOffset;
    if (mLoop && (mLoopStart < flFrame)) {
        const float flSpan = mLoopEnd - mLoopStart;
        float flInLoop = std::fmod(flFrame - mLoopStart, flSpan);
        if (flInLoop < 0.0f) {
            flInLoop += flSpan;
        }
        flFrame = mLoopStart + flInLoop;
    }
    flFrame = mAnim->FilterFrame(flFrame);

    const std::vector<Key<Vector3>> &keys = mAnim->mFramesOwner->mTransKeys;
    const float flFirst = keys.front().frame;
    if (!(flFrame < flFirst)) {
        mAnim->MakeTransform(flFrame, *pXfm, 1);
        return;
    }
    mAnim->MakeTransform(flFirst, *pXfm, 1);
    // Yes, a channel of one key divides by the frame of the key after its last.
    const float flSpan = keys[1].frame - flFirst;
    Vector3 tangent;
    InterpVector(keys, RndTransAnim::kInterpSpline, pXfm->mTranslation, &tangent, flFirst);
    const float flScale = (flFrame - flFirst) / flSpan;
    tangent.x *= flScale;
    tangent.z *= flScale;
    tangent.y *= flScale;
    pXfm->mTranslation.x += tangent.x;
    pXfm->mTranslation.z += tangent.z;
    pXfm->mTranslation.y += tangent.y;
}
