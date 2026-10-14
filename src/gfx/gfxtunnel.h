#pragma once

#include <list>
#include <utility>
#include <vector>

#include "gfx/gfxarena.h"
#include "gfx/tnlgeom.h"
#include "gfx/tnltrackrange.h"
#include "math/color.h"
#include "math/interpolator.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/rndcam.h"
#include "rnd/rndenviron.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndview.h"

// The headers of these classes include this header, so the classes are declared here instead.
class DataArray;
class GemEraseFX;
class GfxTunnelPanel;
class ParticleArms;
class PlayerCamFX;
class SpotLightFX;
class TnlBlurs;
class TnlCripFX;
class TnlDivider;
class TnlFire;
class TnlFlashFX;
class TnlGem;
class TnlGemCapFX;
class TnlGems;
class TnlPlayer;
class TnlSlowdownFX;
class TnlStreakGemFX;
class TnlTrackFX;
class TnlWinShell;

namespace Rnd {
class Cam;
class Light;
class Mat;
class Mesh;
struct Particle;
class TransAnim;
} // namespace Rnd

/**
 * Tunnel of tracks the players fly down, with the gems, the cameras of the players, and the
 * effects drawn on the tracks.
 *
 * The RTTI of its nested types names the class. The class is not polymorphic, and the object is
 * 0x240 bytes. The titles of the display routines are inferred from the display routines that
 * forward to them.
 */
class GfxTunnel {
public:
    /** The bits of BarData::mFlags. */
    enum BarFlag {
        kBarFlagNoReveal = 0x4,    /*!< The panel of the bar is dropped instead of revealed. */
        kBarFlagCheckpoint = 0x8,  /*!< The bar shows `panel ckpt.mat`. */
        kBarFlagFinish = 0x10,     /*!< The bar shows `panel ckpt finish.mat`. */
        kBarFlagBeforeSong = 0x20, /*!< The bar ends before the song starts. */
    };

    /** The kinds of the capture bursts of mGemBurstViews, in order. */
    enum GemBurst {
        kGemBurstGem = 0,  /*!< `gem explode <n>.view`. */
        kGemBurstAuto = 1, /*!< `auto_explode <n>.view`. */
        kGemBurstMult = 2, /*!< `mult_explode <n>.view`. */
        kGemBurstSlow = 3, /*!< `slow_explode <n>.view`. */
        kGemBurstFree = 4, /*!< `free_explode <n>.view`. */
        kGemBurstBump = 5, /*!< `bump_explode <n>.view`. */
        kGemBurstCrip = 6, /*!< `crip_explode <n>.view`. */
        kNumGemBursts = 7, /*!< The number of kinds. */
    };

    /** The powerup gems of mPowerupGemGroups, in order. */
    enum PowerupGem {
        kPowerupGemAuto = 0, /*!< `gem_auto`. */
        kPowerupGemMult = 1, /*!< `gem_mult`. */
        kPowerupGemSlow = 2, /*!< `gem_slow`. */
        kPowerupGemFree = 3, /*!< `gem_free`. */
        kPowerupGemBump = 4, /*!< `gem_bump`. */
        kPowerupGemCrip = 5, /*!< `gem_crip`. */
        kNumPowerupGems = 6, /*!< The number of powerup gems. */
    };

    /** The look of one bar of a track, 0x14 bytes. */
    struct BarData {
        float mTick;  /*!< The tick the bar starts at. */
        float mTicks; /*!< The length of the bar in ticks. */
        int mInSong;  /*!< Whether the bar maps to a bar of the song. */
        int mOwner;   /*!< The player who owns the bar, or -1. */
        int mFlags;   /*!< A set of BarFlag. */
    };

    /** The state of one track, 0x2c bytes. */
    class TrackData {
    public:
        /** The type of a track before the tunnel gives it one. */
        static constexpr int kNoType = 5;

        /** The tick of a capture that has not happened. */
        static constexpr float kNoTick = -1e9f;

        /** Construct a track with no index, no bars, and no capture. */
        TrackData()
            : mIndex(-1), mInstrument(0), mType(kNoType), mCaptureStartTick(kNoTick),
              mCaptureEndTick(kNoTick) {
        }

        /**
         * Find the first bar that ends after a tick, dropping the bars that ended more than
         * half a bar before the song.
         *
         * @param flTick The tick.
         * @param flNow The tick of the song.
         * @param pbFound Receives whether the bar starts at or before flTick.
         * @return The bar, or the end of mBars.
         * @ghidraAddress NTSC-U/C: 0x001de340
         * @ghidraAddress PAL: 0x001e70e0
         */
        std::list<BarData>::iterator FindBar(float flTick, float flNow, int *pbFound);

        int mIndex;                        /*!< The track, or -1. */
        int mInstrument;                   /*!< The instrument, one of GfxManager::Instrument. */
        int mType;                         /*!< The type of the track. */
        std::list<BarData> mBars;          /*!< The bars set on the track, in order of tick. */
        std::vector<signed char> mPlayers; /*!< The players on the track. */
        float mCaptureStartTick;           /*!< The tick the last capture of the track started. */
        float mCaptureEndTick;             /*!< The tick after the last captured phrase. */
    };

    /** A line of the cross section of the tunnel, from one point to the other. */
    typedef std::pair<Vector2, Vector2> Segment;

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
    static RndView *CreateMainView();

