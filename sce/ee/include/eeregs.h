#ifndef EEREGS_H
#define EEREGS_H

/**
 * Emotion Engine and GS hardware registers. Each macro is a pointer to its register, as in
 * `*D_STAT = value`.
 */

/** Timer 0 counter register. */
#define T0_COUNT ((volatile unsigned int *)0x10000000)
/** Timer 0 mode register. */
#define T0_MODE ((volatile unsigned int *)0x10000010)
/** Timer 3 mode register. */
#define T3_MODE ((volatile unsigned int *)0x10001810)

/** IPU command register, which also reads back the data of the last command. */
#define IPU_CMD ((volatile unsigned int *)0x10002000)
/** IPU control register. */
#define IPU_CTRL ((volatile unsigned int *)0x10002010)

/** DMA channel 0 (VIF0) control register. */
#define D0_CHCR ((volatile unsigned int *)0x10008000)
/** DMA channel 1 (VIF1) control register. */
#define D1_CHCR ((volatile unsigned int *)0x10009000)
/** DMA channel 1 (VIF1) tag address register. */
#define D1_TADR ((volatile unsigned int *)0x10009030)
/** DMA channel 2 (GIF) control register. */
#define D2_CHCR ((volatile unsigned int *)0x1000a000)
/** DMA channel 2 (GIF) quadword count register. */
#define D2_QWC ((volatile unsigned int *)0x1000a020)
/** DMA channel 2 (GIF) tag address register. */
#define D2_TADR ((volatile unsigned int *)0x1000a030)
/** DMA channel 3 (from the IPU) control register. */
#define D3_CHCR ((volatile unsigned int *)0x1000b000)
/** DMA channel 3 (from the IPU) memory address register. */
#define D3_MADR ((volatile unsigned int *)0x1000b010)
/** DMA channel 3 (from the IPU) quadword count register. */
#define D3_QWC ((volatile unsigned int *)0x1000b020)
/** DMA channel 4 (to the IPU) control register. */
#define D4_CHCR ((volatile unsigned int *)0x1000b400)
/** DMA channel 4 (to the IPU) memory address register. */
#define D4_MADR ((volatile unsigned int *)0x1000b410)
/** DMA channel 4 (to the IPU) quadword count register. */
#define D4_QWC ((volatile unsigned int *)0x1000b420)
/** DMA channel 4 (to the IPU) tag address register. */
#define D4_TADR ((volatile unsigned int *)0x1000b430)
/** DMA channel 8 (from the scratchpad) memory address register. */
#define D8_MADR ((volatile unsigned int *)0x1000d010)
/** DMA controller control register. */
#define D_CTRL ((volatile unsigned int *)0x1000e000)
/** The low byte of the DMA controller control register, which holds the MFIFO and stall fields. */
#define D_CTRL_LOW ((volatile unsigned char *)0x1000e000)
/** DMA controller interrupt status register. */
#define D_STAT ((volatile unsigned int *)0x1000e010)
/** DMA controller MFIFO ring size register. */
#define D_RBSR ((volatile unsigned int *)0x1000e040)
/** DMA controller MFIFO ring base register. */
#define D_RBOR ((volatile unsigned int *)0x1000e050)
/** DMA controller write register of the hold state of every channel. */
#define D_ENABLEW ((volatile unsigned int *)0x1000f590)

/** Serial port interrupt status register. */
#define SIO_ISR ((volatile unsigned int *)0x1000f130)
/** Serial port transmit FIFO. */
#define SIO_TXFIFO ((volatile unsigned char *)0x1000f180)

/** GS privileged register that merges the two read circuits. */
#define GS_PMODE ((volatile unsigned long long *)0x12000000)
/** GS privileged register of the video synchronisation mode. */
#define GS_SMODE2 ((volatile unsigned long long *)0x12000020)
/** GS privileged register of the frame buffer read by circuit 1. */
#define GS_DISPFB1 ((volatile unsigned long long *)0x12000070)
/** GS privileged register of the display area of circuit 1. */
#define GS_DISPLAY1 ((volatile unsigned long long *)0x12000080)
/** GS privileged register of the frame buffer read by circuit 2. */
#define GS_DISPFB2 ((volatile unsigned long long *)0x12000090)
/** GS privileged register of the display area of circuit 2. */
#define GS_DISPLAY2 ((volatile unsigned long long *)0x120000a0)
/** GS privileged register of the background colour. */
#define GS_BGCOLOR ((volatile unsigned long long *)0x120000e0)
/** GS system status register. */
#define GS_CSR ((volatile unsigned long long *)0x12001000)

#endif
