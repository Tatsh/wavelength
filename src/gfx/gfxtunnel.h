#pragma once

#include <vector>

#include "gfx/gfxarena.h"
#include "gfx/tnlgeom.h"
#include "gfx/tnltrackrange.h"
#include "math/color.h"
#include "math/vector3.h"
#include "rnd/environ.h"
#include "rnd/view.h"

// The headers of these classes include this header, so the classes are declared here instead.
class PlayerCamFX;
class TnlGem;
class TnlGems;

/**
 * Tunnel of tracks the players fly down, with the gems, the cameras of the players, and the
 * effects drawn on the tracks.
 *
 * The RTTI of its nested types names the class. The class is not polymorphic, and the object is
 * 0x240 bytes. Only the members the display calls are declared, and their titles are inferred from
 * the display routines that forward to them.
 */
class GfxTunnel {
public:
    /**
     * Build the tunnel of a song.
     *
     * @param instruments The instrument of each track.
     * @param trackTypes The type of each track.
     * @param pArena The arena.
     * @param nOption The option of the tracks.
     * @param fStartTick The tick the song starts at.
     * @param fSongTicks The length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x001d82b8
     * @ghidraAddress PAL: 0x001e1058
     */
    GfxTunnel(const std::vector<int> &instruments,
              const std::vector<int> &trackTypes,
              GfxArena *pArena,
              int nOption,
              float fStartTick,
              float fSongTicks);

    /**
     * Destroy the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x001dc078
     * @ghidraAddress PAL: 0x001e4e18
     */
    ~GfxTunnel();

    /**
     * Find the main view of the tunnel and set its camera from the configuration.
     *
     * @return The view.
     * @ghidraAddress NTSC-U/C: 0x001db6b8
     * @ghidraAddress PAL: 0x001e4458
     */
    static Rnd::View *CreateMainView();

    /**
     * Tell the spotlight and the cursors of the players that a gem leaves the tunnel.
     *
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001dd888
     * @ghidraAddress PAL: 0x001e6628
     */
    void OnGemRemoved(const TnlGem *pGem);

    /**
     * Start a flash at a position with the first free flash effect.
     *
     * @param pPos The position.
     * @param flSize The size the flash starts at.
     * @param flGrowth The growth of the flash per tick.
     * @return Whether a flash effect was free.
     * @ghidraAddress NTSC-U/C: 0x001ddd00
     * @ghidraAddress PAL: 0x001e6aa0
     */
    bool Flash(const Vector3 *pPos, float flSize, float flGrowth);

    /**
     * Start a capture burst at a gem with the first free burst effect, through a random view.
     *
     * @param pViews The views to choose from.
     * @param nTrack The track.
     * @param bErase Whether the burst erases the gem.
     * @param flTick The tick of the gem.
     * @param flLateral The position across the track, 0 to 1.
     * @return Whether a burst effect was free.
     * @ghidraAddress NTSC-U/C: 0x001ddd90
     * @ghidraAddress PAL: 0x001e6b30
     */
    bool Burst(const std::vector<Rnd::View *> *pViews,
               char nTrack,
               bool bErase,
               float flTick,
               float flLateral);

    /**
     * Report the instrument of a track.
     *
     * @param nTrack The track.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x001de0e8
     * @ghidraAddress PAL: 0x001e6e88
     */
    int GetTrackInstrument(int nTrack);

    /**
     * Give a track another instrument.
     *
     * @param nTrack The track.
     * @param nInstrument The instrument.
     * @ghidraAddress NTSC-U/C: 0x001de100
     * @ghidraAddress PAL: 0x001e6ea0
     */
    void SetTrackInstrument(int nTrack, int nInstrument);

    /**
     * Report the type of a track.
     *
     * @param nTrack The track.
     * @return The type.
     * @ghidraAddress NTSC-U/C: 0x001de190
     * @ghidraAddress PAL: 0x001e6f30
     */
    int GetTrackType(int nTrack);

