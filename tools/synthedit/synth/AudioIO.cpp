#include "synth/AudioIO.h"

#include <string.h>

#include <encvag.h>

#include "os/Debug.h"
#include "os/File.h"

namespace {

// Bytes of PCM data that one 16-byte VAG block encodes, 28 16-bit samples.
const int WAV_BLOCK_SIZE = 56;
const int kVagBlockSize = 16;
const int kVagExtraBlocks = 3;

const int kStreamChannels = 2;
const int kStreamSampleRate = 48000;
const int kStreamBitsPerSample = 16;
const int kStreamFrameBytes = 4;
const int kStreamChunkBytes = 0x400;
const int kStreamSilenceBytes = 0x800;
const int kStreamSamplesPerChunk = 256;
const int kStreamSampleBytes = 2;

const short kEncodeModeNormal = 1;

// Block attributes of the encoder.
const short kVagOneShot = 0;
const short kVagOneShotEnd = 1;
const short kVagLoopStart = 2;
const short kVagLoopBody = 3;
const short kVagLoopEnd = 4;

// 0x100321cc
const char kRiffTag[] = { 'R', 'I', 'F', 'F' };
// 0x100321d0
const char kWaveTag[] = { 'W', 'A', 'V', 'E' };
// 0x100321d4
const char kDataTag[] = { 'd', 'a', 't', 'a' };

const int kTagSize = 4;

// The encoder takes 16-bit samples and VAG bytes as shorts; the buffers here are bytes.
inline short *AsShorts(unsigned char *bytes) {
    return reinterpret_cast<short *>(bytes);
}

// 0x10029730
bool ReadWavHeader(File *file, WavFormatChunk *format, std::vector<unsigned char> &data) {
    char tag[kTagSize];
    unsigned int size;

    file->Read(tag, sizeof(tag));
    if (memcmp(tag, kRiffTag, sizeof(tag)) != 0) {
        TheDebug.Notify("Error: Your wave file is not a standard Microsoft RIFF wav file.\n");
        return false;
    }
    file->Read(&size, sizeof(size));
    if (size == 0) {
        TheDebug.Notify("Error: Badly formatted wav file.\n");
        return false;
    }
    file->Read(tag, sizeof(tag));
    if (memcmp(tag, kWaveTag, sizeof(tag)) != 0) {
        TheDebug.Notify("Audio file is not a .wav formatted file.\n");
        return false;
    }
    file->Read(format, sizeof(*format)); // The tag of the format chunk is not checked.
    file->Read(tag, sizeof(tag));
    if (memcmp(tag, kDataTag, sizeof(tag)) != 0) {
        TheDebug.Notify("Error: Badly formatted wav file.\n");
        return false;
    }
    file->Read(&size, sizeof(size));
    if (size == 0) {
        TheDebug.Notify("Error: The wav data in this file has zero length.\n");
        return false;
    }
    data.resize(size, 0);
    file->Read(&data[0], size);
    return true;
}

} // namespace

bool AudioIO::ReadWavData(std::vector<unsigned char> &data) {
    mGoodRead = false;
    File *file = NewFile(mFilename.c_str(), FILE_OPEN_READ, 0);
    if (file == NULL) {
        return false;
    }
    WavFormatChunk format;
    bool goodRead = ReadWavHeader(file, &format, data);
    if (!goodRead) {
        String message(FormatString("The preceeding error was in file: %s.\n", mFilename.c_str()));
        TheDebug.Notify(message.c_str());
        delete file;
        return false;
    }
    mBitsPerSample = format.mBitsPerSample;
    mSampleRate = format.mSamplesPerSec;
    mNumChannels = format.mNumChannels;
    mVagSize = (static_cast<int>(data.size()) / WAV_BLOCK_SIZE + kVagExtraBlocks) * kVagBlockSize;
    delete file;
    mGoodRead = true;
    return true;
}

AudioIO::AudioIO(const String &filename) : mFilename(filename), mVagSize(0), mGoodRead(false) {
    std::vector<unsigned char> data;
    (void)ReadWavData(data); // The read only records the format.
}

AudioIO::~AudioIO() {
}

