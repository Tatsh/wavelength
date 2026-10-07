#pragma once

#include <vector>

#include "game/gametrackselector.h"
#include "game/trackselector.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Referee of an online session that settles which track each player is on.
 *
 * The RTTI records the commands the class schedules. The class is not polymorphic. The hosting
 * console builds one. Every request of a player to change track is settled after a delay, a
 * player that the change displaces is bumped to a random free track, and the assignment is sent
 * to every console in an ArbiterPacket every 200 milliseconds and after every change.
 */
class NetArbiter {
public:
    /**
     * Take over the assignment of a game's track selector.
     *
     * @param pGameTrackSelector The track selector of the game.
     * @ghidraAddress NTSC-U/C: 0x00127aa0
     * @ghidraAddress PAL: 0x00129298
     */
    explicit NetArbiter(GameTrackSelector *pGameTrackSelector);

    /**
     * Send the first assignment.
     *
     * @ghidraAddress NTSC-U/C: 0x00127c60
     * @ghidraAddress PAL: 0x00129458
     */
    void Start();

    /**
     * Schedule the settling of a player's request to move to a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x00127c80
     * @ghidraAddress PAL: 0x00129478
     */
    void RequestTrack(int nPlayer, int nTrack);

    /**
     * Settle a player's move to a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nVersion The sequence number of the player's update, or -1.
     * @ghidraAddress NTSC-U/C: 0x00127d00
     * @ghidraAddress PAL: 0x001294f8
     */
    void UpdatePlayer(int nPlayer, int nTrack, int nVersion);

    /**
     * Send the assignment to every console and schedule the next send.
     *
     * @ghidraAddress NTSC-U/C: 0x00127f30
     */
    void Broadcast();

    /**
     * Take a player off every track.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x00128310
     * @ghidraAddress PAL: 0x00129b08
     */
    void RemovePlayer(int nPlayer);

    /**
     * Report the track the arbiter assigns a player.
     *
     * @param nPlayer The player.
     * @return The track, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x00128330
     */
    int GetTrack(int nPlayer);

    /**
     * Sequence number of the last ArbiterPacket sent.
     *
     * @ghidraAddress NTSC-U/C: 0x003af840
     */
    static int sVersion;

private:
    GameTrackSelector *mGameTrackSelector; /*!< The track selector of the game. */
    TrackSelector mTrackSelector;          /*!< The assignment the arbiter settles. */
    int mLocalPlayer;                      /*!< The player on this console, or -1. */
    Ptr<Command> mBroadcastCommand;        /*!< The command that calls Broadcast(). */
    std::vector<int> mVersions;            /*!< The last update sequence number of each player. */
    float mDelayMs;                        /*!< The delay before a request is settled. */
};
