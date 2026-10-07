#pragma once

/**
 * Listener a MIDI file reader reports each track and event to.
 *
 * The RTTI includes the class name, and the class has no base. The vptr is the only member.
 */
class MidiReceiver {
public:
    /**
     * Release the receiver.
     *
     * @ghidraAddress NTSC-U/C: 0x0033a9b8
     * @ghidraAddress PAL: 0x003a7ef0
     */
    virtual ~MidiReceiver() {
    }

    /**
     * Begin a track.
     *
     * @param nTrack The track index.
     */
    virtual void OnNewTrack(unsigned char nTrack) = 0;

    /** End the track. */
    virtual void OnEndTrack() = 0;

    /** End the file. */
    virtual void OnAllDone() = 0;

    /**
     * Report a channel message.
     *
     * @param nTick The tick.
     * @param nStatus The status byte.
     * @param nData1 The first data byte.
     * @param nData2 The second data byte.
     */
    virtual void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) = 0;

    /**
     * Report a tempo change.
     *
     * @param nTick The tick.
     * @param nMicrosecondsPerBeat The new tempo.
     */
    virtual void OnTempo(int nTick, int nMicrosecondsPerBeat) = 0;

    /**
     * Report a text meta event.
     *
     * @param nTick The tick.
     * @param pszText The text.
     * @param nType The meta event type.
     */
    virtual void OnText(int nTick, const char *pszText, unsigned char nType) = 0;

    /**
     * Report a time signature.
     *
     * @param nTick The tick.
     * @param nNumerator The beats per bar.
     * @param nDenominator The beat unit.
     */
    virtual void OnTimeSignature(int nTick, int nNumerator, int nDenominator) = 0;
};
