#pragma once

#include "os/command.h"
#include "os/ptr.h"

/**
 * Remix effect that gates the volume of chosen mixer channels on and off in time with the song.
 *
 * The RTTI records only the command the class schedules. The class is not polymorphic. The effect
 * runs while at least one channel is chosen: each gate period starts with the channels at their
 * mixer volume and silences them half a period later.
 */
class Stutter {
public:
    /** Number of MIDI channels. */
    static constexpr int kNumChannels = 16;

    /**
     * Construct a stopped effect, with the gate period chosen from the tempo of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00154720
     * @ghidraAddress PAL: 0x00155f88
     */
    Stutter();

    /**
     * Stop the effect.
     *
     * @ghidraAddress NTSC-U/C: 0x00154818
     * @ghidraAddress PAL: 0x00156080
     */
    ~Stutter();

    /**
     * Choose or release a channel, and start or stop the effect to match.
     *
     * A released channel returns to its mixer volume.
     *
     * @param nChannel The channel.
     * @param nOn 1 to choose the channel, 0 to release it.
     * @ghidraAddress NTSC-U/C: 0x001549c0
     * @ghidraAddress PAL: 0x00156228
     */
    void SetChannel(int nChannel, int nOn);

private:
    /**
     * Start gating at the next period boundary.
     *
     * @ghidraAddress NTSC-U/C: 0x00154878
     * @ghidraAddress PAL: 0x001560e0
     */
    void Start();

    /**
     * Stop gating and return the chosen channels to their mixer volume.
     *
     * @ghidraAddress NTSC-U/C: 0x00154950
     * @ghidraAddress PAL: 0x001561b8
     */
    void Stop();

    /**
     * Open the gate and schedule the next opening.
     *
     * @ghidraAddress NTSC-U/C: 0x00154a60
     * @ghidraAddress PAL: 0x001562c8
     */
    void Open();

    /**
     * Close the gate and schedule the next closing.
     *
     * @ghidraAddress NTSC-U/C: 0x00154ab8
     * @ghidraAddress PAL: 0x00156320
     */
    void Close();

    /**
     * Set the volume of every chosen channel.
     *
     * @param bOpen Whether the gate is open.
     * @ghidraAddress NTSC-U/C: 0x00154b10
     * @ghidraAddress PAL: 0x00156378
     */
    void SetVolumes(bool bOpen);

    /**
     * Set the volume of a channel, its mixer volume when the gate is open and 0 otherwise.
     *
     * @param nChannel The channel.
     * @param bOpen Whether the gate is open.
     * @ghidraAddress NTSC-U/C: 0x00154b88
     * @ghidraAddress PAL: 0x001563f0
     */
    void SetVolume(int nChannel, bool bOpen);

    int mRunning;                /*!< Whether the effect gates. */
    int mChannels[kNumChannels]; /*!< 1 for each chosen channel. */
    int mPeriodTicks;            /*!< The ticks of one gate period. */
    Ptr<Command> mOpenCmd;       /*!< The command that calls Open(). */
    Ptr<Command> mCloseCmd;      /*!< The command that calls Close(). */
};
