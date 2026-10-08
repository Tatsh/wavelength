#pragma once

#include <cstddef>
#include <list>

#include "math/vector2.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndtransformable.h"

/**
 * View that shows the children of another view, its children owner, under its own transform and
 * frame.
 *
 * The RTTI includes the class name and records RndAnimatable, RndDrawable, RndTransformable, and
 * RndCollideable as bases. A view is shown only while its frame is in mVisibleRange, when the range
 * is not empty.
 */
class RndView :
    public RndAnimatable,
    public RndDrawable,
    public RndTransformable,
    public RndCollideable {
public:
    /** The bit of the Copy() flags that shares the source's children owner. */
    static constexpr int kCopyShareChildren = 0x20;

    /**
     * Construct a view that owns its children and is in range.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00247a50
     * @ghidraAddress PAL: 0x002504f0
     */
    explicit RndView(const char *pszName);

    /**
     * Drop the reference on the children owner.
     *
     * @ghidraAddress NTSC-U/C: 0x00393420
     * @ghidraAddress PAL: 0x00401a90
     */
    ~RndView() override;

    /**
     * Allocate a view from its pool.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(RndView), "RndView", 0);
    }

    /**
     * Return a view to its pool.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolFree(sizeof(RndView), pBlock);
    }

    /**
     * Record whether the unfiltered frame is in a non-empty mVisibleRange.
     *
     * The filtered frame is ignored.
     *
     * @param fFrame The filtered frame.
     * @return Whether the view is in range.
     * @ghidraAddress NTSC-U/C: 0x002481f8
     * @ghidraAddress PAL: 0x00250c98
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Add the view, when the children owner's children have something to draw, then the view's
     * children, to a list.
     *
     * The children owner's animations and transforms are first brought to this view's frame and
     * transform.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002488d8
     * @ghidraAddress PAL: 0x00251378
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Draw the children owner's children at this view's frame and transform, when in range.
     *
     * @return Whether to draw the view's children.
     * @ghidraAddress NTSC-U/C: 0x002480f0
     * @ghidraAddress PAL: 0x00250b90
     */
    int DrawShowing() override;

    /**
     * Update the world transform when in range.
     *
     * @param pParent The parent, or null.
     * @param bForce Non-zero to update even when the transform is not out of date.
     * @return What RndTransformable::UpdateWorldXfm() reports, or 0 out of range.
     * @ghidraAddress NTSC-U/C: 0x00248240
     * @ghidraAddress PAL: 0x00250ce0
     */
    int UpdateWorldXfm(RndTransformable *pParent, int bForce) override;

    /**
     * Write a description of the view and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00248410
     * @ghidraAddress PAL: 0x00250eb0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the view and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002484e0
     * @ghidraAddress PAL: 0x00250f80
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * A children owner replaced by null becomes the view itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00248290
     * @ghidraAddress PAL: 0x00250d30
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00393960
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another view and its bases.
     *
     * The children owner is the view itself when the source owns its children and the flags do
     * not include kCopyShareChildren. Otherwise it is the source's.
     *
     * @param pSource The view to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x002487e0
     * @ghidraAddress PAL: 0x00251280
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002485c8
     * @ghidraAddress PAL: 0x00251068
     */
    void Load(BinStream &stream) override;

    /**
     * Set the range of frames the view shows in, and put the view in range.
     *
     * The name is inferred.
     *
     * @param fStart The first frame.
     * @param fEnd The frame after the last.
     * @ghidraAddress NTSC-U/C: 0x00248270
     * @ghidraAddress PAL: 0x00250d10
     */
    void SetVisibleRange(float fStart, float fEnd);

    /**
     * Put the view in or out of range.
     *
     * The name is inferred.
     *
     * @param nInRange Non-zero for in range.
     * @ghidraAddress NTSC-U/C: 0x00248288
     * @ghidraAddress PAL: 0x00250d28
     */
    void SetInRange(int nInRange);

    /**
     * Create a view.
     *
     * @param pszName The registry key.
     * @return The view.
     * @ghidraAddress NTSC-U/C: 0x00393970
     * @ghidraAddress PAL: 0x00402078
     */
    static RndObject *New(const char *pszName) {
        return new RndView(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `View`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a64
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a68
     */
    static int sRev;

    RndView *mChildrenOwner; /*!< The view whose children this one shows. */
    Vector2 mVisibleRange;   /*!< The frames the view shows in, empty for all. */
    int mInRange;            /*!< Non-zero while the view is shown. */

protected:
    /**
     * Drop the reference on the children owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002483a0
     * @ghidraAddress PAL: 0x00250e40
     */
    void ReleaseRefs();

    /**
     * Take a reference on the children owner, and put the view in range.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002483d0
     * @ghidraAddress PAL: 0x00250e70
     */
    void AcquireRefs();

    /**
     * Bring the children owner's animations to this view's frame and its transforms under this
     * view's transform.
     *
     * The binary expands the routine inline in DrawShowing() and ListDrawables(). The name is
     * inferred.
     */
    void SyncChildren();
};
