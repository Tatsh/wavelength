#pragma once

#include <cstddef>
#include <list>
#include <vector>

#include "math/plane.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndtransformable.h"
#include "rnd/rndview.h"

/**
 * Looping track of views, each shown over a window of frames and moved on by a fixed distance
 * every loop.
 *
 * The RTTI includes the class name and records RndAnimatable, RndCollideable, RndTransformable,
 * and RndDrawable as bases. A section shows its view while the frame, less the loops already run,
 * is in the window that ends at the section frame. The visible views are drawn farthest along
 * first.
 */
class RndArena :
    public RndAnimatable,
    public RndCollideable,
    public RndTransformable,
    public RndDrawable {
public:
    /**
     * One view of the track and the window it is shown in.
     *
     * The RTTI includes the nested class name.
     */
    struct Section {
        /** Construct a section without a view that teleports and sorts by its start. */
        Section()
            : mView(nullptr), mFrame(0.0f), mDelta(0.0f), mTeleport(1), mSortStart(1), mLoop(0) {
        }

        RndView *mView; /*!< The view, or null. */
        float mFrame;   /*!< The frame the window ends at in the first loop. */
        float mDelta;   /*!< The length of the window, in frames. */
        int mTeleport;  /*!< Non-zero to move the view on by the loop distance every loop. */
        int mSortStart; /*!< Non-zero to sort the view by the start of its window. */
        int mLoop;      /*!< The loop the view was last placed for. */
    };

    /**
     * One entry of the drawing order.
     *
     * The RTTI includes the nested class name.
     */
    class Sort {
    public:
        /** Construct an entry without a view. */
        Sort() : mView(nullptr) {
        }

        /**
         * Order the entries farthest along first.
         *
         * @param other The entry to compare with.
         * @return Whether this entry has the greater sort key.
         */
        bool operator<(const Sort &other) const {
            return mSortKey > other.mSortKey;
        }

        RndView *mView; /*!< The view to draw, or null. */
        float mSortKey; /*!< The frame of the view's window in its current loop. */
    };

    /**
     * Construct an empty track that loops every 1000 frames over no distance.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00249c18
     * @ghidraAddress PAL: 0x00252698
     */
    explicit RndArena(const char *pszName);

    /**
     * Drop the references on the views.
     *
     * @ghidraAddress NTSC-U/C: 0x00249660
     * @ghidraAddress PAL: 0x002520e0
     */
    ~RndArena() override;

    /**
     * Allocate a track, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Arena", 0);
    }

    /**
     * Release a track.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Place and show the view of each section for the filtered frame, then sort the drawing
     * order.
     *
     * @param fFrame The filtered frame. The routine reads mFilteredFrame instead.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x002493b8
     * @ghidraAddress PAL: 0x00251e38
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Update the world transform, then that of every showing view.
     *
     * @param pParent The parent, or null.
     * @param bForce Non-zero to update even an unchanged transform.
     * @return The result of the base update. The views receive it as their force flag.
     * @ghidraAddress NTSC-U/C: 0x002494b0
     * @ghidraAddress PAL: 0x00251f30
     */
    int UpdateWorldXfm(RndTransformable *pParent, int bForce) override;

    /**
     * Draw the views in the drawing order.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x00249550
     * @ghidraAddress PAL: 0x00251fd0
     */
    int DrawShowing() override;

    using RndCollideable::Collide;

    /**
     * Add what each showing view strikes to a list.
     *
     * The children are not tested.
     *
     * @param segment The segment, in world space.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002495c8
     * @ghidraAddress PAL: 0x00252048
     */
    void Collide(const Segment &segment, std::list<Collision> &collisions) override;

    /**
     * Write a description of the track and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00248b90
     * @ghidraAddress PAL: 0x00251630
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the track and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00248c88
     * @ghidraAddress PAL: 0x00251728
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a view with another object.
     *
     * A replacement that is not a view, or null, clears the section's view. A drawing order entry
     * of the view is cleared.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00248ff8
     * @ghidraAddress PAL: 0x00251a78
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00394b88
     * @ghidraAddress PAL: 0x00403278
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another track and its bases.
     *
     * @param pSource The track to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00248aa0
     * @ghidraAddress PAL: 0x00251540
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * Version 1 adds the animatable base, version 2 the other bases, and version 5 mAnimateViews.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00248da0
     * @ghidraAddress PAL: 0x00251830
     */
    void Load(BinStream &stream) override;

    /**
     * Create a track.
     *
     * @param pszName The registry key.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x00394b98
     * @ghidraAddress PAL: 0x00403288
     */
    static RndObject *New(const char *pszName) {
        return new RndArena(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Arena`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a6c
     * @ghidraAddress PAL: 0x0042f904
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a70
     */
    static int sRev;

    /**
     * The version Load() read last. The section reader reads by this version.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c820
     */
    static int sLoadRev;

    Vector3 mLoopDist;              /*!< The distance a teleporting view moves on every loop. */
    float mLoopFrames;              /*!< The length of a loop, in frames. */
    std::vector<Section> mSections; /*!< The sections. */
    std::list<Sort> mSorts;         /*!< The drawing order, one entry per section. */
    int mAnimateViews; /*!< Non-zero to set the frame of each shown view. The title is inferred. */

protected:
    /**
     * Drop the references on the views and empty the drawing order.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00248f80
     * @ghidraAddress PAL: 0x00251a00
     */
    void ReleaseRefs();

    /**
     * Take a reference on each view and give the drawing order one entry per section.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00248ee8
     * @ghidraAddress PAL: 0x00251968
     */
    void AcquireRefs();

    /**
     * Place a section's view for the filtered frame and show it while the frame is in its window.
     *
     * The name is inferred.
     *
     * @param section The section.
     * @ghidraAddress NTSC-U/C: 0x00249198
     * @ghidraAddress PAL: 0x00251c18
     */
    void UpdateSection(Section &section);

    /**
     * Move a teleporting section's view on by the loop distance for each loop between its loop
     * and a new one, and record the new loop.
     *
     * The name is inferred.
     *
     * @param section The section.
     * @param nLoop The new loop.
     * @ghidraAddress NTSC-U/C: 0x002492a8
     * @ghidraAddress PAL: 0x00251d28
     */
    void SetSectionLoop(Section &section, int nLoop);
};

/**
 * Write a section's view, frame, loop, window, and flags.
 *
 * @param stream The stream to write to.
 * @param section The section.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0024a360
 * @ghidraAddress PAL: 0x00252de0
 */
PrnStream &operator<<(PrnStream &stream, const RndArena::Section &section);

/**
 * Write a section's view name, frame, loop, window, and flags.
 *
 * The binary expands the routine inline in the vector writer.
 *
 * @param stream The stream to write to.
 * @param section The section.
 * @return The stream.
 */
BinStream &operator<<(BinStream &stream, const RndArena::Section &section);

/**
 * Read a section the section writer wrote, or an earlier version of it.
 *
 * The version is the one RndArena::Load() read last. Version 1 adds the window, version 3 the
 * teleport flag, and version 4 the sort flag.
 *
 * @param stream The stream to read from.
 * @param section Receives the section.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0024a450
 * @ghidraAddress PAL: 0x00252ed0
 */
BinStream &operator>>(BinStream &stream, RndArena::Section &section);
