#pragma once

#include <vector>

#include "os/prnstream.h"

/**
 * Display of the play field and the heads-up display of every player.
 *
 * The RTTI records the class as deriving from PrnStream. The one instance is TheGfxManager. Only
 * the members its callers here use are declared, and their names are inferred from the displays
 * they drive.
 */
class GfxManager : public PrnStream {
public:
    /** The values of Poll() that WorldMgr tests. */
    enum PollResult {
        kPollWorldReady = 1, /*!< The display is ready for a loading world to load its assets. */
        kPollUnloaded = 2,   /*!< The display finished changing back after an unload. */
    };

    /**
     * Release the display.
     *
     * @ghidraAddress NTSC-U/C: 0x001b1790
     * @ghidraAddress PAL: 0x001ba530
     */
    ~GfxManager() override;

    /**
     * Discard text written to the display as a stream. The body is empty.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x001b4450
     * @ghidraAddress PAL: 0x001bd1f0
     */
    void Print([[maybe_unused]] const char *pszText) override {
    }

    /** What SetPendingPointsResult() shows happened to a player's pending points. */
    enum PendingPointsResult {
        kPendingPointsCleared = 0,  /*!< The pending points were reset. */
        kPendingPointsCaptured = 1, /*!< The pending points were added to the score. */
        kPendingPointsLost = 2,     /*!< The pending points were forfeited. */
    };

    /**
     * Start the camera move that ends a song.
     *
     * @param fTime The time the move starts at, or a negative value for now.
     * @return The duration of the move in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001b3560
     * @ghidraAddress PAL: 0x001bc300
     */
    float StartOutro(float fTime);

    /**
     * Report whether the camera move StartOutro() started is done.
     *
     * @return Whether the move is done.
     * @ghidraAddress NTSC-U/C: 0x001b35a8
     * @ghidraAddress PAL: 0x001bc348
     */
    bool IsOutroDone();

    /**
     * Hide a player's pending points.
     *
     * @param nPlayer The player's index.
     * @ghidraAddress NTSC-U/C: 0x001b5150
     * @ghidraAddress PAL: 0x001bdef0
     */
    void HidePendingPoints(int nPlayer);

    /**
     * Show what happened to a player's pending points.
     *
     * @param nPlayer The player's index.
     * @param result What happened to the points.
     * @ghidraAddress NTSC-U/C: 0x001b5170
     * @ghidraAddress PAL: 0x001bdf10
     */
    void SetPendingPointsResult(int nPlayer, PendingPointsResult result);

    /**
     * Show or hide the labels of the tracks.
     *
     * @param bShow Whether the labels are shown.
     * @ghidraAddress NTSC-U/C: 0x001b51f8
     * @ghidraAddress PAL: 0x001bdf98
     */
    void ShowTrackLabels(bool bShow);

    /**
     * Show the slowdown a player deployed.
     *
     * @param nPlayer The player's index.
     * @param fStartTick The tick the song reaches the slow speed.
     * @param fEndTick The tick the song starts to speed up again.
     * @param fStopTick The tick the song is back at its speed.
     * @ghidraAddress NTSC-U/C: 0x001b5260
     * @ghidraAddress PAL: 0x001be000
     */
    void ShowSlowdown(int nPlayer, float fStartTick, float fEndTick, float fStopTick);

    /**
     * Show or hide a player's freestyle effect.
     *
     * @param nPlayer The player index.
     * @param bActive Show the effect.
     * @param nColumn The column the effect is drawn at.
     * @ghidraAddress NTSC-U/C: 0x001b5cc0
     * @ghidraAddress PAL: 0x001bea60
     */
    void SetFreestyle(int nPlayer, bool bActive, int nColumn);

    /**
     * Return a player's freestyle effect to its idle state.
     *
     * @param nPlayer The player index.
     * @ghidraAddress NTSC-U/C: 0x001b5ce0
     * @ghidraAddress PAL: 0x001bea80
     */
    void ResetFreestyle(int nPlayer);

