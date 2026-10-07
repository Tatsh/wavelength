#pragma once

#include "game/inputevents.h"
#include "game/track.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * One participant in a song: the track the participant plays, the score, and the streak.
 *
 * The RTTI includes the class name. The class has no base. LocalPlayer derives from it for a
 * player on this console. A player's points accumulate as pending points while a phrase is
 * played, and CommitPendingPoints() adds them to the score scaled by the streak multiplier and the
 * multiplier power-up, or LosePendingPoints() forfeits them. Every change is shown through
 * TheGfxManager.
 */
class Player {
public:
    /**
     * Construct a player with no track, no score, no streak, and no power-up.
     *
     * @param nIndex The player's index.
     * @param nTicksPerBar The length of a bar in ticks, the unit of the multiplier power-up.
     * @ghidraAddress NTSC-U/C: 0x0012d9b0
     * @ghidraAddress PAL: 0x0012f188
     */
    Player(int nIndex, int nTicksPerBar);

    /**
     * Cancel the multiplier power-up's end and release its command.
     *
     * @ghidraAddress NTSC-U/C: 0x0012da40
     * @ghidraAddress PAL: 0x0012f218
     */
    virtual ~Player();

    /**
     * Move to a track and log the move.
     *
     * @param pTrack The track.
     * @ghidraAddress NTSC-U/C: 0x0012daa0
     * @ghidraAddress PAL: 0x0012f278
     */
    virtual void SetTrack(Track *pTrack);

    /**
     * Hide the player's power-up icon.
     *
     * mPowerup is unchanged.
     *
     * @ghidraAddress NTSC-U/C: 0x0012dae8
     * @ghidraAddress PAL: 0x0012f2c0
     */
    virtual void HidePowerup();

    /**
     * Withdraw the scheduled end of the multiplier power-up.
     *
     * @ghidraAddress NTSC-U/C: 0x0012db10
     * @ghidraAddress PAL: 0x0012f2e8
     */
    virtual void CancelMultiplier();

    /**
     * Set the score and show it.
     *
     * @param nScore The score.
     * @ghidraAddress NTSC-U/C: 0x0012dc70
     * @ghidraAddress PAL: 0x0012f448
     */
    virtual void SetScore(int nScore);

    /**
     * Add points to the score and show the new score.
     *
     * @param nPoints The points.
     * @ghidraAddress NTSC-U/C: 0x0012dcb0
     * @ghidraAddress PAL: 0x0012f488
     */
    virtual void AddScore(int nPoints);

    /**
     * Add the pending points to the score and clear them.
     *
     * The points added are the pending points times the multiplier power-up value times the streak
     * multiplier. A capture worth no points shows the pending points as cleared rather than
     * captured.
     *
     * @param bHide Whether the pending points display is hidden after a capture worth points.
     * @ghidraAddress NTSC-U/C: 0x0012dd80
     * @ghidraAddress PAL: 0x0012f558
     */
    virtual void CommitPendingPoints(bool bHide);

    /**
     * Forfeit the pending points.
     *
     * @ghidraAddress NTSC-U/C: 0x0012de40
     * @ghidraAddress PAL: 0x0012f618
     */
    virtual void LosePendingPoints();

    /**
     * Start the multiplier power-up and schedule its end.
     *
     * The power-up lasts GameConfig::mMultiplierDurationBars bars from the current tick. A power-up
     * already running is restarted.
     *
     * @ghidraAddress NTSC-U/C: 0x0012de78
     * @ghidraAddress PAL: 0x0012f650
     */
    virtual void ActivateMultiplier();

    /**
     * Set whether the player is catching.
     *
     * @param bCatching Whether the player is catching.
     * @ghidraAddress NTSC-U/C: 0x0012e0c8
     * @ghidraAddress PAL: 0x0012f8a0
     */
    virtual void SetCatching(bool bCatching);

    /**
     * Set the remix repeat state.
     *
     * @param bRepeat The state.
     * @ghidraAddress NTSC-U/C: 0x0012e150
     * @ghidraAddress PAL: 0x0012f928
     */
    virtual void SetRepeat(bool bRepeat);

    /**
     * Report the controller port of the player.
     *
     * @return -1, because a player of this class has no controller.
     * @ghidraAddress NTSC-U/C: 0x0033dbd0
     * @ghidraAddress PAL: 0x003ab108
     */
    virtual int GetPadNum() const {
        return -1;
    }

    /**
     * Report the player's index.
     *
     * @return The index.
     */
    int GetIndex() const {
        return mIndex;
    }

    /**
     * Report the score.
     *
     * @return The score.
     */
    int GetScore() const {
        return mScore;
    }

    /**
     * Report the player's track.
     *
     * @return The track, or null.
     * @ghidraAddress NTSC-U/C: 0x0012dae0
     * @ghidraAddress PAL: 0x0012f2b8
     */
    Track *GetTrack() const;

    /**
     * Hand a note event to the player's track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0012db48
     * @ghidraAddress PAL: 0x0012f320
     */
    void HandleInput(const PlayNoteEvent &event);

