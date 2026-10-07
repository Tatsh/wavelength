#pragma once

#include "mid/trackbuilder.h"

/**
 * Checker of the channel messages of one MIDI track for authoring errors.
 *
 * The errors are messages on the wrong channel, duplicate notes, program changes, and controller
 * messages, and a note before the first program or bank change. The class is not polymorphic and
 * has no RTTI. The name is inferred. This header declares only the members a ScratchTrackBuilder
 * uses.
 */
class MidiValidator {
public:
    /**
     * Construct a checker.
     *
     * @param pszName The track name the error messages give.
     * @param pfnError The routine that reports an error, or null to print it.
     * @param nChannel The channel every message must use.
     * @param bRequirePrograms Report a note that precedes the first program change.
     * @ghidraAddress NTSC-U/C: 0x0027e1e8
     * @ghidraAddress PAL: 0x00287b00
     */
    MidiValidator(const char *pszName,
                  TrackBuilder::ErrorHandler pfnError,
                  int nChannel,
                  bool bRequirePrograms);

    /**
     * Release the checker.
     *
     * @ghidraAddress NTSC-U/C: 0x0033cd90
     * @ghidraAddress PAL: 0x003aa2c8
     */
    ~MidiValidator();

    /**
     * Check one channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     * @ghidraAddress NTSC-U/C: 0x0027e340
     * @ghidraAddress PAL: 0x00287c58
     */
    void Check(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2);
};
