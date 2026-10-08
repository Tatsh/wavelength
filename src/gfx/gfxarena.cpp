#include "gfx/gfxarena.h"

#include <algorithm>

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/constructo.h"
#include "gfx/gfxmanager.h"
#include "math/vector2.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/cam.h"
#include "rnd/manager.h"

namespace {

// The camera paths are made from this tick, two bars before the song.
constexpr float kPathStartTick = -7680.0f;

// The ticks and lengths of an arena before its configuration gives them.
constexpr float kUnset = -1e9f;
constexpr float kNoFreestyle = 1e9f;
constexpr int kNoUnlockIndex = -1;

// A song this short or shorter leaves the camera path as it was made.
constexpr float kNoSongTicks = 1.0f;

// The stage of a movie material that has not found its texture.
constexpr char kNoStage = -1;

// Each constraint of `freestyle_constraints` ramps the freestyle in and out over this many frames.
constexpr float kFreestyleRamp = 0.1f;

constexpr int kMaxPathLength = 128;

// The time PollLoad() allows the trigger stream, which is never reached.
constexpr int kStreamTimeout = 999999999;

// The beat events of HandleBeat().
constexpr signed char kBeatStart = 'A';
constexpr signed char kBeatEnd = 'B';

// The panel flags ResetPanels() clears.
constexpr char kAllPanelFlags =
    GfxArena::kPanelLowEnergy | GfxArena::kPanelBeat | GfxArena::kPanelFrequency;

// The configuration nodes of an `arena_paths` entry.
constexpr int kFileNode = 1;
constexpr int kTriggersNode = 2;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

// Insert a key where AtFrame() places its frame, after any key of the same frame.
inline void AddKey(std::vector<Key<float>> &keys, float fValue, float fFrame) {
    const Key<float> *pPrev;
    const Key<float> *pNext;
    float fRatio;
    const int nIndex = AtFrame(keys, fFrame, pPrev, pNext, fRatio);
    keys.insert(keys.begin() + nIndex, Key<float>{fValue, fFrame});
}

// Stretch the frames of keys by a scale and an offset.
template <class K>
void FitKeys(std::list<K> &keys, float fScale, float fOffset) {
    for (K &key : keys) {
        key.mFrame = (key.mFrame * fScale) + fOffset;
    }
}

} // namespace

GfxArena::Creator GfxArena::sCreators[] = {Constructo::Create, nullptr};
float GfxArena::sLowEnergy = 0.16666f;

GfxArena::GfxArena(const char *pszName) : mName(pszName) {
    mPanelFlags = 0;
    mUnlockIndex = kNoUnlockIndex;
    mFrameScale = 1.0f;
    mCamPathLength = kUnset;
    mFreestyle = kNoFreestyle;
    mLoaded = 0;
    mTriggersLoaded = 0;
    mTriggerStream = nullptr;
    mLoader = nullptr;
    mView = nullptr;
    mOwnsView = 0;
    mCamPath = nullptr;
    mFrameOffset = 0.0f;
    mLength = kUnset;
    mSongTicks = kUnset;
    mEnviron = nullptr;
    mResultShown = 0;
    mWon = 0;
    mBeatTick = kUnset;

    DataArray *pGfx = SystemConfig()->FindArray("gfx", true);
    mConfig = pGfx->FindArray("arena_paths", true)->FindArray(pszName, true);
    mConfig->FindFloat("length", &mLength, true);
    mConfig->FindFloat("path_length", &mCamPathLength, true);
    mConfig->FindInt("path", &mUnlockIndex, true);
    Vector2 blinkRange;
    pGfx->FindVector("juice_blink_range", &blinkRange, true);
    sLowEnergy = blinkRange.y;
    if (mConfig->Sym(kFileNode)[0] == '\0') {
        mOwnsView = 1;
        mView = dynamic_cast<Rnd::View *>(
            Rnd::TheManager.Create(Rnd::g_viewClassName.mStr, "arena view placeholder"));
        mLoaded = 1;
    }

    DataArray *pConstraints = mConfig->FindArray("freestyle_constraints", false);
    if (pConstraints == nullptr) {
        return;
    }
    mFreestyle = 0.0f;
    for (int i = 1; i < pConstraints->Size(); ++i) {
        DataArray *pConstraint = pConstraints->Array(i);
        const float fStart = pConstraint->Float(0);
        const float fEnd = pConstraint->Float(1);
        AddKey(mFreestyleKeys, 0.0f, fStart);
        AddKey(mFreestyleKeys, 1.0f, fStart + kFreestyleRamp);
        AddKey(mFreestyleKeys, 1.0f, fEnd - kFreestyleRamp);
        AddKey(mFreestyleKeys, 0.0f, fEnd);
    }
    if (mFreestyleKeys.empty()) {
        AddKey(mFreestyleKeys, 0.0f, 0.0f);
    }
}

