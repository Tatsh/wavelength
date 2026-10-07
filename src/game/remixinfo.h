#pragma once

#include "os/binstream.h"
#include "os/datetime.h"

/** Bytes of RemixInfo::mName, the terminator included. */
constexpr int kRemixInfoNameSize = 64;

/**
 * Description of a saved remix. A remix file stores one ahead of the remix.
 *
 * The class is not polymorphic. The name comes from the type information the standard library
 * containers of the class record. The object is 0xa0 bytes. Members its callers here do not use
 * are reserved.
 */
class RemixInfo {
public:
    /** Construct a cleared description. */
    RemixInfo() {
        Reset();
    }

    /**
     * Clear every field.
     *
     * @ghidraAddress NTSC-U/C: 0x0027e8b0
     * @ghidraAddress PAL: 0x002881c8
     */
    void Reset();

    /**
     * Report the bytes a description occupies in a stream.
     *
     * @return The size, a version byte more than the object.
     * @ghidraAddress NTSC-U/C: 0x0027ea98
     * @ghidraAddress PAL: 0x002883b0
     */
    static int WireSize();

    int mReserved00[2];             // +0x00, not yet recovered.
    char mName[kRemixInfoNameSize]; /*!< The name the remix is saved under. +0x08 */
    int mDataSize;                  /*!< The size of the remix that follows in bytes. +0x48 */
    int mReserved4C[16];            // +0x4c, not yet recovered.
    DateTime mDate;                 /*!< The six-byte date the inline constructor clears. +0x8c */
    char mReserved92[7];            // +0x92, not yet recovered.
    signed char mReadOnly;          /*!< Non-zero when the remix may not be saved over. +0x99 */
    char mReserved9A[6];            // +0x9a, not yet recovered.
};

/**
 * Write a description to a stream, after a version byte.
 *
 * @param stream The stream.
 * @param info The description.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027eaa0
 */
BinStream &operator<<(BinStream &stream, const RemixInfo &info);

/**
 * Read a description from a stream.
 *
 * @param stream The stream.
 * @param info Receives the description.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027eb10
 */
BinStream &operator>>(BinStream &stream, RemixInfo &info);
