#pragma once

#include <list>
#include <vector>

#include "math/color.h"
#include "math/key.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndparticlesys.h"

/**
 * Animation of the start colour range, the end colour range, and the emission rate of a particle
 * system.
 *
 * The RTTI includes the class name and records RndAnimatable as the one base. The keys may belong
 * to another animation, the frames owner. Several animations then share one set. A colour
 * key moves a range by its low end and retains its width. An emission rate key sets the low end of
 * the rate, and the high end follows at the ratio the two ends had.
 */
class RndParticleSysAnim : public RndAnimatable {
public:
    /** The bit of the Copy() flags that shares the source's keys rather than copying them. */
    static constexpr int kCopyShareKeys = 0x80;

    /**
     * Construct an animation of nothing without keys.
     *
     * The animation is its own frames owner. The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit RndParticleSysAnim(const char *pszName);

    /**
     * Drop the references on the particle system and the frames owner.
     *
     * @ghidraAddress NTSC-U/C: 0x003909b0
     * @ghidraAddress PAL: 0x003ff0b8
     */
    ~RndParticleSysAnim() override;

    /**
     * Report the frame of the last key of any channel of the frames owner.
     *
     * @return The frame, or 0 without keys.
     * @ghidraAddress NTSC-U/C: 0x00240498
     * @ghidraAddress PAL: 0x00248fc8
     */
    float EndFrame() override;

    /**
     * Add the particle system to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002403d8
     * @ghidraAddress PAL: 0x00248f08
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Apply a frame to the particle system.
     *
     * Each channel with keys sets its part of the system.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x00240550
     * @ghidraAddress PAL: 0x00249080
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Write a description of the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023fde8
     * @ghidraAddress PAL: 0x00248918
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023ff40
     * @ghidraAddress PAL: 0x00248a70
     */
    void Save(BinStream &stream) override;

    /**
     * Replace the particle system or the frames owner with another object.
     *
     * A replacement of another class, or null, clears the particle system. A frames owner
     * replaced by null becomes the animation itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0023fc68
     * @ghidraAddress PAL: 0x00248798
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00390d08
     * @ghidraAddress PAL: 0x003ff410
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another animation and its base.
     *
     * The keys are copied when the source has its own and the flags do not include
     * kCopyShareKeys. Otherwise the source's frames owner becomes this one's.
     *
     * @param pSource The animation to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x002402e8
     * @ghidraAddress PAL: 0x00248e18
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * Version 1 adds the emission rate ratio.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002400e0
     * @ghidraAddress PAL: 0x00248c10
     */
    void Load(BinStream &stream) override;

    /**
     * Create an animation.
     *
     * @param pszName The registry key.
     * @return The animation.
     * @ghidraAddress NTSC-U/C: 0x00390d18
     * @ghidraAddress PAL: 0x003ff420
     */
    static RndObject *New(const char *pszName) {
        return new RndParticleSysAnim(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `ParticleSysAnim`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a40
     * @ghidraAddress PAL: 0x0042f8d8
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a44
     * @ghidraAddress PAL: 0x0042f8dc
     */
    static int sRev;

protected:
    /**
     * Drop the references on the particle system and the frames owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00240040
     * @ghidraAddress PAL: 0x00248b70
     */
    void ReleaseRefs();

    /**
     * Take references on the particle system and the frames owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00240090
     * @ghidraAddress PAL: 0x00248bc0
     */
    void AcquireRefs();

private:
    RndParticleSys *mParticleSys;            /*!< The particle system the animation drives. */
    std::vector<Key<Color>> mStartColorKeys; /*!< The low end of the start colour range. */
    std::vector<Key<Color>> mEndColorKeys;   /*!< The low end of the end colour range. */
    std::vector<Key<float>> mEmitRateKeys;   /*!< The low end of the emission rate. */
    RndParticleSysAnim *mFramesOwner;        /*!< The animation that has the keys. */
    float mEmitRateRatio; /*!< The ratio of the high end of the emission rate to the low end. */
};
