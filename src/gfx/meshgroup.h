#pragma once

#include <list>

#include "math/transform.h"
#include "rnd/rndmesh.h"
#include "rnd/rndmultimesh.h"
#include "rnd/rndparticlesys.h"

class TnlGem;

/**
 * The gems of one type and distance, drawn as the instances of a multimesh.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x1c bytes. The groups of
 * a type are chained from the nearest to the farthest. A composite group draws several meshes
 * through its multimesh in turn.
 */
class MeshGroup {
public:
    /**
     * Construct a group and empty the instances of its multimesh.
     *
     * @param pMultiMesh The multimesh.
     * @param pMeshes The meshes of a composite group, moved into the group, or null.
     * @param pParticles The glow particles of the gems, or null.
     * @param pTransforms The pool of free instance transforms.
     * @param nMaxGems The most gems the tunnel places at once.
     * @param flLookahead The ticks beyond the minimum screen size of the mesh the group draws to.
     * @ghidraAddress NTSC-U/C: 0x001f4168
     * @ghidraAddress PAL: 0x001fcf08
     */
    MeshGroup(RndMultiMesh *pMultiMesh,
              std::list<RndMesh *> *pMeshes,
              RndParticleSys *pParticles,
              std::list<Transform> *pTransforms,
              int nMaxGems,
              float flLookahead);

    /**
     * Give a gem an instance, and a glow particle when the group has particles.
     *
     * An empty pool first grows by 64 transforms.
     *
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001f42b0
     * @ghidraAddress PAL: 0x001fd050
     */
    void Bind(TnlGem *pGem);

    /**
     * Return the instance and the glow particle of a gem.
     *
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001f4398
     * @ghidraAddress PAL: 0x001fd138
     */
    void Unbind(TnlGem *pGem);

    /**
     * Move a gem to this group from another.
     *
     * @param pFrom The group the gem leaves.
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001f4410
     * @ghidraAddress PAL: 0x001fd1b0
     */
    void TakeGem(MeshGroup *pFrom, TnlGem *pGem);

    /** @return The multimesh that draws the instances. */
    RndMultiMesh *GetMultiMesh() const {
        return mMultiMesh;
    }

    /** @return The glow particles of the gems, or null. */
    RndParticleSys *GetParticles() const {
        return mParticles;
    }

    /** @return The meshes of a composite group. */
    const std::list<RndMesh *> &GetMeshes() const {
        return mMeshes;
    }

    /** @return The farthest ticks ahead the group draws. */
    float GetLookahead() const {
        return mLookahead;
    }

    /** @return The group of the next distance, or null. */
    MeshGroup *GetNext() const {
        return mNext;
    }

    /** @param pNext The group of the next distance, or null. */
    void SetNext(MeshGroup *pNext) {
        mNext = pNext;
    }

private:
    RndMultiMesh *mMultiMesh;          /*!< The multimesh that draws the instances. */
    RndParticleSys *mParticles;        /*!< The glow particles of the gems, or null. */
    std::list<RndMesh *> mMeshes;      /*!< The meshes of a composite group. */
    std::list<Transform> *mTransforms; /*!< The pool of free instance transforms. */
    float mLookahead;                  /*!< The farthest ticks ahead the group draws. */
    MeshGroup *mNext;                  /*!< The group of the next distance, or null. */
};
