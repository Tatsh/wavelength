#pragma once

/**
 * The settings of the game options screen.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the screen that sets
 * the members. Every setter marks the settings modified. Only the members its callers here use are
 * declared.
 */
class GameOptions {
public:
    /** Values of mFreqSize. */
    enum FreqSize {
        kFreqSizeLarge = 0,  /*!< The large display. */
        kFreqSizeSmall = 1,  /*!< The small display. */
        kFreqSizeHidden = 2, /*!< No display. */
    };

    /**
     * Construct the default settings, unmodified.
     *
     * @ghidraAddress NTSC-U/C: 0x0027c9b8
     * @ghidraAddress PAL: 0x00286328
     */
    GameOptions();

    /**
     * Set the size of the display of the tracks.
     *
     * @param nFreqSize One of FreqSize.
     * @ghidraAddress NTSC-U/C: 0x0027ca10
     * @ghidraAddress PAL: 0x00286380
     */
    void SetFreqSize(int nFreqSize);

    /**
     * Set the speaker output mode.
     *
     * @param nOutputMode The mode.
     * @ghidraAddress NTSC-U/C: 0x0027ca20
     * @ghidraAddress PAL: 0x00286390
     */
    void SetOutputMode(int nOutputMode);

    /**
     * Show or hide the hints.
     *
     * @param nHelpText 1 to show the hints, 0 to hide them.
     * @ghidraAddress NTSC-U/C: 0x0027ca30
     * @ghidraAddress PAL: 0x002863a0
     */
    void SetHelpText(int nHelpText);

    /**
     * Turn controller vibration on or off.
     *
     * @param nForceFeedback 1 for on, 0 for off.
     * @ghidraAddress NTSC-U/C: 0x0027ca40
     * @ghidraAddress PAL: 0x002863b0
     */
    void SetForceFeedback(int nForceFeedback);

    /**
     * Apply the speaker output mode to the synthesiser.
     *
     * @ghidraAddress NTSC-U/C: 0x0027caf0
     * @ghidraAddress PAL: 0x00286460
     */
    void ApplyOutputMode();

    int mFreqSize;                /*!< The size of the display of the tracks, one of FreqSize. */
    int mOutputMode;              /*!< The speaker output mode. */
    int mHelpText;                /*!< Whether hints are shown, as a word of 0 or 1. */
    int mForceFeedback;           /*!< Whether controller vibration is on. */
    unsigned char mReserved10[6]; // +0x10, cleared by the constructor and not yet identified.
    int mModified;                /*!< Whether a setter has changed the settings. */
};
