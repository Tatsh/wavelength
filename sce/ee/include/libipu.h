#ifndef LIBIPU_H
#define LIBIPU_H

#ifdef __cplusplus
extern "C" {
#endif

/** IPU library: command synchronisation with the image processing unit. */

/** The state of the two IPU DMA channels and the IPU that sceIpuStopDMA() saves. */
typedef struct {
    unsigned int d4madr;  /*!< D4_MADR, the input channel address. */
    unsigned int d4tadr;  /*!< D4_TADR, the input channel tag address. */
    unsigned int d4qwc;   /*!< D4_QWC, the input channel quadword count. */
    unsigned int d4chcr;  /*!< D4_CHCR, the input channel control. */
    unsigned int d3madr;  /*!< D3_MADR, the output channel address. */
    unsigned int d3qwc;   /*!< D3_QWC, the output channel quadword count. */
    unsigned int d3chcr;  /*!< D3_CHCR, the output channel control. */
    unsigned int ipubp;   /*!< IPU_BP, the bitstream position. */
    unsigned int ipuctrl; /*!< IPU_CTRL, the IPU control. */
} sceIpuDmaEnv;

/**
 * Reset the IPU and load its default tables.
 *
 * @ghidraAddress NTSC-U/C: 0x0030efb0
 * @ghidraAddress PAL: 0x0037b560
 */
void sceIpuInit(void);

/**
 * Stop both IPU DMA channels and save their state and the IPU's.
 *
 * @param env Receives the state.
 * @ghidraAddress NTSC-U/C: 0x0030eca8
 * @ghidraAddress PAL: 0x0037b258
 */
void sceIpuStopDMA(sceIpuDmaEnv *env);

/**
 * Restart both IPU DMA channels from the state sceIpuStopDMA() saved.
 *
 * @param env The state.
 * @ghidraAddress NTSC-U/C: 0x0030ed90
 * @ghidraAddress PAL: 0x0037b340
 */
void sceIpuRestartDMA(sceIpuDmaEnv *env);

/**
 * Wait for the IPU to finish its command, or report whether it is still busy.
 *
 * @param mode 0 to wait until IPU_CTRL reports the IPU idle, 1 to report its busy bit only.
 * @param timeout Unused by the shipped library.
 * @return The busy bit for mode 1, and 0 otherwise.
 * @ghidraAddress NTSC-U/C: 0x0030eee0
 * @ghidraAddress PAL: 0x0037b490
 */
int sceIpuSync(int mode, unsigned short timeout);

#ifdef __cplusplus
}
#endif

#endif
