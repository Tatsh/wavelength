#include "rnd/rndenviron.h"

#include <algorithm>

#include "os/debug.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

const char *RndEnviron::sClassName = "Environ";
RndEnviron *RndEnviron::sCurrent = nullptr;
RndEnviron *RndEnviron::sDefault = nullptr;
int RndEnviron::sRev = 0;

RndEnviron::RndEnviron(const char *pszName) : RndObject(pszName) {
    mAmbientColor.r = 0.0f;
    mAmbientColor.a = 1.0f;
    mAmbientColor.g = 0.0f;
    mAmbientColor.b = 0.0f;
    mFogStart = 0.0f;
    mFogEnd = 1.0f;
    mFogDensity = 1.0f;
    mFogColor.r = 1.0f;
    mFogColor.a = 1.0f;
    mFogColor.g = 1.0f;
    mFogColor.b = 1.0f;
    mFogMode = kFogNone;
}

RndEnviron::~RndEnviron() {
    ReleaseRefs();
}

void RndEnviron::ListDrawObjects(std::list<RndObject *> &objects) {
    for (RndLight *pLight : mLights) {
        objects.push_back(pLight);
    }
}

void RndEnviron::DumpText(PrnStream &stream) {
    RndObject::DumpText(stream);
    RndDrawable::DumpText(stream);
    if (stream.mDumpLevel <= 0) {
        return;
    }
    stream << "[RndEnviron]\n";
    stream << "lights:" << mLights << "\n";
    stream << "ambientColor:" << mAmbientColor << " fogStart:" << mFogStart << "\n";
    stream << "fogEnd:" << mFogEnd << " fogDensity:" << mFogDensity << "\n";
    stream << "fogColor:" << mFogColor << " fogMode:" << static_cast<FogMode>(mFogMode) << "\n";
}

void RndEnviron::Save(BinStream &stream) {
    stream.WriteEndian(&sRev, sizeof(sRev));
    RndDrawable::Save(stream);
    stream << mLights;
    stream.WriteEndian(&mAmbientColor.r, sizeof(mAmbientColor.r));
    stream.WriteEndian(&mAmbientColor.g, sizeof(mAmbientColor.g));
    stream.WriteEndian(&mAmbientColor.b, sizeof(mAmbientColor.b));
    stream.WriteEndian(&mAmbientColor.a, sizeof(mAmbientColor.a));
    stream.WriteEndian(&mFogStart, sizeof(mFogStart));
    stream.WriteEndian(&mFogEnd, sizeof(mFogEnd));
    stream.WriteEndian(&mFogDensity, sizeof(mFogDensity));
    stream.WriteEndian(&mFogColor.r, sizeof(mFogColor.r));
    stream.WriteEndian(&mFogColor.g, sizeof(mFogColor.g));
    stream.WriteEndian(&mFogColor.b, sizeof(mFogColor.b));
    stream.WriteEndian(&mFogColor.a, sizeof(mFogColor.a));
    stream.WriteEndian(&mFogMode, sizeof(mFogMode));
}

void RndEnviron::Replace(RndObject *pFrom, RndObject *pTo) {
    RndDrawable::Replace(pFrom, pTo);
    auto it = mLights.begin();
    while (it != mLights.end()) {
        RndLight *&pLight = *it;
        if (static_cast<RndObject *>(pLight) == pTo) {
            DebugNotify("%s already in %s", pTo->mName.c_str(), mName.c_str());
        }
        if (static_cast<RndObject *>(pLight) == pFrom) {
            if (pFrom != nullptr) {
                pFrom->RemoveRef(this);
            }
            if (pLight != nullptr) {
                pLight = pTo != nullptr ? dynamic_cast<RndLight *>(pTo) : nullptr;
            }
            if (pLight != nullptr) {
                pLight->AddRef(this);
            }
        }
        if (pLight == nullptr) {
            it = mLights.erase(it);
        } else {
            ++it;
        }
    }
}

