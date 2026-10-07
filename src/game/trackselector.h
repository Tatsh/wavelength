#pragma once

#include <vector>

/**
 * Grid of which players occupy which track, in the order the players arrived on each track.
 *
 * The RTTI includes the nested TrackSelector::TrackInfo. The class is not polymorphic. Only the
 * members GameTrackSelector uses are declared.
 */
class TrackSelector {
public:
    /**
     * The players on one track, in the order they arrived.
     *
     * The RTTI includes the class name.
     */
    struct TrackInfo {
        std::vector<int> mPlayers; /*!< The players, or -1 for an empty slot. */
    };

    /**
     * Construct a grid of tracks with no players.
     *
     * @param nTracks The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x0013ea10
     * @ghidraAddress PAL: 0x001402e0
     */
    explicit TrackSelector(int nTracks);

    /**
     * Add a player on a track.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x0013eb08
     * @ghidraAddress PAL: 0x001403d8
     */
    void AddPlayer(int nTrack);

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
     * Move a player to the end of a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track, or -1 to remove the player from every track.
     * @ghidraAddress NTSC-U/C: 0x0013ee38
     * @ghidraAddress PAL: 0x00140708
     */
    void SetPlayerTrack(int nPlayer, int nTrack);

    /**
     * Swap a player with the player in a slot of a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nSlot The slot.
     * @ghidraAddress NTSC-U/C: 0x0013eed0
     * @ghidraAddress PAL: 0x001407a0
     */
    void SwapPlayer(int nPlayer, int nTrack, int nSlot);

    /**
     * Remove the empty slots of every track after a swap.
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
     * Report the first player on a track.
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
     * @return The players, in the order they arrived.
     * @ghidraAddress NTSC-U/C: 0x0013f0e0
     * @ghidraAddress PAL: 0x001409b0
     */
    const int *GetPlayersOnTrack(int nTrack) const;

    std::vector<int> mPlayerTracks;     /*!< The track of each player, or -1. */
    std::vector<TrackInfo> mTrackInfos; /*!< The players on each track. */
    int mNeedsCompact;                  /*!< Set by SwapPlayer() and cleared by Compact(). */
};
