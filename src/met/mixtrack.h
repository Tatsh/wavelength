#pragma once

#include <cstddef>

#include "gs/muse.h"
#include "gs/muselooper.h"
#include "os/command.h"
#include "os/mem.h"
#include "os/ptr.h"
#include "os/ramp.h"

/**
 * One looping track of the front-end music, with a volume that fades in and out.
 *
 * The RTTI includes the class name in the nested class MixTrack::VolumeRamp. The class is not
 * polymorphic. The object is 0x24 bytes. The volume is the expression controller of the track's
 * MIDI channel. A track that fades out stops looping once the fade has ended.
 */
class MixTrack : public MuseLooper {
public:
    /**
     * Ramp of the volume of a track. Each new value goes to MixTrack::SetVolume().
     *
     * The RTTI includes the class name and records Ramp as the base. The object is 0x3c bytes.
     */
    class VolumeRamp : public Ramp {
    public:
        /**
         * Construct a ramp at 0 on the scheduler of the front end.
         *
         * No out-of-line body exists. The constructor of MixTrack expands it inline.
         *
         * @param pOwner The track.
         */
        explicit VolumeRamp(MixTrack *pOwner);

        /**
         * Stop the ramp.
         *
         * @ghidraAddress NTSC-U/C: 0x00356008
         * @ghidraAddress PAL: 0x003c32b8
         */
        ~VolumeRamp() override;

        /**
         * Set the volume of the track.
         *
         * @param fValue The volume.
         * @param nTick The tick. The routine does not read it.
         * @ghidraAddress NTSC-U/C: 0x00356078
         * @ghidraAddress PAL: 0x003c3328
         */
        void Apply(float fValue, int nTick) override;

        MixTrack *mOwner; /*!< The track. */
    };

    /**
     * Allocate a track from the tagged heap under the tag `MixTrack`.
     *
     * No out-of-line body exists. The callers expand the call inline.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "MixTrack", 0);
    }

    /**
     * Release a track to the tagged heap.
     *
     * No out-of-line body exists. The destructor expands the call inline.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Construct a silent, stopped track.
     *
     * The `music_max_volume` entry of the metagame configuration gives the volume the track fades
     * in to.
     *
     * @param pMuse The music of the track.
     * @param nLength The ticks of one loop.
     * @param nChannel The MIDI channel of the music.
     * @ghidraAddress NTSC-U/C: 0x0016a0c8
     * @ghidraAddress PAL: 0x0016d250
     */
    MixTrack(Muse *pMuse, int nLength, unsigned char nChannel);

    /**
     * Stop the track and release its ramp.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a1a8
     * @ghidraAddress PAL: 0x0016d330
     */
    ~MixTrack();

    /**
     * Start the track at a position of its loop and fade its volume up to the maximum.
     *
     * @param nFadeTicks The ticks the fade lasts.
     * @param nPosition The position of the loop the track starts at.
     * @ghidraAddress NTSC-U/C: 0x0016a228
     * @ghidraAddress PAL: 0x0016d3b0
     */
    void FadeIn(int nFadeTicks, int nPosition);

    /**
     * Fade a playing track's volume to 0 and stop the track once the fade has ended.
     *
     * @param nFadeTicks The ticks the fade lasts.
     * @ghidraAddress NTSC-U/C: 0x0016a2a8
     * @ghidraAddress PAL: 0x0016d430
     */
    void FadeOut(int nFadeTicks);

    /**
     * Stop the track and its fade at once and return its volume to full.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a318
     * @ghidraAddress PAL: 0x0016d4a0
     */
    void StopNow();

    /**
     * Report whether the track plays.
     *
     * @return Whether the track plays.
     * @ghidraAddress NTSC-U/C: 0x0016a378
     * @ghidraAddress PAL: 0x0016d500
     */
    bool IsPlaying() const;

    /**
     * Send a volume, limited to the range from 0 to 1, as the expression of the track's channel.
     *
     * @param fVolume The volume.
     * @ghidraAddress NTSC-U/C: 0x0016a398
     * @ghidraAddress PAL: 0x0016d520
     */
    void SetVolume(float fVolume);

    /**
     * Start moving the volume to a target.
     *
     * @param fTarget The target.
     * @param nTicks The ticks the move lasts.
     * @ghidraAddress NTSC-U/C: 0x0016a420
     * @ghidraAddress PAL: 0x0016d5a8
     */
    void RampTo(float fTarget, int nTicks);

    VolumeRamp *mRamp;      /*!< The ramp of the volume. */
    unsigned char mChannel; /*!< The MIDI channel of the music. */
    float mMaxVolume;       /*!< `music_max_volume`, the volume FadeIn() fades to. */
    Ptr<Command> mStopCmd;  /*!< The command that calls MuseLooper::Stop() after a fade out. */
};
