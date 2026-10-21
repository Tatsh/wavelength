#include "gfx/tnlfire.h"

#include <algorithm>

#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "gfx/tnlgeom.h"
#include "math/rand.h"
#include "math/transformops.h"
#include "os/string.h"
#include "rnd/rndmanager.h"
#include "script/dataarray.h"

namespace {

// The rows of a basis.
constexpr int kBasisX = 0;
constexpr int kBasisY = 1;
constexpr int kBasisZ = 2;

// The tick of a chunk or a fire that has not started, and the tick of one that never comes.
constexpr float kUnsetTick = -1e9f;
constexpr float kNeverTick = 1e9f;

// The place across a track, and between the outer tracks, of the middle.
constexpr float kMiddle = 0.5f;

// A gem the head passes is one at most this many ticks behind the next chunk, a bar less a tick.
constexpr float kGemWindowTicks = 1919.0f;

// The gem ticks of one capture the fire makes room for at the start.
constexpr int kGemTicksReserve = 12;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(pszName));
}

} // namespace

float TnlFire::sCaptureSpeed = 4.0f;
float TnlFire::sAutoCaptureSpeed = 64.0f;
float TnlFire::sPanelSize = 1.0f;
int TnlFire::sMaxChunks = 3;
float TnlFire::sChunkYDisplace = 0.5f;
int TnlFire::sChunksLit = 1;
std::vector<Vector3> TnlFire::sChunkScales;
Vector2 TnlFire::sCaptureInterval{120.0f, 120.0f};
Vector2 TnlFire::sAutoCaptureInterval{120.0f, 120.0f};
Vector3 TnlFire::sChunkForce{0.0f, 0.0f, -1e-6f};
Vector3 TnlFire::sCaptureVelocity[] = {{0.0f, 0.0f, 1e-4f}, {0.0f, 0.0f, 1e-4f}};
Vector3 TnlFire::sAutoCaptureVelocity[] = {{0.0f, 0.0f, 1e-4f}, {0.0f, 0.0f, 1e-4f}};
Vector2 TnlFire::sChunkLifespan{480.0f, 960.0f};

TnlFire::Emitter::Emitter() : mSys(nullptr) {
}

TnlFire::Emitter::~Emitter() {
    if (mSys != nullptr) {
        mSys->GetEmitRate().x = mEmitRateLow;
        mSys->GetEmitRate().y = mEmitRateHigh;
        mSys->SetForce(mForce);
    }
}

void TnlFire::Emitter::Init(RndParticleSys *pSys, GfxTunnel *) {
    mSys = pSys;
    const float flScale = sPanelSize;
    mEmitRateLow = pSys->GetEmitRate().x;
    mEmitRateHigh = pSys->GetEmitRate().y;
    mForce.x = pSys->GetForce().x * flScale;
    mForce.z = pSys->GetForce().z * flScale;
    mForce.y = pSys->GetForce().y * flScale;
    mSys->GetSize().x *= flScale;
    mSys->GetSize().y *= flScale;
    mSys->GetSpeed().x *= flScale;
    mSys->GetSpeed().y *= flScale;
    Vector3 &low = mSys->GetPosLow();
    Vector3 &high = mSys->GetPosHigh();
    low.x *= flScale;
    low.y *= flScale;
    low.z *= flScale;
    high.x *= flScale;
    high.y *= flScale;
    high.z *= flScale;
    mSys->SetShowing(1);
    Stop();
    mSys->FreeAllParticles();
}

void TnlFire::Emitter::SetColor(const Color *pColor) {
    if (mSys == nullptr) {
        return;
    }
    Color color = *pColor;
    color.a = 1.0f;
    mSys->SetStartColorLow(Color{1.0f, 1.0f, 1.0f, 1.0f});
    mSys->SetStartColorHigh(color);
    color.a = 0.0f;
    mSys->SetEndColorLow(color);
    mSys->SetEndColorHigh(color);
}

void TnlFire::Emitter::Rotate(const Vector3 *pBasis) {
    if (mSys == nullptr) {
        return;
    }
    const Vector3 &x = pBasis[kBasisX];
    const Vector3 &y = pBasis[kBasisY];
    const Vector3 &z = pBasis[kBasisZ];
    Vector3 force = mForce;
    force.x = (x.x * mForce.x) + (y.x * mForce.y) + (z.x * mForce.z);
    force.y = (x.y * mForce.x) + (y.y * mForce.y) + (z.y * mForce.z);
    force.z = (x.z * mForce.x) + (y.z * mForce.y) + (z.z * mForce.z);
    mSys->SetForce(force);
}

