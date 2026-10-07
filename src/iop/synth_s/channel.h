#pragma once

/** Positions in Channel::mControllerEnable, one per controller whose change updates voices. */
enum ChannelControllerEnable {
    kEnablePan = 0,        /*!< Pan, controller 10. */
    kEnablePitchBend = 1,  /*!< Pitch bend. */
    kEnableVolume = 2,     /*!< Volume, controller 7. */
    kEnableExpression = 3, /*!< Expression, controller 17. */
    kEnableTranspose = 4,  /*!< Transposition, controller 91. */
    kNumControllerEnables, /*!< Number of positions. */
};

/**
 * The state of one MIDI channel.
 *
 * The class has no RTTI, and its name is inferred from its role. A change to a controller whose
 * #mControllerEnable entry is at least 64 updates the channel's sounding voices at once; otherwise
 * it applies only to later notes.
 */
class Channel {
public:
    static constexpr int kNoBank = -1;    /*!< #mBank when the channel has no bank. */
    static constexpr int kNoProgram = -1; /*!< #mProgram when the channel has no program. */

    /** #mBusMode value that defers to each sample descriptor's bus and mix. */
    static constexpr unsigned char kBusModeFromSampleDesc = 4;

    /** Construct a channel with the defaults Init() sets. */
    Channel() {
        Init();
    }

    /**
     * Reset the controllers ResetAllControllers() covers, retaining the bank, program, pitch
     * bend, and transposition.
     *
     * It is never called.
     *
     * @ghidraAddress NTSC-U/C: 0x000020b8
     * @ghidraAddress PAL: 0x000020b8
     */
    void ResetControllers();

    /**
     * Reset every field, clearing the bank and program.
     *
     * @ghidraAddress NTSC-U/C: 0x00002114
     * @ghidraAddress PAL: 0x00002114
     */
    void Init();

    /**
     * Set the transposition.
     *
     * @param transpose Transposition in semitones.
     * @ghidraAddress NTSC-U/C: 0x00002188
     * @ghidraAddress PAL: 0x00002188
     */
    void SetTranspose(signed char transpose);

    int mBank;                       /*!< Bank slot, or #kNoBank. */
    int mProgram;                    /*!< Program slot in the bank, or #kNoProgram. */
    unsigned char mVolume;           /*!< Volume, controller 7. */
    unsigned char mExpression;       /*!< Expression, controller 17. */
    unsigned char mPan;              /*!< Pan, controller 10, where 64 is the centre. */
    signed char mPitchBendSemitones; /*!< Pitch bend, whole semitones. */
    signed char mPitchBendCents;     /*!< Pitch bend, remaining cents. */
    signed char mTranspose;          /*!< Transposition in semitones, controller 91. */
    unsigned char mPriority;         /*!< Voice priority, controller 16; lower is stolen first. */
    unsigned char mBusMode;          /*!< Dry and reverb mix, controller 93. */
    unsigned short mBus;             /*!< Preferred core, controller 94. */
    unsigned int mDetune;            /*!< Nonzero to add vibrato, controller 18. */
    unsigned int mStereo;            /*!< Nonzero for hard-panned pairs, controller 19. */
    unsigned int mMonophonic;        /*!< Nonzero to silence before a note, controller 13. */
    unsigned char mControllerEnable[kNumControllerEnables]; /*!< Live-update switches. */
};
