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
        pMesh = mMultiMesh->GetMesh();
    }
    mLookahead = pMesh->GetMinScreen() + flLookahead;
    mMultiMesh->GetTransforms().clear();
    if (mParticles != nullptr) {
        mParticles->GetParticlesOwner()->SetNumParticles(nMaxGems + kSpareCount);
    }
}

void MeshGroup::Bind(TnlGem *pGem) {
    std::list<Transform> &instances = mMultiMesh->GetTransforms();
    if (mTransforms->empty()) {
        // The original fills the new transforms from an uninitialised temporary.
        mTransforms->resize(kSpareCount, Transform());
    }
    instances.splice(instances.begin(), *mTransforms, mTransforms->begin());
    pGem->SetTransform(instances.begin());
    if (mParticles == nullptr) {
        pGem->SetParticle(nullptr);
        return;
    }
    Particle *pParticle = mParticles->AllocParticle();
    pGem->SetParticle(pParticle);
    if (pParticle != nullptr) {
        pParticle->mCol = mParticles->GetStartColorLow();
        pGem->GetParticle()->mSize = mParticles->GetSize().x;
    }
}

void MeshGroup::Unbind(TnlGem *pGem) {
    if (pGem->GetParticle() != nullptr) {
        mParticles->FreeParticle(pGem->GetParticle());
        pGem->SetParticle(nullptr);
    }
    mTransforms->splice(mTransforms->end(), mMultiMesh->GetTransforms(), pGem->GetTransform());
}

void MeshGroup::TakeGem(MeshGroup *pFrom, TnlGem *pGem) {
    // Yes, the binary copies the transform of the gem and never reads the copy.
    [[maybe_unused]] const Transform xfm = *pGem->GetTransform();
    pFrom->Unbind(pGem);
    Bind(pGem);
}