void RndEnviron::Copy(const RndObject *pSource, int nFlags) {
    const RndEnviron *pEnviron =
        pSource != nullptr ? dynamic_cast<const RndEnviron *>(pSource) : nullptr;
    RndDrawable::Copy(pSource, nFlags);
    ReleaseRefs();
    if ((nFlags & kCopyKeepLights) == 0) {
        mLights = pEnviron->mLights;
    }
    mAmbientColor = pEnviron->mAmbientColor;
    mFogStart = pEnviron->mFogStart;
    mFogEnd = pEnviron->mFogEnd;
    mFogDensity = pEnviron->mFogDensity;
    mFogColor = pEnviron->mFogColor;
    mFogMode = pEnviron->mFogMode;
    AcquireRefs();
}

void RndEnviron::Load(BinStream &stream) {
    int nRev;
    stream.ReadEndian(&nRev, sizeof(nRev));
    if (nRev > sRev) {
        DebugNotify("Can't load new Environ");
        return;
    }
    RndDrawable::Load(stream);
    ReleaseRefs();
    stream >> mLights;
    stream.ReadEndian(&mAmbientColor.r, sizeof(mAmbientColor.r));
    stream.ReadEndian(&mAmbientColor.g, sizeof(mAmbientColor.g));
    stream.ReadEndian(&mAmbientColor.b, sizeof(mAmbientColor.b));
    stream.ReadEndian(&mAmbientColor.a, sizeof(mAmbientColor.a));
    stream.ReadEndian(&mFogStart, sizeof(mFogStart));
    stream.ReadEndian(&mFogEnd, sizeof(mFogEnd));
    stream.ReadEndian(&mFogDensity, sizeof(mFogDensity));
    stream.ReadEndian(&mFogColor.r, sizeof(mFogColor.r));
    stream.ReadEndian(&mFogColor.g, sizeof(mFogColor.g));
    stream.ReadEndian(&mFogColor.b, sizeof(mFogColor.b));
    stream.ReadEndian(&mFogColor.a, sizeof(mFogColor.a));
    int nFogMode;
    stream.ReadEndian(&nFogMode, sizeof(nFogMode));
    mFogMode = nFogMode;
    AcquireRefs();
}

void RndEnviron::AddLight(RndLight *pLight) {
    if (std::find(mLights.begin(), mLights.end(), pLight) != mLights.end()) {
        DebugNotify("%s already in %s", pLight->mName.c_str(), mName.c_str());
        return;
    }
    if (pLight != nullptr) {
        pLight->AddRef(this);
    }
    mLights.push_back(pLight);
}

void RndEnviron::CreateDefault() {
    RndObject *pObject = TheManager.Create(sClassName, "[default environ]");
    RndEnviron *pEnviron = pObject != nullptr ? dynamic_cast<RndEnviron *>(pObject) : nullptr;
    sDefault = pEnviron;
    pEnviron->mInternal = 1;
    RndCam::sDefault->AddDraw(pEnviron, nullptr);
}

void RndEnviron::ReleaseRefs() {
    for (RndLight *pLight : mLights) {
        if (pLight != nullptr) {
            pLight->RemoveRef(this);
        }
    }
}

void RndEnviron::AcquireRefs() {
    for (RndLight *pLight : mLights) {
        if (pLight != nullptr) {
            pLight->AddRef(this);
        }
    }
}

PrnStream &operator<<(PrnStream &stream, RndEnviron::FogMode eMode) {
    switch (eMode) {
    case RndEnviron::kFogNone:
        stream << "None";
        break;
    case RndEnviron::kFogVertExp:
        stream << "VertExp";
        break;
    case RndEnviron::kFogVertExp2:
        stream << "VertExp2";
        break;
    case RndEnviron::kFogVertLinear:
        stream << "VertLinear";
        break;
    case RndEnviron::kFogPixelExp:
        stream << "PixelExp";
        break;
    case RndEnviron::kFogPixelExp2:
        stream << "PixelExp2";
        break;
    case RndEnviron::kFogPixelLinear:
        stream << "PixelLinear";
        break;
    }
    return stream;
}
