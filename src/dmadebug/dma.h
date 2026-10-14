#ifndef DMADEBUG_DMA_H
#define DMADEBUG_DMA_H

/**
 * DMA controller simulation of the DMAdebug packet checker.
 *
 * The checker steps a copy of a channel's registers through a transfer a quadword at a time,
 * validating and reporting each DMA tag of a chain. The routine names are inferred.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** EE DMA channels, in the order of their names. */
enum {
    DMA_CHANNEL_VIF0 = 0,     /*!< Channel 0, VIF0. */
    DMA_CHANNEL_VIF1 = 1,     /*!< Channel 1, VIF1. */
    DMA_CHANNEL_GIF = 2,      /*!< Channel 2, GIF. */
    DMA_CHANNEL_FROM_IPU = 3, /*!< Channel 3, from the IPU. */
    DMA_CHANNEL_TO_IPU = 4,   /*!< Channel 4, to the IPU. */
    DMA_CHANNEL_SIF0 = 5,     /*!< Channel 5, SIF0. */
    DMA_CHANNEL_SIF1 = 6,     /*!< Channel 6, SIF1. */
    DMA_CHANNEL_SIF2 = 7,     /*!< Channel 7, SIF2. */
    DMA_CHANNEL_FROM_SPR = 8, /*!< Channel 8, from the scratchpad. */
    DMA_CHANNEL_TO_SPR = 9,   /*!< Channel 9, to the scratchpad. */
};

/** The registers of a channel the simulation steps. */
struct dma_channel {
    unsigned int chcr; /*!< Channel control. */
    unsigned int madr; /*!< Address of the next quadword of data. */
    unsigned int qwc;  /*!< Quadwords of data left before the next tag. */
    unsigned int tadr; /*!< Address of the current tag. */
    unsigned int asr0; /*!< First return address of the call stack. */
    unsigned int asr1; /*!< Second return address of the call stack. */
    unsigned int sadr; /*!< Scratchpad address of a scratchpad channel. */
};

/**
 * Index of the channel being checked, one of the DMA_CHANNEL values.
 *
 * @ghidraAddress NTSC-U/C: 0x005198c8
 */
extern int g_nDmaChannel;

/**
 * The quadword the last step moved, or null.
 *
 * @ghidraAddress NTSC-U/C: 0x005198cc
 */
extern const unsigned int *g_pDmaQword;

/**
 * Non-zero when the quadword the last step moved is a DMA tag.
 *
 * @ghidraAddress NTSC-U/C: 0x005198d0
 */
extern int g_nDmaQwordIsTag;

/**
 * Move the next quadword of the channel, validating each tag a chain reads.
 *
 * Leaves the quadword in g_pDmaQword, or null there when the step moved none.
 *
 * @return The flags of the messages the step raised.
 * @ghidraAddress NTSC-U/C: 0x002a2638
 * @ghidraAddress PAL: 0x002ac5a8
 */
int DMA_step(void);

#ifdef __cplusplus
}
#endif

#endif
