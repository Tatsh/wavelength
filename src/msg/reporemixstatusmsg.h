#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"

/**
 * Progress or result of a transfer of a remix, NetLobby::UploadRemix() or
 * NetLobby::DownloadRemix().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class RepoRemixStatusMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039f060
     * @ghidraAddress PAL: 0x0040dd40
     */
    ~RepoRemixStatusMsg() override = default;

    /** Values of mStatus. */
    enum Status {
        kStatusFailed = -100,       /*!< The transfer failed. */
        kStatusAlreadyExists = -61, /*!< The remix is already stored. */
        kStatusDone = 0,            /*!< The transfer finished. */
        kStatusProgress = 5,        /*!< mProgress reports the part transferred. */
    };

    /**
     * Construct a message.
     *
     * Inline.
     *
     * @param nStatus One of Status, or another negative error.
     * @param fProgress The part transferred, from 0 to 1.
     */
    RepoRemixStatusMsg(int nStatus, float fProgress) : mStatus(nStatus), mProgress(fProgress) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "RepoRemixStatusMsg", 0);
    }

    /**
     * Release a message.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new RepoRemixStatusMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nRepoRemixStatusMsgType.
     */
    int Type() override {
        return g_nRepoRemixStatusMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `RepoRemixStatusMsg`.
     */
    const char *GetName() const override {
        return "RepoRemixStatusMsg";
    }

    /**
     * Write the status and the progress.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268d40
     * @ghidraAddress PAL: 0x00272948
     */
    void PrintExtra(PrnStream &stream) const override;

    int mStatus;     /*!< One of Status, or another negative error. */
    float mProgress; /*!< The part transferred, from 0 to 1, with kStatusProgress. */
};