void AudioIO::GetStreamData(std::vector<unsigned char> &data) {
    std::vector<unsigned char> rawData;
    bool goodRead = ReadWavData(rawData);
    ASSERT(goodRead);
    int numBytesInWavFile = rawData.size();
    if (GetNumChannels() != kStreamChannels) {
        TheDebug.Notify("Streaming wav files must be stereo.\n");
        return;
    }
    if (GetSampleRate() != kStreamSampleRate) {
        TheDebug.Notify("Streaming samples must be sampled at 48kHz.\n");
        return;
    }
    if (GetBitsPerSample() != kStreamBitsPerSample) {
        TheDebug.Notify("Streaming samples must be 16 bit.\n");
        return;
    }
    ASSERT(numBytesInWavFile % kStreamFrameBytes == 0);
    int numBytesWeWant = numBytesInWavFile;
    if (numBytesWeWant % kStreamChunkBytes > 0) {
        numBytesWeWant += kStreamChunkBytes;
        numBytesWeWant -= numBytesWeWant % kStreamChunkBytes;
    }
    numBytesWeWant += kStreamSilenceBytes;
    ASSERT(numBytesWeWant % kStreamFrameBytes == 0);
    data.clear();
    data.resize(numBytesWeWant, 0);
    for (int i = 0; i < numBytesWeWant; ++i) {
        data[i] = 0;
    }

    unsigned char *wavData = &rawData[0];
    int byteCounter = 0;
    unsigned char *left = NULL;
    unsigned char *right = NULL;
    left = wavData;
    right = left + kStreamSampleBytes;
    unsigned char *streamData = &data[0];
    int chunk = 0;
    while (byteCounter < numBytesInWavFile) {
        short leftSamples[kStreamSamplesPerChunk];
        short rightSamples[kStreamSamplesPerChunk];
        memset(leftSamples, 0, sizeof(leftSamples));
        memset(rightSamples, 0, sizeof(rightSamples));
        for (int i = 0; i < kStreamSamplesPerChunk; ++i) {
            memcpy(&leftSamples[i], left, kStreamSampleBytes);
            memcpy(&rightSamples[i], right, kStreamSampleBytes);
            left += kStreamFrameBytes;
            right += kStreamFrameBytes;
            byteCounter += kStreamFrameBytes;
            ASSERT(byteCounter <= numBytesInWavFile);
            if (byteCounter == numBytesInWavFile) {
                break;
            }
        }
        memcpy(streamData + chunk * kStreamChunkBytes, leftSamples, sizeof(leftSamples));
        memcpy(streamData + chunk * kStreamChunkBytes + sizeof(leftSamples),
               rightSamples,
               sizeof(rightSamples));
        ++chunk;
    }
}

void AudioIO::FindZeroCrossings(std::vector<int> &crossings) {
    crossings.clear();
    std::vector<unsigned char> wavData;
    bool goodRead = ReadWavData(wavData);
    ASSERT(goodRead);
    unsigned char *rawData = &wavData[0];
    ASSERT(rawData);
    int numBytesInWavFile = wavData.size();
    ASSERT(numBytesInWavFile % 2 == 0);
    short sample = 0;
    short previous = sample;
    int numSamples = numBytesInWavFile / 2; // Computed and never used.
    for (int i = 0; i < numBytesInWavFile; i += WAV_BLOCK_SIZE) {
        memcpy(&previous, rawData + i, sizeof(previous));
        memcpy(&sample, rawData + i + sizeof(previous), sizeof(sample));
        if ((previous <= 0 && sample >= 0) || (previous >= 0 && sample <= 0)) {
            int crossing = i / 2;
            crossings.push_back(crossing);
        }
    }
}