    /**
     * Start the intro of the song display.
     *
     * The name is inferred.
     *
     * @param fArgument The command argument, -1 from a scheduled command.
     * @return The duration of the intro, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001b34c8
     * @ghidraAddress PAL: 0x001bc268
     */
    float StartIntro(float fArgument);

    /**
     * Run the intro of the song display again.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001b3520
     * @ghidraAddress PAL: 0x001bc2c0
     */
    void RestartIntro();

    /**
     * Advance the display to a song time.
     *
     * @param fTime The song time.
     * @return The display state.
     * @ghidraAddress NTSC-U/C: 0x001b37f8
     * @ghidraAddress PAL: 0x001bc598
     */
    int Poll(float fTime);

    /**
     * Begin loading the display assets that follow the song.
     *
     * The name is inferred.
     *
     * @param nFirst The first argument. The meaning is not yet recovered.
     * @param nSecond The second argument. The meaning is not yet recovered.
     * @ghidraAddress NTSC-U/C: 0x001b4f40
     * @ghidraAddress PAL: 0x001bdce0
     */
    void BeginLoad(int nFirst, int nSecond);

    /**
     * Show the lyric text.
     *
     * @param pszText The text.
     * @param bFirstLine Show the text on the first line.
     * @ghidraAddress NTSC-U/C: 0x001b5340
     * @ghidraAddress PAL: 0x001be0e0
     */
    void SetLyricText(const char *pszText, bool bFirstLine);

    /**
     * Report whether the display finished its pending work.
     *
     * The name is inferred.
     *
     * @return True when the display is idle.
     * @ghidraAddress NTSC-U/C: 0x001b56e8
     * @ghidraAddress PAL: 0x001be488
     */
    bool IsIdle();

    /**
     * Mark the winner of a song.
     *
     * @param nPlayer The winner's index, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x001b5728
     * @ghidraAddress PAL: 0x001be4c8
     */
    void SetWinner(int nPlayer);

    /**
     * Rank the score bars at the end of a section and fire the triggers that wait on it.
     *
     * Every caller passes the section, and the body does not read it.
     *
     * @param nPlayer The player whose completion the triggers receive.
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x001b5db0
     * @ghidraAddress PAL: 0x001beb50
     */
    void CompleteStage(int nPlayer, int nSection);

    /**
     * Show a message of one or two lines.
     *
     * @param pszLine The first line.
     * @param pszSecondLine The second line, or null.
     * @param nPlayer The player the message is for, or -1 for every player.
     * @param fDurationMs The time the message stays on the screen, in milliseconds.
     * @param fScale The scale of the text.
     * @param fOffsetY The vertical offset of the message.
     * @param fOffsetX The horizontal offset of the message.
     * @ghidraAddress NTSC-U/C: 0x001b5320
     * @ghidraAddress PAL: 0x001be0c0
     */
    void ShowMessage(const char *pszLine,
                     const char *pszSecondLine,
                     int nPlayer,
                     float fDurationMs,
                     float fScale,
                     float fOffsetY,
                     float fOffsetX);

    /**
     * Show which players are on a track.
     *
     * @param nTrack The track.
     * @param nPlayers The number of players on the track.
     * @param pPlayers The players, in the order they arrived, or null when there are none.
     * @ghidraAddress NTSC-U/C: 0x001b5730
     * @ghidraAddress PAL: 0x001be4d0
     */
    void SetTrackPlayers(int nTrack, int nPlayers, const int *pPlayers);

    /**
     * Show a player on a catch track.
     *
     * @param nPlayer The player's index.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001b57b0
     * @ghidraAddress PAL: 0x001be550
     */
    void ShowPlayerOnTrack(int nPlayer, int nTrack);