    /**
     * Report the track a player is on.
     *
     * @param nPlayer The player.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x001de1a8
     * @ghidraAddress PAL: 0x001e6f48
     */
    int GetPlayerTrack(int nPlayer);

    /**
     * Report whether a track is open, its effect not in the state that closes it.
     *
     * @param nTrack The track.
     * @return Whether the track is open.
     * @ghidraAddress NTSC-U/C: 0x001de200
     * @ghidraAddress PAL: 0x001e6fa0
     */
    bool IsTrackOpen(int nTrack) const;

    /**
     * Report whether a track is captured.
     *
     * @param nTrack The track.
     * @return Whether the effect of the track is in its captured state.
     * @ghidraAddress NTSC-U/C: 0x001de240
     * @ghidraAddress PAL: 0x001e6fe0
     */
    bool IsTrackCaptured(char nTrack) const;

    /**
     * Report the offset of the head-up display from the camera.
     *
     * @return The offset.
     * @ghidraAddress NTSC-U/C: 0x001de268
     * @ghidraAddress PAL: 0x001e7008
     */
    const Vector3 *GetHudOffset();

    /**
     * Advance the tunnel.
     *
     * @param fTicks The ticks since the last poll.
     * @param fTime The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001de480
     * @ghidraAddress PAL: 0x001e7220
     */
    void Poll(float fTicks, float fTime);

    /**
     * Advance the cameras of the players.
     *
     * @param fTicks The ticks since the last poll.
     * @param fTime The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001dec18
     * @ghidraAddress PAL: 0x001e79b8
     */
    void PollCameras(float fTicks, float fTime);

    /**
     * Blend the outer cameras with the closing of the letterbox.
     *
     * @param fLetterbox How far the letterbox is closed, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001dec88
     * @ghidraAddress PAL: 0x001e7a28
     */
    void SetLetterbox(float fLetterbox);

    /**
     * Draw the layer behind the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001ded28
     * @ghidraAddress PAL: 0x001e7ac8
     */
    void DrawBackground();

    /**
     * Draw the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001dede8
     * @ghidraAddress PAL: 0x001e7b88
     */
    void DrawTracks();

    /**
     * Draw the layer of the cameras of the players.
     *
     * @ghidraAddress NTSC-U/C: 0x001def00
     * @ghidraAddress PAL: 0x001e7ca0
     */
    void DrawCameras();

    /**
     * Draw the layer in front of the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001def68
     * @ghidraAddress PAL: 0x001e7d08
     */
    void DrawForeground();

    /**
     * Report the width of a panel of the tunnel.
     *
     * @return The width, a constant.
     * @ghidraAddress NTSC-U/C: 0x001df1f0
     * @ghidraAddress PAL: 0x001e7f90
     */
    float PanelWidth() const;

    /**
     * Report the time a capture burst takes to advance eight quarter turns.
     *
     * @return The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001df200
     * @ghidraAddress PAL: 0x001e7fa0
     */
    static float BurstSpinPeriod();

    /**
     * Collect the ticks of the gems of a track between two ticks.
     *
     * @param pTicks Receives the ticks after the ticks it already has.
     * @param nTrack The track.
     * @param flFromTick The first tick.
     * @param flToTick The tick the collection stops at.
     * @ghidraAddress NTSC-U/C: 0x001dd908
     * @ghidraAddress PAL: 0x001e66a8
     */
    void CollectGemTicks(std::vector<float> *pTicks, char nTrack, float flFromTick, float flToTick);

