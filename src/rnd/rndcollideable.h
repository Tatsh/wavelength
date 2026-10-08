#pragma once

#include <list>

#include "math/plane.h"
#include "math/vector2.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndobject.h"

/**
 * Mix-in for an object that can be hit-tested, and that tests a list of other collideables.
 *
 * The RTTI includes the class name and records RndObject as a virtual base. Every entry of
 * mCollides registers this object as a referrer through RndObject::AddRef().
 */
class RndCollideable : public virtual RndObject {
public:
    /**
     * One recorded intersection.
     *
     * The RTTI includes the nested class name.
     */
    struct Collision {
        RndCollideable *mObject; /*!< The collideable that was struck. */
        float mDistance;         /*!< Distance along the segment. */
    };

    /**
     * Construct a collideable without children.
     *
     * The constructor has no out-of-line copy.
     */
    RndCollideable() {
    }

    /**
     * Drop the references on the children.
     *
     * @ghidraAddress NTSC-U/C: 0x0037fb80
     * @ghidraAddress PAL: 0x003ee238
     */
    ~RndCollideable() override;

    /**
     * Test a segment against the collideable and add what it strikes to a list.
     *
     * The base body tests the children.
     *
     * @param segment The segment.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002224c8
     * @ghidraAddress PAL: 0x0022b298
     */
    virtual void Collide(const Segment &segment, std::list<Collision> &collisions);

    /**
     * Test a screen point against the collideable and add what it strikes to a list.
     *
     * The base body tests the children.
     *
     * @param point The screen point.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00222568
     * @ghidraAddress PAL: 0x0022b338
     */
    virtual void Collide(const Vector2 &point, std::list<Collision> &collisions);

    /**
     * Write a description of the collideable.
     *
     * Nothing is written unless the dump level of the stream is above 0.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002221d8
     * @ghidraAddress PAL: 0x0022afa8
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the names of the children.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00222248
     * @ghidraAddress PAL: 0x0022b018
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a child with another object.
     *
     * A replacement that is not a collideable, or null, removes the child.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00222020
     * @ghidraAddress PAL: 0x0022adf0
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Copy the children when the flags request them.
     *
     * @param pSource The collideable to copy from.
     * @param nFlags The set of fields to copy. RndObject::kCopyChildLists copies the children.
     * @ghidraAddress NTSC-U/C: 0x00222430
     * @ghidraAddress PAL: 0x0022b200
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002223b0
     * @ghidraAddress PAL: 0x0022b180
     */
    void Load(BinStream &stream) override;

    /**
     * Find the collideable whose children include this one.
     *
     * @return The parent, or null.
     * @ghidraAddress NTSC-U/C: 0x00221f58
     * @ghidraAddress PAL: 0x0022ad28
     */
    RndCollideable *Parent();

    /**
     * Append a child.
     *
     * A collideable already among the children is not added again.
     *
     * @param pCollide The new child.
     * @return Whether the child was added.
     * @ghidraAddress NTSC-U/C: 0x00222608
     * @ghidraAddress PAL: 0x0022b3d8
     */
    bool AddCollide(RndCollideable *pCollide);

    /**
     * Remove a child.
     *
     * @param pCollide The child.
     * @ghidraAddress NTSC-U/C: 0x00222700
     * @ghidraAddress PAL: 0x0022b4d0
     */
    void RemoveCollide(RndCollideable *pCollide);

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0998
     */
    static int sRev;

    std::list<RndCollideable *> mCollides; /*!< The children, tested after the collideable. */

protected:
    /**
     * Drop the reference on every child without emptying the list.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002222a0
     * @ghidraAddress PAL: 0x0022b070
     */
    void ReleaseCollideRefs();

    /**
     * Take a reference on every child.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00222328
     * @ghidraAddress PAL: 0x0022b0f8
     */
    void AcquireCollideRefs();
};
