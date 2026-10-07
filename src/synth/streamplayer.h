#pragma once

/**
 * Stream of sound data read from a file and handed to the sound output a run at a time.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x18 bytes.
 * Synth::SetStream() gives the output the stream to play. Only the members SongPreview uses are
 * declared, and the routines are not reconstructed.
 */
class StreamPlayer {
public:
    /**
     * Open a stream file, or `SoundFX/default.str` when the file does not open.
     *
     * @param nChannel The value recorded first in the object. SongPreview passes 0.
     * @param pszFile The stream file.
     * @param bAudio Whether the file is one of the `audio` streams, passed to the file opener.
     * @ghidraAddress NTSC-U/C: 0x00251a90
     */
    StreamPlayer(int nChannel, const char *pszFile, bool bAudio);

    /**
     * Close the stream file.
     *
     * @ghidraAddress NTSC-U/C: 0x00251b58
     */
    ~StreamPlayer();

    /**
     * Set whether the stream plays. The sound output clears the flag once the file has ended.
     *
     * @param bPlaying Whether the stream plays.
     * @ghidraAddress NTSC-U/C: 0x00251e08
     */
    void SetPlaying(bool bPlaying);

    /**
     * Report whether the stream plays.
     *
     * @return Whether the stream plays.
     * @ghidraAddress NTSC-U/C: 0x00251e10
     */
    bool IsPlaying() const;
};