    /**
     * Show a player on the freestyle track.
     *
     * @param nPlayer The player's index.
     * @param nType The track type of the freestyle track.
     * @param nInstrument The instrument of the freestyle track.
     * @param bVictory Whether the freestyle rewards a won song.
     * @param nReserved Every caller passes 0.
     * @ghidraAddress NTSC-U/C: 0x001b5970
     * @ghidraAddress PAL: 0x001be710
     */
    void ShowPlayerOnFreestyleTrack(
        int nPlayer, int nType, int nInstrument, bool bVictory, int nReserved);

    /**
     * Set the size of the display of the tracks of a player.
     *
     * @param nPlayer The index of the player.
     * @param nSize One of GameOptions::FreqSize.
     * @ghidraAddress NTSC-U/C: 0x001b5708
     * @ghidraAddress PAL: 0x001be4a8
     */
    void SetFreqSize(int nPlayer, int nSize);

    /**
     * Report the track the camera of a player shows.
     *
     * @param nPlayer The player's index.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x001b59f8
     * @ghidraAddress PAL: 0x001be798
     */
    int GetViewedTrack(int nPlayer);

    /**
     * Hide the pointers to the next phrase.
     *
     * @param nPlayer The player's index.
     * @ghidraAddress NTSC-U/C: 0x001b5ad8
     * @ghidraAddress PAL: 0x001be878
     */
    void HideNextPhrase(int nPlayer);

    /**
     * Point at the next phrase on a track.
     *
     * @param nTrack The track.
     * @param fTick The tick of the first gem of the phrase.
     * @param nGemType The type of that gem.
     * @ghidraAddress NTSC-U/C: 0x001b5af8
     * @ghidraAddress PAL: 0x001be898
     */
    void ShowNextPhrase(int nTrack, float fTick, int nGemType);

    /**
     * Show the result of a song.
     *
     * @param bWon Whether the song was won.
     * @param nMode The mode of the display.
     * @param bUnlocked Whether winning the song unlocked a new one.
     * @ghidraAddress NTSC-U/C: 0x001b5e00
     * @ghidraAddress PAL: 0x001beba0
     */
    void ShowResult(bool bWon, int nMode, bool bUnlocked);

    /**
     * Show a track as enabled or disabled.
     *
     * @param nTrack The track.
     * @param bEnabled Whether the track is enabled.
     * @ghidraAddress NTSC-U/C: 0x001b5f78
     * @ghidraAddress PAL: 0x001bed18
     */
    void SetTrackEnabled(int nTrack, bool bEnabled);

    /**
     * Place a checkpoint on the play field.
     *
     * @param nPlayer The player's index.
     * @param fTick The tick of the checkpoint.
     * @param fOffset The offset of the checkpoint.
     * @param fScale The scale of the checkpoint.
     * @ghidraAddress NTSC-U/C: 0x001b5f98
     * @ghidraAddress PAL: 0x001bed38
     */
    void AddCheckpoint(int nPlayer, float fTick, float fOffset, float fScale);

    /**
     * Set the display option a solo player toggles.
     *
     * @param bOption The option.
     * @ghidraAddress NTSC-U/C: 0x001b6060
     * @ghidraAddress PAL: 0x001bee00
     */
    void SetOption(bool bOption);

    /**
     * Report the display option a solo player toggles.
     *
     * @return The option.
     * @ghidraAddress NTSC-U/C: 0x001b6080
     * @ghidraAddress PAL: 0x001bee20
     */
    bool GetOption();

    /**
     * Show or hide a player on the play field.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the player is shown.
     * @ghidraAddress NTSC-U/C: 0x001b60e8
     * @ghidraAddress PAL: 0x001bee88
     */
    void SetPlayerShown(int nPlayer, bool bShown);

    /**
     * Show or hide the catcher of a player.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the catcher is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6110
     * @ghidraAddress PAL: 0x001beeb0
     */
    void SetCatcherShown(int nPlayer, bool bShown);

    /**
     * Show or hide the score of a player.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the score is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6150
     * @ghidraAddress PAL: 0x001beef0
     */
    void SetScoreShown(int nPlayer, bool bShown);

    /**
     * Show or hide the lane display of a player.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the lane display is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6170
     * @ghidraAddress PAL: 0x001bef10
     */
    void SetLaneShown(int nPlayer, bool bShown);