    /**
     * Load the settings of the tunnel and of the effects it draws, and with a tunnel rebuild its
     * panels from them.
     *
     * The tunnel is used without a test for null unless nNumTracks is negative, where a null
     * tunnel returns at once. A nonzero nOption also loads the settings of each player, rebuilds
     * the panels, and reloads the triggers.
     *
     * @param pTunnel The tunnel.
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param nNumTracks The number of tracks, or a negative number for the tracks of pTunnel.
     * @param nOption The fourth argument every loader of the tunnel graphics receives.
     * @ghidraAddress NTSC-U/C: 0x001d7248
     * @ghidraAddress PAL: 0x001dffe8
     */
    static void LoadConfig(
        GfxTunnel *pTunnel, DataArray *pConfig, DataArray *pDefaults, int nNumTracks, int nOption);

    /**
     * Find the outer cameras, one for each player, and fit them to the screen.
     *
     * @return The number of cameras.
     * @ghidraAddress NTSC-U/C: 0x001da970
     * @ghidraAddress PAL: 0x001e3710
     */
    int BuildOuterCams();

    /**
     * Collect the views of a numbered series, `<format>` with 1, 2, and on, until one is missing.
     *
     * The tunnel itself is not read. A series with no views is reported through the debug output.
     *
     * @param pViews Receives the views after the views it already has.
     * @param pszFormat The format of the names, with one integer.
     * @ghidraAddress NTSC-U/C: 0x001dbaa8
     * @ghidraAddress PAL: 0x001e4848
     */
    void LoadViews(std::vector<RndView *> *pViews, const char *pszFormat);

    /**
     * Load the colour of each instrument from its panel material and colour each track.
     *
     * The materials are `panel mat <instrument>.mat`, or `panel mat <instrument> multi.mat` when
     * the community is not GameDb::kCommunitySolo and the rule set is GameDb::kRuleSetGame. The
     * tracks are black with the `black_panels` cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x001dbcc8
     * @ghidraAddress PAL: 0x001e4a68
     */
    void LoadTrackColors();

    /**
     * Give each track the sprite gem group of its instrument.
     *
     * @ghidraAddress NTSC-U/C: 0x001dc020
     * @ghidraAddress PAL: 0x001e4dc0
     */
    void UpdateTrackGemGroups();

    /**
     * Show or hide the panels of the tracks.
     *
     * @param bShow Whether the panels show.
     * @ghidraAddress NTSC-U/C: 0x001dc8c8
     * @ghidraAddress PAL: 0x001e5668
     */
    void SetShowPanels(int bShow);

    /**
     * Show or hide the boundaries between the sections of the song.
     *
     * @param bShow Whether the boundaries show.
     * @ghidraAddress NTSC-U/C: 0x001dc8d8
     * @ghidraAddress PAL: 0x001e5678
     */
    void SetShowDividers(int bShow);

    /**
     * Show or hide the targets of the activators, `target <n>_<c>.mesh` for each colour.
     *
     * @param bShow Whether the targets show.
     * @ghidraAddress NTSC-U/C: 0x001dc8e8
     * @ghidraAddress PAL: 0x001e5688
     */
    void SetShowActivator(int bShow);

    /**
     * Build the panels of the tracks from the cross section of the tunnel.
     *
     * @param bResetPanels Whether every panel then returns to its plain state.
     * @param pCamPath The camera path of the arena.
     * @ghidraAddress NTSC-U/C: 0x001dca10
     * @ghidraAddress PAL: 0x001e57b0
     */
    void BuildPanels(int bResetPanels, RndTransAnim *pCamPath);

    /**
     * Fit the boundaries between the sections to the bounds of a cross section.
     *
     * @param pSegments The lines of the cross section.
     * @param flScale The scroll speed of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x001d35b8
     * @ghidraAddress PAL: 0x001dc358
     */
    static void LayoutDividers(const std::vector<Segment> *pSegments, float flScale);

    /**
     * Scale the meshes `HUD <kind>_gem<n>.mesh` of one kind of powerup gem.
     *
     * @param pScale The scale of each axis.
     * @param pszKind The kind of powerup.
     * @ghidraAddress NTSC-U/C: 0x001d3790
     * @ghidraAddress PAL: 0x001dc530
     */
    static void ScaleHudGemMeshes(const Vector3 *pScale, const char *pszKind);

    /**
     * Undo the scroll speed of the tunnel on the powerup gems of the head-up display.
     *
     * @param flScrollSpeed The scroll speed of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x001d3918
     * @ghidraAddress PAL: 0x001dc6b8
     */
    static void ScaleHudGems(float flScrollSpeed);

    /**
     * Run the script command `blend_to_path`.
     *
     * @param pCommand The command.
     * @param pUserData The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001df2c8
     * @ghidraAddress PAL: 0x001e8068
     */
    static void OnBlendToPath(DataArray *pCommand, void *pUserData);

    /**
     * Run the script command `arena_done_in`.
     *
     * @param pCommand The command.
     * @param pUserData The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001df448
     * @ghidraAddress PAL: 0x001e81e8
     */
    static void OnArenaDoneIn(DataArray *pCommand, void *pUserData);

    /**
     * Run the script command `set_particle_player_color`.
     *
     * @param pCommand The command.
     * @param pUserData The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001df578
     * @ghidraAddress PAL: 0x001e8318
     */
    static void OnSetParticlePlayerColor(DataArray *pCommand, void *pUserData);

    /**
     * Run the script command `gfx_debug`.
     *
     * @param pCommand The command.
     * @param pUserData The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001df6b8
     * @ghidraAddress PAL: 0x001e8458
     */
    static void OnGfxDebug(DataArray *pCommand, void *pUserData);

    /**
     * Report the bars of the track of a player through the debug output.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001df6d8
     * @ghidraAddress PAL: 0x001e8478
     */
    void PrintBars(int nPlayer);