void TnlFire::Emitter::Start() {
    if (mSys == nullptr) {
        return;
    }
    mSys->FreeAllParticles();
    mSys->GetEmitRate().y = mEmitRateHigh;
    mSys->GetEmitRate().x = mEmitRateLow;
}

void TnlFire::Emitter::Stop() {
    if (mSys == nullptr) {
        return;
    }
    mSys->GetEmitRate().y = 0.0f;
    mSys->GetEmitRate().x = 0.0f;
}

TnlFire::ChunkModel::~ChunkModel() = default;

void TnlFire::ChunkModel::CreateMultiMesh() {
    mMultiMesh.reset(dynamic_cast<RndMultiMesh *>(TheManager.Create(
        RndMultiMesh::sClassName, FormatString("tnlfire chunk 0x%08x.mm", this))));
    mMultiMesh->SetMesh(mMesh);
}

TnlFire::Chunk::Chunk() : mModel(nullptr), mStartTick(kUnsetTick) {
}

void TnlFire::Chunk::Spawn(ChunkModel *pModel,
                           int bAuto,
                           int nTrack,
                           const Transform *pCellXfm,
                           const Transform *,
                           std::list<Transform> *pFreeXfms,
                           float flTick) {
    mModel = pModel;
    const float flDisplace = sChunkYDisplace;
    Vector3 &pos = mXfm.mTranslation;
    pos.x = pCellXfm->mBasisY.x * flDisplace;
    pos.z = pCellXfm->mBasisY.z * flDisplace;
    pos.y = pCellXfm->mBasisY.y * flDisplace;
    pos.x += pCellXfm->mTranslation.x;
    pos.z += pCellXfm->mTranslation.z;
    pos.y += pCellXfm->mTranslation.y;

    const Vector3 *pRange = bAuto ? sAutoCaptureVelocity : sCaptureVelocity;
    const float flX = RandomFloat(pRange[kRangeMin].x, pRange[kRangeMax].x);
    const float flY = RandomFloat(pRange[kRangeMin].y, pRange[kRangeMax].y);
    const float flZ = RandomFloat(pRange[kRangeMin].z, pRange[kRangeMax].z);
    mVelocity.x = flX;
    mVelocity.y = flY;
    mVelocity.z = flZ;
    MultiplyVector(*pCellXfm, mVelocity, mVelocity);

    mStartTick = flTick;
    mEndTick = flTick + RandomFloat(sChunkLifespan.x, sChunkLifespan.y);

    const Vector3 &scale = sChunkScales[nTrack];
    mXfm.mBasisX.x = pCellXfm->mBasisX.x * scale.x;
    mXfm.mBasisX.z = pCellXfm->mBasisX.z * scale.x;
    mXfm.mBasisX.y = pCellXfm->mBasisX.y * scale.x;
    mXfm.mBasisY.x = pCellXfm->mBasisY.x * scale.y;
    mXfm.mBasisY.z = pCellXfm->mBasisY.z * scale.y;
    mXfm.mBasisY.y = pCellXfm->mBasisY.y * scale.y;
    mXfm.mBasisZ.x = pCellXfm->mBasisZ.x * scale.z;
    mXfm.mBasisZ.z = pCellXfm->mBasisZ.z * scale.z;
    mXfm.mBasisZ.y = pCellXfm->mBasisZ.y * scale.z;

    std::list<Transform> &instances = pModel->mMultiMesh->GetTransforms();
    instances.splice(instances.begin(), *pFreeXfms, pFreeXfms->begin());
    mInstance = instances.begin();
}

void TnlFire::Chunk::Reset(std::list<Transform> *pFreeXfms) {
    pFreeXfms->splice(pFreeXfms->end(), mModel->mMultiMesh->GetTransforms(), mInstance);
    mModel = nullptr;
    mStartTick = kUnsetTick;
}