    /**
     * Hand a button event to the player's track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0012db80
     * @ghidraAddress PAL: 0x0012f358
     */
    void HandleInput(const BtnEvent<10> &event);

    /**
     * Hand a button event to the player's track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0012dbb8
     * @ghidraAddress PAL: 0x0012f390
     */
    void HandleInput(const BtnEvent<8> &event);

    /**
     * Hand a stick event to the player's track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0012dbf0
     * @ghidraAddress PAL: 0x0012f3c8
     */
    void HandleInput(const StickEvent<2> &event);

    /**
     * Hand a stick event to the player's track.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0012dc28
     * @ghidraAddress PAL: 0x0012f400
     */
    void HandleInput(const StickEvent<6> &event);

    /**
     * Mark the player as having quit the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0012dc60
     * @ghidraAddress PAL: 0x0012f438
     */
    void Abort();

    /**
     * Set the pending points and show them scaled by the multiplier power-up value.
     *
     * @param nPoints The points.
     * @param bHide Whether a display of zero points is hidden as well as cleared.
     * @ghidraAddress NTSC-U/C: 0x0012dcf0
     * @ghidraAddress PAL: 0x0012f4c8
     */
    void SetPendingPoints(int nPoints, bool bHide);

    /**
     * End the multiplier power-up.
     *
     * The command that ActivateMultiplier() schedules calls this.
     *
     * @ghidraAddress NTSC-U/C: 0x0012df80
     * @ghidraAddress PAL: 0x0012f758
     */
    void EndMultiplier();

    /**
     * Set the streak and show the streak multipliers it earns.
     *
     * @param nStreak The streak.
     * @ghidraAddress NTSC-U/C: 0x0012e008
     * @ghidraAddress PAL: 0x0012f7e0
     */
    void SetStreak(int nStreak);

    /**
     * Lengthen the streak by one.
     *
     * @ghidraAddress NTSC-U/C: 0x0012e080
     * @ghidraAddress PAL: 0x0012f858
     */
    void IncrementStreak();

    /**
     * Reset the streak to zero.
     *
     * @ghidraAddress NTSC-U/C: 0x0012e0a0
     * @ghidraAddress PAL: 0x0012f878
     */
    void ResetStreak();

    /**
     * Report the streak.
     *
     * @return The streak.
     * @ghidraAddress NTSC-U/C: 0x0012e0c0
     * @ghidraAddress PAL: 0x0012f898
     */
    int GetStreak() const;

    /**
     * Report whether the player is catching.
     *
     * @return Whether the player is catching.
     * @ghidraAddress NTSC-U/C: 0x0012e0d0
     * @ghidraAddress PAL: 0x0012f8a8
     */
    bool GetCatching() const;

    /**
     * Report the kind of power-up the player has.
     *
     * GameLogic::OnDeployPowerup() selects the deployment by the value.
     *
     * @return One of GameLogic::Powerup, or GameLogic::kPowerupNone.
     * @ghidraAddress NTSC-U/C: 0x0012e0d8
     * @ghidraAddress PAL: 0x0012f8b0
     */
    int GetPowerup() const;

    /**
     * Record the kind of power-up the player has and show it.
     *
     * A power-up gained is reported to TheGameCallback when one is installed.
     *
     * @param nPowerup One of GameLogic::Powerup, or GameLogic::kPowerupNone.
     * @ghidraAddress NTSC-U/C: 0x0012e0e0
     * @ghidraAddress PAL: 0x0012f8b8
     */
    void SetPowerup(int nPowerup);

    /**
     * Report the remix repeat state.
     *
     * @return The state.
     * @ghidraAddress NTSC-U/C: 0x0012e148
     * @ghidraAddress PAL: 0x0012f920
     */
    bool GetRepeat() const;

    /**
     * Report the streak multiplier a streak earns.
     *
     * The multiplier is one more than the streak, capped at GameConfig::mStreakMultiplierMaxSolo
     * with one player and at GameConfig::mStreakMultiplierMaxMulti otherwise.
     *
     * @param nStreak The streak.
     * @return The multiplier.
     * @ghidraAddress NTSC-U/C: 0x0012e158
     * @ghidraAddress PAL: 0x0012f930
     */
    int StreakMultiplier(int nStreak) const;

protected:
    int mIndex;                         /*!< The player's index. */
    int mTicksPerBar;                   /*!< The song ticks in one bar. */
    Track *mTrack;                      /*!< The track the player plays, or null. */
    int mScore;                         /*!< The score. */
    int mPendingPoints;                 /*!< The points of the phrase being played. */
    int mMultiplierValue;               /*!< 1, or the multiplier power-up value while active. */
    int mStreak;                        /*!< The streak. */
    bool mCatching;                     /*!< Whether the player is catching. */
    bool mAborted;                      /*!< Whether the player has quit the song. */
    bool mRepeat;                       /*!< The remix repeat state. */
    int mReserved;                      // +0x28, set to -1 and not read by any routine here.
    Ptr<Command> mMultiplierEndCommand; /*!< The command that calls EndMultiplier(). */
    int mPowerup;                       /*!< The kind of power-up the player has. */
};
