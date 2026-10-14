#pragma once

/**
 * Buffer of bytes a producer appends and a consumer takes in runs.
 *
 * The RTTI includes the class name and records no base. The data members precede the vptr, which
 * sits at `+0x20`. Only the members its callers here use are declared.
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
     * Report whether the consumer took every byte and the producer has no more.
     *
     * @return Whether the stream ended.
     * @ghidraAddress NTSC-U/C: 0x0029fa28
     * @ghidraAddress PAL: 0x002a96e0
     */
    bool IsEof();

    /**
     * Take the next run of bytes.
     *
     * @param nMaxBytes The most bytes the caller accepts.
     * @param pnBytes Receives the size of the run.
     * @return The run, or null when no byte is ready.
     * @ghidraAddress NTSC-U/C: 0x0029fae8
     * @ghidraAddress PAL: 0x002a97a0
     */
    char *AcquireReadRun(int nMaxBytes, int *pnBytes);

    /**
     * Release the run AcquireReadRun() handed out and clear the caller's pointer.
     *
     * A null run is ignored.
     *
     * @param ppRun The caller's pointer to the run.
     * @ghidraAddress NTSC-U/C: 0x0029fd18
     * @ghidraAddress PAL: 0x002a99d0
     */
    void CommitReadRun(char **ppRun);

    int mSize;       /*!< The size of the buffer in bytes. */
    char *mBuffer;   /*!< The buffer. */
    char *mReadPos;  /*!< The next byte the consumer takes. */
    char *mWritePos; /*!< The next byte the producer fills. */
    char *mEnd;      /*!< The end of the buffer. */
    int mReserved14; // +0x14, not yet identified.
    int mInputDone;  /*!< 1 once the producer has no more bytes. */
    int mReserved1c; // +0x1c, not yet identified. The constructor stores 16.
};
