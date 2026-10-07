#pragma once

/**
 * The settings of the game options screen.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the screen that sets
 * the members. Only the members its callers here read are declared.
 */
struct GameOptions {
    /** Values of mFreqSize. */
    enum FreqSize {
        kFreqSizeLarge = 0,  /*!< The large display. */
        kFreqSizeSmall = 1,  /*!< The small display. */
        kFreqSizeHidden = 2, /*!< No display. */
    };

    int mFreqSize;      /*!< The size of the display of the tracks, one of FreqSize. +0x00 */
    int mOutputMode;    /*!< The speaker output mode. +0x04 */
    int mHelpText;      /*!< Whether hints are shown, as a word of 0 or 1. +0x08 */
    int mForceFeedback; /*!< Whether controller vibration is on. The name is inferred. +0x0c */
};
