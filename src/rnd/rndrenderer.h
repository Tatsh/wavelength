#pragma once

#include "math/color.h"
#include "math/rect.h"
#include "math/vector2.h"

/**
 * Base of the platform renderer, the owner of the display and of the frame.
 *
 * The RTTI includes the class name. Ps is the one subclass, and its single instance at `0x0043ba00`
 * is the renderer TheRnd addresses. Most of the base bodies are empty.
 */
class RndRenderer {
public:
    /**
     * Set an opaque grey of 0.3, a display of 640 by 480 at 16 bits, and a timer ceiling of 50.
     *
     * @ghidraAddress NTSC-U/C: 0x00240cf8
     * @ghidraAddress PAL: 0x00249828
     */
    RndRenderer();

    /**
     * Read the `rnd` section of the configuration.
     *
     * The section sets the display size and depth, the overlays, and the clear colour.
     *
     * @ghidraAddress NTSC-U/C: 0x00240d60
     * @ghidraAddress PAL: 0x00249890
     */
    virtual void Init();

    /**
     * Shut down the display.
     *
     * The base body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00390f58
     * @ghidraAddress PAL: 0x003ff660
     */
    virtual void Terminate();

    /**
     * Set the colour the frame is cleared to.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00390f60
     * @ghidraAddress PAL: 0x003ff668
     */
    virtual void SetClearColor(const Color &color);

    /**
     * Show or hide the timer overlay.
     *
     * @param bShow Non-zero to show the overlay.
     * @param nMaxMs The time the overlay's scale spans, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00390f70
     * @ghidraAddress PAL: 0x003ff678
     */
    virtual void SetShowTimers(int bShow, int nMaxMs);

    /**
     * Show or hide the statistics overlay.
     *
     * @param bShow Non-zero to show the overlay.
     * @ghidraAddress NTSC-U/C: 0x00390f80
     * @ghidraAddress PAL: 0x003ff688
     */
    virtual void SetShowStats(int bShow);

    /**
     * Show or hide the frame rate overlay.
     *
     * @param bShow Non-zero to show the overlay.
     * @ghidraAddress NTSC-U/C: 0x00390f88
     * @ghidraAddress PAL: 0x003ff690
     */
    virtual void SetShowRate(int bShow);

    /**
     * Write the drawing statistics.
     *
     * The base body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00390f90
     * @ghidraAddress PAL: 0x003ff698
     */
    virtual void DumpStats();

    /**
     * Save the frame to an image file.
     *
     * The base body is empty.
     *
     * @param pszName The base name of the file.
     * @ghidraAddress NTSC-U/C: 0x00390f98
     * @ghidraAddress PAL: 0x003ff6a0
     */
    virtual void ScreenDump(const char *pszName);

    /**
     * Fill a rectangle of the screen.
     *
     * The base body is empty.
     *
     * @param rect The rectangle in pixels.
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00390fa0
     * @ghidraAddress PAL: 0x003ff6a8
     */
    virtual void DrawRect(const Rect &rect, const Color &color);

    /**
     * Draw text with the line font.
     *
     * The base body draws nothing and reports the top of the text.
     *
     * @param pszText The text. A newline starts a line.
     * @param pos The top left corner in pixels.
     * @param charSize The size of a character in pixels.
     * @param color The colour.
     * @return The top of the line after the text.
     * @ghidraAddress NTSC-U/C: 0x00390fa8
     * @ghidraAddress PAL: 0x003ff6b0
     */
    virtual float DrawString(const char *pszText,
                             const Vector2 &pos,
                             const Vector2 &charSize,
                             const Color &color);

    /**
     * Open a frame.
     *
     * The base body counts the frame in mFrameCount.
     *
     * @ghidraAddress NTSC-U/C: 0x00390fb0
     * @ghidraAddress PAL: 0x003ff6b8
     */
    virtual void BeginFrame();

    /**
     * Close the frame BeginFrame() opened and present it.
     *
     * The base body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00390fc0
     * @ghidraAddress PAL: 0x003ff6c8
     */
    virtual void EndFrame();

    /**
     * Clear the back buffer and present it outside a frame.
     *
     * The base body is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00390fc8
     * @ghidraAddress PAL: 0x003ff6d0
     */
    virtual void ClearAndFlip();

    /**
     * Point drawing at the back buffer again.
     *
     * The base body is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00390fd0
     * @ghidraAddress PAL: 0x003ff6d8
     */
    virtual void RestoreFrameBufferTarget();

    /**
     * Report the height of a pixel relative to its width.
     *
     * The base reports the display height over the display width.
     *
     * @return The ratio.
     * @ghidraAddress NTSC-U/C: 0x00390fd8
     * @ghidraAddress PAL: 0x003ff6e0
     */
    virtual float AspectRatio();

    /**
     * Report whether the renderer reads texture files.
     *
     * The base reports false. The name is inferred.
     *
     * @return Whether textures are read.
     * @ghidraAddress NTSC-U/C: 0x00390ff8
     * @ghidraAddress PAL: 0x003ff700
     */
    virtual bool SupportsTextures();

    Color mClearColor; /*!< Colour the frame is cleared to. */
    int mScreenWidth;  /*!< Display width in pixels. */
    int mScreenHeight; /*!< Display height in pixels. */
    int mScreenBpp;    /*!< Display depth in bits per pixel. */
    int mTimerMaxMs;   /*!< Time the timer overlay's scale spans, in milliseconds. */
    int mFrameCount;   /*!< Frames opened by BeginFrame(). */
    int mShowTimers;   /*!< Non-zero to show the timer overlay. */
    int mShowStats;    /*!< Non-zero to show the statistics overlay. */
    int mShowRate;     /*!< Non-zero to show the frame rate overlay. */
};

/**
 * The renderer, the Ps instance at `0x0043ba00` seen through its base.
 *
 * @ghidraAddress NTSC-U/C: 0x0043bea0
 */
extern RndRenderer *TheRnd;