bool TnlFire::Chunk::Update(float flTicks) {
    const float flTick = TheGameDb->GetSongTick();
    if (mEndTick <= flTick) {
        return true;
    }
    mVelocity.x += sChunkForce.x * flTicks;
    mVelocity.y += sChunkForce.y * flTicks;
    mVelocity.z += sChunkForce.z * flTicks;
    Vector3 &pos = mXfm.mTranslation;
    pos.x += mVelocity.x * flTicks;
    pos.y += mVelocity.y * flTicks;
    pos.z += mVelocity.z * flTicks;

    RndTransAnim *pAnim = mModel->mTransAnim;
    Transform tumble;
    pAnim->MakeTransform(pAnim->FilterFrame(flTick - mStartTick), tumble, 1);
    sceVu0MulAffineMatrixXyz(&mInstance->mBasisX.x, &mXfm.mBasisX.x, &tumble.mBasisX.x);
    return false;
}

TnlFire::TnlFire(const char *pszName, int nPlayer, GfxTunnel *pTunnel, bool bChunks)
    : mTunnel(pTunnel), mStartTick(0.0f), mEndTick(kUnsetTick), mSpeed(0.0f), mTrack(-1),
      mTick(kUnsetTick), mActive(0), mAuto(0), mPlayer(nPlayer) {
    if (bChunks) {
        DataArray *pModeConfig = GetModeGfxConfig();
        DataArray *pChunks;
        FindConfigArray(pModeConfig, GetGfxConfig(), "capture_chunks", &pChunks, true);
        mModels.resize(pChunks->Size() - 1);
        mTotalWeight = 0.0f;
        if ((TheGameDb->GetCommunity() == GameDb::kCommunitySolo) ||
            (TheGameDb->GetRuleSet() != GameDb::kRuleSetGame)) {
            mMat = FindObject<RndMat>("panelchunk nrg.mat");
        } else {
            mMat = FindObject<RndMat>("panelchunk multi.mat");
        }
        for (int i = 1; i < pChunks->Size(); ++i) {
            ChunkModel &model = mModels[i - 1];
            DataArray *pEntry = pChunks->Array(i);
            model.mWeight = pEntry->Float(0);
            mTotalWeight += model.mWeight;
            model.mMesh = FindObject<RndMesh>(pEntry->Sym(1));
            model.mTransAnim = FindObject<RndTransAnim>(pEntry->Sym(2));
            model.CreateMultiMesh();
            model.mMesh->SetMat(mMat);
        }
        for (int i = sMaxChunks; i > 0; --i) {
            mFreeChunks.push_back(Chunk());
            mFreeXfms.push_back(Transform());
        }
    }
    mView = FindObject<RndView>(FormatString("%s.view", pszName));
    mInner.Init(FindObject<RndParticleSys>(FormatString("%s.ps", pszName)), pTunnel);
    if (nPlayer == kChunkPlayer) {
        mOuter.Init(FindObject<RndParticleSys>(FormatString("%sa.ps", pszName)), pTunnel);
    }
    mGemTicks.reserve(kGemTicksReserve);
}

void TnlFire::Disable(const char *pszName, RndView *pParent) {
    RndView *pView = FindObject<RndView>(FormatString("%s.view", pszName));
    RndParticleSys *pSys = FindObject<RndParticleSys>(FormatString("%s.ps", pszName));
    pView->ClearDraws();
    pView->ClearTrans();
    pView->RemoveAllAnims();
    pParent->RemoveDraw(pView);
    pSys->SetNumParticles(0);
}

bool TnlFire::Start(int nPlayer,
                    int nTrack,
                    int bAuto,
                    const Color *pInnerColor,
                    const Color *pOuterColor,
                    const Color *pTrackColor,
                    const std::vector<float> *pGemTicks,
                    float flTick,
                    float flStartTick,
                    float flEndTick) {
    if (mActive || (nPlayer != mPlayer) || !mChunks.empty()) {
        return false;
    }
    mAuto = bAuto;
    if (bAuto) {
        mSpeed = sAutoCaptureSpeed;
        mInterval = sAutoCaptureInterval;
    } else {
        mSpeed = sCaptureSpeed;
        mInterval = sCaptureInterval;
    }
    mTick = flTick;
    mNextChunkTick = (mPlayer == kChunkPlayer) ? flStartTick : kNeverTick;
    mStartTick = flStartTick;
    mEndTick = flEndTick;
    mTrack = nTrack;
    mTrackColor = *pTrackColor;
    if (!bAuto) {
        mInner.SetColor(pInnerColor);
        mOuter.SetColor(pOuterColor);
        const Vector3 upright[] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
        mInner.Rotate(upright);
        mOuter.Rotate(upright);
        mInner.Start();
        mOuter.Start();
    }
    mActive = 1;
    if (pGemTicks != nullptr) {
        mGemTicks.resize(pGemTicks->size(), 0.0f);
        std::copy(pGemTicks->begin(), pGemTicks->end(), mGemTicks.begin());
    } else {
        mGemTicks.clear();
        mTunnel->CollectGemTicks(&mGemTicks, static_cast<char>(nTrack), flStartTick, flEndTick);
    }
    return true;
}

