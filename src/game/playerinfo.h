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

    /** @return Non-zero for a player of this console. */
    int GetLocal() const {
        return mLocal;
    }

    /**
     * Set whether the player is on this console.
     *
     * @param nLocal Non-zero for a player of this console.
     */
    void SetLocal(int nLocal) {
        mLocal = nLocal;
    }

    /** @return The slot. It selects the player's sounds and colours. */
    int GetSlot() const {
        return mSlot;
    }

    /**
     * Set the slot of the player.
     *
     * @param nSlot The slot.
     */
    void SetSlot(int nSlot) {
        mSlot = nSlot;
    }

    /** @return The score GameDb::SetPlayerScore() recorded. */
    int GetScore() const {
        return mScore;
    }

    /**
     * Set the score of the player.
     *
     * @param nScore The score.
     */
    void SetScore(int nScore) {
        mScore = nScore;
    }

    /** @return The rank of the score among the players, 0 for the best, or -1. */
    int GetRank() const {
        return mRank;
    }

    /**
     * Set the rank of the player.
     *
     * @param nRank The rank.
     */
    void SetRank(int nRank) {
        mRank = nRank;
    }

    /** @return The copy of the profile. */
    Campaign &GetProfile() {
        return mProfile;
    }

    /** @return The copy of the profile. */
    const Campaign &GetProfile() const {
        return mProfile;
    }

    /**
     * Replace the copy of the profile.
     *
     * @param profile The profile to copy.
     */
    void SetProfile(const Campaign &profile) {
        mProfile = profile;
    }

    /** @return The position in the order of the online session, or -1. */
    int GetNetOrder() const {
        return mNetOrder;
    }

    /**
     * Set the position in the order of the online session.
     *
     * @param nNetOrder The position, or -1.
     */
    void SetNetOrder(int nNetOrder) {
        mNetOrder = nNetOrder;
    }

private:
    int mLocal;        /*!< Non-zero for a player of this console. */
    int mSlot;         /*!< The slot, which selects the player's sounds and colours. */
    int mScore;        /*!< The score GameDb::SetPlayerScore() recorded. */
    int mRank;         /*!< The rank of mScore among the players, 0 for the best, or -1. */
    Campaign mProfile; /*!< The copy of the profile. */
    int mNetOrder;     /*!< The position in the order of the online session, or -1. */
};
