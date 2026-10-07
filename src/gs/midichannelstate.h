#pragma once

#include "gs/muse.h"
#include "gs/stdmidimuse.h"
#include "os/ptr.h"

/**
 * The controller state of one MIDI channel, as the channel messages of a track change it.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. MuseFactory and
 * AxeTrackContourBuilder track the state of their channel with one. The state turns into the
 * muses that restore the program, the controllers, the pressure, and the pitch bend in force at
 * the start of a piece cut from the middle of a track.
 */
class MidiChannelState {
public:
    /** Number of MIDI controllers. */
    static constexpr int kNumControllers = 128;

    /**
     * Construct the state of a channel with nothing set.
     *
     * @param nChannel The MIDI channel.
     * @ghidraAddress NTSC-U/C: 0x001580b0
     * @ghidraAddress PAL: 0x00159938
     */
    explicit MidiChannelState(unsigned char nChannel);

    /**
     * Release the muses.
     *
     * @ghidraAddress NTSC-U/C: 0x00158170
     * @ghidraAddress PAL: 0x001599f8
     */
    ~MidiChannelState();

    /**
     * Record a channel message of the channel that changes the state.
     *
     * Program change, channel pressure, pitch bend, and control change messages for controllers
     * below 96 count. Other messages are ignored.
     *
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00158228
     * @ghidraAddress PAL: 0x00159ab0
     */
    void OnMessage(unsigned char nStatus, unsigned char nData1, unsigned char nData2);

    /**
     * Report the muse that restores the state, built again when the state changed.
     *
     * @return The muse, or null when nothing is set.
     * @ghidraAddress NTSC-U/C: 0x00158640
     * @ghidraAddress PAL: 0x00159ec8
     */
    Muse *GetMuse();

    /**
     * Clear the state.
     *
     * @ghidraAddress NTSC-U/C: 0x00158cb0
     * @ghidraAddress PAL: 0x0015a538
     */
    void Reset();

    /**
     * Clear the state and change the channel.
     *
     * @param nChannel The MIDI channel.
     * @ghidraAddress NTSC-U/C: 0x00158e08
     */
    void SetChannel(unsigned char nChannel);

private:
    /**
     * Report the muse of the program, made again when the program changed.
     *
     * @return The muse, or null when no program is set.
     * @ghidraAddress NTSC-U/C: 0x00158350
     * @ghidraAddress PAL: 0x00159bd8
     */
    StdMidiMuse *GetProgramMuse();

    /**
     * Report the muse of the channel pressure, made again when the pressure changed.
     *
     * @return The muse, or null when no pressure is set.
     * @ghidraAddress NTSC-U/C: 0x001583f0
     * @ghidraAddress PAL: 0x00159c78
     */
    StdMidiMuse *GetPressureMuse();

    /**
     * Report the muse of the pitch bend, made again when the bend changed.
     *
     * @return The muse, or null when no bend is set.
     * @ghidraAddress NTSC-U/C: 0x00158490
     * @ghidraAddress PAL: 0x00159d18
     */
    StdMidiMuse *GetPitchBendMuse();

    /**
     * Report the muse of a controller, made again when its value changed.
     *
     * @param nController The controller.
     * @return The muse, or null when the controller is not set.
     * @ghidraAddress NTSC-U/C: 0x00158568
     * @ghidraAddress PAL: 0x00159df0
     */
    StdMidiMuse *GetControllerMuse(unsigned char nController);

    unsigned char mChannel;                             /*!< The MIDI channel. */
    int mProgram;                                       /*!< The program, or -1. */
    int mPressure;                                      /*!< The channel pressure, or -1. */
    int mBendLow;                                       /*!< The low bits of the bend, or -1. */
    int mBendHigh;                                      /*!< The high bits of the bend, or -1. */
    int mControllers[kNumControllers];                  /*!< The controller values, or -1. */
    Ptr<StdMidiMuse> mProgramMuse;                      /*!< The muse of the program. */
    Ptr<StdMidiMuse> mPressureMuse;                     /*!< The muse of the pressure. */
    Ptr<StdMidiMuse> mBendMuse;                         /*!< The muse of the bend. */
    Ptr<StdMidiMuse> mControllerMuses[kNumControllers]; /*!< The muse of each controller. */
    Ptr<Muse> mMuse;                                    /*!< The muse GetMuse() built last. */
    int mChanged;                                       /*!< Whether the state changed since. */
};
