#pragma once

/**
 * Ring buffer of bytes a producer appends and a consumer takes in runs.
 *
 * The RTTI includes the class name and records no base. The data members precede the vptr, which
 * sits at `+0x20`. Every routine reports the state through DebugPrint() whenever the write head
 * has passed the end of the buffer.
 */
class StreamBuffer {
public:
    /**
     * Allocate a cleared buffer.
     *
     * @param nSize The size of the buffer in bytes.
     * @ghidraAddress NTSC-U/C: 0x0029f940
     * @ghidraAddress PAL: 0x002a95f8
     */
    explicit StreamBuffer(int nSize);

    /**
     * Free the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x0029f9c8
     * @ghidraAddress PAL: 0x002a9680
     */
    virtual ~StreamBuffer();

    /**
     * Append bytes behind the write head, wrapping at the end of the buffer.
     *
     * The caller first checks EmptySpaceForFilling(). The global g_nAddWasEntered is set while the
     * routine runs, and it stays set when there is no byte to append.
     *
     * @param pData The bytes, or null to only record bLast.
     * @param nBytes The number of bytes.
     * @param bLast Non-zero when the producer has no more bytes.
     * @ghidraAddress NTSC-U/C: 0x0029fe68
     * @ghidraAddress PAL: 0x002a9b20
     */
    virtual void Add(const char *pData, int nBytes, int bLast);

    /**
     * Let the producer append what it has.
     *
     * The member is pure in this class.
     */
    virtual void Poll() = 0;

    /**
     * Move both heads back to the start of the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x002a0120
     * @ghidraAddress PAL: 0x002a9dd8
     */
    virtual void Rewind();

    /**
     * Report whether the consumer took every byte and the producer has no more.
     *
     * A write head past the end of the buffer also counts as the end.
     *
     * @return Whether the stream ended.
     * @ghidraAddress NTSC-U/C: 0x0029fa28
     * @ghidraAddress PAL: 0x002a96e0
     */
    bool IsEof();

    /**
     * Report whether the consumer has taken every byte appended so far.
     *
     * The name is inferred.
     *
     * @return Whether the read head is at the write head.
     * @ghidraAddress NTSC-U/C: 0x0029fa88
     * @ghidraAddress PAL: 0x002a9740
     */
    bool IsEmpty();

    /**
     * Take the next run of bytes, after a Poll().
     *
     * A run ends at the write head or at the end of the buffer.
     *
     * @param nMaxBytes The most bytes the caller accepts.
     * @param pnBytes Receives the size of the run.
     * @return The run, or null when no byte is ready.
     * @ghidraAddress NTSC-U/C: 0x0029fae8
     * @ghidraAddress PAL: 0x002a97a0
     */
    char *AcquireReadRun(int nMaxBytes, int *pnBytes);

    /**
     * Release the run AcquireReadRun() handed out, Poll(), and clear the caller's pointer.
     *
     * A null run is ignored.
     *
     * @param ppRun The caller's pointer to the run.
     * @ghidraAddress NTSC-U/C: 0x0029fd18
     * @ghidraAddress PAL: 0x002a99d0
     */
    void CommitReadRun(char **ppRun);

    /**
     * Report the room the producer may fill, which keeps mMargin bytes free.
     *
     * @return The room in bytes.
     * @ghidraAddress NTSC-U/C: 0x002a0070
     * @ghidraAddress PAL: 0x002a9d28
     */
    int EmptySpaceForFilling();

    /**
     * Report the bytes the consumer has not taken yet.
     *
     * The name is inferred.
     *
     * @param pbInputDone Receives mInputDone.
     * @return The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x002a00a8
     * @ghidraAddress PAL: 0x002a9d60
     */
    int BytesAvailable(int *pbInputDone);

    int mSize;          /*!< The size of the buffer in bytes. */
    char *mBuffer;      /*!< The buffer. */
    char *mReadHead;    /*!< The next byte the consumer takes. */
    char *mWriteHead;   /*!< The next byte the producer fills. */
    char *mEndOfBuffer; /*!< The end of the buffer. */
    char *mReadRunEnd;  /*!< The end of the run AcquireReadRun() handed out, or null. */
    int mInputDone;     /*!< 1 once the producer has no more bytes. */
    int mMargin;        /*!< The bytes EmptySpaceForFilling() keeps free, 16. */

private:
    // Report the heads through DebugPrint() when the write head is not before the end.
    inline void CheckHeads();
};

/**
 * Non-zero while StreamBuffer::Add() runs.
 *
 * @ghidraAddress NTSC-U/C: 0x003b24d4
 * @ghidraAddress PAL: 0x00431394
 */
extern int g_nAddWasEntered;