    /**
     * Set the scroll speed of the lanes.
     *
     * The name is inferred.
     *
     * @param fSpeed The speed, 1 for normal.
     * @ghidraAddress NTSC-U/C: 0x001b60a0
     * @ghidraAddress PAL: 0x001bee40
     */
    void SetScrollSpeed(float fSpeed);

    /**
     * Remove the gems from the lanes.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001b61b0
     * @ghidraAddress PAL: 0x001bef50
     */
    void ClearLanes();

    /**
     * Remove every element of the song display.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001b61e0
     * @ghidraAddress PAL: 0x001bef80
     */
    void ClearAll();

    /**
     * Activate or deactivate the song display.
     *
     * The name is inferred.
     *
     * @param bActive Activate the display.
     * @ghidraAddress NTSC-U/C: 0x001b6250
     * @ghidraAddress PAL: 0x001beff0
     */
    void SetActive(bool bActive);

    /**
     * Set the energy meter of a player.
     *
     * @param nPlayer The player's index.
     * @param fEnergy The energy, in percent.
     * @ghidraAddress NTSC-U/C: 0x001b4d68
     * @ghidraAddress PAL: 0x001bdb08
     */
    void SetEnergy(int nPlayer, float fEnergy);

    /**
     * Show a player's score and the points just added to it.
     *
     * @param nPlayer The player's index.
     * @param nScore The new score.
     * @param nMultiplier The streak multiplier the points were scored at.
     * @param nPoints The points just added, or 0.
     * @ghidraAddress NTSC-U/C: 0x001b4f90
     * @ghidraAddress PAL: 0x001bdd30
     */
    void SetScore(int nPlayer, int nScore, int nMultiplier, int nPoints);

    /**
     * Show a player's streak multiplier and the one the next capture earns.
     *
     * @param nPlayer The player's index.
     * @param nMultiplier The current streak multiplier.
     * @param nNextMultiplier The streak multiplier after one more capture.
     * @param bBoosted Whether a multiplier power-up is active.
     * @ghidraAddress NTSC-U/C: 0x001b4fd8
     * @ghidraAddress PAL: 0x001bdd78
     */
    void SetStreakMultiplier(int nPlayer, int nMultiplier, int nNextMultiplier, bool bBoosted);

    /**
     * Show the points a player's current phrase is worth.
     *
     * @param nPlayer The player's index.
     * @param nPoints The points.
     * @ghidraAddress NTSC-U/C: 0x001b50d0
     * @ghidraAddress PAL: 0x001bde70
     */
    void ShowPendingPoints(int nPlayer, int nPoints);

    /**
     * Mark whether a player leads the score.
     *
     * @param nPlayer The player's index.
     * @param bLeader Whether the player leads.
     * @ghidraAddress NTSC-U/C: 0x001b5190
     * @ghidraAddress PAL: 0x001bdf30
     */
    void SetLeader(int nPlayer, bool bLeader);

    /**
     * Show the power-up icon of a player.
     *
     * @param nPlayer The player's index.
     * @param nPowerup The kind of power-up the player holds, or GameLogic::kPowerupNone to hide
     *        the icon.
     * @ghidraAddress NTSC-U/C: 0x001b5218
     * @ghidraAddress PAL: 0x001bdfb8
     */
    void ShowPowerup(int nPlayer, int nPowerup);

    /**
     * Show the song tick a player's multiplier power-up ends at.
     *
     * @param nPlayer The player's index.
     * @param fTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001b5240
     * @ghidraAddress PAL: 0x001bdfe0
     */
    void SetMultiplierEndTick(int nPlayer, float fTick);

    /**
     * Show a player crippling another.
     *
     * @param nAttacker The index of the player who deployed the crippler.
     * @param nVictim The index of the player struck.
     * @ghidraAddress NTSC-U/C: 0x001b5280
     * @ghidraAddress PAL: 0x001be020
     */
    void ShowCripple(int nAttacker, int nVictim);

