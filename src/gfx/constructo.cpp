#include "gfx/constructo.h"

#include <algorithm>
#include <cstring>

#include "os/debug.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "rnd/rndview.h"

namespace {

// The building parts are `buildV08_01.part` to `buildV08_03.part`.
constexpr int kFirstPart = 1;
constexpr int kEndPart = 4;

// The configuration node of the file of an arena.
constexpr int kFileNode = 1;

// The axes MovingPSPlane::Update() takes as the normal.
constexpr char kAxisX = 'x';
constexpr char kAxisY = 'y';

constexpr int kPrefixLength = 10;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

// The static initialiser of the unit, and the routine that runs it.
// NTSC-U/C: 0x001ee090, PAL: 0x001f6e30
// NTSC-U/C: 0x001ee0c8, PAL: 0x001f6e68

Constructo::Constructo(const char *pszName) : GfxArena(pszName) {
    if (std::strcmp(pszName, "ConstructoP1") == 0) {
        mLevel = kLevelP1;
    } else if (std::strcmp(pszName, "ConstructoP2") == 0) {
        mLevel = kLevelP2;
    } else if (std::strcmp(pszName, "ConstructoP3") == 0) {
        mLevel = kLevelP3;
    } else if (std::strcmp(pszName, "ConstructoP4") == 0) {
        mLevel = kLevelP4;
    } else if (std::strcmp(pszName, "Constructo_Boss") == 0) {
        mLevel = kLevelBoss;
    } else {
        DebugWarn("unrecognized constructo arena name: %s", pszName);
    }
}

GfxArena *Constructo::Create(const char *pszName) {
    if (std::strncmp(pszName, "Constructo", kPrefixLength) != 0) {
        return nullptr;
    }
    return new Constructo(pszName);
}

Constructo::~Constructo() = default;

void Constructo::SetSongTicks(float fSongTicks) {
    GfxArena::SetSongTicks(fSongTicks);
    if (mLevel == kLevelBoss) {
        return;
    }
    RndView *pView = FindObject<RndView>("Group_buildingV08.view");
    if (pView == nullptr) {
        DebugPrint("Constructo warning: couldn't find custom collision\n"
                   "anim parent Group_buildingV08.view in\n%s",
                   mConfig->Sym(kFileNode));
        return;
    }
    for (int i = kFirstPart; i < kEndPart; ++i) {
        String name(FormatString("buildV08_%02d.part", i));
        RndParticleSys *pParticles = FindObject<RndParticleSys>(name.c_str());
        if (pParticles == nullptr) {
            DebugPrint("Constructo warning: couldn't find custom collision\nParticleSys %s in\n%s",
                       name.c_str(),
                       mConfig->Sym(kFileNode));
            return;
        }
        name = FormatString("collideBall%02d.mesh", i);
        RndTransformable *pPlane = FindObject<RndTransformable>(name.c_str());
        if (pPlane == nullptr) {
            DebugPrint(
                "Constructo warning: couldn't find custom collision\nTransformable %s in\n%s",
                name.c_str(),
                mConfig->Sym(kFileNode));
            return;
        }
        if (std::find(pView->mAnims.begin(), pView->mAnims.end(), pParticles) ==
            pView->mAnims.end()) {
            DebugNotify("couldn't find %s in\nanim list of %s, in file\n%s",
                        pParticles->mName.c_str(),
                        pView->mName.c_str(),
                        mConfig->Sym(kFileNode));
            continue;
        }
        pView->RemoveAnim(pParticles);
        pParticles->mCollide = 1;
        mPlanes.push_back(MovingPSPlane{pParticles, pPlane, kAxisY});
    }
}

void Constructo::Poll(float fTick, float fTime) {
    GfxArena::Poll(fTick, fTime);
    // Yes, the binary scales the tick without the frame offset of the camera path.
    const float fFrame = fTick * mFrameScale;
    for (MovingPSPlane &plane : mPlanes) {
        plane.Update(fFrame);
    }
}

void Constructo::MovingPSPlane::Update(float fFrame) {
    const Vector3 *pNormal;
    if (mAxis == kAxisX) {
        pNormal = &mPlane->mWorldXfm.mBasisX;
    } else if (mAxis == kAxisY) {
        pNormal = &mPlane->mWorldXfm.mBasisY;
    } else {
        pNormal = &mPlane->mWorldXfm.mBasisZ;
    }
    const Vector3 &origin = mPlane->mWorldXfm.mTranslation;
    mParticles->mCollidePlane =
        Plane{pNormal->x,
              pNormal->y,
              pNormal->z,
              -((pNormal->x * origin.x) + (pNormal->y * origin.y) + (pNormal->z * origin.z))};
    mParticles->SetFrame(fFrame);
}
