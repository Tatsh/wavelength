#pragma once

#include <vector>

#include "utl/Str.h"

/**
 * Format chunk of a RIFF WAVE file, as the reader takes it in one read.
 *
 * The name is inferred.
 */
struct WavFormatChunk {
    char mTag[4];                  /*!< Chunk identifier, `fmt ` in a valid file. */
    int mSize;                     /*!< Bytes of the chunk after this word. */
    unsigned short mFormatTag;     /*!< Encoding, 1 for PCM. */
    unsigned short mNumChannels;   /*!< Interleaved channels. */
    int mSamplesPerSec;            /*!< Sample rate in hertz. */
    int mAvgBytesPerSec;           /*!< Data rate in bytes per second. */
    unsigned short mBlockAlign;    /*!< Bytes of one frame of every channel. */
    unsigned short mBitsPerSample; /*!< Bits of one sample of one channel. */
};

/**
 * Source WAV file of a sample, and its conversion to the console's formats.
 *
 * The class has no RTTI, and its name is inferred from its source file. The object is 0x28 bytes.
 * The constructor reads the file once to learn its format. Every conversion reads it again.
 */
class AudioIO {
public:
    /**
     * Read the format of a WAV file.
     *
     * @param filename The path of the file.
     * @ghidraAddress 0x10029a16
     */
    explicit AudioIO(const String &filename);

    /**
     * Release the reader.
     *
     * @ghidraAddress 0x10029abd
     */
    ~AudioIO();

    /**
     * Read the file's samples and record its format.
     *
     * @param data Receives the bytes of the data chunk.
     * @return Whether the file was a WAV file with data.
     * @ghidraAddress 0x10029898
     */
    bool ReadWavData(std::vector<unsigned char> &data);

    /**
     * Convert a 48 kHz 16-bit stereo file to the layout of a streamed sample. Every 0x400 bytes
     * store 256 samples of the left channel and then 256 of the right. The result has at least
     * 0x800 bytes of silence at the end. A file in another format produces a notice and no data.
     *
     * @param data Receives the converted data.
     * @ghidraAddress 0x10029ad3
     */
    void GetStreamData(std::vector<unsigned char> &data);

    /**
     * List the start of every 28-sample block at which the waveform crosses zero.
     *
     * @param crossings Receives the sample index of each crossing.
     * @ghidraAddress 0x10029f04
     */
    void FindZeroCrossings(std::vector<int> &crossings);

    /**
     * Encode the file as VAG data, looping between two samples when the points are valid.
     *
     * @param vagData Receives the VAG data.
     * @param iLoopStart The first sample of the loop.
     * @param iLoopEnd The sample at which the loop ends, or zero for a one-shot sample.
     * @ghidraAddress 0x1002a0c3
     */
    void GetVagData(std::vector<unsigned char> &vagData, int iLoopStart, int iLoopEnd);

    /**
     * Report whether the last read failed.
     *
     * @return Whether the last read failed.
     * @ghidraAddress 0x100202b0
     */
    bool operator!() const {
        return !mGoodRead;
    }

    /**
     * Report the number of channels.
     *
     * @return The number of channels.
     */
    int GetNumChannels() const {
        return mNumChannels;
    }

    /**
     * Report the sample rate.
     *
     * @return The sample rate in hertz.
     */
    int GetSampleRate() const {
        return mSampleRate;
    }

    /**
     * Report the size of one sample.
     *
     * @return The size in bits.
     */
    int GetBitsPerSample() const {
        return mBitsPerSample;
    }

    /**
     * Report the path of the file.
     *
     * @return The path.
     */
    String &GetFilename() {
        return mFilename;
    }

    /**
     * Report the size of the VAG data of the file, before any loop adjustment.
     *
     * @return The size in bytes.
     */
    int GetVagSize() const {
        return mVagSize;
    }

private:
    int mNumChannels;   /*!< Channels of the file. */
    int mSampleRate;    /*!< Sample rate of the file in hertz. */
    int mBitsPerSample; /*!< Bits of one sample. */
    String mFilename;   /*!< Path of the file. */
    int mVagSize;       /*!< Bytes of the VAG data, three blocks more than the samples need. */
    bool mGoodRead;     /*!< Whether the last read succeeded. */
};