GfxArena *GfxArena::Create(const char *pszName) {
    for (Creator *pCreator = sCreators; *pCreator != nullptr; ++pCreator) {
        GfxArena *pArena = (*pCreator)(pszName);
        if (pArena != nullptr) {
            return pArena;
        }
    }
    return new GfxArena(pszName);
}

GfxArena::~GfxArena() {
    if (mOwnsView && (mView != nullptr)) {
        delete mView;
    }
    delete mTriggerStream;
}

void GfxArena::LoadConfig() {
    DataArray *pGfx = SystemConfig()->FindArray("gfx", true);
    mConfig = pGfx->FindArray("arena_paths", true)->FindArray(mName.c_str(), true);
    if (mConfig->Sym(kTriggersNode)[0] != '\0') {
        TheTriggerMgr.Load(mConfig->Sym(kTriggersNode), nullptr);
    }
    Vector2 blinkRange;
    pGfx->FindVector("juice_blink_range", &blinkRange, true);
    sLowEnergy = blinkRange.y;
}

void GfxArena::LoadTriggers() {
    if (mConfig->Sym(kTriggersNode)[0] == '\0') {
        mTriggersLoaded = 1;
        return;
    }
    char szPath[kMaxPathLength];
    DataArray::MakeCompiledPath(szPath, mConfig->Sym(kTriggersNode), false);
    mTriggerStream = new AsyncStream(szPath, true);
}

void GfxArena::PollLoad(bool bHudLoaded) {
    if (!mLoaded && (mLoader != nullptr) && mLoader->IsLoaded()) {
        for (Rnd::Object *pObject : mLoader->mObjects) {
            if (mEnviron == nullptr) {
                mEnviron = dynamic_cast<Rnd::Environ *>(pObject);
            }
        }
        Rnd::Cam *pCam = FindObject<Rnd::Cam>("outer 1.cam");
        if ((pCam != nullptr) && (mCamPath == nullptr)) {
            for (Rnd::Object *pRef : pCam->mRefs) {
                Rnd::TransAnim *pAnim = dynamic_cast<Rnd::TransAnim *>(pRef);
                if ((pAnim == nullptr) || (pAnim->mTrans != pCam)) {
                    continue;
                }
                mCamPath = pAnim;
                pAnim->SetTrans(nullptr);
                Transform identity{Vector3{1.0f, 0.0f, 0.0f},
                                   Vector3{0.0f, 1.0f, 0.0f},
                                   Vector3{0.0f, 0.0f, 1.0f},
                                   Vector3{0.0f, 0.0f, 0.0f, 1.0f}};
                pCam->SetLocalXfm(identity);
                break;
            }
        }
        if (mView == nullptr) {
            mView = FindObject<Rnd::View>("arena top.view");
        }
        mSnow = FindObject<Rnd::Movie>("snow.ipu");
        mMovies.resize(kNumMovies, nullptr);
        for (int i = 0; i < kNumMovies; ++i) {
            mMovies[i] = FindObject<Rnd::Movie>(FormatString("movie%d.mov", i + 1));
            CollectMovieMats(mMovies[i]->mTex, &mMovieMats);
            CollectMatAnims(mMovies[i]->mTex, &mMatAnims);
            DetachAnim(mMovies[i]);
        }
        for (Rnd::MatAnim *pAnim : mMatAnims) {
            DetachAnim(pAnim);
        }
        mLoaded = 1;
    }
    if (mTriggersLoaded || (mTriggerStream == nullptr)) {
        return;
    }
    if (!mTriggerStream->Ready(kStreamTimeout) || !bHudLoaded) {
        return;
    }
    TheTriggerMgr.Load(mConfig->Sym(kTriggersNode), mTriggerStream);
    delete mTriggerStream;
    mTriggersLoaded = 1;
    mTriggerStream = nullptr;
}

