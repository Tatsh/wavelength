#pragma once

#include "game/player.h"
#include "game/track.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Player on this console, controlled through one controller port.
 *
 * The RTTI records the class as deriving from Player. In an online session every change to the
 * track, the score, the catching state, or the repeat state is sent to the other consoles through
 * Broadcast(), which also repeats itself every 500 milliseconds.
 */
class LocalPlayer : public Player {
public:
    /**
     * Construct a player for a controller port.
     *
     * @param nIndex The player's index.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nPadNum The controller port.
     * @ghidraAddress NTSC-U/C: 0x001225e0
     * @ghidraAddress PAL: 0x00123d60
     */
    LocalPlayer(int nIndex, int nTicksPerBar, int nPadNum);

    /**
     * Release the broadcast command.
     *
     * @ghidraAddress NTSC-U/C: 0x00122650
     * @ghidraAddress PAL: 0x00123dd0
     */
    ~LocalPlayer() override;

    /**
     * Move to a track, move this player's count in TheMixer from the old track to the new one,
     * and broadcast.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001226a0
     * @ghidraAddress PAL: 0x00123e20
     */
    void SetTrack(Track *pTrack) override;

    /**
     * Set the score, show it, and broadcast.
     *
     * @param nScore The score.
     * @ghidraAddress NTSC-U/C: 0x00122710
     * @ghidraAddress PAL: 0x00123e90
     */
    void SetScore(int nScore) override;

    /**
     * Add points to the score, show the new score, and broadcast.
     *
     * @param nPoints The points.
     * @ghidraAddress NTSC-U/C: 0x00122740
     * @ghidraAddress PAL: 0x00123ec0
     */
    void AddScore(int nPoints) override;

    /**
     * Start the multiplier power-up and, in an online session, announce it with a
     * MultiplierPacket.
     *
     * @ghidraAddress NTSC-U/C: 0x001227e0
     * @ghidraAddress PAL: 0x00123f60
     */
    void ActivateMultiplier() override;

    /**
     * Set whether the player is catching, and broadcast when it changes.
     *
     * @param bCatching Whether the player is catching.
     * @ghidraAddress NTSC-U/C: 0x001227a0
     * @ghidraAddress PAL: 0x00123f20
     */
    void SetCatching(bool bCatching) override;

    /**
     * Set the remix repeat state and broadcast.
     *
     * @param bRepeat The state.
     * @ghidraAddress NTSC-U/C: 0x00122868
     * @ghidraAddress PAL: 0x00123fe8
     */
    void SetRepeat(bool bRepeat) override;

    /**
     * Report the controller port of the player.
     *
     * @return mPadNum.
     * @ghidraAddress NTSC-U/C: 0x0033ac40
     * @ghidraAddress PAL: 0x003a8178
     */
    int GetPadNum() const override {
        return mPadNum;
    }

    /**
     * Add the pending points to the score, hide the pending points, and broadcast.
     *
     * @ghidraAddress NTSC-U/C: 0x00122770
     * @ghidraAddress PAL: 0x00123ef0
     */
    virtual void Capture();

    /**
     * Send this player's state to the other consoles and schedule the next broadcast.
     *
     * Only an online session broadcasts. The game and duel rule sets send a PlayerUpdatePacket, the
     * remix rule set sends a RemixerUpdatePacket, and any other rule set sends nothing. A broadcast
     * already scheduled is withdrawn first, and the next one is scheduled 500 milliseconds later.
     *
     * @ghidraAddress NTSC-U/C: 0x00122898
     * @ghidraAddress PAL: 0x00124018
     */
    void Broadcast();

    /**
     * Sequence number of the last PlayerUpdatePacket sent.
     *
     * @ghidraAddress NTSC-U/C: 0x003af764
     */
    static int sPlayerUpdateVersion;

    /**
     * Sequence number of the last RemixerUpdatePacket sent.
     *
     * @ghidraAddress NTSC-U/C: 0x003af768
     */
    static int sRemixerUpdateVersion;

private:
    int mPadNum;                    /*!< The controller port. */
    Ptr<Command> mBroadcastCommand; /*!< The command that calls Broadcast(). */
};
