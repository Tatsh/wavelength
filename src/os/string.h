#pragma once

#include "os/prnstream.h"

/**
 * Growable text that is also a stream: text printed to it is appended.
 *
 * The RTTI records the class as deriving from PrnStream. An empty string shares one static
 * buffer instead of owning storage. Only the members its callers here use are declared.
 */
class String : public PrnStream {
public:
    /**
     * Copy a C string.
     *
     * @param pszText The text, or null for an empty string.
     * @ghidraAddress NTSC-U/C: 0x0029e9b0
     * @ghidraAddress PAL: 0x002a8670
     */
    String(const char *pszText);

    /**
     * Copy another string.
     *
     * @param other The string.
     * @ghidraAddress NTSC-U/C: 0x0029ea40
     * @ghidraAddress PAL: 0x002a8700
     */
    String(const String &other);

    /**
     * Release the storage.
     *
     * @ghidraAddress NTSC-U/C: 0x0029ead8
     * @ghidraAddress PAL: 0x002a8798
     */
    ~String() override;

    /**
     * Append text.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0029eb48
     * @ghidraAddress PAL: 0x002a8808
     */
    void Print(const char *pszText) override;

    /**
     * Report the text.
     *
     * @return The text, terminated.
     */
    const char *c_str() const {
        return mText;
    }

private:
    int mLength;   /*!< The length of the text. */
    int mCapacity; /*!< The size of the owned buffer, including the terminator. */
    char *mText;   /*!< The text, owned unless it is the shared empty buffer. */
};
