#pragma once

#include <list>

#include "gfx/gsdoublebuffer.h"
#include "math/color.h"
#include "math/rect.h"
#include "math/vector2.h"
#include "os/memhandle.h"
#include "rnd/dmapacket.h"
#include "rnd/rndrenderer.h"

/**
 * The PlayStation 2 renderer.
 *
 * The RTTI includes the name and records RndRenderer as the one base. ThePs is the one instance,
 * and TheRnd points at it. The frame is built in the scratchpad by DmaPacket::shared() and sent
 * from there through the MFIFO ring in main memory to VIF1. mGsRegs shadows the GS registers, and
 * SetGsReg() writes a register only when its value changes.
 */
class Ps : public RndRenderer {
public:
    /** The number of GS registers mGsRegs shadows. */
    static constexpr int kGsRegCount = 128;

    /**
     * Construct a renderer with a full-screen blur rectangle and register the `toggle_heapnum`
     * command.
     *
     * @ghidraAddress NTSC-U/C: 0x0020fbe8
     * @ghidraAddress PAL: 0x00218a00
     */
    Ps();

    /**
     * Release the frame's pending memory handles list.
     *
     * @ghidraAddress NTSC-U/C: 0x0037c868
     * @ghidraAddress PAL: 0x003eaf98
     */
    virtual ~Ps();

    /**
     * Read the configuration, round the display to a size the GS shows, and set the display up.
     *
     * The width becomes 640, 512, 384, or 256, and the height 512, 448, 256, or 224, each the
     * largest that does not exceed the request. A height of 448 or 224 selects NTSC output. The
     * MFIFO ring is allocated, the packet builder is placed after it in the scratchpad, and the
     * PlayStation 2 factories replace the generic ones. The modal message routine of TheDebug
     * becomes ModalCallback().
     *
     * @ghidraAddress NTSC-U/C: 0x00210330
     * @ghidraAddress PAL: 0x00219148
     */
    void Init() override;

    /**
     * Restore the generic factories, stop the vertical blank handler, and release the MFIFO ring.
     *
     * @ghidraAddress NTSC-U/C: 0x00210650
     * @ghidraAddress PAL: 0x00219468
     */
    void Terminate() override;

    /**
     * Set the colour the frame is cleared to and write it into the clears of both buffers.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00211010
     * @ghidraAddress PAL: 0x00219e28
     */
    void SetClearColor(const Color &color) override;

    /**
     * Pass the drawing statistics and the free GS memory to DebugPrint().
     *
     * @ghidraAddress NTSC-U/C: 0x002115b8
     * @ghidraAddress PAL: 0x0021a3d0
     */
    void DumpStats() override;

    /**
     * Save the buffer on display to a numbered BMP file.
     *
     * An open frame is closed first and opened again afterwards.
     *
     * @param pszName The base name of the file.
     * @ghidraAddress NTSC-U/C: 0x00211c88
     * @ghidraAddress PAL: 0x0021aaa0
     */
    void ScreenDump(const char *pszName) override;

    /**
     * Fill a rectangle of the screen, blended and in front of everything.
     *
     * @param rect The rectangle in pixels.
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00211748
     * @ghidraAddress PAL: 0x0021a560
     */
    void DrawRect(const Rect &rect, const Color &color) override;

    /**
     * Draw text with the line font.
     *
     * The characters from `A` to `_`, the lowercase letters, and the characters from `'` to `:`
     * have glyphs. Every other character draws nothing. Every character but a newline advances the
     * pen by one and a half cells, and a newline starts a line two cells lower.
     *
     * @param pszText The text.
     * @param pos The top left corner in pixels.
     * @param charSize The size of a cell in pixels.
     * @param color The colour.
     * @return The top of the line after the text.
     * @ghidraAddress NTSC-U/C: 0x0020fce0
     * @ghidraAddress PAL: 0x00218af8
     */
    float DrawString(const char *pszText,
                     const Vector2 &pos,
                     const Vector2 &charSize,
                     const Color &color) override;

    /**
     * Swap the buffers, restart the MFIFO drain, and set the frame up.
     *
     * The statistics are cleared, the default camera and the default material are applied, the
     * texture cache ages, and every named timer is reset.
     *
     * @ghidraAddress NTSC-U/C: 0x00211410
     * @ghidraAddress PAL: 0x0021a228
     */
    void BeginFrame() override;