void AudioIO::GetVagData(std::vector<unsigned char> &vagData, int iLoopStart, int iLoopEnd) {
    std::vector<unsigned char> rawData;
    bool goodRead = ReadWavData(rawData);
    ASSERT(goodRead);
    int wavSize = rawData.size();
    bool looping = iLoopStart < iLoopEnd && iLoopEnd != 0;
    if (iLoopEnd <= iLoopStart && iLoopEnd != 0) {
        String message(FormatString(
            "The sample using audio file --%s-- has the loop end point earlier or equal to the "
            "loop start point. Sample will not be looped.",
            mFilename.c_str()));
        TheDebug.Notify(message.c_str());
        looping = false;
    }

    iLoopStart *= kStreamSampleBytes;
    iLoopEnd *= kStreamSampleBytes;
    if (iLoopStart > wavSize || iLoopEnd > wavSize) {
        String message(
            FormatString("Error: Filename: %s - Loop end point = %i, file size = %i. Not looping.",
                         mFilename.c_str(),
                         iLoopEnd,
                         wavSize));
        TheDebug.Notify(message.c_str());
        looping = false;
    }
    if (looping && iLoopEnd - iLoopStart < WAV_BLOCK_SIZE) {
        String message(FormatString("Error: Filename = %s, loop points w/i single VAG block. loop "
                                    "start = %i, loop end = %i. Not looping.",
                                    mFilename.c_str(),
                                    iLoopStart,
                                    iLoopEnd));
        TheDebug.Notify(message.c_str());
        looping = false;
    }

    int roughSize = 0;
    if (looping) {
        roughSize = iLoopEnd / WAV_BLOCK_SIZE;
        if (iLoopEnd >= roughSize * WAV_BLOCK_SIZE) {
            ASSERT(iLoopEnd < (roughSize + 1) * WAV_BLOCK_SIZE);
            ++roughSize;
        }
        ++roughSize;
        TheDebug.Printf("roughSize = %i, iLoopStart = %i, iLoopEnd = %i, totalSamples = %i.\n",
                        roughSize,
                        iLoopStart,
                        iLoopEnd,
                        roughSize * WAV_BLOCK_SIZE);
    } else {
        roughSize = wavSize / WAV_BLOCK_SIZE;
        if (wavSize > roughSize * WAV_BLOCK_SIZE) {
            ASSERT(wavSize < (roughSize + 1) * WAV_BLOCK_SIZE);
            ++roughSize;
        }
        ++roughSize;
        ++roughSize;
    }
    roughSize *= kVagBlockSize;
    vagData.clear();
    vagData.resize(roughSize, 0);

    EncVagInit(kEncodeModeNormal);
    int vagMarker = 0;
    int wavMarker = 0;
    for (int i = 0; i < kVagBlockSize; ++i) {
        vagData[i] = 0;
        ++vagMarker;
    }
    int blockAttribute;
    while (wavSize - wavMarker > WAV_BLOCK_SIZE) {
        if (!looping) {
            blockAttribute = kVagOneShot;
        } else if (iLoopStart >= wavMarker && iLoopStart < wavMarker + WAV_BLOCK_SIZE) {
            blockAttribute = kVagLoopStart;
            TheDebug.Printf("starting loop at sample #%i.\n", wavMarker);
        } else if (iLoopEnd >= wavMarker && iLoopEnd < wavMarker + WAV_BLOCK_SIZE) {
            blockAttribute = kVagLoopEnd;
            TheDebug.Printf("stopping loop at sample #%i.\n", wavMarker);
        } else {
            blockAttribute = kVagLoopBody;
        }
        EncVag(AsShorts(&rawData[wavMarker]),
               AsShorts(&vagData[vagMarker]),
               static_cast<short>(blockAttribute));
        vagMarker += kVagBlockSize;
        wavMarker += WAV_BLOCK_SIZE;
        if (blockAttribute == kVagLoopEnd) {
            break;
        }
    }
    TheDebug.Printf("**** out of the encoding loop, wavMarker = %i.\n", wavMarker);

    if (looping) {
        if (blockAttribute != kVagLoopEnd) {
            blockAttribute = kVagLoopEnd;
            std::vector<unsigned char> lastBlock(WAV_BLOCK_SIZE, 0);
            memcpy(&lastBlock[0], &rawData[wavMarker], iLoopEnd - wavMarker);
            EncVag(AsShorts(&lastBlock[0]),
                   AsShorts(&vagData[vagMarker]),
                   static_cast<short>(blockAttribute));
        }
    } else {
        ASSERT(wavSize - wavMarker <= WAV_BLOCK_SIZE);
        blockAttribute = kVagOneShotEnd;
        std::vector<unsigned char> lastBlock(WAV_BLOCK_SIZE, 0);
        memcpy(&lastBlock[0], &rawData[wavMarker], wavSize - wavMarker);
        EncVag(AsShorts(&lastBlock[0]),
               AsShorts(&vagData[vagMarker]),
               static_cast<short>(blockAttribute));
        vagMarker += kVagBlockSize;
        EncVagFin(AsShorts(&vagData[vagMarker]));
    }
}
