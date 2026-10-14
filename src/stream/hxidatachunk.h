#pragma once

#include "stream/hxstream.h"

class HxIListChunk;
class HxChunkHeader;

/**
 * Input stream over the payload of one RIFF or Standard MIDI File chunk.
 *
 * It derives from HxStream. Its vtable is at `0x007d3ed8`. Offsets and
 * sizes are relative to the payload. The payload lies in another stream between mStart and mEnd.
 * Both constructors set mFatalOnEnd and copy HxStream::mSwapBytes from that stream.
 *
 * SetMarker() sets HxStream::failbit for an offset outside the payload and then overwrites the
 * status with HxStream::goodbit on every path. GetMarker() and SetMarker() still test the range
 * flag.
 */
class HxIDataChunk : public HxStream {
public:
    /**
     * Open the current chunk of a reader, and lock the reader.
     *
     * The header is a heap copy of the reader's current header, and the payload starts at the
     * source stream's read position.
     *
     * @param pReader The reader positioned on the chunk.
     */
    explicit HxIDataChunk(HxIListChunk *pReader);

    /**
     * Read a chunk header at a stream's read position and open that chunk.
     *
     * @param pSource The stream positioned on the chunk header. The chunk does not take
     * ownership.
     */
    explicit HxIDataChunk(HxStream *pSource);

    /**
     * Unlock the reader, if any, and free the header.
     */
    ~HxIDataChunk() override;

    /**
     * Move the read position within the payload.
     *
     * Does nothing while HxStream::failbit or HxStream::badbit is set.
     *
     * @param nOffset The signed distance to move.
     * @param nWhence The origin, one of the HxStreamSeekOrigin values.
     */
    void SetMarker(int nOffset, int nWhence) override;

    /**
     * Report the read position within the payload.
     *
     * @return The position from the start of the payload, or -1 while HxStream::failbit or
     * HxStream::badbit is set.
     */
    int GetMarker() override;

    /**
     * Report the payload size from the chunk header.
     *
     * @return The size in bytes.
     */
    int Size() override;

    /**
     * Move up to nSize bytes of the payload into pDest.
     *
     * Does nothing unless the status is HxStream::goodbit. A read that would cross the end of
     * the payload moves only the bytes before it and sets HxStream::kStatusEnd.
     *
     * @param pDest The destination buffer.
     * @param nSize The number of bytes to move.
     * @return This stream.
     */
    HxStream &ReadData(void *pDest, int nSize) override;

    /**
     * Report the stream the payload lies in.
     *
     * @return The source stream.
     */
    HxStream *UnderlyingStream() override;

private:
    HxIListChunk *mReader; // Reader to unlock on destruction, or null.
    HxStream *mSource;     // Stream the payload lies in.
    HxChunkHeader *mId;    // Heap copy of the chunk header.
    int mStart;            // Source position of the first payload byte.
    int mEnd;              // Source position after the last payload byte.
};
