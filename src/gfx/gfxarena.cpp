#include "gfx/gfxarena.h"

#include <algorithm>
#include <vector>

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/constructo.h"
#include "gfx/gfxmanager.h"
#include "math/key.h"
#include "math/vector2.h"
#include "os/string.h"
#include "os/system.h"
#include "os/timer.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"

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
    return dynamic_cast<T *>(TheManager.Find(pszName));
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
void FitKeys(std::vector<K> &keys, float fScale, float fOffset) {
    for (K &key : keys) {
        key.frame = (key.frame * fScale) + fOffset;
    }
}

} // namespace

// The static initialiser of the unit, and the routine that runs it.
// NTSC-U/C: 0x001ece50, PAL: 0x001f5bf0
// NTSC-U/C: 0x001ece88, PAL: 0x001f5c28

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
    // Yes, the timer is never read.
    Timer timer;
    timer.Start();

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
        mView = dynamic_cast<RndView *>(
            TheManager.Create(RndView::sClassName, "arena view placeholder"));
        mLoaded = 1;
    }

    DataArray *pConstraints = mConfig->FindArray("freestyle_constraints", true);
    // Yes, the binary tests the result of a required lookup.
    if (pConstraints != nullptr) {
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
    timer.Stop();
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
        // Yes, the timer is never read.
        Timer timer;
        timer.Start();
        for (RndObject *pObject : mLoader->mObjects) {
            if (mEnviron == nullptr) {
                mEnviron = dynamic_cast<RndEnviron *>(pObject);
            }
        }
        RndCam *pCam = FindObject<RndCam>("outer 1.cam");
        if ((pCam != nullptr) && (mCamPath == nullptr)) {
            for (RndObject *pRef : pCam->mRefs) {
                RndTransAnim *pAnim = dynamic_cast<RndTransAnim *>(pRef);
                if ((pAnim == nullptr) || (pAnim->mTrans != pCam)) {
                    continue;
                }
                mCamPath = pAnim;
                pAnim->SetTrans(nullptr);
                Transform identity{Vector3{1.0f, 0.0f, 0.0f},
                                   Vector3{0.0f, 1.0f, 0.0f},
                                   Vector3{0.0f, 0.0f, 1.0f},
                                   Vector3{0.0f, 0.0f, 0.0f, 1.0f}};
                pCam->mLocalXfm = identity;
                pCam->mDirty = 1;
                break;
            }
        }
        if (mView == nullptr) {
            mView = FindObject<RndView>("arena top.view");
        }
        mSnow = FindObject<RndMovie>("snow.ipu");
        mMovies.resize(kNumMovies, nullptr);
        for (int i = 0; i < kNumMovies; ++i) {
            mMovies[i] = FindObject<RndMovie>(FormatString("movie%d.mov", i + 1));
            CollectMovieMats(mMovies[i]->mTex, &mMovieMats);
            CollectMatAnims(mMovies[i]->mTex, &mMatAnims);
            DetachAnim(mMovies[i]);
        }
        for (RndMatAnim *pAnim : mMatAnims) {
            DetachAnim(pAnim);
        }
        timer.Stop();
        mLoaded = 1;
    }
    if (mTriggersLoaded || (mTriggerStream == nullptr)) {
        return;
    }
    if (!mTriggerStream->Ready(kStreamTimeout) || !bHudLoaded) {
        return;
    }
    // Yes, the timer is never read.
    Timer timer;
    timer.Start();
    TheTriggerMgr.Load(mConfig->Sym(kTriggersNode), mTriggerStream);
    delete mTriggerStream;
    mTriggersLoaded = 1;
    mTriggerStream = nullptr;
    timer.Stop();
}

void GfxArena::SetCamPath(RndTransAnim *pPath) {
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
    mFrequencyTex = FindObject<RndTex>("fbase_freq.tex");
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
        for (RndMatAnim *pAnim : mMatAnims) {
            pAnim->SetFrame(fSongTick);
        }
    } else if ((mPanelFlags & kPanelBeat) != 0) {
        mMovies[kBeatMovie]->SetFrame(fSongTick - mBeatTick);
    }
    if ((mPanelFlags & kPanelBeat) == 0) {
        for (RndMovie *pMovie : mMovies) {
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

void GfxArena::CollectMovieMats(RndTex *pTex, std::list<MovieMat> *pMats) {
    for (RndObject *pRef : pTex->mRefs) {
        RndMat *pMat = (pRef != nullptr) ? dynamic_cast<RndMat *>(pRef) : nullptr;
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

void GfxArena::CollectMatAnims(RndTex *pTex, std::list<RndMatAnim *> *pAnims) {
    for (RndObject *pRef : pTex->mRefs) {
        RndMatAnim *pAnim = (pRef != nullptr) ? dynamic_cast<RndMatAnim *>(pRef) : nullptr;
        if (pAnim == nullptr) {
            continue;
        }
        if (std::find(pAnims->begin(), pAnims->end(), pAnim) != pAnims->end()) {
            continue;
        }
        for (const RndMatAnim::Stage &stage : pAnim->mKeysOwner->mStages) {
            for (const Key<RndTex *> &key : stage.mTexKeys) {
                if (key.value == pTex) {
                    pAnims->push_back(pAnim);
                    break;
                }
            }
        }
    }
}

void GfxArena::DetachAnim(RndAnimatable *pAnim) {
    bool bRemoved = true;
    while (bRemoved) {
        bRemoved = false;
        for (RndObject *pRef : pAnim->mRefs) {
            RndAnimatable *pParent =
                (pRef != nullptr) ? dynamic_cast<RndAnimatable *>(pRef) : nullptr;
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

void GfxArena::FitCamPath(RndTransAnim *pPath) {
    const float fScale = (mSongTicks - kPathStartTick) / (mLength - kPathStartTick);
    const float fOffset = kPathStartTick - (fScale * kPathStartTick);
    RndTransAnim *pOwner = pPath->mFramesOwner;
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
    RndTex *pTex;
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

void GfxArena::ShowTexture(const MovieMat *pMat, RndTex *pTex) {
    if (pTex == nullptr) {
        pTex = pMat->mTex;
    }
    pMat->mMat->mStages[pMat->mStage].SetTex(pTex);
}
