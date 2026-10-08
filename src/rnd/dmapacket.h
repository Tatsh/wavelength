#pragma once

/** One quadword of a DMA packet, viewed as doublewords, words, or halfwords. */
struct alignas(16) DmaQuadword {
    union {
        unsigned long long mDword[2]; /*!< The two doublewords. */
        unsigned int mWord[4];        /*!< The four words. */
        unsigned short mHalf[8];      /*!< The eight halfwords. */
    };
};

/**
 * Builder of a DMA packet of GIF data, with DMA tags, GIF tags, and optional VIF DIRECT codes.
 *
 * The class has no virtual and no RTTI, and its name is inferred. The renderer keeps its one
 * instance at the start of the scratchpad and fills the two halves after it in turn.
 */
class DmaPacket {
public:
    /** How Close() ends the packet and how the channel transfers it. */
    enum Mode {
        kModeNormal = 0,      /*!< One block of quadwords from the base. */
        kModeChain = 1,       /*!< A chain of DMA tags from the base. */
        kModeChainTagged = 2, /*!< A chain of DMA tags, with each tag transferred as well. */
    };

    /**
     * Report the instance at the start of the scratchpad.
     *
     * @return The packet.
     */
    static DmaPacket &shared();

    /**
     * Close the DMA tag being filled and open a new one at the cursor.
     *
     * @ghidraAddress NTSC-U/C: 0x00211fb0
     * @ghidraAddress PAL: 0x0021adc8
     */
    void OpenDmaTag();

    /**
     * Close the DMA tag being filled.
     *
     * The open GIF tag is closed first with its end-of-packet bit.
     *
     * @param nId The tag ID.
     * @param nAddress The tag's address field.
     * @param nQwc The quadwords the tag covers, or 0 for those written since it was opened.
     * @ghidraAddress NTSC-U/C: 0x00211ff0
     * @ghidraAddress PAL: 0x0021ae08
     */
    void CloseDmaTag(int nId, unsigned int nAddress, int nQwc);

    /**
     * Close the GIF tag being filled.
     *
     * The loop count comes from the quadwords written since the tag was opened unless one is
     * given. An open VIF DIRECT code grows by the tag's quadwords.
     *
     * @param bEop Whether the tag ends the GIF packet, which also ends the VIF DIRECT code.
     * @param nLoops The loop count, or 0 to count the quadwords written.
     * @ghidraAddress NTSC-U/C: 0x002120a0
     * @ghidraAddress PAL: 0x0021aeb8
     */
    void CloseGifTag(int bEop, int nLoops);

    /**
     * Close the GIF tag being filled and open a new one at the cursor.
     *
     * A packet sent through VIF opens a VIF DIRECT code first when none is open.
     *
     * @param nFlg The data format of the tag.
     * @param nRegs The number of register descriptors.
     * @param nRegList The register descriptors.
     * @param nPrim The PRIM value, or 0 for none.
     * @ghidraAddress NTSC-U/C: 0x002121b0
     * @ghidraAddress PAL: 0x0021afc8
     */
    void OpenGifTag(int nFlg, int nRegs, unsigned long long nRegList, int nPrim);

    /**
     * Set the buffer and the channel, and empty the packet.
     *
     * @param pBase The buffer.
     * @param nChannel The DMA channel.
     * @param bVif Whether the channel is a VIF channel.
     * @ghidraAddress NTSC-U/C: 0x002122a8
     * @ghidraAddress PAL: 0x0021b0c0
     */
    void Init(DmaQuadword *pBase, int nChannel, int bVif);

    /**
     * Empty the packet.
     *
     * @ghidraAddress NTSC-U/C: 0x002122d0
     * @ghidraAddress PAL: 0x0021b0e8
     */
    void Reset();

    /**
     * Wait for the channel to stop.
     *
     * @ghidraAddress NTSC-U/C: 0x002122e8
     * @ghidraAddress PAL: 0x0021b100
     */
    void Wait();

    /**
     * Close the packet and program the channel's registers for it.
     *
     * @param eMode How the packet is ended and sent.
     * @param nAddress The main memory address of a scratchpad transfer, or 0.
     * @return The CHCR value that starts the transfer.
     * @ghidraAddress NTSC-U/C: 0x00212328
     * @ghidraAddress PAL: 0x0021b140
     */
    unsigned int Close(Mode eMode, unsigned int nAddress);

    /**
     * Close the packet and start the transfer.
     *
     * @param eMode How the packet is ended and sent.
     * @param nAddress The main memory address of a scratchpad transfer, or 0.
     * @ghidraAddress NTSC-U/C: 0x00212500
     * @ghidraAddress PAL: 0x0021b318
     */
    void Send(Mode eMode, unsigned int nAddress);

    DmaQuadword *mDmaTag;    /*!< The DMA tag being filled, or null. */
    DmaQuadword *mBase;      /*!< The first quadword of the packet. */
    DmaQuadword *mCursor;    /*!< The next quadword to write. */
    DmaQuadword *mGifTag;    /*!< The GIF tag being filled, or null. */
    DmaQuadword *mVifDirect; /*!< The VIF DIRECT code being filled, or null. */
    unsigned short mChannel; /*!< The DMA channel. */
    unsigned short mVif;     /*!< Non-zero when GIF data travels through VIF DIRECT codes. */
};
