#pragma once

#include <memory>
#include <vector>

#include "math/color.h"
#include "math/key.h"
#include "math/quaternion.h"
#include "math/vector3.h"
#include "rnd/rndloader.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndobject.h"
#include "rnd/rndtransanim.h"
#include "script/dataarray.h"

class Avatar;

/**
 * The model of one part of an avatar that an Avatar draws.
 *
 * The RTTI includes the name. The object is 0x70 bytes, and the vptr follows the data members at
 * `+0x64`. TorsoPart, InstrPart, and ArmPart derive from it. The part comes from its entry of the
 * `parts` array of the `avatar` entry of the "db" section. Each choice of the `types` array of
 * that entry names one or two `.rnd` files under `rnd_dir`, and the `anims` array names the
 * meshes the shared `fbase_*.tnm` animations drive.
 */
class AvatarPart {
public:
    /**
     * The callback of the loads of the files of a part.
     *
     * The RTTI includes the nested class name and records RndLoader::Callback as the base. The
     * object is 8 bytes.
     */
    class Callback : public RndLoader::Callback {
    public:
        /**
         * Construct the callback of a part.
         *
         * Inline in the AvatarPart constructor.
         *
         * @param pOwner The part.
         */
        explicit Callback(AvatarPart *pOwner) : mOwner(pOwner) {
        }

        /**
         * Create an object only when no object of the name exists. Otherwise stop the load and
         * request the choice the part is loading again.
         *
         * The name of the class follows the base.
         *
         * @param pExisting The loaded object with the same name, or null when none exists.
         * @param pszName The object name. The routine ignores it.
         * @param pszClass The class name. The routine ignores it.
         * @return 1 when pExisting is null, otherwise -1.
         * @ghidraAddress NTSC-U/C: 0x00276910
         * @ghidraAddress PAL: 0x00280430
         */
        int ShouldLoad(RndObject *pExisting, const char *pszName, const char *pszClass) override;

        AvatarPart *mOwner; /*!< The part. */
    };

    /**
     * Construct the model of a part from its entry of the `parts` array of the `avatar` entry of
     * the "db" section.
     *
     * Each name of the `anims` array of the entry gets an animation `avatar<n>_<name>.tnm` that
     * shares the keys of `fbase_<name>.tnm`. Only the pelvis and the arm bones retain the
     * translation keys.
     *
     * @param nPart The part, one of AvatarPartSet::Part.
     * @param nParent The part the model hangs from, or AvatarPartSet::kNumParts for none.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x00272cf8
     * @ghidraAddress PAL: 0x0027c818
     */
    AvatarPart(int nPart, int nParent, Avatar *pAvatar);

    /**
     * Delete the animations and the loaders of the model.
     *
     * @ghidraAddress NTSC-U/C: 0x00273108
     * @ghidraAddress PAL: 0x0027cc28
     */
    virtual ~AvatarPart();

    /**
     * Pose the model at one frame of its animations.
     *
     * The name is inferred.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x00273350
     * @ghidraAddress PAL: 0x0027ce70
     */
    virtual void SetFrame(float fFrame);

    /**
     * Pose the model between two frames of its animations.
     *
     * The translation of a bone without translation keys is not changed. The name is inferred.
     *
     * @param fFrom The frame blended from.
     * @param fTo The frame blended to.
     * @param fBlend The weight of fTo, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x002733d0
     * @ghidraAddress PAL: 0x0027cef0
     */
    virtual void SetFrames(float fFrom, float fTo, float fBlend);

    /**
     * Hang the model from the joint of its parent.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00274358
     * @ghidraAddress PAL: 0x0027de78
     */
    virtual void Update();

    /**
     * Start loading the files of a choice of the part.
     *
     * The loads replace any load in progress. The name is inferred.
     *
     * @param pszType The choice, a symbol.
     * @ghidraAddress NTSC-U/C: 0x00272b48
     * @ghidraAddress PAL: 0x0027c668
     */
    virtual void SetType(const char *pszType);

    /**
     * Build the meshes of the model once the files SetType() started loading have loaded.
     *
     * The name is inferred.
     *
     * @return False while a file is still loading, otherwise true.
     * @ghidraAddress NTSC-U/C: 0x00273e70
     * @ghidraAddress PAL: 0x0027d990
     */
    virtual bool Rebuild();

    /**
     * Abandon the loads SetType() started and return to the choice the meshes show.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00272ca0
     * @ghidraAddress PAL: 0x0027c7c0
     */
    virtual void ReleaseType();

    /**
     * Draw the model, when it has a mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002732f0
     * @ghidraAddress PAL: 0x0027ce10
     */
    void Draw();

    /**
     * Set the alpha of the material of the model, when it has one.
     *
     * The name is inferred.
     *
     * @param fBlend The alpha, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00273318
     * @ghidraAddress PAL: 0x0027ce38
     */
    void SetBlend(float fBlend);

    /**
     * Colour the model.
     *
     * The colour becomes the light colour of the material.
     *
     * @param pColor The colour. Its alpha is ignored.
     * @ghidraAddress NTSC-U/C: 0x00274448
     * @ghidraAddress PAL: 0x0027df68
     */
    void SetColor(const Color *pColor);