    /**
     * Draw the blur when it is active, end the packet, and send it.
     *
     * @ghidraAddress NTSC-U/C: 0x00211518
     * @ghidraAddress PAL: 0x0021a330
     */
    void EndFrame() override;

    /**
     * Clear the buffer being drawn and show it outside a frame.
     *
     * @ghidraAddress NTSC-U/C: 0x00211570
     * @ghidraAddress PAL: 0x0021a388
     */
    void ClearAndFlip() override;

    /**
     * Point FRAME_1 and XYOFFSET_1 at the buffer being drawn, and dither a 16-bit display.
     *
     * @ghidraAddress NTSC-U/C: 0x00211230
     * @ghidraAddress PAL: 0x0021a048
     */
    void RestoreFrameBufferTarget() override;

    /**
     * Report the ratio of the height of the display to its width, fixed at 3 to 4.
     *
     * @return 0.75.
     * @ghidraAddress NTSC-U/C: 0x0037c858
     * @ghidraAddress PAL: 0x003eaf88
     */
    float AspectRatio() override {
        return kAspect;
    }

    /**
     * Report that the renderer reads texture files.
     *
     * @return true.
     * @ghidraAddress NTSC-U/C: 0x0037c850
     * @ghidraAddress PAL: 0x003eaf80
     */
    bool SupportsTextures() override {
        return true;
    }

    /** The ratio AspectRatio() reports. */
    static constexpr float kAspect = 0.75f;

    /**
     * Show a message on a dark blue screen until the player presses a button.
     *
     * The overlays are hidden while the message shows, and an open frame is closed first and
     * opened again afterwards. A state of 2 is cleared when the button is the one at bit 11. The
     * name is inferred.
     *
     * @param pState The caller's state.
     * @param pszText The message.
     * @ghidraAddress NTSC-U/C: 0x002100c8
     * @ghidraAddress PAL: 0x00218ee0
     */
    void ShowModal(int *pState, const char *pszText);

    /**
     * Show a message through ThePs, the routine Init() gives TheDebug.
     *
     * The name is inferred.
     *
     * @param pState The caller's state.
     * @param pszText The message.
     * @ghidraAddress NTSC-U/C: 0x002100a0
     * @ghidraAddress PAL: 0x00218eb8
     */
    static void ModalCallback(int *pState, const char *pszText);

    /**
     * Report the GS memory the frame buffers and the depth buffer occupy.
     *
     * The name is inferred.
     *
     * @return The size in words.
     * @ghidraAddress NTSC-U/C: 0x002102d8
     * @ghidraAddress PAL: 0x002190f0
     */
    int GetReservedVramWords() const;

    /**
     * Give the display up to the movie player.
     *
     * The scratchpad is saved, the GS path is drained, and the GIF interrupt handler is removed.
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210b88
     * @ghidraAddress PAL: 0x002199a0
     */
    void ReleaseForMovie();

    /**
     * Set the display up, the first time from Init() and again after the movie player.
     *
     * The register shadow is cleared, the GS is reset, the display buffers are placed for the
     * depth, the VU microprograms are sent, and the MFIFO ring is restarted. A depth other than
     * 16, 24, or 32 bits becomes 32. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210bb8
     * @ghidraAddress PAL: 0x002199d0
     */
    void RestoreAfterMovie();

    /**
     * Copy the scratchpad to a new block in main memory, releasing an earlier copy.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210e70
     * @ghidraAddress PAL: 0x00219c88
     */
    void SaveScratchpad();

    /**
     * Copy the block SaveScratchpad() made back to the scratchpad and release it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00210ed8
     * @ghidraAddress PAL: 0x00219cf0
     */
    void RestoreScratchpad();

    /**
     * Write a GS register through an A+D tag unless its shadow already has the value.
     *
     * A mask of 0 writes the whole value without the comparison. Otherwise only the bits of the
     * mask are compared and changed.
     *
     * @param nReg The register.
     * @param qwValue The value.
     * @param qwMask The bits to set, or 0 for all of them.
     * @ghidraAddress NTSC-U/C: 0x00210f28
     * @ghidraAddress PAL: 0x00219d40
     */
    void SetGsReg(int nReg, unsigned long long qwValue, unsigned long long qwMask);

