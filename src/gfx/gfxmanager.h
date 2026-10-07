#pragma once

/**
 * Display of the play field and the heads-up display of every player.
 *
 * The RTTI includes the class name. The one instance is TheGfxManager. Only the members its
 * callers here use are declared, and their names are inferred from the displays they drive.
 */
class GfxManager {
public:
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
    void ShowPlayerOnFreestyleTrack(int nPlayer,
                                    int nType,
                                    int nInstrument,
                                    bool bVictory,
                                    int nReserved);

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
};

/**
 * The display.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b020
 */
extern GfxManager TheGfxManager;
