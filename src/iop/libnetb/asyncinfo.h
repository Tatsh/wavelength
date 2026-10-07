#pragma once

#include "libnetb/asyncblock.h"

/**
 * An asynchronous transfer on one connection, as the EE describes it to the module. A thread of
 * the module receives into or sends from the IOP copy of the buffer and mirrors the buffer and
 * this record into EE memory by SIF DMA. TCP connections transfer a byte stream. UDP and raw
 * connections transfer datagrams through a ring buffer of AsyncBlock records.
 *
 * The module was built without RTTI. The name comes from the module's state dump.
 */
class AsyncInfo {
public:
    /** Lists DumpList() can display. */
    enum List {
        kListRead = 0, /*!< Receiving transfers. */
        kListSend = 1, /*!< Sending transfers. */
    };

    /**
     * Start the thread that receives on the connection. A datagram connection gets its ring
     * buffer first.
     *
     * @return False on success, true when a datagram does not fit the buffer or no thread starts.
     * @ghidraAddress NTSC-U/C: 0x00001a8c
     * @ghidraAddress PAL: 0x00001a4c
     */
    bool StartReadThread();

    /**
     * Start the thread that sends on the connection. A datagram connection gets its ring buffer
     * first.
     *
     * @return False on success, true when a datagram does not fit the buffer or no thread starts.
     * @ghidraAddress NTSC-U/C: 0x00001b40
     * @ghidraAddress PAL: 0x00001afc
     */
    bool StartSendThread();

    /**
     * Add the transfer to the receiving list.
     *
     * @ghidraAddress NTSC-U/C: 0x00001e24
     * @ghidraAddress PAL: 0x00001dd0
     */
    void AppendRead();

    /**
     * Add the transfer to the sending list.
     *
     * @ghidraAddress NTSC-U/C: 0x00001e74
     * @ghidraAddress PAL: 0x00001e20
     */
    void AppendSend();

    /**
     * Find the receiving transfer of a connection.
     *
     * @param cid Connection identifier.
     * @return The transfer, or null.
     * @ghidraAddress NTSC-U/C: 0x00001fac
     * @ghidraAddress PAL: 0x00001f58
     */
    static AsyncInfo *FindRead(int cid);

    /**
     * Find the sending transfer of a connection.
     *
     * @param cid Connection identifier.
     * @return The transfer, or null.
     * @ghidraAddress NTSC-U/C: 0x00001fe4
     * @ghidraAddress PAL: 0x00001f90
     */
    static AsyncInfo *FindSend(int cid);

    /**
     * Stop and delete the receiving thread of a connection, then remove and free its transfer.
     *
     * @param cid Connection identifier.
     * @param timeout Microseconds to wait for the thread to become dormant, polling every 100.
     * @return True when the transfer was freed.
     * @ghidraAddress NTSC-U/C: 0x00001bf4
     * @ghidraAddress PAL: 0x00001bac
     */
    static bool StopReadThread(int cid, int timeout);

    /**
     * Stop and delete the sending thread of a connection, then remove and free its transfer.
     *
     * @param cid Connection identifier.
     * @param timeout Microseconds to wait for the thread to fall asleep, polling every 100.
     * @return True when the transfer was freed.
     * @ghidraAddress NTSC-U/C: 0x00001cf8
     * @ghidraAddress PAL: 0x00001cac
     */
    static bool StopSendThread(int cid, int timeout);

    /**
     * Print the transfers of one list and the state of their threads.
     *
     * @param list A List.
     * @ghidraAddress NTSC-U/C: 0x0000201c
     * @ghidraAddress PAL: 0x00001fc8
     */
    static void DumpList(int list);

    /**
     * Report whether data is waiting.
     *
     * @param blocks True for a datagram connection.
     * @param sending For a datagram connection, true to test the block at the write offset, false
     * to search for a complete block.
     * @param available Set when data is waiting.
     * @return Zero, or the error of the search.
     * @ghidraAddress NTSC-U/C: 0x000035b0
     * @ghidraAddress PAL: 0x00003420
     */
    int PollBlocks(bool blocks, bool sending, bool *available);

    /** First transfer of the receiving list. */
    static AsyncInfo *sReadList;

    /** First transfer of the sending list. */
    static AsyncInfo *sSendList;

