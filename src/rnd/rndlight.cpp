#include "rnd/rndlight.h"

#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndenviron.h"
#include "rnd/rndmanager.h"

namespace {

// The first version without the legacy fields.
constexpr int kRevNoLegacyFloats = 2;

// The floats an early version stores after the colour that nothing reads.
constexpr int kLegacyUnusedFloats = 8;

// The default cone angle, a quarter turn, and the default range.
constexpr float kDefaultAngle = 0.785398185f;
constexpr float kDefaultRange = 1000.0f;

} // namespace

RndLight *RndLight::sDefault = nullptr;
const char *RndLight::sClassName = "Light";
int RndLight::sRev = 2;

RndLight::RndLight(const char *pszName) : RndObject(pszName) {
    mColor.r = 1.0f;
    mColor.a = 1.0f;
    mColor.g = 1.0f;
    mColor.b = 1.0f;
    mOuterAng = kDefaultAngle;
    mRange = kDefaultRange;
    mConstantAtten = 1.0f;
    mType = kTypeDirectional;
    mInnerAng = kDefaultAngle;
    mLinearAtten = 0.0f;
    mQuadraticAtten = 0.0f;
}

RndLight::~RndLight() = default;

void RndLight::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndTransformable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndLight]\n";
    stream << "color:" << mColor << " innerAng:" << mInnerAng << "\n";
    stream << "outerAng:" << mOuterAng << " range:" << mRange << "\n";
    stream << "constantAtten:" << mConstantAtten << " linearAtten:" << mLinearAtten
           << " quadraticAtten:" << mQuadraticAtten << "\n";
    stream << "type:" << static_cast<Type>(mType) << "\n";
}

void RndLight::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndTransformable::Save(stream);
    stream.WriteEndian(&mColor.r, sizeof(mColor.r));
    stream.WriteEndian(&mColor.g, sizeof(mColor.g));
    stream.WriteEndian(&mColor.b, sizeof(mColor.b));
    stream.WriteEndian(&mColor.a, sizeof(mColor.a));
    stream.WriteEndian(&mInnerAng, sizeof(mInnerAng));
    stream.WriteEndian(&mOuterAng, sizeof(mOuterAng));
    stream.WriteEndian(&mRange, sizeof(mRange));
    stream.WriteEndian(&mConstantAtten, sizeof(mConstantAtten));
    stream.WriteEndian(&mLinearAtten, sizeof(mLinearAtten));
    stream.WriteEndian(&mQuadraticAtten, sizeof(mQuadraticAtten));
    stream.WriteEndian(&mType, sizeof(mType));
}

void RndLight::Replace(RndObject *pFrom, RndObject *pTo) {
    RndTransformable::Replace(pFrom, pTo);
}

void RndLight::Copy(const RndObject *pSource, int nFlags) {
    const RndLight *pLight = pSource != nullptr ? dynamic_cast<const RndLight *>(pSource) : nullptr;
    RndTransformable::Copy(pSource, nFlags);
    mColor = pLight->mColor;
    mInnerAng = pLight->mInnerAng;
    mOuterAng = pLight->mOuterAng;
    mRange = pLight->mRange;
    mConstantAtten = pLight->mConstantAtten;
    mLinearAtten = pLight->mLinearAtten;
    mQuadraticAtten = pLight->mQuadraticAtten;
    mType = pLight->mType;
    Sync();
}

void RndLight::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Light");
        return;
    }
    RndTransformable::Load(stream);
    stream.ReadEndian(&mColor.r, sizeof(mColor.r));
    stream.ReadEndian(&mColor.g, sizeof(mColor.g));
    stream.ReadEndian(&mColor.b, sizeof(mColor.b));
    stream.ReadEndian(&mColor.a, sizeof(mColor.a));
    if (nRev < kRevNoLegacyFloats) {
        for (int i = 0; i < kLegacyUnusedFloats; ++i) {
            float fUnused;
            stream.ReadEndian(&fUnused, sizeof(fUnused));
        }
    }
    stream.ReadEndian(&mInnerAng, sizeof(mInnerAng));
    stream.ReadEndian(&mOuterAng, sizeof(mOuterAng));
    stream.ReadEndian(&mRange, sizeof(mRange));
    stream.ReadEndian(&mConstantAtten, sizeof(mConstantAtten));
    stream.ReadEndian(&mLinearAtten, sizeof(mLinearAtten));
    stream.ReadEndian(&mQuadraticAtten, sizeof(mQuadraticAtten));
    int nType;
    stream.ReadEndian(&nType, sizeof(nType));
    mType = nType;
    Sync();
}

void RndLight::CreateDefault() {
    RndObject *pObject = TheManager.Create(sClassName, "[default light]");
    RndLight *pLight = pObject != nullptr ? dynamic_cast<RndLight *>(pObject) : nullptr;
    sDefault = pLight;
    pLight->mInternal = 1;
    RndEnviron::sDefault->AddLight(pLight);
    RndCam::sDefault->AddTrans(sDefault);
}

PrnStream &operator<<(PrnStream &stream, RndLight::Type eType) {
    switch (eType) {
    case RndLight::kTypePoint:
        stream << "Point";
        break;
    case RndLight::kTypeDirectional:
        stream << "Directional";
        break;
    case RndLight::kTypeSpot:
        stream << "Spot";
        break;
    }
    return stream;
}
