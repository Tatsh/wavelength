#pragma once

#include "game/scripttrackdata.h"
#include "mid/trackbuilder.h"

/**
 * TrackBuilder that parses the text events of a song's SCRIPT track into script commands.
 *
 * The RTTI includes the class name and records TrackBuilder as the base.
 */
class ScriptTrackBuilder : public TrackBuilder {
public:
    /**
     * Construct a builder.
     *
     * @param pszName The track name the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nReserved The value the constructor records at `+0x10`.
     * @param pData The data to fill.
     * @ghidraAddress NTSC-U/C: 0x00138b70
     * @ghidraAddress PAL: 0x0013a3d0
     */
    ScriptTrackBuilder(const char *pszName,
                       bool bValidate,
                       ErrorHandler pfnError,
                       int nReserved,
                       ScriptTrackData *pData);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x003407d0
     * @ghidraAddress PAL: 0x003add08
     */
    ~ScriptTrackBuilder() override {
    }

    /**
     * Ignore the start of the track.
     *
     * @param nTrack The track index.
     * @ghidraAddress NTSC-U/C: 0x003407f8
     */
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    /**
     * Ignore the end of the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00340818
     */
    void OnEndTrack() override {
    }

    /**
     * Ignore the end of the file.
     *
     * @ghidraAddress NTSC-U/C: 0x00340800
     */
    void OnAllDone() override {
    }

    /**
     * Ignore a channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x00340820
     */
    void OnMidi([[maybe_unused]] int nTick,
                [[maybe_unused]] unsigned char nStatus,
                [[maybe_unused]] unsigned char nData1,
                [[maybe_unused]] unsigned char nData2) override {
    }

    /**
     * Ignore a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     * @ghidraAddress NTSC-U/C: 0x00340808
     */
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    /**
     * Parse a text event into a command at its tick.
     *
     * @param nTick The tick.
     * @param pszText The command text.
     * @param nType The meta event type, which the builder ignores.
     * @ghidraAddress NTSC-U/C: 0x00138bc8
     * @ghidraAddress PAL: 0x0013a428
     */
    void OnText(int nTick, const char *pszText, unsigned char nType) override;

    /**
     * Ignore a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     * @ghidraAddress NTSC-U/C: 0x00340810
     */
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

    int mReserved10;        // +0x10, recorded and not yet identified.
    ScriptTrackData *mData; /*!< The data to fill. */
};
