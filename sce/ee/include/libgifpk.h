#ifndef LIBGIFPK_H
#define LIBGIFPK_H

#ifdef __cplusplus
extern "C" {
#endif

/** GIF packet library: building DMA chains of GIFtags and A+D register writes. */

/** GIF packet work area, 16 bytes. The member names are inferred. */
typedef struct {
    void *mCurrent;  /*!< Current write position. */
    void *mBase;     /*!< Packet base set at initialisation. */
    void *mDmaTag;   /*!< DMA tag under construction, or null. */
    void *mPrevious; /*!< Start of the open GIFtag, or null. */
} sceGifPkData;

/**
 * Attach a packet to a buffer and clear the pending DMA tag.
 *
 * @param pPacket The packet.
 * @param pBuffer The buffer.
 */
void sceGifPkInit(sceGifPkData *pPacket, void *pBuffer);

/**
 * Rewind a packet to its base and clear the pending DMA tag.
 *
 * @param pPacket The packet.
 */
void sceGifPkReset(sceGifPkData *pPacket);

/**
 * Finalise the pending DMA tag, then open a continue tag.
 *
 * @param pPacket The packet.
 * @param nLoop Third word of the tag.
 * @param nEop Fourth word of the tag.
 * @param nPre Bits merged into the first word of the tag.
 */
void sceGifPkCnt(sceGifPkData *pPacket, int nLoop, int nEop, int nPre);

/**
 * Finalise the pending DMA tag, then open an end tag.
 *
 * @param pPacket The packet.
 * @param nLoop Third word of the tag.
 * @param nEop Fourth word of the tag.
 * @param nPre Bits merged into the first word of the tag.
 */
void sceGifPkEnd(sceGifPkData *pPacket, int nLoop, int nEop, int nPre);

/**
 * Copy a 16-byte GIFtag to the write position and open the GIFtag there.
 *
 * @param pPacket The packet.
 * @param pData The GIFtag.
 */
void sceGifPkOpenGsAD(sceGifPkData *pPacket, const void *pData);

/**
 * Append one A+D pair, the data word followed by the register address.
 *
 * @param pPacket The packet.
 * @param nAddress The register address.
 * @param nData The register data.
 */
void sceGifPkAddGsAD(sceGifPkData *pPacket, int nAddress, unsigned long long nData);

/**
 * Add the loop count of the data written since the open GIFtag to the GIFtag and close it.
 *
 * Pads the write position to 16 bytes with zeroes.
 *
 * @param pPacket The packet.
 */
void sceGifPkCloseGifTag(sceGifPkData *pPacket);

/**
 * Pad the write position to 16 bytes with zeroes and add the quadwords written since the pending
 * DMA tag to its length.
 *
 * @param pPacket The packet.
 */
void sceGifPkTerminate(sceGifPkData *pPacket);

/**
 * Reserve nWords 32-bit words at the write position.
 *
 * @param pPacket The packet.
 * @param nWords Count of 32-bit words.
 * @return The reserved area.
 */
unsigned long long *sceGifPkReserve(sceGifPkData *pPacket, int nWords);

/**
 * Finalise the pending DMA tag, then write a reference tag for nQuadwords quadwords at pData.
 *
 * @param pPacket The packet.
 * @param pData The referenced data.
 * @param nQuadwords Size of the data in quadwords.
 * @param nOption1 Third word of the tag.
 * @param nOption2 Fourth word of the tag.
 * @param nFlag Bits merged into the first word of the tag.
 */
void sceGifPkRef(
    sceGifPkData *pPacket, void *pData, int nQuadwords, int nOption1, int nOption2, int nFlag);

#ifdef __cplusplus
}
#endif

#endif
