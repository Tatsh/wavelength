#ifndef LIBVIFPK_H
#define LIBVIFPK_H

#include <libgraph.h>

#ifdef __cplusplus
extern "C" {
#endif

/** VIF packet library: building VIF1 DMA chains with DIRECT codes and GIFtags. */

/**
 * VIF1 packet under construction, 32 bytes.
 *
 * sceVif1PkInit() sets the first two words to the buffer. The game reads the write pointer to
 * build in place and the base to send the finished chain.
 */
typedef struct {
    unsigned int *pCurrent;      /*!< Write pointer. */
    void *pBase;                 /*!< Start of the buffer. */
    unsigned int mOtherWords[6]; /*!< Open DMA tag, open VIF code, and open GIFtag addresses. */
} sceVif1Packet;

/**
 * Attach a packet to a buffer and clear the open DMA tag.
 *
 * @param pPacket The packet.
 * @param pBase The buffer.
 */
void sceVif1PkInit(sceVif1Packet *pPacket, void *pBase);

/**
 * Rewind a packet to its base and clear the open DMA tag.
 *
 * @param pPacket The packet.
 */
void sceVif1PkReset(sceVif1Packet *pPacket);

/**
 * Close the open DMA tag, then open a continue tag.
 *
 * @param pPacket The packet.
 * @param nOption Bits merged into the first word of the tag.
 */
void sceVif1PkCnt(sceVif1Packet *pPacket, unsigned int nOption);

/**
 * Align the write pointer for a DIRECT code and open one there.
 *
 * @param pPacket The packet.
 * @param bStall Nonzero to set the interrupt bit of the code.
 */
void sceVif1PkOpenDirectCode(sceVif1Packet *pPacket, int bStall);

/**
 * Write a GIFtag at the write pointer and open it.
 *
 * @param pPacket The packet.
 * @param gifTag The GIFtag.
 */
void sceVif1PkOpenGifTag(sceVif1Packet *pPacket, sceGifTag gifTag);

/**
 * Reserve nWords 32-bit words at the write pointer.
 *
 * @param pPacket The packet.
 * @param nWords Count of 32-bit words.
 * @return The reserved area.
 */
unsigned int *sceVif1PkReserve(sceVif1Packet *pPacket, unsigned int nWords);

/**
 * Add the loop count of the data written since the open GIFtag to the GIFtag and close it.
 *
 * Pads the write pointer to 16 bytes with zeroes.
 *
 * @param pPacket The packet.
 */
void sceVif1PkCloseGifTag(sceVif1Packet *pPacket);

/**
 * Add the quadwords written since the open DIRECT code to the code and close it.
 *
 * @param pPacket The packet.
 */
void sceVif1PkCloseDirectCode(sceVif1Packet *pPacket);

/**
 * Close the open DMA tag, then open an end tag.
 *
 * @param pPacket The packet.
 * @param nOption Bits merged into the first word of the tag.
 */
void sceVif1PkEnd(sceVif1Packet *pPacket, unsigned int nOption);

/**
 * Pad the write pointer to 16 bytes with zeroes and add the quadwords written since the open DMA
 * tag to its length.
 *
 * @param pPacket The packet.
 * @return The write pointer.
 */
unsigned int *sceVif1PkTerminate(sceVif1Packet *pPacket);

/**
 * Zero-fill from the write pointer until nSize words past the 2^(nKind + 2) byte boundary at or
 * below it.
 *
 * The fill wraps to the next boundary when the target position is already behind the pointer.
 *
 * @param pPacket The packet.
 * @param nKind Boundary exponent less 2.
 * @param nSize Count of 32-bit words past the boundary.
 */
void sceVif1PkAlign(sceVif1Packet *pPacket, int nKind, int nSize);

#ifdef __cplusplus
}
#endif

#endif
