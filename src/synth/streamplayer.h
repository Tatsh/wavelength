#pragma once

#include <memory>

#include "os/filestreambuffer.h"

/**
 * Stream of sound data read from a file and handed to the sound output a run at a time.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x18 bytes.
 * Synth::SetStream() gives the output the stream to play, and the output polls it once a frame.
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
     * @ghidraAddress PAL: 0x0025a438
     */
    StreamPlayer(int nChannel, const char *pszFile, bool bAudio);

    /**
     * Close the stream file.
     *
     * @ghidraAddress NTSC-U/C: 0x00251b58
     * @ghidraAddress PAL: 0x0025a500
     */
    ~StreamPlayer();

    /**
     * Pass the next run of the file to the sound output, and end the stream once the file ends.
     *
     * The end is sent 40 milliseconds after the last run. A stream that does not play is
     * unchanged. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00251bb8
     * @ghidraAddress PAL: 0x0025a560
     */
    void Poll();

    /**
     * Report whether the file ended and the end of the stream was sent.
     *
     * The name is inferred.
     *
     * @return Whether the stream finished.
     * @ghidraAddress NTSC-U/C: 0x00251dc0
     * @ghidraAddress PAL: 0x0025a768
     */
    bool IsFinished();

    /**
     * Set whether the stream plays. The stream clears the flag once it has finished.
     *
     * @param bPlaying Whether the stream plays.
     * @ghidraAddress NTSC-U/C: 0x00251e08
     * @ghidraAddress PAL: 0x0025a7b0
     */
    void SetPlaying(bool bPlaying);

    /**
     * Report whether the stream plays.
     *
     * @return Whether the stream plays.
     * @ghidraAddress NTSC-U/C: 0x00251e10
     * @ghidraAddress PAL: 0x0025a7b8
     */
    bool IsPlaying() const;

    /** @return The value the constructor received first. */
    int GetChannel() const {
        return mChannel;
    }

    /** @return The reader of the stream file. */
    FileStreamBuffer *GetBuffer() const {
        return mBuffer.get();
    }

private:
    int mChannel;                              /*!< The value the constructor received first. */
    int mPlaying;                              /*!< Non-zero while the stream plays. */
    std::unique_ptr<FileStreamBuffer> mBuffer; /*!< The reader of the stream file. */
    char *mRun;   /*!< The run handed to the sound output last, or null. Borrowed. */
    float mEndMs; /*!< The system time to end the stream at, or -1 before the end. */
    int mEndSent; /*!< Whether the end of the stream was sent. */
};
