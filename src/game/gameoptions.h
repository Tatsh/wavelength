#pragma once

#include "os/binstream.h"
#include "os/datetime.h"

/**
 * The settings of the game options screen, which are saved apart from the players to the
 * `settings` file of the Freq directory.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the screen that sets
 * the members. The object is 0x1c bytes. Every setter marks the settings modified.
 */
class GameOptions {
public:
    /** Values of mFreqSize. */
    enum FreqSize {
        kFreqSizeLarge = 0,  /*!< The large display. */
        kFreqSizeSmall = 1,  /*!< The small display. */
        kFreqSizeHidden = 2, /*!< No display. */
    };

    /** Values of mOutputMode. */
    enum OutputMode {
        kOutputMono = 0,     /*!< Mono output. */
        kOutputStereo = 1,   /*!< Stereo output. */
        kOutputSurround = 2, /*!< Stereo output that surround decoders may invert. */
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
     * Record the clock reading in mDate and mark the settings modified.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0027ca50
     * @ghidraAddress PAL: 0x002863c0
     */
    void UpdateDate();

    /**
     * Report whether mDate falls on the day the clock reads.
     *
     * The name is inferred.
     *
     * @return Whether the day, month, and year match.
     * @ghidraAddress NTSC-U/C: 0x0027ca80
     * @ghidraAddress PAL: 0x002863f0
     */
    bool IsToday() const;

    /**
     * Apply the speaker output mode to the synthesiser.
     *
     * An unknown mode is reported and changes nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x0027caf0
     * @ghidraAddress PAL: 0x00286460
     */
    void ApplyOutputMode();

    /**
     * Record today as the day of the last remix upload, and mark the settings modified.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0027ca50
     * @ghidraAddress PAL: 0x002863c0
     */
    void RecordUpload();

    /**
     * Report whether the last remix upload was today.
     *
     * The name is inferred.
     *
     * @return Whether the day, month, and year of mDate match the clock.
     * @ghidraAddress NTSC-U/C: 0x0027ca80
     * @ghidraAddress PAL: 0x002863f0
     */
    bool UploadedToday() const;

    /**
     * Write the settings after version byte 3.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0027cbe0
     */
    void Save(BinStream &stream) const;

    /**
     * Read the settings Save() wrote, or an older version of them.
     *
     * Before version 3 the output mode is a byte that is ignored, and the mode becomes stereo.
     * Version 0 records only the output mode, and versions before 2 record no date.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0027ccb8
     * @ghidraAddress PAL: 0x00286608
     */
    void Load(BinStream &stream);

    int mFreqSize;      /*!< The size of the display of the tracks, one of FreqSize. */
    int mOutputMode;    /*!< The speaker output mode. */
    int mHelpText;      /*!< Whether hints are shown, as a word of 0 or 1. */
    int mForceFeedback; /*!< Whether controller vibration is on. */
    DateTime mDate;     /*!< The clock reading of the last remix upload. */
    int mModified;      /*!< Whether the settings changed since they were saved. */
};

/**
 * Write settings to a stream, after a version byte.
 *
 * @param stream The stream.
 * @param options The settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027cde0
 * @ghidraAddress PAL: 0x002866f8
 */
BinStream &operator<<(BinStream &stream, const GameOptions &options);

/**
 * Read settings from a stream.
 *
 * @param stream The stream.
 * @param options Receives the settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027ce10
 * @ghidraAddress PAL: 0x00286728
 */
BinStream &operator>>(BinStream &stream, GameOptions &options);
