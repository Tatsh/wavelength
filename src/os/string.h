#pragma once

#include "os/prnstream.h"

/**
 * Shared terminated buffer every empty String points at.
 *
 * @ghidraAddress NTSC-U/C: 0x003b24d0
 */
extern char *g_szStringEmptyBuffer;

/**
 * Growable text buffer that is also a PrnStream.
 *
 * The RTTI includes the class name and records PrnStream as the base. An empty string points
 * mBuffer at g_szStringEmptyBuffer rather than at an allocation of its own.
 */
class String : public PrnStream {
public:
    /**
     * Construct an empty string.
     *
     * The constructor has no out-of-line copy, and it does not set mCapacity.
     */
    String() : mLength(0), mBuffer(g_szStringEmptyBuffer) {
    }

    /**
     * Construct a copy of a C string.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x0029e9b0
     * @ghidraAddress PAL: 0x002a8670
     */
    explicit String(const char *pszText);

    /**
     * Copy another string.
     *
     * @param other The string.
     * @ghidraAddress NTSC-U/C: 0x0029ea40
     * @ghidraAddress PAL: 0x002a8700
     */
    String(const String &other);

    /**
     * Release the text.
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
     * Replace the text with a copy of a C string.
     *
     * @param pszText The text. A null pointer empties the string.
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029eed8
     * @ghidraAddress PAL: 0x002a8b98
     */
    String &operator=(const char *pszText);

    /**
     * Release the text and leave the string empty.
     *
     * The name is inferred.
     *
     * @return The string.
     * @ghidraAddress NTSC-U/C: 0x0029f528
     * @ghidraAddress PAL: 0x002a91e8
     */
    String &Clear();

    /**
     * Report the text.
     *
     * @return The terminated text, which the string retains.
     */
    const char *c_str() const {
        return mBuffer;
    }

    int mLength;   /*!< Characters before the terminator. */
    int mCapacity; /*!< Bytes allocated for mBuffer. */
    char *mBuffer; /*!< The terminated text. */
};