    /**
     * Show a player knocking another off a track with a bumper.
     *
     * @param nAttacker The index of the player who deployed the bumper.
     * @param nVictim The index of the player struck.
     * @param nTrack The track the victim moves to.
     * @ghidraAddress NTSC-U/C: 0x001b52a0
     * @ghidraAddress PAL: 0x001be040
     */
    void ShowBump(int nAttacker, int nVictim, int nTrack);

    /**
     * Build the display of the tracks of a song.
     *
     * @param fStartTick The tick the song starts at.
     * @param fEndTick The tick the song ends at.
     * @param pTickDuration The duration of one tick on the song clock.
     * @param firstList The first list.
     * @param secondList The second list.
     * @param nOption The option.
     * @param flags The flags.
     * @ghidraAddress NTSC-U/C: 0x001b2688
     * @ghidraAddress PAL: 0x001bb428
     */
    void BuildTracks(float fStartTick,
                     float fEndTick,
                     const float *pTickDuration,
                     const std::vector<int> &firstList,
                     const std::vector<int> &secondList,
                     int nOption,
                     const std::vector<bool> &flags);

    /**
     * Place a gem on the play field.
     *
     * @param nTrack The track.
     * @param nSlot The gem button the gem lies under.
     * @param nPlayer The player the gem is for, or -1.
     * @param fTick The tick of the gem.
     * @param nStyle The style of the gem.
     * @param nFlags The flags of the gem.
     * @ghidraAddress NTSC-U/C: 0x001b5a78
     * @ghidraAddress PAL: 0x001be818
     */
    void PlaceGem(int nTrack, int nSlot, int nPlayer, float fTick, int nStyle, int nFlags);

    /**
     * Remove the gems of a track in a range of ticks.
     *
     * The name is inferred.
     *
     * @param nTrack The track.
     * @param bAll Whether every kind of gem is removed.
     * @param fStartTick The first tick of the range.
     * @param fEndTick The tick after the range.
     * @ghidraAddress NTSC-U/C: 0x001b5ab8
     * @ghidraAddress PAL: 0x001be858
     */
    void ClearGems(int nTrack, bool bAll, float fStartTick, float fEndTick);

    /**
     * Show the result of a gem a player played or removed.
     *
     * The name is inferred.
     *
     * @param nTrack The track.
     * @param nSlot The gem button of the gem.
     * @param bHit Whether the gem counts as hit.
     * @param nPlayer The player's index.
     * @param nFlags The flags of the display.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001b5b70
     * @ghidraAddress PAL: 0x001be910
     */
    void ShowGemResult(int nTrack, int nSlot, bool bHit, int nPlayer, int nFlags, float fTick);

    /**
     * Show a player hitting a gem.
     *
     * @param nPlayer The player's index.
     * @param nTrack The track.
     * @param nSlot The gem button the gem lies under.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001b5c40
     * @ghidraAddress PAL: 0x001be9e0
     */
    void HitGem(int nPlayer, int nTrack, int nSlot, float fTick);

    /**
     * Show the bars of a phrase on a track, from the camera of a player.
     *
     * @param nPlayer The player's index.
     * @param nTrack The track.
     * @param bPlayable Whether the player plays the phrase.
     * @param fStartTick The tick the phrase starts at.
     * @param fEndTick The tick after the phrase.
     * @param nStyle The style of the display.
     * @param bSlide Whether the camera moves to the phrase.
     * @ghidraAddress NTSC-U/C: 0x001b5f08
     * @ghidraAddress PAL: 0x001beca8
     */
    void ShowPhrase(int nPlayer,
                    int nTrack,
                    bool bPlayable,
                    float fStartTick,
                    float fEndTick,
                    int nStyle,
                    bool bSlide);

