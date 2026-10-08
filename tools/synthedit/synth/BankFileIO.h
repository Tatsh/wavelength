#pragma once

#include <vector>

#include "os/File.h"
#include "synth/LazyFileStream.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/Str.h"

/**
 * Extension of a bank file. Each translation unit that includes this header has its own copy.
 *
 * @ghidraAddress 0x100c4a18
 * @ghidraAddress 0x100c4a48
 */
static const String kBankExtension(".bnk");

/**
 * Extension of the audio data file that goes with a bank file. Each translation unit that
 * includes this header has its own copy.
 *
 * @ghidraAddress 0x100c4a00
 * @ghidraAddress 0x100c4a30
 */
static const String kAudioDataExtension(".nse");

/**
 * Reader and writer of a bank file and the audio data file beside it.
 *
 * The class has no RTTI, and its name comes from its source file. The object is 0x44 bytes. A
 * bank file is a run of chunks, each an identifier of four characters, a size word, and the data.
 * The writer produces chunks through a FileStream. The reader finds a chunk with asynchronous reads
 * that the caller polls.
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
     * @param ioDirection The direction, an IODirection.
     * @ghidraAddress 0x1002bce8
     */
    BankFileIO(const String &filename, int ioDirection);

    /**
     * Close every file.
     *
     * @ghidraAddress 0x1002be22
     */
    ~BankFileIO();

    /**
     * Report whether the bank file failed to open.
     *
     * @return Whether no bank file is open.
     * @ghidraAddress 0x1002bf05
     */
    bool operator!() const;

    /**
     * Copy a chunk header into eight bytes. No routine calls it.
     *
     * @param header The header, the identifier and then the size word.
     * @param data Receives the bytes.
     * @ghidraAddress 0x1002bf37
     */
    void PackChunkHeader(const char *header, std::vector<unsigned char> &data);

    /**
     * Copy eight bytes into a chunk header. No routine calls it.
     *
     * @param header Receives the identifier and then the size word.
     * @param data The bytes.
     * @ghidraAddress 0x1002bfa5
     */
    void UnpackChunkHeader(char *header, std::vector<unsigned char> &data);

    /**
     * Derive the path of the audio data file from the path of a bank file.
     *
     * @param iBankFileName The path of the bank file.
     * @return The path with the bank extension replaced.
     * @ghidraAddress 0x1002c009
     */
    static String AudioDataFilename(const String &iBankFileName);

    /**
     * Create the audio data file for writing.
     *
     * @ghidraAddress 0x1002c0ce
     */
    void BeginAudioDataOutput();

    /**
     * Append bytes to the audio data file.
     *
     * @param data The bytes.
     * @ghidraAddress 0x1002c1d5
     */
    void WriteAudioData(std::vector<unsigned char> &data);

    /**
     * Close the audio data file.
     *
     * @ghidraAddress 0x1002c23d
     */
    void EndAudioDataOutput();

    /**
     * Start writing a chunk. EndChunk() completes its header.
     *
     * @param chunkType The chunk, a ChunkType.
     * @return The stream of the bank file, positioned after the chunk header.
     * @ghidraAddress 0x1002c281
     */
    BinStream *BeginChunk(int chunkType);

    /**
     * Report the identifier of a chunk.
     *
     * @param chunkType The chunk, a ChunkType.
     * @return The four characters of the identifier, without a terminator.
     * @ghidraAddress 0x1002c397
     */
    const char *GetChunkTag(int chunkType);

    /**
     * Complete the header of the chunk BeginChunk() started.
     *
     * @param stream The stream BeginChunk() returned.
     * @ghidraAddress 0x1002c465
     */
    void EndChunk(BinStream *stream);

    /**
     * Start reading the audio data file. No routine calls it.
     *
     * @param bufferSize The size of each read buffer in bytes.
     * @ghidraAddress 0x1002c508
     */
    void OpenAudioData(int bufferSize);

    /**
     * Poll the reading of the audio data file. No routine calls it.
     *
     * @param oAudioData Receives the data of a completed read, padded with zeros to a multiple of
     * 64 bytes. The capacity must already fit it.
     * @param position Receives the offset of the data in the file.
     * @param gotData Receives whether a read completed.
     * @return Whether more data is to come.
     * @ghidraAddress 0x1002c5cd
     */
    bool GetAudioData(std::vector<unsigned char> &oAudioData, int *position, bool *gotData);

    /**
     * Start looking for a chunk from the start of the bank file.
     *
     * @param chunkType The chunk, a ChunkType.
     * @ghidraAddress 0x1002c7b7
     */
    void FindChunk(int chunkType);

    /**
     * Continue reading the bank file. Call it until it returns false.
     *
     * @param data Receives the data of the chunk FindChunk() requested.
     * @return Whether the read is still in progress.
     * @ghidraAddress 0x1002cb34
     */
    bool ReadChunk(std::vector<unsigned char> &data);

private:
    /** States of the chunk reader. */
    enum ReadState {
        kReadingHeader = 0, /*!< A chunk header is being read. */
        kReadingData = 1,   /*!< The data of the requested chunk is being read. */
        kReadDone = 2,      /*!< Nothing is being read. */
    };

    /**
     * Check a chunk header that has arrived, skipping to the next header until the requested
     * chunk is found.
     *
     * @param data Receives the requested chunk's data once its read starts.
     * @return Whether the read of the requested chunk's data started.
     * @ghidraAddress 0x1002c89e
     */
    bool ReadChunkHeader(std::vector<unsigned char> &data);

    LazyFileStream *mAudioFileStream; /*!< Reader of the audio data file, or null. */
    String mFilename;                 /*!< Path of the bank file. */
    FileStream *mAudioDataOutputFile; /*!< Writer of the audio data file, or null. */
    FileStream *mBankOutputFile;      /*!< Writer of the bank file, or null. */
    File *mBankAsyncInputFile;        /*!< Reader of the bank file, or null. */
    char mChunkTag[4];                /*!< Identifier of the chunk being looked for. */
    int mReadState;                   /*!< State of the chunk reader, a ReadState. */
    int mIODirection;                 /*!< The direction, an IODirection. */
    int mChunkStart;                  /*!< Offset of the header of the chunk being written. */
    char mHeaderData[8];              /*!< The chunk header being read. */
    char mCurTag[4];                  /*!< Identifier of the chunk being written. */
    int mChunkSize;                   /*!< Size of the chunk being written. */
};
