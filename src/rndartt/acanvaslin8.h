#pragma once

#include "rndartt/acanvas8.h"

/**
 * Linear addressing for a canvas of eight-bit indexed pixels.
 *
 * Its RTTI descriptor is at 0x008f09c0, and its single public base is ACanvas8 at offset zero. Like
 * ACanvasLin4 it derives from the format class rather than from ACanvas. Its base supplies the
 * colour conversions and this class supplies everything that depends on how pixels are addressed.
 *
 * Its table at 0x0083f818 runs the same 85 entries as its base's and overrides eighteen, where the
 * four-bit sibling overrides eight. Three are the pure slots ACanvas8 leaves, which makes the class
 * concrete. The other thirteen replace implementations the base already has, because one byte per
 * pixel lets a fill become a memset and a copy become a row copy, where the generic versions
 * address one pixel at a time. Four bits per pixel does not pay off that way, which is why the
 * sibling inherits all thirteen.
 *
 * ONE BYTE IS ONE PIXEL, so the address of a pixel is `mPixels + nY * mBytesPerRow + nX` with no
 * packing and no odd-start flag. The four-bit sibling needs both.
 */
class ACanvasLin8 : public ACanvas8 {
public:
    /**
     * Construct over a bitmap.
     *
     * @param bitmap The bitmap the canvas addresses.
     */
    explicit ACanvasLin8(const ABitmap &bitmap);

    /**
     * Slot 1.
     *
     * The body restores ACanvas's table rather than ACanvas8's, because the inlined destructor
     * chain runs to the root and the intermediate store is dead.
     */
    virtual ~ACanvasLin8();

    /**
     * Slot 13.
     */
    virtual void DrawPixelU(int nX, int nY);

    /**
     * Slot 15.
     */
    virtual void DrawPixel8U(int nX, int nY, int nIndex);

    /**
     * Slot 25.
     */
    virtual int GetPixel8U(int nX, int nY);

    /**
     * Slot 35. One memset across the span.
     */
    virtual void DrawHorzLineU(int nY, int nLeft, int nRight);

    /**
     * Slot 37.
     */
    virtual void DrawVertLineU(int nX, int nTop, int nBottom);

    /**
     * Slot 39. One memset per row.
     */
    virtual void DrawRectU(ARect rect);

    /**
     * Slot 43.
     */
    virtual void DrawClutRectU(ARect rect, const unsigned char *pRemap);

    /**
     * Slot 46.
     *
     * The source position is advanced in place, so the caller sees where the row ended.
     */
    virtual void DrawTmapRow8U(int nY,
                               int nLeft,
                               int nRight,
                               const ABitmap *pSource,
                               APoint *pSourcePosition,
                               const APoint *pSourceStep);

    /**
     * Slot 47. Unpacks nibbles, honouring the source's odd-start flag.
     *
     * The nibble phase starts from the odd-start flag and alternates per pixel, and the low nibble
     * of a byte is the earlier pixel.
     */
    virtual void DrawBitmapLin4U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 49.
     *
     * Three tiers, widest first. An unkeyed source whose pitch matches this canvas's is copied in
     * one block, an unkeyed source is copied a row at a time, and a keyed source is walked a byte
     * at a time.
     */
    virtual void DrawBitmapLin8U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 57. Decodes through ARle8Reader, one row per call.
     */
    virtual void DrawBitmapRle8U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 75.
     */
    virtual void DrawClutBitmapRowLin8U(const ARowInfo &span, const unsigned char *pRemap);

    /**
     * Slot 78.
     *
     * Skips a source index equal to the span transparent colour, where the base skips index zero.
     */
    virtual void DrawBlendBitmapRowLin8U(const ARowInfo &span, const unsigned char *const *ppBlend);

    /**
     * Slot 79.
     *
     * The routine occupies slot 79 of this class's table and does not appear in the 85 entries of
     * the four-bit sibling's table. The sibling does not override the slot.
     */
    virtual void DrawScaledBitmapRowLin8U(const AScaledRowInfo &span);

    /**
     * Slot 83.
     */
    virtual void DrawScaledClutBitmapRowLin8U(const AScaledRowInfo &span,
                                              const unsigned char *pRemap);

    /**
     * Slot 84.
     */
    virtual void DrawScaledBlendBitmapRowLin8U(const AScaledRowInfo &span,
                                               const unsigned char *const *ppBlend);
};
