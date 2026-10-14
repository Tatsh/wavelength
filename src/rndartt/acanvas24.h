#pragma once

#include "rndartt/acanvas.h"

/**
 * Colour conversion for a canvas of 24-bit pixels.
 *
 * Its RTTI descriptor is at 0x008f0910, and its single public base is ACanvas at offset zero.
 *
 * THE CLASS IS ABSTRACT, and the set it leaves pure differs from every sibling's. Its table at
 * 0x00840a28 runs the same 85 entries as ACanvas's, so it adds no virtual, and it fills 18 of the
 * base's pure slots while leaving slots 12, 13, 19, and 29 pointing at the shared pure-virtual
 * stub: the alpha builder, the colourless store, and the two channel-triple pixel accessors. Its
 * layout subclass is ACanvasLin24, whose descriptor records ACanvas24 as its base.
 *
 * WHICH FOUR IT LEAVES IS THE DESIGN OF THE WHOLE FAMILY, and it is exact. A format class
 * implements every colour format OTHER than its own, by converting into its own, and leaves its own
 * format's pixel pair to whichever subclass knows the memory layout. This class leaves the
 * channel-triple pair at 19 and 29, ACanvas15 leaves the 1555 pair at 17 and 27, and ACanvas8
 * leaves the indexed pair at 15 and 25. Each also leaves slot 13, the colourless store. The alpha
 * builder at slot 12 is left by this class and by ACanvas15 and supplied by ACanvas8, which can do
 * it because an indexed format owns a palette whose entries can be rewritten.
 *
 * THE COLOUR MEMBER IS ACCESSED BOTH AS A WORD AND AS BYTES, so it is modelled as a union. The
 * native setter at 0x00630388 stores a whole word at `+0x24` with one instruction, while the index
 * setter at 0x00630448 writes three separate bytes at `+0x24`, `+0x25`, and `+0x26` and never
 * touches `+0x27`. A single word store would have written that fourth byte, so the two accesses
 * are genuinely different in the original rather than one being the compiler's rendering of the
 * other.
 *
 * No body is written yet. Every body is recoverable, and the four pure slots are not this class's
 * to supply.
 */
class ACanvas24 : public ACanvas {
public:
    /**
     * Construct over a bitmap.
     *
     * @param bitmap The bitmap the canvas addresses.
     */
    explicit ACanvas24(const ABitmap &bitmap);

    /**
     * Slot 1.
     */
    virtual ~ACanvas24();

    /**
     * Slot 2. Expands a palette entry into the three channel bytes.
     */
    virtual void SetColor8(int nIndex);

    /**
     * Slot 3.
     */
    virtual void SetColor15(unsigned short nColor);

    /**
     * Slot 4.
     */
    virtual void SetColor24(const unsigned char *pRGB);

    /**
     * Slot 5.
     *
     * The same store as SetColorNative(), in a separate slot. For this format an 8888 colour and
     * the native word are the same value.
     */
    virtual void SetColor32(unsigned int nColor);

    /**
     * Slot 6. Stores the whole word.
     */
    virtual void SetColorNative(unsigned int nColor);

    /**
     * Slot 7.
     */
    virtual int GetColor8();

    /**
     * Slot 8.
     */
    virtual unsigned short GetColor15();

    /**
     * Slot 9.
     */
    virtual void GetColor24(unsigned char *pRGB);

    /**
     * Slot 10.
     *
     * The same load as GetColorNative(), in a separate slot.
     */
    virtual unsigned int GetColor32();

    /**
     * Slot 11. Reads the whole word.
     */
    virtual unsigned int GetColorNative();

    /**
     * Slot 15.
     */
    virtual void DrawPixel8U(int nX, int nY, int nIndex);

    /**
     * Slot 17.
     */
    virtual void DrawPixel15U(int nX, int nY, unsigned short nColor);

    /**
     * Slot 21.
     *
     * The base's slot 21 is the colourless store rather than a format-specific one. Slot 31 is the
     * same case on the read side, and both traps appear in ACanvas8 as well.
     */
    virtual void DrawPixel32U(int nX, int nY, unsigned int nColor);

    /**
     * Slot 23.
     *
     * For this format the native word is an 8888 colour, so the store forwards to DrawPixel32U()
     * through the table rather than converting.
     */
    virtual void DrawPixelNativeU(int nX, int nY, unsigned int nColor);

    /**
     * Slot 25.
     */
    virtual int GetPixel8U(int nX, int nY);

    /**
     * Slot 27.
     */
    virtual unsigned short GetPixel15U(int nX, int nY);

    /** Slot 31. The slot is the colourless read.
     */
    virtual unsigned int GetPixel32U(int nX, int nY);

    /**
     * Slot 33.
     *
     * The read counterpart of DrawPixelNativeU(), forwarding to GetPixel32U().
     */
    virtual unsigned int GetPixelNativeU(int nX, int nY);

protected:
    // Written as a word by the native setter and as three separate bytes by the index setter, which
    // never touches the fourth. The fourth byte's purpose is unrecovered.
    union {
        unsigned int mColorNative;
        unsigned char mColorChannels[4];
    }; // +0x24
};
