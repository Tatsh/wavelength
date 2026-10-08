#pragma once

#include <list>

#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndobject.h"

/**
 * Mix-in for an object that draws itself and then a list of other drawables.
 *
 * The RTTI includes the class name and records RndObject as a virtual base. Every entry of mDraws
 * registers this object as a referrer through RndObject::AddRef(). A drawable that goes away is
 * therefore removed from its parents' lists by Replace().
 */
class RndDrawable : public virtual RndObject {
public:
    /**
     * Construct a showing drawable without a highlight or children.
     *
     * The constructor has no out-of-line copy.
     */
    RndDrawable() : mShowing(1), mHighlight(0) {
    }

    /**
     * Drop the references on the children.
     *
     * @ghidraAddress NTSC-U/C: 0x003809b8
     * @ghidraAddress PAL: 0x003ef060
     */
    ~RndDrawable() override;

    /**
     * Add the objects the drawable uses to a list.
     *
     * The base body adds nothing. The name is inferred.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x003809a0
     */
    virtual void ListObjects(std::list<RndObject *> &objects);

    /**
     * Add the drawables to draw this frame to a list.
     *
     * The base body adds the drawables of the children. The name is inferred.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002241b0
     * @ghidraAddress PAL: 0x0022cf80
     */
    virtual void ListDrawables(std::list<RndDrawable *> &drawables);

    /**
     * Set whether the drawable draws at all.
     *
     * @param nShowing Non-zero to draw.
     * @ghidraAddress NTSC-U/C: 0x00380b00
     */
    virtual void SetShowing(int nShowing) {
        mShowing = nShowing;
    }

    /**
     * Set whether the drawable draws with its highlight treatment.
     *
     * @param nHighlight Non-zero to highlight.
     * @ghidraAddress NTSC-U/C: 0x00224a90
     * @ghidraAddress PAL: 0x0022d848
     */
    virtual void SetHighlight(int nHighlight);

    /**
     * Draw the drawable alone.
     *
     * The base body draws nothing and reports that the children are still to be drawn.
     *
     * @return Non-zero when the children are to be drawn as well.
     * @ghidraAddress NTSC-U/C: 0x00380b08
     */
    virtual int DrawShowing() {
        return 1;
    }

    /**
     * Write a description of the drawable.
     *
     * Nothing is written unless the dump level of the stream is above 0.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00224480
     * @ghidraAddress PAL: 0x0022d250
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the showing flag and the names of the children.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002245c8
     * @ghidraAddress PAL: 0x0022d398
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a child with another object.
     *
     * A replacement that is not a drawable, or null, removes the child.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x002242c8
     * @ghidraAddress PAL: 0x0022d098
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Copy the showing flag, and the children when the flags request them.
     *
     * @param pSource The drawable to copy from.
     * @param nFlags The set of fields to copy. RndObject::kCopyChildLists copies the children.
     * @ghidraAddress NTSC-U/C: 0x00224528
     * @ghidraAddress PAL: 0x0022d2f8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00224750
     */
    void Load(BinStream &stream) override;

    /**
     * Find the drawable whose children include this one.
     *
     * The referrers are searched in the order they registered.
     *
     * @return The parent, or null.
     * @ghidraAddress NTSC-U/C: 0x002240e8
     * @ghidraAddress PAL: 0x0022ceb8
     */
    RndDrawable *Parent();

    /**
     * Draw the drawable and then its children, when it is showing.
     *
     * @ghidraAddress NTSC-U/C: 0x00224238
     * @ghidraAddress PAL: 0x0022d008
     */
    void Draw();

    /**
     * Add a child before another.
     *
     * A drawable already among the children is not added again.
     *
     * @param pDraw The new child.
     * @param pBefore The child to insert before, or null to append.
     * @return Whether the child was added.
     * @ghidraAddress NTSC-U/C: 0x002247f8
     * @ghidraAddress PAL: 0x0022d5b0
     */
    bool AddDraw(RndDrawable *pDraw, RndDrawable *pBefore);

    /**
     * Remove a child.
     *
     * @param pDraw The child.
     * @ghidraAddress NTSC-U/C: 0x00224918
     * @ghidraAddress PAL: 0x0022d6d0
     */
    void RemoveDraw(RndDrawable *pDraw);

    /**
     * Remove every child.
     *
     * @ghidraAddress NTSC-U/C: 0x002249e8
     * @ghidraAddress PAL: 0x0022d7a0
     */
    void ClearDraws();

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09a4
     */
    static int sRev;

    int mShowing;                    /*!< Non-zero to draw. */
    int mHighlight;                  /*!< Non-zero to draw with the highlight treatment. */
    std::list<RndDrawable *> mDraws; /*!< The children, drawn after the drawable. */

protected:
    /**
     * Drop the reference on every child without emptying the list.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00224640
     */
    void ReleaseDrawRefs();

    /**
     * Take a reference on every child.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002246c8
     */
    void AcquireDrawRefs();
};