    /**
     * Start the fire of a captured track with the first free fire effect, or else the one that
     * started first.
     *
     * @param nPlayer The player who captured the track, or -1.
     * @param nTrack The track.
     * @param bAuto Whether the capture is automatic.
     * @param flTick The tick of the capture.
     * @param flStartTick The first tick of the fire.
     * @param flEndTick The last tick of the fire.
     * @param pInnerColor The colour of the inner flames.
     * @param pOuterColor The colour of the outer flames.
     * @param pGemTicks The ticks of the gems the fire burns, or null to collect them.
     * @return Whether a fire effect started.
     * @ghidraAddress NTSC-U/C: 0x001dde98
     * @ghidraAddress PAL: 0x001e6c38
     */
    bool StartFire(int nPlayer,
                   char nTrack,
                   int bAuto,
                   float flTick,
                   float flStartTick,
                   float flEndTick,
                   const Color *pInnerColor,
                   const Color *pOuterColor,
                   const std::vector<float> *pGemTicks);

    /**
     * Report whether the activator of a player is at rest.
     *
     * @param nPlayer The player.
     * @return Whether the activator is at rest.
     * @ghidraAddress NTSC-U/C: 0x001de1c8
     * @ghidraAddress PAL: 0x001e6f68
     */
    bool IsActivatorIdle(int nPlayer);

    /**
     * Report the particle systems of the tunnel through the debug output.
     *
     * @ghidraAddress NTSC-U/C: 0x001df7f0
     * @ghidraAddress PAL: 0x001e8590
     */
    void ReportParticles();

    /**
     * Show the energy of the play field.
     *
     * @param fEnergy The energy, from 0 to 1.
     * @param nZone The zone of the energy.
     * @param fZoneFraction The position of the energy within its zone.
     * @ghidraAddress NTSC-U/C: 0x001df810
     * @ghidraAddress PAL: 0x001e85b0
     */
    void SetEnergy(float fEnergy, int nZone, float fZoneFraction);

    /**
     * Show the streak multiplier of a player.
     *
     * @param nPlayer The player.
     * @param nMultiplier The multiplier.
     * @param nNextMultiplier The multiplier of the next streak.
     * @param pszMultiplier The text of the multiplier.
     * @param pszNextMultiplier The text of the multiplier of the next streak.
     * @param bBoosted Whether a powerup boosts the multiplier.
     * @ghidraAddress NTSC-U/C: 0x001df828
     * @ghidraAddress PAL: 0x001e85c8
     */
    void SetStreakMultiplier(int nPlayer,
                             int nMultiplier,
                             int nNextMultiplier,
                             const char *pszMultiplier,
                             const char *pszNextMultiplier,
                             bool bBoosted);

    /**
     * Set the tick the multiplier powerup of a player ends at.
     *
     * @param nPlayer The player.
     * @param fTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001df8b0
     * @ghidraAddress PAL: 0x001e8650
     */
    void SetMultiplierEndTick(int nPlayer, float fTick);

    /**
     * Show the slowdown a player deployed.
     *
     * @param nPlayer The player.
     * @param fStartTick The tick the song reaches the slow speed.
     * @param fEndTick The tick the song starts to speed up again.
     * @param fStopTick The tick the song is back at its speed.
     * @ghidraAddress NTSC-U/C: 0x001df8f8
     * @ghidraAddress PAL: 0x001e8698
     */
    void ShowSlowdown(int nPlayer, float fStartTick, float fEndTick, float fStopTick);

    /**
     * Show a player crippling another.
     *
     * @param nAttacker The attacking player.
     * @param nVictim The crippled player.
     * @ghidraAddress NTSC-U/C: 0x001df980
     * @ghidraAddress PAL: 0x001e8720
     */
    void ShowCripple(int nAttacker, int nVictim);

    /**
     * Start the transition of the cameras of a bump.
     *
     * @param nAttacker The attacking player.
     * @param nVictim The bumped player.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001dfa08
     * @ghidraAddress PAL: 0x001e87a8
     */
    void ShowBump(int nAttacker, int nVictim, int nTrack);

    /**
     * Move a player to a track.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nTrackType The type of the track.
     * @param nInstrument The instrument of the track.
     * @ghidraAddress NTSC-U/C: 0x001dfa98
     * @ghidraAddress PAL: 0x001e8838
     */
    void SetPlayerTrack(int nPlayer, int nTrack, int nTrackType, int nInstrument);