void TnlFire::Reset(bool bClearChunks) {
    mInner.Stop();
    mOuter.Stop();
    if (bClearChunks) {
        for (std::list<Chunk>::iterator it = mChunks.begin(); it != mChunks.end();) {
            std::list<Chunk>::iterator next = it;
            ++next;
            it->Reset(&mFreeXfms);
            mFreeChunks.splice(mFreeChunks.begin(), mChunks, it);
            it = next;
        }
    }
    mActive = 0;
}

float TnlFire::StartTick(int nPlayer) const {
    return (nPlayer == mPlayer) ? mStartTick : kNeverTick;
}

void TnlFire::Update(float flTicks, float) {
    const float flTick = TheGameDb->GetSongTick();
    mView->SetFrame(TheGameDb->GetSongTime());
    for (std::list<Chunk>::iterator it = mChunks.begin(); it != mChunks.end();) {
        if (it->Update(flTicks)) {
            std::list<Chunk>::iterator next = it;
            ++next;
            it->Reset(&mFreeXfms);
            mFreeChunks.splice(mFreeChunks.begin(), mChunks, it);
            it = next;
        } else {
            ++it;
        }
    }
    if (!mActive) {
        return;
    }

    float flHead = mStartTick + ((flTick - mTick) * mSpeed);
    bool bDone = false;
    if (mEndTick <= flHead) {
        flHead = mEndTick;
        bDone = true;
    }
    const float flBuiltEnd = mTunnel->GetGeom()->BuiltEndTick();
    if (flBuiltEnd <= flHead) {
        flHead = flBuiltEnd;
        bDone = true;
    }
    Transform cellXfm;
    mTunnel->GetGeom()->CellXfm(mTrack, &cellXfm, true, true, flHead, kMiddle);
    mView->SetLocalXfm(cellXfm);
    mView->SetDirty(1);

    while (mNextChunkTick <= flHead) {
        const std::vector<float>::iterator gem =
            std::lower_bound(mGemTicks.begin(), mGemTicks.end(), mNextChunkTick - kGemWindowTicks);
        if ((gem != mGemTicks.end()) && (*gem <= mNextChunkTick) && !mFreeChunks.empty()) {
            TnlGeom *pGeom = mTunnel->GetGeom();
            pGeom->CellXfm(mTrack, &cellXfm, true, true, mNextChunkTick, kMiddle);
            Transform middleXfm;
            pGeom->BlendCell(&middleXfm,
                             false,
                             true,
                             static_cast<float>(pGeom->GetNumTracks() - 1) * kMiddle,
                             mNextChunkTick,
                             kMiddle);
            float flRoll = RandomFloat(0.0f, mTotalWeight);
            std::vector<ChunkModel>::iterator model = mModels.begin();
            for (; model != mModels.end(); ++model) {
                flRoll -= model->mWeight;
                if (flRoll <= 0.0f) {
                    break;
                }
            }
            if (model == mModels.end()) {
                --model; // Yes, the binary takes the model before the end even with no models.
            }
            mChunks.splice(mChunks.begin(), mFreeChunks, mFreeChunks.begin());
            mChunks.front().Spawn(&*model, mAuto, mTrack, &cellXfm, &middleXfm, &mFreeXfms, flTick);
        }
        mNextChunkTick += RandomFloat(mInterval.x, mInterval.y);
    }
    if (bDone) {
        Reset(false);
    }
}

void TnlFire::Draw() {
    if (mChunks.empty()) {
        return;
    }
    mMat->SetBaseColor(mTrackColor);
    for (const ChunkModel &model : mModels) {
        model.mMultiMesh->Draw();
    }
}
