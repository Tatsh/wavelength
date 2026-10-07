#pragma once

#include <vector>

#include "game/freestyletrack.h"
#include "game/player.h"
#include "game/track.h"
#include "game/trackselector.h"

/**
 * Assignment of the players of a game to its tracks, with the rotation between the tracks.
 *
 * The RTTI includes the class name through a member command. The class is not polymorphic. The
 * catch tracks come first in mTracks, and the freestyle track, when the song has one, comes last.
 * The first player on each track is the one the track plays for. Every change is shown through
 * TheGfxManager, and the controllers of the first player on a track pulse on the beat.
 */
class GameTrackSelector {
public:
    /**
     * Construct the assignment over the tracks of a song, with no players.
     *
     * @param tracks The catch tracks.
     * @param pFreestyleTrack The freestyle track, or null.
     * @param nFreestyleType The track type of the freestyle track.
     * @param nFreestyleInstrument The instrument of the freestyle track.
     * @ghidraAddress NTSC-U/C: 0x00115bb8
     * @ghidraAddress PAL: 0x00117350
     */
    GameTrackSelector(const std::vector<Track *> &tracks,
                      FreestyleTrack *pFreestyleTrack,
                      int nFreestyleType,
                      int nFreestyleInstrument);

    /**
     * Report whether a track is the freestyle track.
     *
     * @param nTrack The track.
     * @return Whether the song has a freestyle track and the track is the last one.
     * @ghidraAddress NTSC-U/C: 0x00116008
     * @ghidraAddress PAL: 0x001177a0
     */
    bool IsFreestyleTrack(int nTrack) const;

    /**
     * Add a player on a track.
     *
     * @param pPlayer The player.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x00116040
     * @ghidraAddress PAL: 0x001177d8
     */
    void AddPlayer(Player *pPlayer, int nTrack);

    /**
     * Report the number of players.
     *
     * @return The number of players.
     * @ghidraAddress NTSC-U/C: 0x00116228
     * @ghidraAddress PAL: 0x001179c0
     */
    int GetNumPlayers() const;

    /**
     * Report the number of tracks, the freestyle track included.
     *
     * @return The number of tracks.
     * @ghidraAddress NTSC-U/C: 0x00116248
     * @ghidraAddress PAL: 0x001179e0
     */
    int GetNumTracks() const;

    /**
     * Move a player to the freestyle track.
     *
     * @param nPlayer The player.
     * @param bVictory Whether the freestyle rewards a won song.
     * @ghidraAddress NTSC-U/C: 0x00116268
     * @ghidraAddress PAL: 0x00117a00
     */
    void EnterFreestyle(int nPlayer, bool bVictory);

    /**
     * Move a player to a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001162a0
     * @ghidraAddress PAL: 0x00117a38
     */
    void MovePlayer(int nPlayer, int nTrack);

    /**
     * Move a player to a track or off every track, and give each track its first player.
     *
     * @param nPlayer The player.
     * @param nTrack The track, or -1 to remove the player from every track.
     * @param bVictory Whether a move to the freestyle track rewards a won song.
     * @ghidraAddress NTSC-U/C: 0x001162c0
     * @ghidraAddress PAL: 0x00117a58
     */
    void SetPlayerTrack(int nPlayer, int nTrack, bool bVictory);

    /**
     * Move a player to the previous track, unless the player is on the first one.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00116510
     * @ghidraAddress PAL: 0x00117ca8
     */
    void RotatePrevious(int nPlayer);

    /**
     * Move a player to the next track, unless that is the last track.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00116558
     * @ghidraAddress PAL: 0x00117cf0
     */
    void RotateNext(int nPlayer);

    /**
     * Report the track of a player.
     *
     * @param nPlayer The player.
     * @return The track, or -1.
     * @ghidraAddress NTSC-U/C: 0x001165b8
     * @ghidraAddress PAL: 0x00117d50
     */
    int GetPlayerTrack(int nPlayer) const;

    /**
     * Report the slot of a player on its track.
     *
     * @param nPlayer The player.
     * @return The slot, 0 for the first player on the track.
     * @ghidraAddress NTSC-U/C: 0x001165d8
     * @ghidraAddress PAL: 0x00117d70
     */
    int GetPlayerSlot(int nPlayer) const;

    /**
     * Swap a player with the player in a slot of a track.
     *
     * Resync() applies the swap to the tracks.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nSlot The slot.
     * @ghidraAddress NTSC-U/C: 0x001165f8
     * @ghidraAddress PAL: 0x00117d90
     */
    void SwapPlayer(int nPlayer, int nTrack, int nSlot);

    /**
     * Give every player its track and every track its first player after a swap.
     *
     * @ghidraAddress NTSC-U/C: 0x00116618
     * @ghidraAddress PAL: 0x00117db0
     */
    void Resync();

    /**
     * Remove a player from every track and hide its displays.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00116868
     * @ghidraAddress PAL: 0x00118000
     */
    void RemovePlayer(int nPlayer);

    /**
     * Show the players on a track.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001168e8
     * @ghidraAddress PAL: 0x00118080
     */
    void RefreshTrack(int nTrack);

    TrackSelector mSelector;       /*!< The grid of the players on the tracks. */
    std::vector<Player *> mPlayers; /*!< Every player, by index. */
    std::vector<Track *> mTracks;  /*!< The catch tracks, then the freestyle track. */
    int mFreestyleType;            /*!< The track type of the freestyle track. */
    int mFreestyleInstrument;      /*!< The instrument of the freestyle track. */
    bool mHasFreestyle;            /*!< Whether the song has a freestyle track. */
};
