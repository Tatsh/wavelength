#include "gfx/meshgroup.h"

#include "gfx/tnlgem.h"

namespace {

// The pools take this many particles and transforms beyond the most gems placed at once.
constexpr int kSpareCount = 64;

} // namespace

MeshGroup::MeshGroup(RndMultiMesh *pMultiMesh,
                     std::list<RndMesh *> *pMeshes,
                     RndParticleSys *pParticles,
                     std::list<Transform> *pTransforms,
                     int nMaxGems,
                     float flLookahead)
    : mMultiMesh(pMultiMesh), mParticles(pParticles), mTransforms(pTransforms), mNext(nullptr) {
    RndMesh *pMesh;
    if (pMeshes != nullptr) {
        mMeshes.splice(mMeshes.begin(), *pMeshes);
        pMesh = mMeshes.front();
    } else {
        pMesh = mMultiMesh->mMesh;
    }
    mLookahead = pMesh->mMinScreen + flLookahead;
    mMultiMesh->mTransforms.clear();
    if (mParticles != nullptr) {
        mParticles->mParticlesOwner->SetNumParticles(nMaxGems + kSpareCount);
    }
}

void MeshGroup::Bind(TnlGem *pGem) {
    std::list<Transform> &instances = mMultiMesh->mTransforms;
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
    Particle *pParticle = mParticles->AllocParticle();
    pGem->mParticle = pParticle;
    if (pParticle != nullptr) {
        pParticle->mCol = mParticles->mStartColorLow;
        pGem->mParticle->mSize = mParticles->mSize.x;
    }
}

void MeshGroup::Unbind(TnlGem *pGem) {
    if (pGem->mParticle != nullptr) {
        mParticles->FreeParticle(pGem->mParticle);
        pGem->mParticle = nullptr;
    }
    mTransforms->splice(mTransforms->end(), mMultiMesh->mTransforms, pGem->mTransform);
}

void MeshGroup::TakeGem(MeshGroup *pFrom, TnlGem *pGem) {
    // Yes, the binary copies the transform of the gem and never reads the copy.
    [[maybe_unused]] const Transform xfm = *pGem->mTransform;
    pFrom->Unbind(pGem);
    Bind(pGem);
}
