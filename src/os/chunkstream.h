#pragma once

#include "os/binstream.h"
#include "os/file.h"

/**
 * The mark EndChunk() sends and ChunkStream::Mark() recognises, `chunk`.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2360
 */
extern const char *g_szChunkMark;

/**
 * BinStream over a file stored as a header and a run of blocks, each optionally deflated.
 *
 * The RTTI includes the class name and records BinStream as the base. A reader reads the header
 * and the blocks in the background, two block buffers in turn. A writer gathers a block in memory
 * until the block reaches the chunk size at a mark of g_szChunkMark, and rewrites the header when
 * the stream is deleted.
 */
class ChunkStream : public BinStream {
public:
    /** Directions a stream opens in. */
    enum Mode {
        kModeRead = 0,  /*!< Read the file. */
        kModeWrite = 1, /*!< Create or replace the file. */
    };

    /** Blocks the header describes at most. */
    static constexpr int kMaxBlocks = 50;

    /** The header at the start of the file. */
    struct Header {
        /**
         * Construct the header of a file with one empty block.
         *
         * @param bCompressed Whether the blocks are deflated.
         * @ghidraAddress NTSC-U/C: 0x00295b70
         * @ghidraAddress PAL: 0x0029f788
         */
        explicit Header(bool bCompressed);

        unsigned int mMagic;         /*!< kMagicCompressed or kMagicPlain. */
        int mHeaderSize;             /*!< The size of this header in bytes. */
        int mBlockCount;             /*!< Entries of mBlockSizes in use. */
        int mMaxBlockSize;           /*!< The largest block before decoding, in bytes. */
        int mBlockSizes[kMaxBlocks]; /*!< The size of each block in the file, in bytes. */
    };

    /** mMagic of a file with deflated blocks. */
    static constexpr unsigned int kMagicCompressed = 0xccbedeaf;

    /** mMagic of a file with plain blocks. */
    static constexpr unsigned int kMagicPlain = 0xcabedeaf;

    /**
     * Open a file and start reading its header, or write a fresh header.
     *
     * @param pszFile The file.
     * @param eMode The direction.
     * @param nChunkSize The size at which a writer ends a block at a mark.
     * @param bCompressed Whether a writer deflates the blocks.
     * @param bLittleEndian Whether values are stored in the console's byte order.
     * @ghidraAddress NTSC-U/C: 0x002952a0
     * @ghidraAddress PAL: 0x0029eec8
     */
    ChunkStream(
        const char *pszFile, Mode eMode, int nChunkSize, bool bCompressed, bool bLittleEndian);

    /**
     * Write the last block and the final header of a writer, and close the file.
     *
     * @ghidraAddress NTSC-U/C: 0x002953f8
     * @ghidraAddress PAL: 0x0029f020
     */
    ~ChunkStream() override;

    /**
     * Copy bytes out of the current block.
     *
     * The caller first waits for the block through Eof(). The bounds are not checked.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x002954e8
     * @ghidraAddress PAL: 0x0029f100
     */
    void Read(void *pData, int nBytes) override;

    /**
     * Append bytes to the block being gathered.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x00295538
     * @ghidraAddress PAL: 0x0029f150
     */
    void Write(const void *pData, int nBytes) override;

    /**
     * Do nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x002955f0
     */
    void Flush() override {
    }

    /**
     * Report that the stream cannot seek.
     *
     * @param nOffset Not used.
     * @param eFrom Not used.
     * @ghidraAddress NTSC-U/C: 0x002955f8
     * @ghidraAddress PAL: 0x0029f210
     */
    void Seek(int nOffset, SeekType eFrom) override;

    /**
     * Report that the stream cannot report a position.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00295618
     * @ghidraAddress PAL: 0x0029f230
     */
    int Tell() override;

    /**
     * Advance the background reads, and report whether the current block is still unavailable.
     *
     * @return Whether no byte of a new block is ready yet.
     * @ghidraAddress NTSC-U/C: 0x00295640
     * @ghidraAddress PAL: 0x0029f258
     */
    bool Eof() override;

    /**
     * Report whether the file failed to open or a block failed to write.
     *
     * @return mFail.
     * @ghidraAddress NTSC-U/C: 0x00295960
     */
    bool Fail() override;

    /**
     * End the block at a mark of g_szChunkMark once it has reached the chunk size.
     *
     * @param pszMark The mark.
     * @ghidraAddress NTSC-U/C: 0x00295968
     * @ghidraAddress PAL: 0x0029f580
     */
    void Mark(const char *pszMark) override;

private:
    /**
     * Record the gathered block's size and write it once it reaches the chunk size or bLast is set.
     *
     * @param bLast Whether the block is the last one.
     * @ghidraAddress NTSC-U/C: 0x002959b0
     * @ghidraAddress PAL: 0x0029f5c8
     */
    void FlushBlock(bool bLast);

    /**
     * Deflate the gathered block when the stream is compressed, and write it.
     *
     * @param nBytes The size of the gathered block.
     * @return The size written.
     * @ghidraAddress NTSC-U/C: 0x00295a88
     * @ghidraAddress PAL: 0x0029f6a0
     */
    int WriteBlock(int nBytes);

    /**
     * Point mData at the block just read, inflating it first when the stream is compressed.
     *
     * @ghidraAddress NTSC-U/C: 0x00295b00
     */
    void DecodeBlock();

    File *mFile;        /*!< The file, or null when it did not open. */
    int mFail;          /*!< Non-zero when the file did not open or a block write failed. */
    Mode mMode;         /*!< The direction. */
    Header mHeader;     /*!< The header. */
    int mCompressed;    /*!< Non-zero when the blocks are deflated. */
    int mChunkSize;     /*!< The size at which a writer ends a block at a mark. */
    int mBufferSize;    /*!< The size of each block buffer. */
    int mBuffer;        /*!< The index of the block buffer being filled, or -1. */
    int mBlockReady;    /*!< Non-zero once the current block was read. */
    int mPosition;      /*!< The position in the current block. */
    int mHeaderPending; /*!< Non-zero while a reader waits for the header. */
    int *mBlock;        /*!< The size entry of the current block. */
    int *mBlockEnd;     /*!< The end of the size entries in use. */
    char *mBuffers[2];  /*!< The block buffers a reader fills in turn. */
    char *mWork;        /*!< The gathered block of a writer, or the inflated block. */
    char *mData;        /*!< The decoded current block. */
    float mStallMs;     /*!< SystemMs() when a reader first waited for a block, or 0. */
};

/**
 * Send g_szChunkMark to a stream.
 *
 * Streams take the routine as a manipulator.
 *
 * @param stream The stream.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00295260
 * @ghidraAddress PAL: 0x0029ee88
 */
BinStream &EndChunk(BinStream &stream);
