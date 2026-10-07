#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Progress or result of a transfer of a remix, NetLobby::UploadRemix() or
 * NetLobby::DownloadRemix().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class RepoRemixStatusMsg : public Message {
public:
    /** Values of mStatus. */
    enum Status {
        kStatusAlreadyExists = -61, /*!< The remix is already stored. */
        kStatusDone = 0,            /*!< The transfer finished. */
        kStatusProgress = 5,        /*!< mProgress reports the part transferred. */
    };

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override;

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

    int mStatus;     /*!< One of Status, or another negative error. */
    float mProgress; /*!< The part transferred, from 0 to 1, with kStatusProgress. */
};
