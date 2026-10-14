#ifndef LIBGRAPH_H
#define LIBGRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Graphics library: GS environment packets, image transfers, double buffering, and vertical blank
 * synchronisation.
 */

/**
 * Image download descriptor, 112 bytes.
 *
 * Four VIF codes, a GIFtag, and five register-and-address pairs for BITBLTBUF, TRXPOS, TRXREG,
 * FINISH, and TRXDIR.
 */
typedef struct {
    unsigned long long mWords[14]; /*!< The descriptor words. */
} sceGsStoreImage __attribute__((aligned(16)));

/**
 * One GIFtag, 16 bytes.
 *
 * DMA reads it by quadword. It and every structure that includes it are therefore quadword
 * aligned.
 */
typedef struct {
    unsigned long long mWords[2]; /*!< The tag words. */
} sceGifTag __attribute__((aligned(16)));

/**
 * The five display registers in the order sceGsSetDefDispEnv() fills them.
 *
 * The game writes each word to the privileged register of the same name.
 */
typedef struct {
    unsigned long long pmode;   /*!< PMODE. */
    unsigned long long smode2;  /*!< SMODE2. */
    unsigned long long dispfb;  /*!< DISPFB. */
    unsigned long long display; /*!< DISPLAY. */
    unsigned long long bgcolor; /*!< BGCOLOR. */
} sceGsDispEnv;

/**
 * Eight A+D register and address pairs, 128 bytes. sceGsSetDefDrawEnv() returns 8, the pair count.
 *
 * The game reads FRAME_1 from the first pair, writes ZBUF_1 into the second, and reads XYOFFSET_1
 * from the third. GfxDevice::SwapBuffers() copies every register word into its register shadow.
 * The order of the remaining five follows from that copy.
 */
typedef struct {
    unsigned long long frame1;         /*!< FRAME_1. */
    unsigned long long frame1addr;     /*!< Address of FRAME_1. */
    unsigned long long zbuf1;          /*!< ZBUF_1. */
    unsigned long long zbuf1addr;      /*!< Address of ZBUF_1. */
    unsigned long long xyoffset1;      /*!< XYOFFSET_1. */
    unsigned long long xyoffset1addr;  /*!< Address of XYOFFSET_1. */
    unsigned long long scissor1;       /*!< SCISSOR_1. */
    unsigned long long scissor1addr;   /*!< Address of SCISSOR_1. */
    unsigned long long prmodecont;     /*!< PRMODECONT. */
    unsigned long long prmodecontaddr; /*!< Address of PRMODECONT. */
    unsigned long long colclamp;       /*!< COLCLAMP. */
    unsigned long long colclampaddr;   /*!< Address of COLCLAMP. */
    unsigned long long dthe;           /*!< DTHE. */
    unsigned long long dtheaddr;       /*!< Address of DTHE. */
    unsigned long long test1;          /*!< TEST_1. */
    unsigned long long test1addr;      /*!< Address of TEST_1. */
} sceGsDrawEnv1;

/** The GS RGBAQ register. */
typedef struct {
    unsigned char R; /*!< Red. */
    unsigned char G; /*!< Green. */
    unsigned char B; /*!< Blue. */
    unsigned char A; /*!< Alpha. */
    float Q;         /*!< Q texture coordinate. */
} sceGsRgbaq;

/**
 * Six A+D register and address pairs, 96 bytes. sceGsSetDefClear() returns 6, the pair count.
 *
 * The pair names follow GS register order and are inferred.
 */
typedef struct {
    unsigned long long testa;     /*!< TEST_1 disabling the depth test. */
    unsigned long long testaaddr; /*!< Address of the first TEST_1. */
    unsigned long long prim;      /*!< PRIM selecting a sprite. */
    unsigned long long primaddr;  /*!< Address of PRIM. */
    union {
        sceGsRgbaq rgbaq;             /*!< Clear colour, by field. */
        unsigned long long rgbaqWord; /*!< Clear colour, as one word. */
    };
    unsigned long long rgbaqaddr; /*!< Address of RGBAQ. */
    unsigned long long xyz2a;     /*!< First corner of the sprite. */
    unsigned long long xyz2aaddr; /*!< Address of the first XYZ2. */
    unsigned long long xyz2b;     /*!< Second corner of the sprite. */
    unsigned long long xyz2baddr; /*!< Address of the second XYZ2. */
    unsigned long long testb;     /*!< TEST_1 restoring the requested depth test. */
    unsigned long long testbaddr; /*!< Address of the second TEST_1. */
} sceGsClear;

/**
 * The Sony double buffer, 0x230 bytes.
 *
 * sceGsSetDefDBuff() fills the clear colour of each half at +0x100 and +0x1f0. Under this layout
 * those offsets are the RGBAQ pair of each sceGsClear.
 */
