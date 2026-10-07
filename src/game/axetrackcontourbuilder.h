#pragma once

#include <utility>
#include <vector>

#include "game/axetrackdata.h"
#include "gs/axecontour.h"
#include "gs/midichannelstate.h"
#include "mid/midivalidator.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that reads the notes of a guitar track into AxeTrackData.
 *
 * The RTTI includes the class name and records TrackBuilder as the base. The object is 0x4b0
 * bytes. The gem buttons are notes in sets of three, one on each button, and each set gives the
 * span of the song its notes play over. The other channel messages are the notes and the sounds
 * of the spans.
 */
class AxeTrackContourBuilder : public TrackBuilder {
public:
    /** One channel message of the track. */
    struct MidiMsg {
        int mTick;             /*!< The tick. */
        unsigned char mStatus; /*!< The status byte. */
        unsigned char mData1;  /*!< The first data byte. */
        unsigned char mData2;  /*!< The second data byte. */
    };

    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The MIDI channel of the track.
     * @param nTicksPerBar The length of a bar.
     * @param nIntroTicks The length of the song intro, which must not have notes.
     * @param pData The data to fill.
     * @ghidraAddress NTSC-U/C: 0x00147ea0
     * @ghidraAddress PAL: 0x00149860
     */
    AxeTrackContourBuilder(const char *pszName,
                           bool bValidate,
                           ErrorHandler pfnError,
                           int nChannel,
                           int nTicksPerBar,
                           int nIntroTicks,
                           AxeTrackData *pData);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x00147f68
     * @ghidraAddress PAL: 0x00149928
     */
    ~AxeTrackContourBuilder() override;

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x00348558
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Build the notes of each set of three gem button notes, and add them to the data.
     *
     * @ghidraAddress NTSC-U/C: 0x00148078
     * @ghidraAddress PAL: 0x00149a38
     */
    void OnEndTrack() override;

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00348560
     */
    void OnAllDone() override {
    }

    /**
     * Record a gem button note, or collect any other channel message.
     *
     * A note-off message moves back to a multiple of 30 ticks.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x001486f0
     * @ghidraAddress PAL: 0x0014a0b0
     */
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00348568
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Ignore a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     * @ghidraAddress NTSC-U/C: 0x00348570
     */
    void OnText([[maybe_unused]] int nTick,
                [[maybe_unused]] const char *pszText,
                [[maybe_unused]] unsigned char nType) override {
    }

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x00348578
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    /**
     * Report the gem button a note plays.
     *
     * @param nNote The note.
     * @return The button, 0 to 2, or -1 for a note that is not a gem button.
     * @ghidraAddress NTSC-U/C: 0x00147e40
     * @ghidraAddress PAL: 0x00149800
     */
    static int ButtonForNote(unsigned char nNote);

    AxeTrackData *mData;                        /*!< The data to fill. */
    int mTicksPerBar;                           /*!< The length of a bar. */
    int mIntroTicks;                            /*!< The length of the song intro. */
    std::vector<MidiMsg> mMessages;             /*!< The channel messages in tick order. */
    std::vector<std::pair<int, int> > mButtons; /*!< The start and end tick of each note. */
    MidiChannelState mChannelState;             /*!< The controller state the notes start with. */
    MidiValidator mValidator;                   /*!< The checker of the channel messages. */

private:
    /**
     * Build the notes of a span from the channel messages.
     *
     * @param itMessage The first channel message of the span.
     * @param nEndTick The tick the span ends at.
     * @return The notes.
     * @ghidraAddress NTSC-U/C: 0x00148288
     * @ghidraAddress PAL: 0x00149c48
     */
    AxeContour *BuildContour(std::vector<MidiMsg>::iterator itMessage, int nEndTick);
};
