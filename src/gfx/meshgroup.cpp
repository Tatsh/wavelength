#include "gfx/meshgroup.h"

#include "gfx/tnlgem.h"

namespace {

// The pools take this many particles and transforms beyond the most gems placed at once.
constexpr int kSpareCount = 64;

} // namespace

MeshGroup::MeshGroup(Rnd::MultiMesh *pMultiMesh,
                     std::list<Rnd::Mesh *> *pMeshes,
                     Rnd::ParticleSys *pParticles,
                     std::list<Transform> *pTransforms,
                     int nMaxGems,
                     float flLookahead)
    : mMultiMesh(pMultiMesh), mParticles(pParticles), mTransforms(pTransforms), mNext(nullptr) {
    Rnd::Mesh *pMesh;
    if (pMeshes != nullptr) {
        mMeshes.splice(mMeshes.begin(), *pMeshes);
        pMesh = mMeshes.front();
    } else {
        pMesh = mMultiMesh->GetMesh();
    }
    mLookahead = pMesh->mMinScreen + flLookahead;
    mMultiMesh->GetTransforms().clear();
    if (mParticles != nullptr) {
        mParticles->mParticlesOwner->SetPoolSize(nMaxGems + kSpareCount);
    }
}

void MeshGroup::Bind(TnlGem *pGem) {
    std::list<Transform> &instances = mMultiMesh->GetTransforms();
    if (mTransforms->empty()) {
        // The original fills the new transforms from an uninitialised temporary.
        mTransforms->resize(kSpareCount, Transform());
    }
    instances.splice(instances.begin(), *mTransforms, mTransforms->begin());
    pGem->mTransform = instances.begin();
    if (mParticles == nullptr) {
        pGem->mParticle = nullptr;
        return;
    }
    Rnd::Particle *pParticle = mParticles->AllocParticle();
    pGem->mParticle = pParticle;
    if (pParticle != nullptr) {
        pParticle->mCol = mParticles->mStartColorLow;
        pGem->mParticle->mSize = mParticles->mSizeLow;
    }
}

void MeshGroup::Unbind(TnlGem *pGem) {
    if (pGem->mParticle != nullptr) {
        mParticles->FreeParticle(pGem->mParticle);
        pGem->mParticle = nullptr;
    }
    mTransforms->splice(mTransforms->end(), mMultiMesh->GetTransforms(), pGem->mTransform);
}

void MeshGroup::TakeGem(MeshGroup *pFrom, TnlGem *pGem) {
    pFrom->Unbind(pGem);
    Bind(pGem);
}
