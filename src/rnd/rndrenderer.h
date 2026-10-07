#pragma once

#include "math/color.h"

/**
 * Base of the platform renderer, the owner of the display and of the frame.
 *
 * The RTTI includes the class name. Ps is the one subclass, and its single instance at `0x0043ba00`
 * is the renderer TheRnd addresses. The class introduces its virtuals after twelve words of data,
 * and the vptr therefore sits at `+0x30`. The constructor at `0x00240cf8` sets the clear colour to
 * an opaque grey of 0.3, the screen to 640 by 480 at 16 bits, and the word at `+0x1c` to 50.
 *
 * The vtable runs thirteen slots after the type function. The four slots the main loop calls are
 * declared here. Slot 1 is Init(), slot 2 Terminate(), slot 11 BeginFrame(), and slot 12
 * EndFrame().
 */
class RndRenderer {
public:
    /**
     * Read the renderer section of the configuration and bring up the display.
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

    Color mClearColor; /*!< Colour the frame is cleared to. */
    int mScreenWidth;  /*!< Display width in pixels. */
    int mScreenHeight; /*!< Display height in pixels. */
    int mScreenBpp;    /*!< Display depth in bits per pixel. */
    int mReserved1C;   // +0x1c, 50 from the constructor, also written by vtable slot 4.
    int mFrameCount;   /*!< Frames opened by BeginFrame(). */
    int mReserved24;   // +0x24, written by vtable slot 4.
    int mReserved28;   // +0x28, written by vtable slot 5.
    int mReserved2C;   // +0x2c, written by vtable slot 6.
};

/**
 * The renderer, the Ps instance at `0x0043ba00` seen through its base.
 *
 * @ghidraAddress NTSC-U/C: 0x0043bea0
 */
extern RndRenderer *TheRnd;