typedef struct {
    sceGsDispEnv disp[2]; /*!< Display registers of each half. */
    sceGifTag giftag0;    /*!< GIFtag of the first half. */
    sceGsDrawEnv1 draw0;  /*!< Draw environment of the first half. */
    sceGsClear clear0;    /*!< Clear packet of the first half. */
    sceGifTag giftag1;    /*!< GIFtag of the second half. */
    sceGsDrawEnv1 draw1;  /*!< Draw environment of the second half. */
    sceGsClear clear1;    /*!< Clear packet of the second half. */
} sceGsDBuff;

/**
 * Four A+D register and address pairs, 64 bytes, for ALPHA_1, PABE, TEXA, and FBA_1.
 *
 * sceGsSetDefAlphaEnv() returns 4, the pair count.
 */
typedef struct {
    unsigned long long mWords[8]; /*!< The pair words. */
} sceGsAlphaEnv;

/**
 * Reset VIF1, VU1, and the GIF, and prime VIF1 through its FIFO.
 *
 * Waits for VU1 to stop after the reset.
 *
 * @ghidraAddress NTSC-U/C: 0x0030ea90
 * @ghidraAddress PAL: 0x0037b040
 */
void sceGsResetPath(void);

/**
 * Reset the graphics state.
 *
 * Mode 1 clears the vertical blank flag. Mode 5 replays the video setup and retains the vertical
 * blank handler. Mode 0 also removes the handler. Other modes return at once.
 *
 * @param nMode The reset mode.
 * @param nInterlace Interlace setting.
 * @param nOutputMode Video output mode.
 * @param nFieldMode Field or frame mode.
 * @ghidraAddress NTSC-U/C: 0x0030f1e8
 * @ghidraAddress PAL: 0x0037b798
 */
void sceGsResetGraph(short nMode, short nInterlace, short nOutputMode, short nFieldMode);

/**
 * Wait for the next vertical blank.
 *
 * @param nMode The wait mode.
 * @return The field the vertical blank began. Progressive modes report field 1.
 * @ghidraAddress NTSC-U/C: 0x0030fc48
 * @ghidraAddress PAL: 0x0037c1f8
 */
int sceGsSyncV(int nMode);

/**
 * Acknowledge the vertical blank start interrupt, spin until it is raised again, and acknowledge it
 * once more. Interrupts are disabled around each acknowledgement.
 *
 * sceGsSyncV() and the game both call it.
 *
 * @ghidraAddress NTSC-U/C: 0x003173b0
 * @ghidraAddress PAL: 0x00383c70
 */
void VSync(void);

/**
 * Fill the four alpha environment pairs.
 *
 * @param pAlpha The pairs.
 * @param nPabe PABE setting.
 * @return 4, the pair count.
 * @ghidraAddress NTSC-U/C: 0x00621c20
 * @ghidraAddress PAL: 0x006627b0
 */
int sceGsSetDefAlphaEnv(sceGsAlphaEnv *pAlpha, short nPabe);

/**
 * Fill the five display registers.
 *
 * The output mode selects the timing (NTSC, PAL, or 480p). An unknown mode only reports an error.
 * Modes other than NTSC and PAL add the display offsets the kernel of a newer console reports.
 *
 * @param pDisp The registers.
 * @param nPsm Pixel storage format.
 * @param nWidth Width in pixels.
 * @param nHeight Height in pixels.
 * @param nDx Horizontal display offset.
 * @param nDy Vertical display offset.
 * @ghidraAddress NTSC-U/C: 0x0030f438
 * @ghidraAddress PAL: 0x0037b9e8
 */
void sceGsSetDefDispEnv(
    sceGsDispEnv *pDisp, short nPsm, short nWidth, short nHeight, short nDx, short nDy);

/**
 * Write a display environment to the privileged display registers.
 *
 * @param pDisp The registers to write.
 * @ghidraAddress NTSC-U/C: 0x0030f7b0
 * @ghidraAddress PAL: 0x0037bd60
 */
void sceGsPutDispEnv(const sceGsDispEnv *pDisp);

/**
 * Fill the eight draw environment pairs.
 *
 * The depth buffer follows the frame buffer, dithering follows the colour depth, and the test word
 * follows the depth test.
 *
 * @param pDraw The pairs.
 * @param nPsm Pixel storage format of the frame buffer.
 * @param nWidth Width in pixels.
 * @param nHeight Height in pixels.
 * @param nZTest Depth test.
 * @param nZPsm Pixel storage format of the depth buffer.
 * @return 8, the pair count.
 * @ghidraAddress NTSC-U/C: 0x0030f870
 * @ghidraAddress PAL: 0x0037be20
 */