    /**
     * Set the look of one bar of a track.
     *
     * The name is inferred.
     *
     * @param nTrack The track.
     * @param nPlayer The player whose camera shows the bar.
     * @param nOwner The player who owns the bar, or -1.
     * @param bInSong Whether the bar maps to a bar of the song.
     * @param bVisible Whether the bar is shown.
     * @param nFlags The flags of the bar.
     * @param nRiff The riff of the bar, or -1.
     * @param fTick The tick the bar starts at.
     * @param fTicks The length of the bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x001b5f30
     * @ghidraAddress PAL: 0x001becd0
     */
    void SetBar(int nTrack,
                int nPlayer,
                int nOwner,
                bool bInSong,
                bool bVisible,
                int nFlags,
                signed char nRiff,
                float fTick,
                float fTicks);

    /**
     * Register the display script commands.
     *
     * @ghidraAddress NTSC-U/C: 0x001b1cc0
     * @ghidraAddress PAL: 0x001baa60
     */
    void Init();

    /**
     * Unregister what Init() registered.
     *
     * @ghidraAddress NTSC-U/C: 0x001b1ed8
     * @ghidraAddress PAL: 0x001bac78
     */
    void Terminate();

    /**
     * Return the display to its state with no world.
     *
     * @ghidraAddress NTSC-U/C: 0x001b2030
     * @ghidraAddress PAL: 0x001badd0
     */
    void Reset();

    /**
     * Build the display for a world that starts loading.
     *
     * @param fTime The time the display starts at. WorldMgr passes -1.
     * @ghidraAddress NTSC-U/C: 0x001b2068
     * @ghidraAddress PAL: 0x001bae08
     */
    void Load(float fTime);

    /**
     * Start changing the display back after the world was destroyed.
     *
     * @ghidraAddress NTSC-U/C: 0x001b3458
     * @ghidraAddress PAL: 0x001bc1f8
     */
    void Unload();

    /**
     * Draw the display.
     *
     * @param fTime The running world's position in ticks, or 0 when no world is running.
     * @ghidraAddress NTSC-U/C: 0x001b3d10
     * @ghidraAddress PAL: 0x001bcab0
     */
    void Draw(float fTime);

    /**
     * Show the instrument a track changed to.
     *
     * @param nTrack The track.
     * @param nInstrument The instrument.
     * @ghidraAddress NTSC-U/C: 0x001b5878
     * @ghidraAddress PAL: 0x001be618
     */
    void SetTrackInstrument(int nTrack, int nInstrument);

    /**
     * Take a gem off the play field.
     *
     * The name is inferred.
     *
     * @param nTrack The track.
     * @param nSlot The gem button the gem lies under.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001b5a98
     * @ghidraAddress PAL: 0x001be838
     */
    void RemoveGem(int nTrack, int nSlot, float fTick);

    /**
     * Set the meter that shows how much of a phrase a player has caught.
     *
     * The name is inferred.
     *
     * @param nPlayer The player's index.
     * @param fLevel The level, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001b5b20
     * @ghidraAddress PAL: 0x001be8c0
     */
    void SetCatchMeter(int nPlayer, float fLevel);

    /**
     * Show a player catching a phrase.
     *
     * The name is inferred.
     *
     * @param nPlayer The player's index.
     * @param nTrack The track.
     * @param nStyle The style of the display. DuelTrack passes 0.
     * @param bQuiet Whether the status display is not told. DuelTrack passes false.
     * @param fStartTick The tick the phrase starts at.
     * @param fEndTick The tick after the phrase.
     * @ghidraAddress NTSC-U/C: 0x001b5d00
     * @ghidraAddress PAL: 0x001beaa0
     */
    void
    ShowCapture(int nPlayer, int nTrack, int nStyle, bool bQuiet, float fStartTick, float fEndTick);

    /**
     * Dim one bar of a track.
     *
     * @param nTrack The track.
     * @param fTick The tick the bar starts at.
     * @ghidraAddress NTSC-U/C: 0x001b5f58
     * @ghidraAddress PAL: 0x001becf8
     */
    void DimBar(int nTrack, float fTick);
};

/**
 * The display.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b020
 */
extern GfxManager TheGfxManager;