    /**
     * Move a player to the freestyle track.
     *
     * @param nPlayer The player.
     * @param bVictory Whether the freestyle rewards a won song.
     * @param nReserved Every caller passes 0.
     * @ghidraAddress NTSC-U/C: 0x001dfb70
     * @ghidraAddress PAL: 0x001e8910
     */
    void SetPlayerFreestyle(int nPlayer, bool bVictory, int nReserved);

    /**
     * Report the track the camera of a player shows.
     *
     * @param nPlayer The player.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x001dfbf0
     * @ghidraAddress PAL: 0x001e8990
     */
    int GetViewedTrack(int nPlayer);

    /**
     * Move the freestyle effect of a player.
     *
     * @param nPlayer The player.
     * @param position The stick position.
     * @ghidraAddress NTSC-U/C: 0x001dfc18
     * @ghidraAddress PAL: 0x001e89b8
     */
    void SetFreestylePosition(int nPlayer, const Vector3 &position);

    /**
     * Show a note a scratch plays on the track of a player.
     *
     * @param nPlayer The player.
     * @param fTick The tick of the note.
     * @param fDuration The length of the note in ticks.
     * @ghidraAddress NTSC-U/C: 0x001dfc50
     * @ghidraAddress PAL: 0x001e89f0
     */
    void ShowScratchNote(int nPlayer, float fTick, float fDuration);

    /**
     * Record the players on a track.
     *
     * @param nTrack The track.
     * @param nPlayers The number of players.
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x001dfc98
     * @ghidraAddress PAL: 0x001e8a38
     */
    void SetTrackPlayers(int nTrack, int nPlayers, const int *pPlayers);

    /**
     * Place a gem on a track.
     *
     * @param nTrack The track.
     * @param nSlot The gem button.
     * @param nPlayer The player the gem is for, or -1.
     * @param fTick The tick of the gem.
     * @param nStyle The style of the gem.
     * @param nFlags The flags of the gem.
     * @ghidraAddress NTSC-U/C: 0x001dfd68
     * @ghidraAddress PAL: 0x001e8b08
     */
    void PlaceGem(int nTrack, int nSlot, int nPlayer, float fTick, int nStyle, int nFlags);

    /**
     * Remove one gem of a track.
     *
     * @param nTrack The track.
     * @param nSlot The gem button.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e0010
     * @ghidraAddress PAL: 0x001e8db0
     */
    void RemoveGem(int nTrack, int nSlot, float fTick);

    /**
     * Remove the gems of a track in a range of ticks.
     *
     * @param nTrack The track.
     * @param bAll Whether every kind of gem is removed.
     * @param fStartTick The first tick of the range.
     * @param fEndTick The tick after the range.
     * @ghidraAddress NTSC-U/C: 0x001e00d0
     * @ghidraAddress PAL: 0x001e8e70
     */
    void ClearGems(int nTrack, bool bAll, float fStartTick, float fEndTick);

    /**
     * Hide the pointers to the next phrase.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001e0338
     * @ghidraAddress PAL: 0x001e90d8
     */
    void HideNextPhrase(int nPlayer);

    /**
     * Point at the next phrase on a track.
     *
     * @param nTrack The track.
     * @param fTick The tick of the first gem of the phrase.
     * @param nGemType The type of that gem.
     * @ghidraAddress NTSC-U/C: 0x001e03a0
     * @ghidraAddress PAL: 0x001e9140
     */
    void ShowNextPhrase(int nTrack, float fTick, int nGemType);

    /**
     * Show the catch meter of a player.
     *
     * @param nPlayer The player.
     * @param fLevel The level of the meter.
     * @ghidraAddress NTSC-U/C: 0x001e03d8
     * @ghidraAddress PAL: 0x001e9178
     */
    void SetCatchMeter(int nPlayer, float fLevel);

