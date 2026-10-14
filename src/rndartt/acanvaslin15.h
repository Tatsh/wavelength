#pragma once

#include "rndartt/acanvas15.h"

/**
 * Linear addressing for a canvas of 1555 pixels.
 *
 * Its RTTI descriptor is at 0x008ef2a0, and its single public base is ACanvas15 at offset zero.
 * Like the other layout classes it derives from the format class. Its base converts every other
 * colour format into 1555, and this class supplies everything that depends on where a pixel sits
 * in memory.
 *
 * Its table at 0x0083cb58 runs the same 85 entries as its base's and overrides sixteen. Four are
 * the pure slots ACanvas15 leaves, which makes the class concrete: the alpha builder at 12, the
 * colourless store at 13, and the 1555 pixel pair at 17 and 27. The other twelve replace
 * implementations the base already has, because knowing the layout lets a fill walk memory
 * directly.
 *
 * TWO BYTES ARE ONE PIXEL, so a pixel address is `mPixels + nY * mBytesPerRow + nX * 2`.
 *
 * THE ALPHA BUILDER IS WHY SLOT 12 IS LEFT PURE BY THE DIRECT-COLOUR FORMATS. Here it walks every
 * pixel of the bitmap, setting the 1555 alpha bit on each whose colour differs from the key and
 * clearing it on each that matches, so it needs the pixel layout. The indexed format supplies the
 * same slot in its own format class instead, because there the work is rewriting palette entries
 * and no layout is involved. That asymmetry was inferred from the slot tables and is confirmed
 * here by the body.
 *
 * Every override that reads a palette converts the entry to 1555 through APackRgb1555From8888().
 * The block copies and the textured row resolve the palette from the source, then the canvas, then
 * ACanvas::palDefault, and the span overrides read ARowInfo::mPalette or AScaledRowInfo::mPalette.
 * Each returns without drawing when the palette is null.
 */
class ACanvasLin15 : public ACanvas15 {
public:
    /**
     * Construct over a bitmap.
     *
     * The body calls ACanvas's constructor directly rather than ACanvas15's, because the format
     * class's own constructor is trivial and inlined away.
     *
     * @param bitmap The bitmap the canvas addresses.
     */
    explicit ACanvasLin15(const ABitmap &bitmap);

    /**
     * Slot 1.
     *
     * The body restores ACanvas's table rather than ACanvas15's, because the inlined destructor
     * chain runs to the root and the intermediate store is dead.
     */
    virtual ~ACanvasLin15();

    /**
     * Slot 12. Walks every pixel, keying the alpha bit off the colour.
     */
    virtual void SetAlphaValues(unsigned int nColorKey);

    /**
     * Slot 13.
     */
    virtual void DrawPixelU(int nX, int nY);

    /**
     * Slot 17.
     */
    virtual void DrawPixel15U(int nX, int nY, unsigned short nColor);

    /**
     * Slot 27.
     */
    virtual unsigned short GetPixel15U(int nX, int nY);

    /**
     * Slot 35.
     */
    virtual void DrawHorzLineU(int nY, int nLeft, int nRight);

    /**
     * Slot 37.
     */
    virtual void DrawVertLineU(int nX, int nTop, int nBottom);

    /**
     * Slot 39.
     */
    virtual void DrawRectU(ARect rect);

    /**
     * Slot 46.
     */
    virtual void DrawTmapRow8U(int nY,
                               int nLeft,
                               int nRight,
                               const ABitmap *pSource,
                               APoint *pSourcePosition,
                               const APoint *pSourceStep);

    /**
     * Slot 47.
     *
     * The key is compared against the low byte of the transparent colour.
     */
    virtual void DrawBitmapLin4U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 49.
     */
    virtual void DrawBitmapLin8U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 51.
     */
    virtual void DrawBitmapLin15U(const ABitmap &source, int nX, int nY);

    /**
     * Slot 75.
     *
     * The key is compared against the low byte of the transparent colour.
     */
    virtual void DrawClutBitmapRowLin8U(const ARowInfo &span, const unsigned char *pRemap);

    /**
     * Slot 79.
     *
     * The key is compared against the whole transparent colour word, where
     * DrawScaledClutBitmapRowLin8U() compares its low byte.
     */
    virtual void DrawScaledBitmapRowLin8U(const AScaledRowInfo &span);

    /**
     * Slot 83.
     */
    virtual void DrawScaledClutBitmapRowLin8U(const AScaledRowInfo &span,
                                              const unsigned char *pRemap);
};
