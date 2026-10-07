#ifndef SIF_H
#define SIF_H

#ifdef __cplusplus
extern "C" {
#endif

/** SIF DMA services of the resident sifman library. */

/** One transfer from IOP memory to EE memory. */
typedef struct {
    void *data; /*!< Source in IOP memory. */
    void *addr; /*!< Destination in EE memory. */
    int size;   /*!< Byte count. */
    int mode;   /*!< Transfer mode bits. */
} sceSifDmaData;

/**
 * Queue transfers to the EE.
 *
 * @param sdd The transfers.
 * @param len Number of transfers.
 * @return The transfer identifier, or zero when the queue is full.
 */
unsigned int sceSifSetDma(sceSifDmaData *sdd, int len);

/** Initialise the SIF DMA interface. */
void sceSifInit(void);

/**
 * Report whether the SIF DMA interface is initialised.
 *
 * @return Nonzero once sceSifInit() has run.
 */
int sceSifCheckInit(void);

#ifdef __cplusplus
}
#endif

#endif
