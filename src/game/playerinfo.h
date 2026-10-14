#pragma once

#include "game/campaign.h"

/**
 * One player of the game GameDb sets up, with a copy of the player's profile.
 *
 * The RTTI includes the name. The record is 0x10c bytes, and GameDb allocates each one on the heap.
 * The constructor has no out-of-line copy.
 */
class PlayerInfo {
public:
    /**
     * Construct a player with no score and no rank.
     *
     * @param nLocal Non-zero for a player of this console.
     * @param nSlot The slot of the player.
     * @param profile The profile to copy.
     * @param nNetOrder The position of the player in the order of the online session.
     */
    PlayerInfo(int nLocal, int nSlot, const Campaign &profile, int nNetOrder)
        : mLocal(nLocal), mSlot(nSlot), mScore(0), mRank(-1), mProfile(profile),
          mNetOrder(nNetOrder) {
    }

    int mLocal;        /*!< Non-zero for a player of this console. */
    int mSlot;         /*!< The slot, which selects the player's sounds and colours. */
    int mScore;        /*!< The score GameDb::SetPlayerScore() recorded. */
    int mRank;         /*!< The rank of mScore among the players, 0 for the best, or -1. */
    Campaign mProfile; /*!< The copy of the profile. */
    int mNetOrder;     /*!< The position in the order of the online session, or -1. */
};
