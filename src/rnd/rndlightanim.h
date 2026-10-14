#pragma once

#include <list>
#include <vector>

#include "math/color.h"
#include "math/key.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndlight.h"

/**
 * Animation of the colour of a light.
 *
 * The RTTI includes the class name and records RndAnimatable as the one base. The keys may belong
 * to another animation, the keys owner. Several animations then share one set.
 */
class RndLightAnim : public RndAnimatable {
public:
    /** The bit of the Copy() flags that shares the source's keys rather than copying them. */
    static constexpr int kCopyShareKeys = 2;

    /**
     * Construct an animation of nothing without keys.
     *
     * The animation is its own keys owner. The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit RndLightAnim(const char *pszName);

    /**
     * Drop the references on the light and the keys owner.
     *
     * @ghidraAddress NTSC-U/C: 0x00395a78
     */
    ~RndLightAnim() override;

    /**
     * Report the frame of the last colour key of the keys owner.
     *
     * @return The frame, or 0 without keys.
     * @ghidraAddress NTSC-U/C: 0x0024b980
     */
    float EndFrame() override;

    /**
     * Add the light to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0024b8c0
     * @ghidraAddress PAL: 0x00254328
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Set the colour of the light from the keys of the keys owner.
     *
     * Without keys the light receives its own colour again.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x0024b9a8
     * @ghidraAddress PAL: 0x00254410
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Write a description of the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0024b2b8
     * @ghidraAddress PAL: 0x00253d20
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0024b3a8
     * @ghidraAddress PAL: 0x00253e10
     */
    void Save(BinStream &stream) override;

    /**
     * Replace the light or the keys owner with another object.
     *
     * A replacement of another class, or null, clears the light. A keys owner replaced by null
     * becomes the animation itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0024b138
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00395d18
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another animation and its base.
     *
     * The keys are copied when the source has its own and the flags do not include
     * kCopyShareKeys. Otherwise the source's keys owner becomes this one's.
     *
     * @param pSource The animation to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0024b7f0
     * @ghidraAddress PAL: 0x00254258
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * Version 0 stores a further colour key list before the diffuse keys and another after them,
     * which are read and discarded.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0024b518
     * @ghidraAddress PAL: 0x00253f80
     */
    void Load(BinStream &stream) override;

    /**
     * Create an animation.
     *
     * @param pszName The registry key.
     * @return The animation.
     * @ghidraAddress NTSC-U/C: 0x00395d30
     */
    static RndObject *New(const char *pszName) {
        return new RndLightAnim(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `LightAnim`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a7c
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a80
     */
    static int sRev;

    RndLight *mLight;                     /*!< The light the animation drives. */
    std::vector<Key<Color>> mDiffuseKeys; /*!< The colour keys. */
    RndLightAnim *mKeysOwner;             /*!< The animation that has the keys. */

protected:
    /**
     * Drop the references on the light and the keys owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0024b478
     * @ghidraAddress PAL: 0x00253ee0
     */
    void ReleaseRefs();

    /**
     * Take references on the light and the keys owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0024b4c8
     * @ghidraAddress PAL: 0x00253f30
     */
    void AcquireRefs();
};