    /**
     * Show the result of a gem.
     *
     * @param nTrack The track.
     * @param nSlot The gem button.
     * @param bHit Whether the gem counts as hit.
     * @param nPlayer The player.
     * @param nFlags The flags of the display.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e0408
     * @ghidraAddress PAL: 0x001e91a8
     */
    void ShowGemResult(int nTrack, int nSlot, bool bHit, int nPlayer, int nFlags, float fTick);

    /**
     * Show a player hitting a gem.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nSlot The gem button.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e05c8
     * @ghidraAddress PAL: 0x001e9368
     */
    void HitGem(int nPlayer, int nTrack, int nSlot, float fTick);

    /**
     * Show or hide the freestyle effect of a player.
     *
     * @param nPlayer The player.
     * @param bActive Whether the effect is shown.
     * @param nColumn The column of the effect.
     * @ghidraAddress NTSC-U/C: 0x001e0678
     * @ghidraAddress PAL: 0x001e9418
     */
    void SetFreestyle(int nPlayer, bool bActive, int nColumn);

    /**
     * Reset the freestyle effect of a player.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001e06e0
     * @ghidraAddress PAL: 0x001e9480
     */
    void ResetFreestyle(int nPlayer);

    /**
     * Show a player catching a phrase.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nStyle The style of the display.
     * @param bQuiet Whether the status display is not told.
     * @param fStartTick The tick the phrase starts at.
     * @param fEndTick The tick after the phrase.
     * @ghidraAddress NTSC-U/C: 0x001e0708
     * @ghidraAddress PAL: 0x001e94a8
     */
    void
    ShowCapture(int nPlayer, int nTrack, int nStyle, bool bQuiet, float fStartTick, float fEndTick);

    /**
     * Show the result of the song.
     *
     * @param bWon Whether the song was won.
     * @param nMode The mode of the display.
     * @param bCampaignWin Whether a won song outside practice is shown.
     * @param bUnlocked Whether winning the song unlocked a new one.
     * @ghidraAddress NTSC-U/C: 0x001e0820
     * @ghidraAddress PAL: 0x001e95c0
     */
    void ShowResult(bool bWon, int nMode, bool bCampaignWin, bool bUnlocked);

    /**
     * Start the boss journey.
     *
     * @ghidraAddress NTSC-U/C: 0x001e0908
     * @ghidraAddress PAL: 0x001e96a8
     */
    void StartBossJourney();

    /**
     * Show the bars of a phrase on a track from the camera of a player.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param bPlayable Whether the player plays the phrase.
     * @param fStartTick The tick the phrase starts at.
     * @param fEndTick The tick after the phrase.
     * @param nStyle The style of the display.
     * @param bSlide Whether the camera moves to the phrase.
     * @ghidraAddress NTSC-U/C: 0x001e0928
     * @ghidraAddress PAL: 0x001e96c8
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
     * @param nTrack The track.
     * @param nPlayer The player whose camera shows the bar.
     * @param nOwner The player who owns the bar, or -1.
     * @param bInSong Whether the bar maps to a bar of the song.
     * @param bVisible Whether the bar is shown.
     * @param nFlags The flags of the bar.
     * @param nRiff The riff of the bar, or -1.
     * @param fTick The tick the bar starts at.
     * @param fTicks The length of the bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x001e0a00
     * @ghidraAddress PAL: 0x001e97a0
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
     * Dim one bar of a track.
     *
     * @param nTrack The track.
     * @param fTick The tick the bar starts at.
     * @ghidraAddress NTSC-U/C: 0x001e0e88
     * @ghidraAddress PAL: 0x001e9c28
     */
    void DimBar(int nTrack, float fTick);

    /**
     * Show a track as enabled or disabled.
     *
     * @param nTrack The track.
     * @param bEnabled Whether the track is enabled.
     * @ghidraAddress NTSC-U/C: 0x001e0f40
     * @ghidraAddress PAL: 0x001e9ce0
     */
    void SetTrackEnabled(int nTrack, bool bEnabled);

