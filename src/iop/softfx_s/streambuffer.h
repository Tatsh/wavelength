#pragma once

#include "softfx_s/ringbuffer.h"

/** The packet StreamBuffer::SendStatus() sends. Only its first 16 bytes are transferred. */
struct StatusPacket {
    int mValue;       /*!< The value sent. */
    int mPadding[15]; /*!< Zero. */
};

/**
 * The ring buffer the EE streams samples into. After each change it tells the EE how much it may
 * send by writing the free space to an EE address over SIF DMA.
 *
 * The module was built without RTTI. The name is inferred from the class's routines.
 */
class StreamBuffer : public RingBuffer {
public:
    /** Buffer size in bytes. */
    static constexpr unsigned int kSize = 0xf000;

    /**
     * Allocate the buffer.
     *
     * @param size Requested size. The binary ignores it and always allocates #kSize bytes.
     * @ghidraAddress NTSC-U/C: 0x00003270
     * @ghidraAddress PAL: 0x00003270
     */
    explicit StreamBuffer(unsigned int size);

    /**
     * Free the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x000033e8
     * @ghidraAddress PAL: 0x000033e8
     */
    ~StreamBuffer() override;

    /**
     * Write a word to EE memory over SIF DMA.
     *
     * @param value The word.
     * @param address EE address of a 16-byte destination.
     * @return The transfer identifier.
     * @ghidraAddress NTSC-U/C: 0x000032b0
     * @ghidraAddress PAL: 0x000032b0
     */
    static int SendStatus(int value, void *address);

    /**
     * Record the EE address that receives the free space, and write 1 there.
     *
     * @param address EE address.
     * @ghidraAddress NTSC-U/C: 0x00003344
     * @ghidraAddress PAL: 0x00003344
     */
    void SetStatusAddress(void *address);

    /**
     * Append bytes and schedule a notification.
     *
     * @param data Bytes to append, or null to record only the flag.
     * @param size Byte count, or zero to record only the flag.
     * @param endOfStream Whether the EE has no more data.
     * @ghidraAddress NTSC-U/C: 0x00003368
     * @ghidraAddress PAL: 0x00003368
     */
    void Write(const void *data, unsigned int size, bool endOfStream) override;

    /**
     * Send the free space to the EE when a notification is scheduled and the space is positive.
     *
     * @ghidraAddress NTSC-U/C: 0x00003394
     * @ghidraAddress PAL: 0x00003394
     */
    void Notify() override;

private:
    int mReserved;        // +0x24, cleared by the constructor and never read.
    void *mStatusAddress; /*!< EE address that receives the free space. */
    bool mNotifyPending;  /*!< A write has happened since the last notification. */
};
