#pragma once

#include <vector>

/**
 * Assignment of the players to the tracks and to the slots of each track, in the order the players
 * arrived on each track.
 *
 * The RTTI includes the nested TrackSelector::TrackInfo. The class is not polymorphic.
 */
class TrackSelector {
public:
    /**
     * The players on one track, by slot.
     *
     * The RTTI includes the class name. An empty slot stores -1, and the occupied slots precede the
     * empty ones except after TrackSelector::SwapPlayer().
     */
    class TrackInfo {
    public:
        /** The number of slots of a track. */
        static constexpr int kNumSlots = 4;

        /**
         * Construct a track with every slot empty.
         *
         * @ghidraAddress NTSC-U/C: 0x0013e718
         * @ghidraAddress PAL: 0x0013ffe8
         */
        TrackInfo();

        /**
         * Report the slot of a player.
         *
         * @param nPlayer The player.
         * @return The slot, or the number of slots when the player is not on the track.
         * @ghidraAddress NTSC-U/C: 0x0013e7c0
         * @ghidraAddress PAL: 0x00140090
         */
        int Find(int nPlayer) const;

        /**
         * Report the number of occupied slots.
         *
         * @return The number of slots that do not store -1.
         * @ghidraAddress NTSC-U/C: 0x0013e800
         * @ghidraAddress PAL: 0x001400d0
         */
        int Count() const;

        /**
         * Put a player in the slot after the occupied ones.
         *
         * @param nPlayer The player.
         * @ghidraAddress NTSC-U/C: 0x0013e850
         * @ghidraAddress PAL: 0x00140120
         */
        void Add(int nPlayer);

        /**
         * Move each player into the first slot of the run of empty slots before it.
         *
         * @ghidraAddress NTSC-U/C: 0x0013e890
         * @ghidraAddress PAL: 0x00140160
         */
        void Compact();

        /**
         * Take a player out of its slot and move the later players one slot down.
         *
         * @param nPlayer The player.
         * @ghidraAddress NTSC-U/C: 0x0013e978
         * @ghidraAddress PAL: 0x00140248
         */
        void Remove(int nPlayer);

        std::vector<int> mPlayers; /*!< The player in each slot, or -1. */
    };

    /**
     * Construct an assignment of no players to a number of tracks.
     *
     * @param nTracks The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0013ea10
     * @ghidraAddress PAL: 0x001402e0
     */
    explicit TrackSelector(int nTracks);

    /**
     * Add the next player to the first empty slot of a track.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x0013eb08
     * @ghidraAddress PAL: 0x001403d8
     */
    void AddPlayer(int nTrack);

    /**
     * Add the next player to a slot of a track.
     *
     * @param nTrack The track.
     * @param nSlot The slot.
     * @ghidraAddress NTSC-U/C: 0x0013ec78
     * @ghidraAddress PAL: 0x00140548
     */
    void AddPlayer(int nTrack, int nSlot);

    /**
     * Report the number of players.
     *
     * @return The number of players.
     * @ghidraAddress NTSC-U/C: 0x0013ee08
     * @ghidraAddress PAL: 0x001406d8
     */
    int GetNumPlayers() const;

    /**
     * Report the number of tracks.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0013ee20
     * @ghidraAddress PAL: 0x001406f0
     */
    int GetNumTracks() const;

    /**
     * Move a player to the first empty slot of a track, or off every track.
     *
     * @param nPlayer The player.
     * @param nTrack The track, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x0013ee38
     * @ghidraAddress PAL: 0x00140708
     */
    void SetPlayerTrack(int nPlayer, int nTrack);

    /**
     * Swap a player with the player in a slot of a track.
     *
     * The other player, when there is one, takes the slot the player left. Compact() removes the
     * gaps the swap can leave.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nSlot The slot.
     * @ghidraAddress NTSC-U/C: 0x0013eed0
     * @ghidraAddress PAL: 0x001407a0
     */
    void SwapPlayer(int nPlayer, int nTrack, int nSlot);

    /**
     * Compact every track after a swap.
     *
     * @ghidraAddress NTSC-U/C: 0x0013efc0
     * @ghidraAddress PAL: 0x00140890
     */
    void Compact();

    /**
     * Report the track of a player.
     *
     * @param nPlayer The player.
     * @return The track, or -1.
     * @ghidraAddress NTSC-U/C: 0x0013f050
     * @ghidraAddress PAL: 0x00140920
     */
    int GetPlayerTrack(int nPlayer) const;

    /**
     * Report the slot of a player on its track.
     *
     * @param nPlayer The player.
     * @return The slot, 0 for the first player on the track.
     * @ghidraAddress NTSC-U/C: 0x0013f068
     * @ghidraAddress PAL: 0x00140938
     */
    int GetPlayerSlot(int nPlayer) const;

    /**
     * Report the player in the first slot of a track.
     *
     * @param nTrack The track.
     * @return The player, or -1.
     * @ghidraAddress NTSC-U/C: 0x0013f0a0
     * @ghidraAddress PAL: 0x00140970
     */
    int GetFirstPlayer(int nTrack) const;

    /**
     * Report the number of players on a track.
     *
     * @param nTrack The track.
     * @return The number of players.
     * @ghidraAddress NTSC-U/C: 0x0013f0b8
     * @ghidraAddress PAL: 0x00140988
     */
    int GetNumPlayersOnTrack(int nTrack) const;

    /**
     * Report the players on a track.
     *
     * @param nTrack The track.
     * @return The player in each slot, or -1.
     * @ghidraAddress NTSC-U/C: 0x0013f0e0
     * @ghidraAddress PAL: 0x001409b0
     */
    const int *GetPlayersOnTrack(int nTrack) const;

    std::vector<int> mPlayerTracks;     /*!< The track of each player, or -1. */
    std::vector<TrackInfo> mTrackInfos; /*!< The players on each track. */
    int mNeedsCompact;                  /*!< Set by SwapPlayer() and cleared by Compact(). */
};