    int mDataWaitingSize;            /*!< Bytes waiting, or a negative error. */
    int mLastReturnedFlag;           /*!< Flags of the last transfer. */
    int mMaxPacketSize;              /*!< Largest datagram. */
    unsigned int mReserved;          /*!< Never touched by the module. */
    int mCid;                        /*!< Connection identifier. */
    int mInetFlag;                   /*!< Flags passed to each transfer. */
    unsigned int mBufferSize;        /*!< Size of the buffer. */
    unsigned char *mBufferStart;     /*!< The buffer. */
    AsyncInfo *mOtherProcessorsInfo; /*!< The EE copy of this record. */
    AsyncInfo *mNext;                /*!< Next transfer of the list. */
    int mThid;                       /*!< Transfer thread. */
    unsigned char *mOtherWriteLoc;   /*!< The EE copy of the buffer. */
    int mClose;                      /*!< 1 when the thread must stop. */
    int mType;                       /*!< A #sceInetType. */
    int mRemotePort;                 /*!< Remote port. */
    unsigned int mWriteOffset;       /*!< Offset of the current block of the ring buffer. */

private:
    /**
     * Thread entry that receives a TCP stream into the buffer and mirrors it to the EE.
     *
     * @param transfer The AsyncInfo.
     * @ghidraAddress NTSC-U/C: 0x000012e4
     * @ghidraAddress PAL: 0x000012c4
     */
    static void TcpReceiveThread(void *transfer);

    /**
     * Thread entry that receives datagrams into the ring buffer.
     *
     * @param transfer The AsyncInfo.
     * @ghidraAddress NTSC-U/C: 0x0000165c
     * @ghidraAddress PAL: 0x00001624
     */
    static void BlockReceiveThread(void *transfer);

    /**
     * Thread entry that sends the waiting bytes of the buffer on a TCP connection.
     *
     * @param transfer The AsyncInfo.
     * @ghidraAddress NTSC-U/C: 0x00001738
     * @ghidraAddress PAL: 0x00001700
     */
    static void TcpSendThread(void *transfer);

    /**
     * Thread entry that sends the complete blocks of the ring buffer.
     *
     * @param transfer The AsyncInfo.
     * @ghidraAddress NTSC-U/C: 0x000018fc
     * @ghidraAddress PAL: 0x000018bc
     */
    static void BlockSendThread(void *transfer);

    /**
     * Remove the transfer from the receiving list.
     *
     * @ghidraAddress NTSC-U/C: 0x00001ec4
     * @ghidraAddress PAL: 0x00001e70
     */
    void RemoveRead();

    /**
     * Remove the transfer from the sending list.
     *
     * @ghidraAddress NTSC-U/C: 0x00001f38
     * @ghidraAddress PAL: 0x00001ee4
     */
    void RemoveSend();

    /**
     * Send the datagram of the block at the write offset and release the block.
     *
     * @param kind 0 for UDP, 1 for raw IP.
     * @param name Protocol name for messages.
     * @return Zero, 2 for a bad argument, or the error of a send or a DMA transfer.
     * @ghidraAddress NTSC-U/C: 0x000028bc
     * @ghidraAddress PAL: 0x00002774
     */
    int SendBlock(int kind, const char *name);

    /**
     * Receive one datagram into the block at the write offset.
     *
     * @param kind 0 for UDP, 1 for raw IP.
     * @param name Protocol name for messages.
     * @return Zero, 2 for a bad argument, or the error of a receive or a DMA transfer.
     * @ghidraAddress NTSC-U/C: 0x00002af8
     * @ghidraAddress PAL: 0x000029b0
     */
    int ReceiveBlock(int kind, const char *name);

    /**
     * Make the block at the write offset large enough for a datagram, splitting off the rest of
     * the free space and skipping the end of the ring buffer when it is too small.
     *
     * @param size Datagram size.
     * @param reserved Receives the block size, header included.
     * @param ready Set when the block is ready.
     * @return Zero, 1 when the write offset is past the buffer, 2 for a null argument, or the error
     * of a DMA transfer.
     * @ghidraAddress NTSC-U/C: 0x0000372c
     * @ghidraAddress PAL: 0x0000359c
     */
    int ReserveBlock(unsigned int size, unsigned int *reserved, bool *ready);

    /**
     * Find the next complete block from the write offset, skipping padding blocks.
     *
     * @param block Receives the block, or null.
     * @param found Set when a block was found.
     * @return Zero, or 2 for a null argument.
     * @ghidraAddress NTSC-U/C: 0x00003968
     * @ghidraAddress PAL: 0x000037d8
     */
    int FindFilledBlock(AsyncBlock **block, bool *found);

    /**
     * Copy a record to EE memory, printing a message when the DMA queue is full.
     *
     * @param data Source.
     * @param destination Destination in EE memory.
     * @param size Byte count.
     * @param message Message for a full DMA queue.
     */
    static inline void CopyToEe(const void *data, void *destination, int size, const char *message);

    /**
     * Start a transfer thread, setting up the ring buffer of a datagram connection.
     *
     * @param name Thread name.
     * @param blockEntry Entry for a datagram connection.
     * @param streamEntry Entry for a TCP connection.
     * @param priority Thread priority.
     * @return False on success.
     */
    inline bool StartThread(const char *name,
                            void (*blockEntry)(void *),
                            void (*streamEntry)(void *),
                            int priority);
};