    Callback mCallback;       /*!< The callback of the loads of the part. */
    DataArray *mData;         /*!< The entry of the part in the `parts` array. */
    const char *mType;        /*!< The choice the meshes show, or null before the first. */
    const char *mPendingType; /*!< The choice SetType() last requested, or null. */
    Color mColor;             /*!< The colour, opaque white until SetColor(). +0x20 */
    RndMesh *mMesh;           /*!< The `draw_root` mesh drawn, or null. */
    RndMesh *mTransRoot;      /*!< The `trans_root` mesh the model hangs from, or null. */
    RndMesh *mJoint;          /*!< The joint mesh of the parent the model hangs from, or null. */
    RndMat *mMat;             /*!< The material of the model, or null. */

    /** The animations of the bones the `anims` array lists, in its order. */
    std::vector<RndTransAnim *> mAnims;

    AvatarPart *mParent;                /*!< The part the model hangs from, or null. */
    Avatar *mAvatar;                    /*!< The avatar the part belongs to. */
    std::unique_ptr<RndLoader> mLoader; /*!< The load of the files the meshes come from, or null. */

    /** The load of the first file of mPendingType, or null. */
    std::unique_ptr<RndLoader> mPendingLoader;

    /** The load of the second file of mPendingType, or null. */
    std::unique_ptr<RndLoader> mPendingExtraLoader;

protected:
    /**
     * Give a mesh and every mesh drawn under it the material of the model.
     *
     * The name is inferred.
     *
     * @param pMesh The mesh.
     * @ghidraAddress NTSC-U/C: 0x00273238
     * @ghidraAddress PAL: 0x0027cd58
     */
    void SetMatTree(RndMesh *pMesh);

    /**
     * Find the joint mesh the model hangs from, `avatar<n>_<trans_root>_joint.mesh`.
     *
     * The parent calls it once its meshes are built. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002743a0
     * @ghidraAddress PAL: 0x0027dec0
     */
    void FindJoint();

    /**
     * Interpolate the rotation keys of an animation at a frame.
     *
     * The binary expands the routine inline. Nothing is written for no keys.
     *
     * @param pAnim The animation. Its frames owner has the keys.
     * @param fFrame The frame.
     * @param out Receives the rotation.
     */
    static void RotationAt(const RndTransAnim *pAnim, float fFrame, Quat &out) {
        const Key<Quat> *pPrev;
        const Key<Quat> *pNext;
        float fRatio;
        AtFrame(pAnim->mFramesOwner->mRotKeys, fFrame, pPrev, pNext, fRatio);
        if (pPrev != nullptr) {
            QuatSlerp(pPrev->value, pNext->value, out, fRatio);
        }
    }

    /**
     * Blend the rotations of an animation at two frames.
     *
     * The binary expands the routine inline.
     *
     * @param pAnim The animation.
     * @param fFrom The frame blended from.
     * @param fTo The frame blended to.
     * @param fBlend The weight of fTo.
     * @param out Receives the rotation.
     */
    static void
    BlendRotation(const RndTransAnim *pAnim, float fFrom, float fTo, float fBlend, Quat &out) {
        Quat from;
        Quat to;
        RotationAt(pAnim, fFrom, from);
        RotationAt(pAnim, fTo, to);
        QuatSlerp(from, to, out, fBlend);
    }

    /**
     * Blend the translations of an animation at two frames.
     *
     * The binary expands the routine inline.
     *
     * @param pAnim The animation.
     * @param fFrom The frame blended from.
     * @param fTo The frame blended to.
     * @param fBlend The weight of fTo.
     * @param out Receives the translation.
     */
    static void BlendTranslation(
        const RndTransAnim *pAnim, float fFrom, float fTo, float fBlend, Vector3 &out) {
        Vector3 from;
        Vector3 to;
        InterpKeys(pAnim->mFramesOwner->mTransKeys, fFrom, from);
        InterpKeys(pAnim->mFramesOwner->mTransKeys, fTo, to);
        Interp(from, to, fBlend, out);
    }

    /**
     * Find a mesh by name.
     *
     * The binary expands the routine inline.
     *
     * @param pszName The name.
     * @return The mesh, or null when no mesh has the name.
     */
    static RndMesh *FindMesh(const char *pszName) {
        return dynamic_cast<RndMesh *>(TheManager.Find(pszName));
    }

    /**
     * Set the local transform of a mesh to the identity and mark it out of date.
     *
     * The binary expands the routine inline.
     *
     * @param pMesh The mesh.
     */
    static void ResetLocalXfm(RndMesh *pMesh) {
        Transform identity;
        identity.mBasisX = {1.0f, 0.0f, 0.0f};
        identity.mBasisY = {0.0f, 1.0f, 0.0f};
        identity.mBasisZ = {0.0f, 0.0f, 1.0f};
        identity.mTranslation = {0.0f, 0.0f, 0.0f, 1.0f};
        pMesh->mLocalXfm = identity;
        pMesh->mDirty = 1;
    }
};