void GfxArena::SetCamPath(Rnd::TransAnim *pPath) {
    if (mCamPath == nullptr) {
        mCamPath = pPath;
    }
}

void GfxArena::SetSongTicks(float fSongTicks) {
    if (fSongTicks <= kNoSongTicks) {
        mFrameScale = 1.0f;
        mFrameOffset = 0.0f;
        mSongTicks = 0.0f;
    } else {
        mSongTicks = fSongTicks;
        mFrameScale = (mLength - kPathStartTick) / (fSongTicks - kPathStartTick);
        mFrameOffset = kPathStartTick - (mFrameScale * kPathStartTick);
        if (mCamPath != nullptr) {
            FitCamPath(mCamPath);
        }
    }
    mFrequencyTex = FindObject<Rnd::Tex>("fbase_freq.tex");
}

void GfxArena::Poll(float fTick, float fTime) {
    const float fFrame = (mFrameScale * fTick) + mFrameOffset;
    mView->SetFrame(fFrame);
    TheTriggerMgr.MetagameEvent(fTick, TheGameDb->mSongTime, fTime);
    TheTriggerMgr.Poll();
    mView->UpdateWorldXfm(nullptr, 0);
    const float fSongTick = TheGameDb->mSongTick;
    SetPanelFlags(kPanelFrequency, TheGfxManager.mWinnerDrawn != 0);
    if (mPanelFlags == 0) {
        for (Rnd::MatAnim *pAnim : mMatAnims) {
            pAnim->SetFrame(fSongTick);
        }
    } else if ((mPanelFlags & kPanelBeat) != 0) {
        mMovies[kBeatMovie]->SetFrame(fSongTick - mBeatTick);
    }
    if ((mPanelFlags & kPanelBeat) == 0) {
        for (Rnd::Movie *pMovie : mMovies) {
            pMovie->SetFrame(fTick);
        }
    }
    InterpKeys(mFreestyleKeys, fFrame, mFreestyle);
}

void GfxArena::ResetPanels() {
    SetPanelFlags(kAllPanelFlags, false);
}

void GfxArena::SetEnergy(int, float fEnergy) {
    if (mResultShown && mWon) {
        return;
    }
    SetPanelFlags(kPanelLowEnergy, fEnergy <= sLowEnergy);
}

void GfxArena::ShowResult(bool bWon) {
    mWon = bWon;
    mResultShown = 1;
    if (bWon) {
        SetPanelFlags(kPanelLowEnergy, false);
    }
}

void GfxArena::HandleBeat(signed char nEvent) {
    if (mMovies.empty()) {
        return;
    }
    if (nEvent == kBeatStart) {
        SetPanelFlags(kPanelBeat, true);
        mBeatTick = TheGameDb->mSongTick;
        mMovies[kBeatMovie]->SetFrame(0.0f);
    } else if (nEvent == kBeatEnd) {
        SetPanelFlags(kPanelBeat, false);
    }
}