int sceGsSetDefDrawEnv(
    sceGsDrawEnv1 *pDraw, short nPsm, short nWidth, short nHeight, short nZTest, short nZPsm);

/**
 * Fill the six clear packet pairs.
 *
 * The packet disables testing, draws one sprite in the clear colour, then restores the requested
 * test.
 *
 * @param pClear The pairs.
 * @param nZTest Depth test to restore.
 * @param nX Left edge.
 * @param nY Top edge.
 * @param nWidth Width in pixels.
 * @param nHeight Height in pixels.
 * @param nRed Red of the clear colour.
 * @param nGreen Green of the clear colour.
 * @param nBlue Blue of the clear colour.
 * @param nAlpha Alpha of the clear colour.
 * @param nZ Depth of the clear.
 * @return 6, the pair count.
 * @ghidraAddress NTSC-U/C: 0x0030fa58
 * @ghidraAddress PAL: 0x0037c008
 */
int sceGsSetDefClear(sceGsClear *pClear,
                     short nZTest,
                     short nX,
                     short nY,
                     short nWidth,
                     short nHeight,
                     unsigned long long nRed,
                     unsigned long long nGreen,
                     unsigned long long nBlue,
                     unsigned long long nAlpha,
                     unsigned int nZ);

/**
 * Fill the display, draw, and clear halves of a double buffer and its two GIFtags.
 *
 * The clear halves stay empty unless nClear requests them. Interlaced modes patch the frame
 * addresses of the first half for the odd field.
 *
 * @param pDBuff The double buffer.
 * @param nPsm Pixel storage format of the frame buffers.
 * @param nWidth Width in pixels.
 * @param nHeight Height in pixels.
 * @param nZTest Depth test.
 * @param nZPsm Pixel storage format of the depth buffer.
 * @param nClear Nonzero to fill the clear halves.
 * @ghidraAddress NTSC-U/C: 0x005e4b30
 * @ghidraAddress PAL: 0x00626cf0
 */
int sceGsSetDefDBuff(sceGsDBuff *pDBuff,
                     short nPsm,
                     short nWidth,
                     short nHeight,
                     short nZTest,
                     short nZPsm,
                     short nClear);

/**
 * Send one draw environment packet through the GIF channel.
 *
 * A busy channel is given a short wait. An expiry reports an error.
 *
 * @param pGifTag The GIFtag that opens the packet.
 * @return 0, or -1 when the wait expires.
 * @ghidraAddress NTSC-U/C: 0x0030fb60
 * @ghidraAddress PAL: 0x0037c110
 */
int sceGsPutDrawEnv(sceGifTag *pGifTag);

/**
 * Fill an image download descriptor.
 *
 * The packet spans 0x70 bytes of GIFtag, register data, and register identifiers.
 *
 * @param pStoreImage The descriptor.
 * @param nSbp Source buffer base pointer.
 * @param nSbw Source buffer width.
 * @param nPsm Pixel storage format.
 * @param nSsx Left edge of the source.
 * @param nSsy Top edge of the source.
 * @param nRrw Width of the transfer in pixels.
 * @param nRrh Height of the transfer in pixels.
 * @return 7, the register count.
 * @ghidraAddress NTSC-U/C: 0x0030fce0
 * @ghidraAddress PAL: 0x0037c290
 */
int sceGsSetDefStoreImage(sceGsStoreImage *pStoreImage,
                          short nSbp,
                          short nSbw,
                          short nPsm,
                          short nSsx,
                          short nSsy,
                          short nRrw,
                          short nRrh);

/**
 * Send an image download descriptor, then receive the pixels at pDest.
 *
 * A width that does not fill whole quadwords rounds the height up and drains the remainder
 * through a stack slot.
 *
 * @param pStoreImage The descriptor.
 * @param pDest Receives the pixels.
 * @return 0, or -1 when a wait times out.
 * @ghidraAddress NTSC-U/C: 0x0030fe20
 * @ghidraAddress PAL: 0x0037c3d0
 */
int sceGsExecStoreImage(sceGsStoreImage *pStoreImage, void *pDest);

/**
 * Wait until VIF1, the GIF, and VU1 are idle, without a time limit.
 *
 * @param nMode Not used.
 * @param nTimeout Not used.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x0030eb30
 * @ghidraAddress PAL: 0x0037b0e0
 */
int sceGsSyncPath(int nMode, unsigned short nTimeout);

/**
 * Install pfnHandler on the vertical blank start interrupt, replacing the previous handler.
 *
 * @param pfnHandler The handler, or null to remove the handler.
 * @return The previous handler.
 * @ghidraAddress NTSC-U/C: 0x003104b0
 * @ghidraAddress PAL: 0x0037ca60
 */
int (*sceGsSyncVCallback(int (*pfnHandler)(int)))(int);

#ifdef __cplusplus
}
#endif

#endif