    /**
     * Report how many quadwords the packet being built still has room for.
     *
     * The name is inferred.
     *
     * @return The quadwords.
     * @ghidraAddress NTSC-U/C: 0x00211100
     * @ghidraAddress PAL: 0x00219f18
     */
    int PacketQwordsFree() const;

    /**
     * Send the packet being built and start the next one in the other half of the scratchpad.
     *
     * The routine waits until the MFIFO ring has room. An empty packet is not sent. The name is
     * inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00211128
     * @ghidraAddress PAL: 0x00219f40
     */
    void Flush();

    /**
     * Release the memory handles of the frame, wait for the vertical blank, and swap the buffers.
     *
     * The shadow takes the context 1 registers of the new draw environment. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002112c0
     * @ghidraAddress PAL: 0x0021a0d8
     */
    void SwapBuffers();

    /**
     * Draw the frame back over itself, offset and translucent, inside mBlurRect.
     *
     * The texture is the buffer on display, and the sprite does not test or write depth. The
     * title is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00211930
     * @ghidraAddress PAL: 0x0021a748
     */
    void DrawBlur();

    /**
     * Release a memory handle at the next buffer swap, once the GS no longer reads it.
     *
     * A null handle is ignored. The name is inferred.
     *
     * @param pHandle The handle, or null.
     * @ghidraAddress NTSC-U/C: 0x00211e70
     * @ghidraAddress PAL: 0x0021ac88
     */
    void ReleaseAfterFrame(MemHandle *pHandle);

    void *mScratchpadCopy; /*!< The copy SaveScratchpad() made, or null. */
    int mNtsc;             /*!< Non-zero for NTSC output, zero for PAL. */
    int mDepthBytes;       /*!< The bytes of a depth buffer pixel. */

    /** The values last written to the GS registers. */
    unsigned long long mGsRegs[kGsRegCount];

    int mSwapVblank;                 /*!< The vertical blank count the next swap waits for. */
    int mDrawBuffer;                 /*!< The buffer being drawn, 0 or 1. */
    int mReserved458;                // +0x458 Cleared by the constructor and never read.
    GsDoubleBuffer *mDisplayBuffers; /*!< The display buffers, at an uncached address. */
    int mReserved460;                // +0x460 Set to 5 by the constructor and never read.
    int mReserved464;                // +0x464 Cleared by the constructor and never read.
    int mReserved468;                // +0x468 Cleared by the constructor and never read.

    /**
     * Whether the full-screen blur runs. The blur action and the enlarging blur set it, and the
     * head-up display clears it when it goes away.
     */
    int mBlurActive;

    float mBlurRect[4];   /*!< The part of the screen DrawBlur() covers, x, y, w, and h. */
    float mBlurAmount;    /*!< The opacity of the frame DrawBlur() draws. */
    int mBlurCount;       /*!< The offset of the frame DrawBlur() draws, in 1/16 pixel. */
    int mInFrame;         /*!< Non-zero between BeginFrame() and EndFrame(). */
    int mScreenDumpCount; /*!< The number ScreenDump() gives the next file. */
    void *mMfifo;         /*!< The MFIFO ring. */
    std::list<MemHandle *> mFrameHandles; /*!< The handles SwapBuffers() releases. */
};

/**
 * The renderer.
 *
 * @ghidraAddress NTSC-U/C: 0x0043ba00
 */
extern Ps ThePs;

/**
 * The vertical blanks counted since start-up.
 *
 * @ghidraAddress NTSC-U/C: 0x003afd08
 */
extern volatile int gVCount;

/**
 * The DMA chain that loads the VU0 microprograms, ending with an END tag.
 *
 * @ghidraAddress NTSC-U/C: 0x003ae240
 */
extern DmaQuadword g_vu0MicrocodeChain[];

/**
 * The DMA chain that loads the VU1 microprograms and resets the VIF1 double buffer, ending with an
 * END tag.
 *
 * @ghidraAddress NTSC-U/C: 0x003ab970
 */
extern DmaQuadword g_vu1MicrocodeChain[];