    /**
     * Place a checkpoint on the tracks.
     *
     * @param fTick The tick of the checkpoint.
     * @param fScale The scale of the checkpoint.
     * @ghidraAddress NTSC-U/C: 0x001e0f70
     * @ghidraAddress PAL: 0x001e9d10
     */
    void AddCheckpoint(float fTick, float fScale);

    /**
     * Remove every checkpoint.
     *
     * @ghidraAddress NTSC-U/C: 0x001e0f90
     * @ghidraAddress PAL: 0x001e9d30
     */
    void ClearCheckpoints();

    /**
     * Set the option of the slide of the cameras.
     *
     * @param bOption The option.
     * @ghidraAddress NTSC-U/C: 0x001e0fb0
     * @ghidraAddress PAL: 0x001e9d50
     */
    void SetOption(bool bOption);

    /**
     * Report the option of the slide of the cameras.
     *
     * @return The option.
     * @ghidraAddress NTSC-U/C: 0x001e0fd0
     * @ghidraAddress PAL: 0x001e9d70
     */
    bool GetOption();

    /**
     * Set the scroll speed of the tracks.
     *
     * @param fSpeed The speed, 1 for normal.
     * @ghidraAddress NTSC-U/C: 0x001e0fe0
     * @ghidraAddress PAL: 0x001e9d80
     */
    void SetScrollSpeed(float fSpeed);

    /**
     * Show or hide the catcher of a player.
     *
     * @param nPlayer The player.
     * @param bShown Whether the catcher is shown.
     * @ghidraAddress NTSC-U/C: 0x001e10a8
     * @ghidraAddress PAL: 0x001e9e48
     */
    void SetCatcherShown(int nPlayer, bool bShown);

    /**
     * Clear the highlights of the cameras and the rows of the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001e10d8
     * @ghidraAddress PAL: 0x001e9e78
     */
    void ClearLanes();

    /**
     * Clear every effect of the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x001e1148
     * @ghidraAddress PAL: 0x001e9ee8
     */
    void ClearAll();

    /**
     * Scale the colours of the lights of an environment for the scroll speed of the tunnel.
     *
     * @param pEnviron The environment.
     * @ghidraAddress NTSC-U/C: 0x001df218
     * @ghidraAddress PAL: 0x001e7fb8
     */
    static void ScaleLights(Rnd::Environ *pEnviron);

    /**
     * The tunnel of the song, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003af9d4
     */
    static GfxTunnel *sCurrent;

    // +0x000 to +0x087 are not yet identified.
    TnlGems *mGems; /*!< The gems. +0x088 */
    TnlGeom *mGeom; /*!< The geometry of the tracks. +0x08c */
    // +0x090 to +0x14b are not yet identified.
    signed char mCrippledTracks; /*!< One bit for each track whose player is crippled. +0x14c */
    // +0x14d to +0x14f are not yet identified.
    int mNumTracks; /*!< The number of tracks. +0x150 */
    // +0x154 to +0x193 are not yet identified.
    /*!< The views of the capture bursts of each player. +0x194 */
    std::vector<std::vector<Rnd::View *>> mPlayerBurstViews;
    /*!< The views of the bursts of each kind of gem. +0x1a4 */
    std::vector<std::vector<Rnd::View *>> mGemBurstViews;
    // +0x1b4 to +0x1b7 are not yet identified.
    TnlTrackRange mChangedRange; /*!< The tracks and ticks the last poll changed. +0x1b8 */
    int mBossJourneyDone;        /*!< Whether the boss journey has ended. +0x1c4 */
    int mIdle;                   /*!< Whether no effect of the tunnel runs. +0x1c8 */
    // +0x1cc to +0x1db are not yet identified.
    PlayerCamFX *mCamFX; /*!< The camera of the players. +0x1dc */
    // +0x1e0 to +0x23f are not yet identified.
};
