#pragma once

#include "os/binstream.h"

/**
 * BinStream that collects the bytes written to it in a growing buffer.
 *
 * The RTTI includes the class name and records BinStream as the base, and the vtable is at
 * `0x003d6f68`. The object is 0x20 bytes. Only the members its callers here use are declared.
 */
class MemStream : public BinStream {
public:
    /**
     * Construct an empty stream with room for 4096 bytes.
     *
     * @param bLittleEndian Store values in the console's byte order.
     * @ghidraAddress NTSC-U/C: 0x0029c9e0
     * @ghidraAddress PAL: 0x002a6608
     */
    explicit MemStream(bool bLittleEndian);

    /**
     * Copy bytes out from the position.
     *
     * @param pData The destination.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0029ca38
     */
    void Read(void *pData, int nBytes) override;

    /**
     * Copy bytes in at the position, growing the buffer.
     *
     * @param pData The source.
     * @param nBytes The number of bytes.
     * @ghidraAddress NTSC-U/C: 0x0029cab0
     */
    void Write(const void *pData, int nBytes) override;

    /**
     * Do nothing, since the bytes stay in memory.
     *
     * @ghidraAddress NTSC-U/C: 0x003ab218
     */
    void Flush() override;

    /**
     * Move the position.
     *
     * @param nOffset The offset in bytes.
     * @param eFrom The origin of the offset.
     * @ghidraAddress NTSC-U/C: 0x0029cb90
     */
    void Seek(int nOffset, SeekType eFrom) override;

    /**
     * Report the position.
     *
     * @return The position in bytes.
     * @ghidraAddress NTSC-U/C: 0x003ab220
     */
    int Tell() override;

    /**
     * Report whether the position is at the end of the bytes.
     *
     * @return Whether no byte is left.
     * @ghidraAddress NTSC-U/C: 0x003ab228
     */
    bool Eof() override;

    /**
     * Report whether a transfer failed.
     *
     * @return Whether a transfer failed.
     * @ghidraAddress NTSC-U/C: 0x003ab248
     */
    bool Fail() override;
};
