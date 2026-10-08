#pragma once

#include <list>

#include "math/transform.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndobject.h"

/**
 * Mix-in for an object with a local transform and a world transform derived from its parent's.
 *
 * The RTTI includes the class name and records RndObject as a virtual base. Every entry of
 * mTransList registers this object as a referrer through RndObject::AddRef().
 */
class RndTransformable : public virtual RndObject {
public:
    /** How the world transform turns to face the current camera. */
    enum Billboard {
        kBillboardNone = 0,            /*!< No billboarding. */
        kBillboardX = 1,               /*!< Face the camera about x. */
        kBillboardY = 2,               /*!< Face the camera about y. */
        kBillboardZ = 4,               /*!< Face the camera about z. */
        kBillboardXZ = 8,              /*!< Face the camera about x and z. */
        kBillboardXYZ = 16,            /*!< Face the camera about all three axes. */
        kBillboardSimpleXYZ = 32,      /*!< Take the camera's orientation. */
        kBillboardLocalRotate = 64,    /*!< Keep the local rotation and move with the parent. */
        kBillboardScale = 128,         /*!< Flag that preserves the scale of the world transform. */
        kBillboardScaleX = 129,        /*!< kBillboardX with the scale preserved. */
        kBillboardScaleY = 130,        /*!< kBillboardY with the scale preserved. */
        kBillboardScaleZ = 132,        /*!< kBillboardZ with the scale preserved. */
        kBillboardScaleXZ = 136,       /*!< kBillboardXZ with the scale preserved. */
        kBillboardScaleXYZ = 144,      /*!< kBillboardXYZ with the scale preserved. */
        kBillboardScaleSimpleXYZ = 160 /*!< kBillboardSimpleXYZ with the scale preserved. */
    };

    /**
     * Construct a transformable with identity transforms, no origin offset, and no billboard.
     *
     * The world transform starts dirty. The constructor has no out-of-line copy.
     */
    RndTransformable();

    /**
     * Drop the references on the children.
     *
     * @ghidraAddress NTSC-U/C: 0x00391e90
     * @ghidraAddress PAL: 0x00400518
     */
    ~RndTransformable() override;

    /**
     * Set the billboard mode.
     *
     * @param nBillboard A Billboard.
     * @ghidraAddress NTSC-U/C: 0x00243c28
     * @ghidraAddress PAL: 0x0024c700
     */
    virtual void SetBillboard(int nBillboard);

    /**
     * Recompute the world transform when it, or the parent's, changed, and then the children's.
     *
     * Without a billboard, the origin is subtracted before the transform applies.
     *
     * @param pParent The parent, or null for a root.
     * @param bForce Non-zero to recompute even when nothing changed.
     * @return Non-zero when the world transform was recomputed.
     * @ghidraAddress NTSC-U/C: 0x00243f98
     * @ghidraAddress PAL: 0x0024ca70
     */
    virtual int UpdateWorldXfm(RndTransformable *pParent, int bForce);

    /**
     * Write a description of the transformable.
     *
     * Nothing is written unless the dump level of the stream is above 0.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002447b8
     * @ghidraAddress PAL: 0x0024d290
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the transforms, the names of the children, the billboard mode, and the origin.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002449b0
     * @ghidraAddress PAL: 0x0024d488
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a child with another object.
     *
     * A replacement that is not a transformable, or null, removes the child.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00245100
     * @ghidraAddress PAL: 0x0024dbd0
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Copy the transforms, the billboard mode, the origin, and the children when the flags
     * request them.
     *
     * @param pSource The transformable to copy from.
     * @param nFlags The set of fields to copy. RndObject::kCopyChildLists copies the children.
     * @ghidraAddress NTSC-U/C: 0x002448c8
     * @ghidraAddress PAL: 0x0024d3a0
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00244e18
     */
    void Load(BinStream &stream) override;

    /**
     * Set the point the world transform subtracts first when there is no billboard.
     *
     * @param origin The point.
     * @ghidraAddress NTSC-U/C: 0x00243c38
     * @ghidraAddress PAL: 0x0024c710
     */
    void SetOrigin(const Vector3 &origin);

    /**
     * Find the transformable whose children include this one.
     *
     * @return The parent, or null.
     * @ghidraAddress NTSC-U/C: 0x00243c50
     * @ghidraAddress PAL: 0x0024c728
     */
    RndTransformable *Parent();

    /**
     * Append a child and mark its world transform dirty.
     *
     * A transformable already among the children is not added again.
     *
     * @param pTrans The new child.
     * @return Whether the child was added.
     * @ghidraAddress NTSC-U/C: 0x00243d18
     * @ghidraAddress PAL: 0x0024c7f0
     */
    bool AddTrans(RndTransformable *pTrans);

    /**
     * Remove a child.
     *
     * @param pTrans The child.
     * @ghidraAddress NTSC-U/C: 0x00243e20
     * @ghidraAddress PAL: 0x0024c8f8
     */
    void RemoveTrans(RndTransformable *pTrans);

    /**
     * Remove every child.
     *
     * @ghidraAddress NTSC-U/C: 0x00243ef0
     * @ghidraAddress PAL: 0x0024c9c8
     */
    void ClearTrans();

    /**
     * Report the world transform turned to face the current camera.
     *
     * Without a billboard the world transform itself is reported. Otherwise the result is built in
     * one shared transform that the next call replaces.
     *
     * @return The transform.
     * @ghidraAddress NTSC-U/C: 0x00244148
     * @ghidraAddress PAL: 0x0024cc20
     */
    const Transform &BillboardXfm();

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a58
     */
    static int sRev;

    std::list<RndTransformable *> mTransList; /*!< The children, which follow this object. */
    Transform mLocalXfm;                      /*!< The transform relative to the parent. */
    Transform mWorldXfm;                      /*!< The transform relative to the world. */
    Vector3 mOrigin;                          /*!< The point the world transform subtracts. */
    int mDirty;                               /*!< Non-zero when mWorldXfm is out of date. */
    int mBillboard;                           /*!< The Billboard mode. */

protected:
    /**
     * Drop the reference on every child without emptying the list.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00244d10
     * @ghidraAddress PAL: 0x0024d7e8
     */
    void ReleaseTransRefs();

    /**
     * Take a reference on every child.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00244d90
     * @ghidraAddress PAL: 0x0024d868
     */
    void AcquireTransRefs();
};

/**
 * Write the name of a billboard mode.
 *
 * @param stream The stream to write to.
 * @param eBillboard The mode. An unknown mode writes nothing.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002452b8
 * @ghidraAddress PAL: 0x0024dd88
 */
PrnStream &operator<<(PrnStream &stream, RndTransformable::Billboard eBillboard);
