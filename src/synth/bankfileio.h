#pragma once

#include <vector>

#include "os/file.h"
#include "os/filestream.h"
#include "os/string.h"
#include "synth/lazyfilestream.h"

/**
 * Extension of a bank file. Each translation unit that includes this header has its own copy.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c840
 * @ghidraAddress NTSC-U/C: 0x0043ca00
 */
static const String kBankExtension(".bnk");

/**
 * Extension of the audio data file that goes with a bank file. Each translation unit that
 * includes this header has its own copy.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c858
 * @ghidraAddress NTSC-U/C: 0x0043ca18
 */
static const String kAudioDataExtension(".nse");

/**
 * Reader of a bank file and the audio data file beside it.
 *
 * The class has no RTTI, and its name comes from the synthesiser editor that shares the source.
 * The object is 0x44 bytes. A bank file is a run of chunks, each an identifier of four characters,
 * a size word, and the data. The reader finds a chunk with asynchronous reads that the caller
 * polls. The writer's routines are absent from the game.
 */
class BankFileIO {
public:
    /** Directions of a BankFileIO. */
    enum IODirection {
        kRead = 0,  /*!< Read an existing bank. */
        kWrite = 1, /*!< Write a bank. */
    };

    /** Chunks of a bank file. */
    enum ChunkType {
        kBankChunk = 0,                  /*!< The bank header, `BANK`. */
        kInstrumentChunk = 1,            /*!< The instruments, `INST`. */
        kSampleDescriptionChunk = 2,     /*!< The sample descriptions, `SDES`. */
        kSampleChunk = 3,                /*!< The samples, `SAMP`. */
        kSampleFilenameChunk = 4,        /*!< The WAV file of each sample, `SAFN`. */
        kBankNameChunk = 5,              /*!< The bank name, `BKNM`. */
        kInstrumentNameChunk = 6,        /*!< The instrument names, `INNM`. */
        kSampleDescriptionNameChunk = 7, /*!< The sample description names, `SDNM`. */
        kSampleNameChunk = 8,            /*!< The sample names, `SANM`. */
    };

    /**
     * Open a bank file for reading or writing.
     *
     * @param filename The path of the bank file.
     * @param nIODirection The direction, an IODirection.
     * @ghidraAddress NTSC-U/C: 0x00250fb0
     * @ghidraAddress PAL: 0x00259960
     */
    BankFileIO(const String &filename, int nIODirection);

    /**
     * Close every file.
     *
     * @ghidraAddress NTSC-U/C: 0x00251080
     * @ghidraAddress PAL: 0x00259a30
     */
    ~BankFileIO();

    /**
     * Report whether the bank file failed to open.
     *
     * @return Whether no bank file is open.
     * @ghidraAddress NTSC-U/C: 0x00251120
     * @ghidraAddress PAL: 0x00259ad0
     */
    bool operator!() const;

    /**
     * Derive the path of the audio data file from the path of a bank file.
     *
     * @param bankFileName The path of the bank file.
     * @return The path with the bank extension replaced.
     * @ghidraAddress NTSC-U/C: 0x00251148
     * @ghidraAddress PAL: 0x00259af8
     */
    static String AudioDataFilename(const String &bankFileName);

    /**
     * Report the identifier of a chunk.
     *
     * @param nChunkType The chunk, a ChunkType.
     * @return The four characters of the identifier, without a terminator, or null for an unknown
     *     chunk.
     * @ghidraAddress NTSC-U/C: 0x002511c0
     * @ghidraAddress PAL: 0x00259b78
     */
    const char *GetChunkTag(int nChunkType);

    /**
     * Start reading the audio data file.
     *
     * @param nBufferSize The size of each read buffer in bytes.
     * @ghidraAddress NTSC-U/C: 0x00251260
     * @ghidraAddress PAL: 0x00259c18
     */
    void OpenAudioData(int nBufferSize);

    /**
     * Poll the reading of the audio data file.
     *
     * @param audioData Receives the data of a completed read, padded with zeros to a multiple of
     *     64 bytes.
     * @param pnPosition Receives the offset of the data in the file.
     * @param pnGotData Receives 1 when a read completed, otherwise 0.
     * @return Whether more data is to come.
     * @ghidraAddress NTSC-U/C: 0x002512c0
     * @ghidraAddress PAL: 0x00259c78
     */
    bool GetAudioData(std::vector<unsigned char> &audioData, int *pnPosition, int *pnGotData);

    /**
     * Start looking for a chunk from the start of the bank file.
     *
     * @param nChunkType The chunk, a ChunkType.
     * @ghidraAddress NTSC-U/C: 0x002513f8
     * @ghidraAddress PAL: 0x00259db0
     */
    void FindChunk(int nChunkType);

    /**
     * Continue reading the bank file. Call it until it returns false.
     *
     * @param data Receives the data of the chunk FindChunk() requested.
     * @return Whether the read is still in progress.
     * @ghidraAddress NTSC-U/C: 0x00251670
     * @ghidraAddress PAL: 0x0025a018
     */
    bool ReadChunk(std::vector<unsigned char> &data);

private:
    /** States of the chunk reader. */
    enum ReadState {
        kReadingHeader = 0, /*!< A chunk header is being read. */
        kReadingData = 1,   /*!< The data of the requested chunk is being read. */
        kReadDone = 2,      /*!< Nothing is being read. */
    };

    /** The size of a chunk identifier. */
    static constexpr int kTagSize = 4;

    /** The size of a chunk header, the identifier and the size word. */
    static constexpr int kHeaderSize = 8;

    /**
     * Check a chunk header that has arrived, skipping to the next header until the requested
     * chunk is found.
     *
     * @param data Receives the requested chunk's data once its read starts.
     * @return Whether the read of the requested chunk's data started.
     * @ghidraAddress NTSC-U/C: 0x00251470
     * @ghidraAddress PAL: 0x00259e28
     */
    bool ReadChunkHeader(std::vector<unsigned char> &data);

    LazyFileStream *mAudioFileStream; /*!< Reader of the audio data file, or null. */
    String mFilename;                 /*!< Path of the bank file. */
    FileStream *mAudioDataOutputFile; /*!< Writer of the audio data file, always null. */
    FileStream *mBankOutputFile;      /*!< Writer of the bank file, or null. */
    File *mBankAsyncInputFile;        /*!< Reader of the bank file, or null. */
    char mChunkTag[kTagSize];         /*!< Identifier of the chunk being looked for. */
    int mReadState;                   /*!< State of the chunk reader, a ReadState. */
    int mIODirection;                 /*!< The direction, an IODirection. */
    int mChunkStart;                  /*!< Offset of the header of the chunk being written. */
    char mHeaderData[kHeaderSize];    /*!< The chunk header being read. */
    char mCurTag[kTagSize];           /*!< Identifier of the chunk being written. */
    int mChunkSize;                   /*!< Size of the chunk being written. */
};