    /**
     * Queue a panel to apply when its track is no longer captured or its reveal ends.
     *
     * @param pPanel The panel, which the tunnel then deletes.
     * @param bDeferred Whether the panel waits until its track is no longer captured.
     * @param flRevealTick The tick the reveal of the panel starts at.
     * @ghidraAddress NTSC-U/C: 0x001de288
     * @ghidraAddress PAL: 0x001e7028
     */
    void AddPanel(GfxTunnelPanel *pPanel, int bDeferred, float flRevealTick);

    /**
     * Report the graphics of a player.
     *
     * @param nPlayer The player.
     * @return The graphics.
     * @ghidraAddress NTSC-U/C: 0x001de468
     * @ghidraAddress PAL: 0x001e7208
     */
    TnlPlayer *GetPlayer(int nPlayer);

    /**
     * Report how far the arena constrains the freestyles at the frame of its camera.
     *
     * @return The constraint, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001de278
     * @ghidraAddress PAL: 0x001e7018
     */
    float GetFreestyleConstraint();

    /**
     * Report whether the bar of a track at a tick maps to the song and has no owner.
     *
     * @param nTrack The track.
     * @param flTick The tick.
     * @return Whether a bar is set at the tick, maps to the song, and has no owner.
     * @ghidraAddress NTSC-U/C: 0x001dd818
     * @ghidraAddress PAL: 0x001e65b8
     */
    bool HasActiveBar(int nTrack, float flTick);

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
     * @return The particle of the flash, or null when no flash effect was free.
     * @ghidraAddress NTSC-U/C: 0x001ddd00
     * @ghidraAddress PAL: 0x001e6aa0
     */
    Particle *Flash(const Vector3 *pPos, float flSize, float flGrowth);

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
    bool Burst(const std::vector<RndView *> *pViews,
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
     * Report whether a track is captured, for a track given as an int.
     *
     * @param nTrack The track.
     * @return Whether the effect of the track is in its captured state.
     * @ghidraAddress NTSC-U/C: 0x001de220
     * @ghidraAddress PAL: 0x001e6fc0
     */
    bool IsTrackCaptured(int nTrack) const;

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
     * Advance the tunnel and every effect it draws.
     *
     * The players of the tracks and the colours of the panels are refreshed first when marked
     * dirty. A track whose effect changed, or a track a crippler wave crossed, is added to
     * mChangedRange, which the gems, the cursors, the arrows, and the spotlights then refresh. The
     * light `tunnel track color.lit` of a solo game takes the colour of the track of its player.
     *
     * @param fTicks The ticks since the last poll.
     * @param fTime The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001de480
     * @ghidraAddress PAL: 0x001e7220
     */
    void Poll(float fTicks, float fTime);

    /**
     * Update the trail of the freestyle of each player.
     *
     * @param fTicks The ticks since the last poll, passed on to each player.
     * @param fTime The time since the last poll, passed on to each player.
     * @ghidraAddress NTSC-U/C: 0x001dec18
     * @ghidraAddress PAL: 0x001e79b8
     */
    void PollCameras(float fTicks, float fTime);

    /**
     * Set how far the letterbox is closed, and move the field of view of the outer cameras from
     * sCameraFov towards sLetterboxFov with it.
     *
     * The cameras are not changed when the value is the value already set.
     *
     * @param fLetterbox How far the letterbox is closed, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001dec88
     * @ghidraAddress PAL: 0x001e7a28
     */
    void SetLetterbox(float fLetterbox);

    /**
     * Draw the fires, the boundaries between the sections, and the panels of the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x001ded28
     * @ghidraAddress PAL: 0x001e7ac8
     */
    void DrawBackground();

    /**
     * Draw the gems, the grid, the transparent layer, the spotlights, the capture bursts, the
     * arrows of the tracks, the cripplers, and the beams of the ships.
     *
     * @ghidraAddress NTSC-U/C: 0x001dede8
     * @ghidraAddress PAL: 0x001e7b88
     */
    void DrawTracks();

    /**
     * Draw the overlays of the tracks and the freestyle of each player.
     *
     * @ghidraAddress NTSC-U/C: 0x001def00
     * @ghidraAddress PAL: 0x001e7ca0
     */
    void DrawCameras();

    /**
     * Draw the overlay camera, the arrows of the ships on each track, the fade of mFadeMesh, and
     * the shell of a slowdown.
     *
     * Once the fade shows, the vortex of the graphics manager draws after the head-up display.
     *
     * @ghidraAddress NTSC-U/C: 0x001def68
     * @ghidraAddress PAL: 0x001e7d08
     */
    void DrawForeground();

    /**
     * Start the fade of mFadeMesh from its current opacity over sFadeToBlackDuration.
     *
     * @param bToBlack Whether the fade ends opaque rather than clear.
     * @ghidraAddress NTSC-U/C: 0x001de028
     * @ghidraAddress PAL: 0x001e6dc8
     */
    void StartFade(bool bToBlack);

    /**
     * Refresh the grids, the cursors, the arrows, and the spotlights over mChangedRange.
     *
     * @ghidraAddress NTSC-U/C: 0x001df0e8
     * @ghidraAddress PAL: 0x001e7e88
     */
    void RefreshChangedRange();

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
     * Collect the ticks of the bars of a track a fire burns between two ticks.
     *
     * The collection starts at the bar of flFromTick. It takes the start of each bar set on the
     * track that maps to the song and has no owner, then the start of each bar after the last bar
     * set, until flToTick. It stops once the vector has no free capacity, and the vector never
     * grows.
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
     * @ghidraAddress NTSC-U/C: 0x001dde98
     * @ghidraAddress PAL: 0x001e6c38
     */
    void StartFire(int nPlayer,
                   int nTrack,
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
     * Report the use of the particles of the particle arms through the debug output.
     *
     * @ghidraAddress NTSC-U/C: 0x001df7f0
     * @ghidraAddress PAL: 0x001e8590
     */
    void ReportParticles();

    /**
     * Show the energy as the frame of `boundary_condition.view`, a thousand times the energy.
     *
     * @param flEnergy The energy, from 0 to 1.
     * @param nZone The zone of the energy. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001df810
     * @ghidraAddress PAL: 0x001e85b0
     */
    void SetEnergy(float flEnergy, int nZone);

    /**
     * Colour the track of a player for a multiplier, and for a local player show the text of the
     * next multiplier on the first arrow of the tracks.
     *
     * @param nPlayer The player.
     * @param nMultiplier The multiplier.
     * @param nNextMultiplier The multiplier of the next streak. The routine does not read it.
     * @param pszMultiplier The text of the multiplier. The routine does not read it.
     * @param pszNextMultiplier The text of the multiplier of the next streak.
     * @param bBoosted Whether a powerup boosts the multiplier. The routine does not read it.
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
     * Start the multiplier on the cursor of a player, unless the player has no graphics.
     *
     * @param nPlayer The player.
     * @param flTick The tick the multiplier ends at.
     * @ghidraAddress NTSC-U/C: 0x001df8b0
     * @ghidraAddress PAL: 0x001e8650
     */
    void SetMultiplierEndTick(int nPlayer, float flTick);

    /**
     * Start the shell of a slowdown, then the blur `slowdown_end` from flEndTick to flStopTick and
     * the blur `slowdown_start` from the song tick to flStartTick.
     *
     * @param nPlayer The player who deployed the slowdown. The routine does not read it.
     * @param flStartTick The tick the song arrives at the slow speed.
     * @param flEndTick The tick the song starts to speed up again.
     * @param flStopTick The tick the song is back at its speed.
     * @ghidraAddress NTSC-U/C: 0x001df8f8
     * @ghidraAddress PAL: 0x001e8698
     */
    void ShowSlowdown(int nPlayer, float flStartTick, float flEndTick, float flStopTick);

    /**
     * Start the first idle crippler, or the last crippler when every crippler is busy.
     *
     * @param nAttacker The attacking player.
     * @param nVictim The crippled player.
     * @ghidraAddress NTSC-U/C: 0x001df980
     * @ghidraAddress PAL: 0x001e8720
     */
    void ShowCripple(int nAttacker, int nVictim);

    /**
     * Tell the activators of two players of a bump, skipping a player with no graphics.
     *
     * @param nAttacker The attacking player.
     * @param nVictim The bumped player.
     * @param nTrack The track of the bump.
     * @ghidraAddress NTSC-U/C: 0x001dfa08
     * @ghidraAddress PAL: 0x001e87a8
     */
    void ShowBump(int nAttacker, int nVictim, int nTrack);

    /**
     * Take a player off the freestyle and move the player and the grid of the player to a track.
     *
     * In a remix the gems of the track of a local player fire hit events. Otherwise the focus
     * track of the gems is cleared when mDefaultFocusTrack is set.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nTrackType The type of the track. The routine does not read it.
     * @param nInstrument The instrument of the track. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001dfa98
     * @ghidraAddress PAL: 0x001e8838
     */
    void SetPlayerTrack(int nPlayer, int nTrack, int nTrackType, int nInstrument);

    /**
     * Start the freestyle of a player and take the player off its track.
     *
     * The gems of mDefaultFocusTrack, when set, fire hit events.
     *
     * @param nPlayer The player.
     * @param bVictory Whether the freestyle rewards a won song.
     * @param bQuick Whether the freestylus assembles in 4 ticks instead of the configured delay.
     * @ghidraAddress NTSC-U/C: 0x001dfb70
     * @ghidraAddress PAL: 0x001e8910
     */
    void SetPlayerFreestyle(int nPlayer, bool bVictory, bool bQuick);

    /**
     * Report the track the ship of the freestyle of a player is over.
     *
     * @param nPlayer The player.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x001dfbf0
     * @ghidraAddress PAL: 0x001e8990
     */
    int GetViewedTrack(int nPlayer);

    /**
     * Set the position of the stick of the freestyle of a player, unless the player has no
     * graphics.
     *
     * @param nPlayer The player.
     * @param stick The position.
     * @ghidraAddress NTSC-U/C: 0x001dfc18
     * @ghidraAddress PAL: 0x001e89b8
     */
    void SetFreestylePosition(int nPlayer, const Vector3 &stick);

    /**
     * Sweep the needle of the scratch freestyle of a player for a note, unless the player has no
     * graphics.
     *
     * @param nPlayer The player.
     * @param flTick The tick of the note.
     * @param flDuration The length of the note in ticks.
     * @ghidraAddress NTSC-U/C: 0x001dfc50
     * @ghidraAddress PAL: 0x001e89f0
     */
    void ShowScratchNote(int nPlayer, float flTick, float flDuration);

    /**
     * Record the players on a track, in order of place, for the next poll.
     *
     * @param nTrack The track.
     * @param nPlayers The number of players.
     * @param pPlayers The players.
     * @ghidraAddress NTSC-U/C: 0x001dfc98
     * @ghidraAddress PAL: 0x001e8a38
     */
    void SetTrackPlayers(int nTrack, int nPlayers, const int *pPlayers);

    /**
     * Place a gem on a track, unless it lies more than 960 ticks behind the song.
     *
     * A gem of a player takes the gem group of the player in a duel and on a pitch track of a
     * remix. Otherwise it takes the gem group of the track and flashes, in a game of several
     * players takes the colour of the player, and while the captures reveal the gems shows from
     * the end of the capture of the track or from the tick the head of a capture would arrive at
     * it. A powerup gem also places a spotlight. The cursors are told of each gem except a slot
     * and a gem of the track.
     *
     * @param nTrack The track.
     * @param nSlot The slot of the gem across the track.
     * @param nPlayer The player the gem is for, or -1.
     * @param flTick The tick of the gem.
     * @param bSlot Whether the gem only marks a slot, with the gem group mSlotGemGroup.
     * @param nPowerup One more than the PowerupGem of a gem of no player, or 0.
     * @ghidraAddress NTSC-U/C: 0x001dfd68
     * @ghidraAddress PAL: 0x001e8b08
     */
    void PlaceGem(int nTrack, int nSlot, int nPlayer, float flTick, int bSlot, int nPowerup);

    /**
     * Remove the gems of a tick and slot of a track, and tell the cursors when the tick is more
     * than 80 ticks ahead of the song.
     *
     * @param nTrack The track.
     * @param nSlot The slot of the gem across the track.
     * @param flTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e0010
     * @ghidraAddress PAL: 0x001e8db0
     */
    void RemoveGem(int nTrack, int nSlot, float flTick);

    /**
     * Remove the gems of a track over a span of ticks, and tell the cursors when the span ends
     * more than 80 ticks ahead of the song.
     *
     * An erase gives each gem of the span a removal tick that grows with its distance from the
     * song, and bursts each mesh gem through mGemErase.
     *
     * @param nTrack The track.
     * @param bErase Whether the gems burst away instead of vanishing at once.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001e00d0
     * @ghidraAddress PAL: 0x001e8e70
     */
    void ClearGems(int nTrack, bool bErase, float flStartTick, float flEndTick);

    /**
     * Hide the arrow of each track.
     *
     * @param bNow Whether the arrows vanish at once.
     * @ghidraAddress NTSC-U/C: 0x001e0338
     * @ghidraAddress PAL: 0x001e90d8
     */
    void HideNextPhrase(bool bNow);

    /**
     * Point the arrow of a track at the first gem of the next phrase, when the tracks have
     * arrows.
     *
     * @param nTrack The track.
     * @param flTick The tick of the gem.
     * @param nSlot The slot of the gem across the track.
     * @ghidraAddress NTSC-U/C: 0x001e03a0
     * @ghidraAddress PAL: 0x001e9140
     */
    void ShowNextPhrase(int nTrack, float flTick, int nSlot);

    /**
     * Set the energy of the cursor of a player.
     *
     * @param nPlayer The player.
     * @param flLevel The level of the catch meter.
     * @ghidraAddress NTSC-U/C: 0x001e03d8
     * @ghidraAddress PAL: 0x001e9178
     */
    void SetCatchMeter(int nPlayer, float flLevel);

    /**
     * Show the result of a gem on the activator of a player.
     *
     * A hit flashes the cell of the gem, pulses the cursor, and bursts the gem through a view of
     * nBurst, or in a duel through a view of the other player. A miss outside a remix places a
     * gem of mMissGemGroup when sShowMissGems is set.
     *
     * @param nTrack The track.
     * @param nSlot The slot of the gem across the track.
     * @param bHit Whether the gem was hit.
     * @param nPlayer The player.
     * @param nBurst The GemBurst of a hit.
     * @param flTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e0408
     * @ghidraAddress PAL: 0x001e91a8
     */
    void ShowGemResult(int nTrack, int nSlot, bool bHit, int nPlayer, int nBurst, float flTick);

    /**
     * Flash the cell of a gem with the flash of a pitched gem.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param nSlot The slot of the gem across the track.
     * @param flTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001e05c8
     * @ghidraAddress PAL: 0x001e9368
     */
    void HitGem(int nPlayer, int nTrack, int nSlot, float flTick);

    /**
     * Record whether a player plays the freestyle, and outside a remix whether the gems of the
     * focus track fire hit events.
     *
     * @param nPlayer The player.
     * @param bPlaying Whether the player plays.
     * @param nColumn The column of the freestyle.
     * @ghidraAddress NTSC-U/C: 0x001e0678
     * @ghidraAddress PAL: 0x001e9418
     */
    void SetFreestyle(int nPlayer, bool bPlaying, int nColumn);

    /**
     * Start a sample on the freestyle of a player.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001e06e0
     * @ghidraAddress PAL: 0x001e9480
     */
    void ResetFreestyle(int nPlayer);

    /**
     * Record the capture of a phrase of a track and start its effect, except in a duel.
     *
     * An automatic capture starts the capture effect of the track. Otherwise a fire burns from the
     * song tick to flEndTick in the colour of the instrument in a solo game, or of the player.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param bAuto Whether the capture is automatic.
     * @param bFreestyle Whether the fire is a fire of the freestyle.
     * @param flStartTick The tick the phrase starts at.
     * @param flEndTick The tick after the phrase.
     * @ghidraAddress NTSC-U/C: 0x001e0708
     * @ghidraAddress PAL: 0x001e94a8
     */
    void ShowCapture(
        int nPlayer, int nTrack, int bAuto, bool bFreestyle, float flStartTick, float flEndTick);

    /**
     * Show the result of the song.
     *
     * A duel fades to black, hides the head-up display, and goes idle unless the song tick is 0.
     * Otherwise the result ends after sWinSequenceDuration for a campaign win, plus
     * mUnlockDoneTicks for an unlock, and a campaign win plays the shell of the win while any
     * other result hides the head-up display.
     *
     * @param bWon Whether the song was won. The routine does not read it.
     * @param nPlayer The player. The routine does not read it.
     * @param bCampaignWin Whether a won song outside practice is shown.
     * @param bUnlocked Whether winning the song unlocked the path of the arena.
     * @ghidraAddress NTSC-U/C: 0x001e0820
     * @ghidraAddress PAL: 0x001e95c0
     */
    void ShowResult(bool bWon, int nPlayer, bool bCampaignWin, bool bUnlocked);

    /**
     * Start the boss journey, which ends mBossDoneTicks after the song tick.
     *
     * @ghidraAddress NTSC-U/C: 0x001e0908
     * @ghidraAddress PAL: 0x001e96a8
     */
    void StartBossJourney();

    /**
     * Point the cursor of a player at a phrase, or refresh the cursor.
     *
     * In a game of several players the colours of the panels are restored at the next poll.
     *
     * @param nPlayer The player.
     * @param nTrack The track.
     * @param bPlayable Whether the cursor targets the phrase, when sShowSeeker is set.
     * @param flStartTick The tick the phrase starts at.
     * @param flEndTick The tick after the phrase.
     * @param nStyle The style of the display, passed to TnlCursor::Refresh().
     * @param bSlide Whether the camera moves to the phrase. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001e0928
     * @ghidraAddress PAL: 0x001e96c8
     */
    void ShowPhrase(int nPlayer,
                    int nTrack,
                    bool bPlayable,
                    float flStartTick,
                    float flEndTick,
                    int nStyle,
                    bool bSlide);

    /**
     * Set the look of one bar of a track, adding the bar when it is not set, and show its panel.
     *
     * A bar that ends by tick 0 is marked kBarFlagBeforeSong. In a remix, or for a bar that does
     * not map to the song past the first two bars, a bar less than a bar after a boundary shows
     * the checkpoint, or the finish at the end of the arena, and a bar less than two bars after a
     * boundary is dropped instead of revealed.
     *
     * @param nTrack The track.
     * @param nPlayer The player. The routine passes it on to ApplyBar().
     * @param nOwner The player who owns the bar, or -1.
     * @param bInSong Whether the bar maps to a bar of the song.
     * @param bNow Whether the panel applies at once rather than through a reveal.
     * @param nFlags A set of BarFlag.
     * @param nInstrument The instrument whose colour the panel takes, or -1.
     * @param flTick The tick the bar starts at.
     * @param flTicks The length of the bar in ticks.
     * @ghidraAddress NTSC-U/C: 0x001e0a00
     * @ghidraAddress PAL: 0x001e97a0
     */
    void SetBar(int nTrack,
                int nPlayer,
                int nOwner,
                bool bInSong,
                bool bNow,
                int nFlags,
                signed char nInstrument,
                float flTick,
                float flTicks);

    /**
     * Darken the panel of one bar of a track by sDuelFailDarkening.
     *
     * @param nTrack The track.
     * @param flTick A tick of the bar.
     * @ghidraAddress NTSC-U/C: 0x001e0e88
     * @ghidraAddress PAL: 0x001e9c28
     */
    void DimBar(int nTrack, float flTick);

    /**
     * Start the effect that enables a track.
     *
     * @param nTrack The track.
     * @param bEnabled Whether the track is enabled. A false value does nothing.
     * @ghidraAddress NTSC-U/C: 0x001e0f40
     * @ghidraAddress PAL: 0x001e9ce0
     */
    void SetTrackEnabled(int nTrack, bool bEnabled);

    /**
     * Add a boundary between the sections of the song.
     *
     * @param pszLabel The text of the boundary, or null for the default text.
     * @param flTick The tick of the boundary.
     * @param flTextScale The scale of the text.
     * @ghidraAddress NTSC-U/C: 0x001e0f70
     * @ghidraAddress PAL: 0x001e9d10
     */
    void AddCheckpoint(const char *pszLabel, float flTick, float flTextScale);

    /**
     * Remove every boundary between the sections of the song.
     *
     * @ghidraAddress NTSC-U/C: 0x001e0f90
     * @ghidraAddress PAL: 0x001e9d30
     */
    void ClearCheckpoints();

    /**
     * Select the near or the far pull back of the camera.
     *
     * @param bOption Whether the far pull back is selected.
     * @ghidraAddress NTSC-U/C: 0x001e0fb0
     * @ghidraAddress PAL: 0x001e9d50
     */
    void SetOption(bool bOption);

    /**
     * Report whether the far pull back of the camera is selected.
     *
     * @return PlayerCamFX::mZoomedOut.
     * @ghidraAddress NTSC-U/C: 0x001e0fd0
     * @ghidraAddress PAL: 0x001e9d70
     */
    int GetOption();

    /**
     * Set the pull back of the camera.
     *
     * @param flZoom The pull back as a multiple of the far one, or 0 for the selected one.
     * @ghidraAddress NTSC-U/C: 0x001e0fe0
     * @ghidraAddress PAL: 0x001e9d80
     */
    void SetZoom(float flZoom);

    /**
     * Blend the camera towards the arena path or back over one second from the current blend.
     *
     * The name is inferred.
     *
     * @param bArena Blend towards the arena path.
     * @ghidraAddress NTSC-U/C: 0x001e1008
     */
    void BlendToArenaPath(bool bArena);

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
     * Reset the activator of each player and put every player at its track at once.
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
     * Divide the colour of each light of an environment by the scroll speed of the tunnel.
     *
     * @param pEnviron The environment.
     * @ghidraAddress NTSC-U/C: 0x001df218
     * @ghidraAddress PAL: 0x001e7fb8
     */
    static void ScaleLights(RndEnviron *pEnviron);

    /**
     * The tunnel of the song, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003af9d4
     */
    static GfxTunnel *sCurrent;

    /**
     * Whether each pad gets its own cameras, `splitscreen`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af9f4
     */
    static int sSplitScreen;

    /**
     * Whether the panels of the tracks show, `show_panels`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af9f8
     */
    static int sShowPanels;

    /**
     * Whether the boundaries between the sections show, `show_dividers`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af9fc
     */
    static int sShowDividers;

    /**
     * Whether the `tunnel` entry has the `invert` flag.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa00
     */
    static int sInvertTunnel;

    /**
     * Whether the `tunnel` entry has the `flip` flag, or the second shape of the `circular` cheat
     * is selected.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa04
     */
    static int sFlipTunnel;

    /**
     * Whether the `tunnel` entry has the `center` flag.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa08
     */
    static int sCenterTunnel;

    /**
     * `opaque_panels`, which is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa0c
     */
    static int sOpaquePanels;

    /**
     * `show_disabled_panels`, which is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa10
     */
    static int sShowDisabledPanels;

    /**
     * Whether the seekers show, `show_seeker`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa14
     */
    static int sShowSeeker;

    /**
     * `seeker_endcaps_always`, which is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa18
     */
    static int sSeekerEndcapsAlways;

    /**
     * `seeker_opacity`, which is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa1c
     */
    static float sSeekerOpacity;

    /**
     * Whether the targets of the activators show, `show_activator`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa20
     */
    static int sShowActivator;

    /**
     * Whether missed gems show, `show_miss_gems`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa24
     */
    static int sShowMissGems;

    /**
     * `tunnel_bars`, which BuildPanels() reads.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa28
     */
    static int sTunnelBars;

    /**
     * `tunnel_low_res_bars`, which BuildPanels() reads.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa2c
     */
    static int sTunnelLowResBars;

    /**
     * `use_slab`, which is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa30
     */
    static int sUseSlab;

    /**
     * `slab_depth` times the scroll speed of the tunnel, which BuildPanels() reads.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa34
     */
    static float sSlabDepth;

    /**
     * `slab_angle` in radians, which BuildPanels() reads.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa38
     */
    static float sSlabAngle;

    /**
     * The field of view of the cameras of the players, `camera_fov`.
     *
     * LoadConfig() converts it to radians. CreateMainView() loads it again and leaves it in
     * degrees.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa3c
     */
    static float sCameraFov;

    /**
     * The field of view of the outer cameras with the letterbox closed, `camera_letterbox_fov` in
     * radians.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa40
     */
    static float sLetterboxFov;

    /**
     * The milliseconds of one turn of a gem, `gem_rotate_period`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa44
     */
    static float sGemRotatePeriod;

    /**
     * The size of the flash of a hit gem, `gem_hit_flash_size` times the scroll speed of the
     * tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa48
     */
    static float sGemHitFlashSize;

    /**
     * The growth of the flash of a hit gem, the scroll speed of the tunnel divided by
     * `gem_hit_flash_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa4c
     */
    static float sGemHitFlashGrowth;

    /**
     * `gem_pitch_flash_size` times the scroll speed of the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa50
     */
    static float sGemPitchFlashSize;

    /**
     * The scroll speed of the tunnel divided by `gem_pitch_flash_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa54
     */
    static float sGemPitchFlashGrowth;

    /**
     * `win_sequence_duration`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa60
     */
    static float sWinSequenceDuration;

    /**
     * `fade_to_black_duration`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa64
     */
    static float sFadeToBlackDuration;

    /**
     * `preview_cam_change_time`, the milliseconds BlendToArenaPath() blends the path over.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa68
     */
    static float sPreviewCamChangeTime;

    /**
     * `duel_fail_darkening`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa6c
     */
    static float sDuelFailDarkening;

    /**
     * `mesh_gem_scale`, which BuildPanels() reads.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa70
     */
    static float sMeshGemScale;

    /**
     * The width of a panel, which BuildPanels() measures.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa78
     */
    static float sPanelWidth;

    /**
     * Set to 7 in each component and never read.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b310
     */
    static Vector3 sReserved310;

    /**
     * The offset of the boundaries between the sections, `boundary_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b320
     */
    static Vector3 sBoundaryOffset;

    /**
     * The colour of the seekers, `seeker_color`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b330
     */
    static Color sSeekerColor;

    /**
     * The depth range `arena_z` of the cameras of the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b340
     */
    static Vector2 sArenaZ;

    /**
     * The depth range `tnl_z` of the cameras of the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b348
     */
    static Vector2 sTnlZ;

    /**
     * The depth range `tnlz_z` of the cameras of the tunnel.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b350
     */
    static Vector2 sTnlzZ;

    /**
     * The brightness of `tunnel track color.lit` over the ticks of the song, rising over the 480
     * ticks before the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b358
     */
    static LinearInterpolator sTrackLightFade;

    /**
     * The cross section of the tunnel, `tunnel`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b378
     */
    static std::vector<Segment> sShape;

    /**
     * The cross section BuildPanels() fits to the tracks, a copy of sShape.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b388
     */
    static std::vector<Segment> sSegments;

    /**
     * The unit normal of each track in the cross section, which BuildPanels() measures from
     * sSegments.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b398
     */
    static std::vector<Vector2> sTrackPoints;

    /**
     * The scale of the capture bursts, which BuildPanels() measures.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b3c0
     */
    static Vector3 sBurstScale;

    /**
     * The scale of a quarter line of the grids, `grid_quarter_scale`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b3d0
     */
    static Vector3 sGridQuarterScale;

    /**
     * The scale of an eighth line of the grids, `grid_eighth_scale`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b3f0
     */
    static Vector3 sGridEighthScale;

    int mTrackPlayersDirty; /*!< Whether SetTrackPlayers() changed the players of a track. */
    int mPanelColorsDirty;  /*!< Whether the next poll restores the colours of the panels. */
    signed char mDefaultFocusTrack;      /*!< The track the gems focus on, or -1. */
    TnlDivider *mDivider;                /*!< The boundaries between the sections of the song. */
    ParticleArms *mArms;                 /*!< The particle arms, or null. */
    std::vector<TnlFire *> mFires;       /*!< The fires of captured tracks. */
    std::vector<TnlTrackFX *> mTrackFX;  /*!< The effects of each track. */
    std::vector<TnlFlashFX *> mFlashes;  /*!< The flashes of the gems, `gem_flash.ps`. */
    std::vector<TnlCripFX *> mCrippleFX; /*!< The crippler of each player. */
    TnlSlowdownFX *mSlowdown;            /*!< The shell of a slowdown, `slowdown_shell`. */
    SpotLightFX *mSpotLights;            /*!< The spotlights of the gems. */
    GemEraseFX *mGemErase;               /*!< The erased gems that burst later. */
    TnlWinShell *mWinShell;              /*!< The shell of a won song. */
    TnlBlurs *mBlurs;                    /*!< The blurs of the slowdowns. */
    std::vector<TnlGemCapFX *> mGemCaps; /*!< The capture bursts of the gems. */
    std::vector<TnlStreakGemFX *> mStreakGems; /*!< The arrow of each track, or none. */
    TnlGems *mGems;                            /*!< The gems. */
    TnlGeom *mGeom;                            /*!< The geometry of the tracks. */
    RndView *mTransparentView;                 /*!< `tnl transparent`. */
    RndView *mGridView;                        /*!< `grid1.view`. */
    GfxArena *mArena;                          /*!< The arena. */
    RndLight *mTrackColorLight;          /*!< `tunnel track color.lit` of a solo game, or null. */
    int mTrackColorPlayer;               /*!< The player mTrackColorLight follows, or -1. */
    RndMesh *mFadeMesh;                  /*!< `tunnel fade.mesh`. */
    LinearInterpolator mFadeCurve;       /*!< The fade of mFadeMesh. */
    std::vector<TnlPlayer *> mPlayers;   /*!< The graphics of each player. */
    alignas(16) Color mSavedClearColor;  /*!< The clear colour of the renderer to restore. */
    std::list<GfxTunnelPanel *> mPanels; /*!< The panels waiting to apply. */
    std::vector<signed char> mInstrumentGemGroups; /*!< The sprite gem group of each instrument. */
    std::vector<signed char> mTrackGemGroups;      /*!< The sprite gem group of each track. */
    std::vector<signed char> mPlayerGemGroups;     /*!< The gem group `gem_<c>` of each player. */
    std::vector<signed char> mPowerupGemGroups;    /*!< The gem group of each PowerupGem. */
    signed char mMissGemGroup;                     /*!< The gem group `gem_miss`. */
    signed char mSlotGemGroup;      /*!< The gem group `gem_slot`, or `gem_duel_slot` in a duel. */
    signed char mGemGroup;          /*!< The gem group `gem_b`, or `monkey` with the cheat. */
    std::vector<TrackData> mTracks; /*!< The state of each track. */
    signed char mCrippledTracks;    /*!< One bit for each track whose player is crippled. */
    int mNumTracks;                 /*!< The number of tracks. */
    std::vector<Color> mInstrumentColors; /*!< The colour of each instrument. */
    std::vector<Color> mTrackColors;      /*!< The colour of each track. */
    RndMat *mCheckpointMat;               /*!< `panel ckpt.mat`. */
    RndMat *mCheckpointFinishMat;         /*!< `panel ckpt finish.mat`. */
    RndCam *mOverlayCam;                  /*!< `tnl overlay.cam`. */
    /*!< The cameras BuildOuterCams() fits, five for each player. */
    std::vector<RndCam *> mOuterCams;
    float mLetterbox; /*!< How far the letterbox is closed, from 0 to 1. */
    /*!< The views of the capture bursts of each player. */
    std::vector<std::vector<RndView *>> mPlayerBurstViews;
    /*!< The views of the bursts of each GemBurst. */
    std::vector<std::vector<RndView *>> mGemBurstViews;
    /*!< Whether the gems and the panels react to the captures of the tracks. Always set. */
    int mCaptureReveals;
    TnlTrackRange mChangedRange; /*!< The tracks and ticks the last poll changed. */
    int mBossJourneyDone;        /*!< Whether the boss journey has ended. */
    int mIdle;                   /*!< Whether no effect of the tunnel runs. */
    float mUnlockDoneTicks;      /*!< The extra ticks of a result that unlocks a path, `unlock`. */
    float mBossDoneTicks;        /*!< The ticks the boss journey lasts, `boss`. */
    float mResultDoneTick;       /*!< The tick the result of the song ends at. */
    float mBossJourneyDoneTick;  /*!< The tick the boss journey ends at. */
    PlayerCamFX *mCamFX;         /*!< The camera of the players. */
    ATanInterpolator mPathBlendCurve; /*!< The blend of the camera towards the arena path. */
    int mPathBlending;                /*!< Whether mPathBlendCurve runs. */
    alignas(16) Vector3 mCenter;      /*!< The centre of the cross section of the tracks. */

private:
    /**
     * Give the panel of a bar its look, at once or through a queued reveal, and refresh the grids
     * on the track.
     *
     * A bar that ended more than 960 ticks before the song is ignored. The panel of an owned bar
     * is revealed while the captures reveal the panels, the reveal is not bNow, and the song has
     * started. The reveal starts at the song tick and waits for the track while the track is
     * captured, or else starts at the tick the head of a capture would arrive at the bar.
     *
     * @param pTrack The track.
     * @param pBar The bar.
     * @param nPlayer The player. The routine does not read it.
     * @param bNow Whether the panel applies at once.
     * @param pColor The colour of the panel, or null.
     * @ghidraAddress NTSC-U/C: 0x001e0cd8
     * @ghidraAddress PAL: 0x001e9a78
     */
    void
    ApplyBar(TrackData *pTrack, const BarData *pBar, int nPlayer, bool bNow, const Color *pColor);
};