void GfxArena::CollectMovieMats(Rnd::Tex *pTex, std::list<MovieMat> *pMats) {
    for (Rnd::Object *pRef : pTex->mRefs) {
        Rnd::Mat *pMat = (pRef != nullptr) ? dynamic_cast<Rnd::Mat *>(pRef) : nullptr;
        if (pMat == nullptr) {
            continue;
        }
        // Yes, the binary discards the result of the search and adds the stage again.
        (void)std::find_if(
            pMats->begin(), pMats->end(), [pMat](const MovieMat &mat) { return mat.mMat == pMat; });
        pMats->push_back(MovieMat{pMat, pTex, kNoStage});
        MovieMat &added = pMats->back();
        for (unsigned int i = 0; i < pMat->mStages.size(); ++i) {
            if (pMat->mStages[i].mTex == pTex) {
                added.mStage = static_cast<char>(i);
                break;
            }
        }
    }
}

void GfxArena::CollectMatAnims(Rnd::Tex *pTex, std::list<Rnd::MatAnim *> *pAnims) {
    for (Rnd::Object *pRef : pTex->mRefs) {
        Rnd::MatAnim *pAnim = (pRef != nullptr) ? dynamic_cast<Rnd::MatAnim *>(pRef) : nullptr;
        if (pAnim == nullptr) {
            continue;
        }
        if (std::find(pAnims->begin(), pAnims->end(), pAnim) != pAnims->end()) {
            continue;
        }
        for (const Rnd::MatAnim::Stage &stage : pAnim->mKeysOwner->mStages) {
            for (const Rnd::MatAnim::Stage::TexKey &key : stage.mTexKeys) {
                if (key.mValue == pTex) {
                    pAnims->push_back(pAnim);
                    break;
                }
            }
        }
    }
}

void GfxArena::DetachAnim(Rnd::Animatable *pAnim) {
    bool bRemoved = true;
    while (bRemoved) {
        bRemoved = false;
        for (Rnd::Object *pRef : pAnim->mRefs) {
            Rnd::Animatable *pParent =
                (pRef != nullptr) ? dynamic_cast<Rnd::Animatable *>(pRef) : nullptr;
            if (pParent == nullptr) {
                continue;
            }
            if (std::find(pParent->mAnims.begin(), pParent->mAnims.end(), pAnim) !=
                pParent->mAnims.end()) {
                pParent->RemoveAnim(pAnim);
                bRemoved = true;
                break;
            }
        }
    }
}

void GfxArena::FitCamPath(Rnd::TransAnim *pPath) {
    const float fScale = (mSongTicks - kPathStartTick) / (mLength - kPathStartTick);
    const float fOffset = kPathStartTick - (fScale * kPathStartTick);
    Rnd::TransAnim *pOwner = pPath->GetFramesOwner();
    FitKeys(pOwner->mTransKeys, fScale, fOffset);
    FitKeys(pOwner->mRotKeys, fScale, fOffset);
    FitKeys(pOwner->mScaleKeys, fScale, fOffset);
}

void GfxArena::SetPanelFlags(char nFlags, bool bOn) {
    if (mMovies.empty()) {
        return;
    }
    const char nOld = mPanelFlags;
    mPanelFlags =
        bOn ? static_cast<char>(mPanelFlags | nFlags) : static_cast<char>(mPanelFlags & ~nFlags);
    if (nOld == mPanelFlags) {
        return;
    }
    Rnd::Tex *pTex;
    if ((mPanelFlags & kPanelFrequency) != 0) {
        pTex = mFrequencyTex;
    } else if ((mPanelFlags & kPanelLowEnergy) != 0) {
        pTex = mSnow->mTex;
    } else if ((mPanelFlags & kPanelBeat) != 0) {
        pTex = mMovies[kBeatMovie]->mTex;
    } else {
        pTex = nullptr;
    }
    for (const MovieMat &mat : mMovieMats) {
        ShowTexture(&mat, pTex);
    }
}

void GfxArena::ShowTexture(const MovieMat *pMat, Rnd::Tex *pTex) {
    if (pTex == nullptr) {
        pTex = pMat->mTex;
    }
    pMat->mMat->mStages[pMat->mStage].SetTex(pTex);
}
