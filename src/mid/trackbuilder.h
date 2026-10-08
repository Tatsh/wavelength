#pragma once

#include "mid/midireceiver.h"
#include "os/string.h"

/**
 * MidiReceiver that turns one MIDI track into game data.
 *
 * The RTTI includes the class name and records MidiReceiver as the base. This header declares
 * only the members a ScratchTrackBuilder uses.
 */
class TrackBuilder : public MidiReceiver {
public:
    /**
     * Routine that reports a build error.
     *
     * @param message The message, with the track name and the location.
     */
    typedef void (*ErrorHandler)(String &message);

    /**
     * Construct a builder.
     *
     * @param nTrack The number of the MIDI track the error messages give.
     * @param bValidate Check the events for authoring errors.
     * @param pfnError The routine that reports an error, or null to print it.
     * @ghidraAddress NTSC-U/C: 0x00280af0
     * @ghidraAddress PAL: 0x0028a3f0
     */
    TrackBuilder(int nTrack, bool bValidate, ErrorHandler pfnError);

    /**
     * Release the builder.
     *
     * @ghidraAddress NTSC-U/C: 0x00280b10
     * @ghidraAddress PAL: 0x0028a410
     */
    ~TrackBuilder() override;

    /**
     * Report a build error at a tick.
     *
     * @param nTick The tick.
     * @param pszMessage The message.
     * @ghidraAddress NTSC-U/C: 0x00280b40
     * @ghidraAddress PAL: 0x0028a440
     */
    void Error(int nTick, const char *pszMessage);

    int mTrack;                 /*!< The number of the MIDI track. */
    bool mValidate;             /*!< Check the events for authoring errors. */
    ErrorHandler mErrorHandler; /*!< The routine that reports an error, or null. */

private:
    /**
     * Compose a message as `<prefix>(MIDI track <track>, tick <MBT>): <message>`.
     *
     * The name is inferred.
     *
     * @param prefix The text before the location.
     * @param nTick The tick, shown at four beats a bar and 480 ticks a beat.
     * @param nTrack The track.
     * @param message The text after the location.
     * @return The message.
     * @ghidraAddress NTSC-U/C: 0x002809a8
     * @ghidraAddress PAL: 0x0028a2a8
     */
    static String FormatMessage(const String &prefix, int nTick, int nTrack, const String &message);
};
