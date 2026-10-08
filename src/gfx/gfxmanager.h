#pragma once

#include <list>
#include <vector>

#include "app/overlay.h"
#include "gfx/gfxarena.h"
#include "gfx/gfxtunnel.h"
#include "gfx/ship.h"
#include "gfx/vortex.h"
#include "math/color.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/asyncstream.h"
#include "os/prnstream.h"
#include "os/string.h"
#include "rnd/rndloader.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Display of the play field and the heads-up display of every player.
 *
 * The RTTI records the class as deriving from PrnStream. The object is 0xd4 bytes, and the one
 * instance is TheGfxManager. The display owns the arena, the tunnel of tracks, the ship of each
 * player, and the head-up display, and most of its routines forward to them.
 */
class GfxManager : public PrnStream {
public:
    /** The values of Poll() that WorldMgr tests. */
    enum PollResult {
        kPollWorldReady = 1, /*!< The display is ready for a loading world to load its assets. */
        kPollUnloaded = 2,   /*!< The display finished changing back after an unload. */
    };

    /** The values of mState. */
    enum State {
        kStateIdle = 0,      /*!< No world is loaded. */
        kStateLoading = 1,   /*!< Load() started to load a world. */
        kStatePlaying = 2,   /*!< BuildTracks() built the tracks of the song. */
        kStateUnloading = 3, /*!< Unload() started to change the display back. */
    };

    /** The values of mLoadStage. */
    enum LoadStage {
        kLoadStageConfig = 0,  /*!< The head-up display has not read its configuration. */
        kLoadStageHud = 1,     /*!< The file of the head-up display loads. */
        kLoadStageObjects = 2, /*!< The head-up display and the ships are built. */
    };

    /** The number of ships the display holds, one for each player. */
    static constexpr int kMaxPlayers = 4;

    /** The instruments of the tracks, which scripts know as `kDrum` through `kFX`. */
    enum Instrument {
        kInstrumentDrum = 0,   /*!< `drum`. */
        kInstrumentBass = 1,   /*!< `bass`. */
        kInstrumentSynth = 2,  /*!< `synth`. */
        kInstrumentGuitar = 3, /*!< `guitar`. */
        kInstrumentVocal = 4,  /*!< `vocal`. */
        kInstrumentFx = 5,     /*!< `fx`. */
        kNumInstruments = 6,   /*!< The number of instruments. */
    };

    /** The colours of the player slots. */
    enum PlayerColor {
        kPlayerColorGreen = 0,   /*!< `green`. */
        kPlayerColorPurple = 1,  /*!< `purple`. */
        kPlayerColorRed = 2,     /*!< `red`. */
        kPlayerColorYellow = 3,  /*!< `yellow`. */
        kPlayerColorNull = 4,    /*!< `null`. */
        kPlayerColorUnknown = 5, /*!< `unknown`. */
        kNumPlayerColors = 6,    /*!< The number of colours. */
    };

    /** What SetPendingPointsResult() shows happened to a player's pending points. */
    enum PendingPointsResult {
        kPendingPointsCleared = 0,  /*!< The pending points were reset. */
        kPendingPointsCaptured = 1, /*!< The pending points were added to the score. */
        kPendingPointsLost = 2,     /*!< The pending points were forfeited. */
    };

    /**
     * Sub-panel of a player's remix menu.
     *
     * The RTTI includes the name as an argument of a command template. The names of the effect
     * panels follow their scene objects `pat`, `chs`, `stt`, `eco`, `bot`, and `bpm`.
     */
    enum RemixSubPanel {
        kRemixSubPanelMain = 0,    /*!< The list of remix panels. */
        kRemixSubPanelPattern = 1, /*!< The `pat` panel, which OvyRemixPanel never builds. */
        kRemixSubPanelChorus = 2,  /*!< The chorus effect. */
        kRemixSubPanelStutter = 3, /*!< The stutter effect. */
        kRemixSubPanelEcho = 4,    /*!< The echo effect. */
        kRemixSubPanelBoot = 5,    /*!< The list of the players the host can remove. */
        kRemixSubPanelTempo = 6,   /*!< The tempo control. */
    };

    /**
     * Loader of one file of the `load` list of the `gfx` configuration.
     *
     * The RTTI names the class. The object is 0x28 bytes. A file object that already exists is
     * loaded again only when force_merge lists it. A skipped object that is not a texture, with
     * merge_all clear, is reported once the file has loaded unless the list
     * `merges_always_allowed` includes it.
     */
    class GfxLoader : public RndLoader::Callback {
    public:
        /**
         * Construct a loader that has not started.
         *
         * @param bDone Whether the load counts as finished.
         * @param bMergeAll Whether every object that already exists is loaded again.
         * @param pForceMerge The objects that are loaded again, or null.
         * @ghidraAddress NTSC-U/C: 0x001b12d0
         * @ghidraAddress PAL: 0x001ba070
         */
        GfxLoader(bool bDone, bool bMergeAll, DataArray *pForceMerge);

        /**
         * Destroy the loader.
         *
         * @ghidraAddress NTSC-U/C: 0x001b1360
         */
        ~GfxLoader() override;

        /**
         * Decide whether the file loads an object.
         *
         * @param pExisting The object of the same name that already exists, or null.
         * @param pszName The name of the object.
         * @param pszClass The class of the object.
         * @return 1 to load the object, or 0 to keep the one that exists.
         * @ghidraAddress NTSC-U/C: 0x001b13c0
         * @ghidraAddress PAL: 0x001ba100
         */
        int ShouldLoad(Rnd::Object *pExisting, const char *pszName, const char *pszClass) override;

        /**
         * Start to load a file.
         *
         * @param pszFile The file.
         * @param nFlags The load flags of Rnd::Manager::AddLoader().
         * @ghidraAddress NTSC-U/C: 0x001b1328
         * @ghidraAddress PAL: 0x001ba0c8
         */
        void Start(const char *pszFile, int nFlags);

        /**
         * Mark the load as finished and report the objects that were skipped.
         *
         * @ghidraAddress NTSC-U/C: 0x001b14e8
         * @ghidraAddress PAL: 0x001ba288
         */
        void Finish();

        String mSkipped;        /*!< The lines of the report of the skipped objects. */
        RndLoader *mLoader;     /*!< The loader of the file, or null. */
        DataArray *mForceMerge; /*!< The objects that are loaded again, or null. */
        int mDone;              /*!< Whether Finish() ran. */
        int mMergeAll;          /*!< Whether every object that already exists loads again. */
    };

    /**
     * Views of the camera of one player, in the order they draw.
     *
     * The RTTI names the type. The views are `<view><n> arena`, `<view><n>`, `<view><n> part2`,
     * `<view><n> part3`, and `<view><n> part4` for the `localview` name of the configuration.
     */
    struct LocalViews {
        Rnd::View *mArena; /*!< The view of the arena. */
        Rnd::View *mMain;  /*!< The main view. */
        Rnd::View *mPart2; /*!< The view drawn before the tracks. */
        Rnd::View *mPart3; /*!< The view drawn before the cameras of the players. */
        Rnd::View *mPart4; /*!< The view drawn before the layer in front of the tracks. */
    };

    /**
     * Message the display lists for debugging.
     *
     * The RTTI names the type. The display only empties its two lists of them.
     */
    struct DebugMsg {
        int mReserved00; // +0x00, not yet identified.
        int mReserved04; // +0x04, not yet identified.
        String mText;    /*!< The text. */
        int mReserved1C; // +0x1c, not yet identified.
    };

    /**
     * Construct an idle display.
     *
     * @ghidraAddress NTSC-U/C: 0x001b1550
     * @ghidraAddress PAL: 0x001ba2f0
     */
    GfxManager();

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

    /**
     * Load the cache of textures, register the display script commands, and read the
     * configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x001b1cc0
     * @ghidraAddress PAL: 0x001baa60
     */
    void Init();

    /**
     * Tear down a loaded world, destroy the vortex, and unregister what Init() registered.
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
     * Start to load the files of the display for a world.
     *
     * @param fTime The time the display starts at. WorldMgr passes -1.
     * @ghidraAddress NTSC-U/C: 0x001b2068
     * @ghidraAddress PAL: 0x001bae08
     */
    void Load(float fTime);

    /**
     * Build the display of the tracks of a song.
     *
     * @param fStartTick The tick the song starts at.
     * @param fEndTick The tick the song ends at.
     * @param pMsPerTick The milliseconds per tick of the song clock.
     * @param instruments The instrument of each track.
     * @param trackTypes The type of each track.
     * @param nOption The option of the tracks.
     * @param unlocked One flag for each path of the song, set for the unlocked ones.
     * @ghidraAddress NTSC-U/C: 0x001b2688
     * @ghidraAddress PAL: 0x001bb428
     */
    void BuildTracks(float fStartTick,
                     float fEndTick,
                     const float *pMsPerTick,
                     const std::vector<int> &instruments,
                     const std::vector<int> &trackTypes,
                     int nOption,
                     const std::vector<bool> &unlocked);

    /**
     * Start changing the display back after the world was destroyed.
     *
     * @ghidraAddress NTSC-U/C: 0x001b3458
     * @ghidraAddress PAL: 0x001bc1f8
     */
    void Unload();

    /**
     * Advance the display.
     *
     * @param fTime The time of the display.
     * @return One of PollResult, or 0.
     * @ghidraAddress NTSC-U/C: 0x001b37f8
     * @ghidraAddress PAL: 0x001bc598
     */
    int Poll(float fTime);

    /**
     * Draw the display.
     *
     * @param fTick The running world's position in ticks, or 0 when no world is running. The
     *              display does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b3d10
     * @ghidraAddress PAL: 0x001bcab0
     */
    void Draw(float fTick);

    /**
     * Start the run of the vortex into the play field.
     *
     * @param fLength The length of the run in milliseconds, or -1 for the configured length.
     * @return The configured length of the run, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001b34c8
     * @ghidraAddress PAL: 0x001bc268
     */
    float StartIntro(float fLength);

    /**
     * Report whether the run StartIntro() began has ended.
     *
     * @return True once the run has ended.
     * @ghidraAddress NTSC-U/C: 0x001b3508
     * @ghidraAddress PAL: 0x001bc2a8
     */
    bool IsIntroFinished() const;

    /**
     * Start the run of the vortex out of the play field.
     *
     * @return The configured length of the run, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001b3520
     * @ghidraAddress PAL: 0x001bc2c0
     */
    float RestartIntro();

    /**
     * Start the run of the vortex at the end of a song.
     *
     * @param fLength The length of the run in milliseconds, or a negative value for the
     *                configured length.
     * @return The configured length of the run, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001b3560
     * @ghidraAddress PAL: 0x001bc300
     */
    float StartOutro(float fLength);

    /**
     * Report whether the run StartOutro() started is done.
     *
     * @return Whether the run is done.
     * @ghidraAddress NTSC-U/C: 0x001b35a8
     * @ghidraAddress PAL: 0x001bc348
     */
    bool IsOutroDone();

    /**
     * Queue a file to stream before the world loads.
     *
     * @param pszFile The file.
     * @ghidraAddress NTSC-U/C: 0x001b42f8
     * @ghidraAddress PAL: 0x001bd098
     */
    void AddPreload(const char *pszFile);

    /**
     * Find the stream of a queued file and wait for it to finish.
     *
     * @param pszFile The file.
     * @return The stream, or null with a warning when the file is not queued.
     * @ghidraAddress NTSC-U/C: 0x001b43b8
     * @ghidraAddress PAL: 0x001bd158
     */
    AsyncStream *FindPreload(const char *pszFile);

    /**
     * Report the instrument of a track.
     *
     * @param nTrack The track.
     * @return The instrument.
     * @ghidraAddress NTSC-U/C: 0x001b42b8
     * @ghidraAddress PAL: 0x001bd058
     */
    int GetTrackInstrument(int nTrack);

    /**
     * Report the type of a track.
     *
     * @param nTrack The track.
     * @return The type.
     * @ghidraAddress NTSC-U/C: 0x001b42d8
     * @ghidraAddress PAL: 0x001bd078
     */
    int GetTrackType(int nTrack);

    /**
     * Run the arena effect of a beat event of the song.
     *
     * @param nEvent The event.
     * @ghidraAddress NTSC-U/C: 0x001b4290
     * @ghidraAddress PAL: 0x001bd030
     */
    void HandleBeat(signed char nEvent);

    /**
     * Start the journey to the boss arena that follows the boss unlock. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001b56b0
     * @ghidraAddress PAL: 0x001be450
     */
    void StartBossJourney();

    /**
     * Report whether the journey StartBossJourney() started is done. The name is inferred.
     *
     * @return Non-zero once the journey is done.
     * @ghidraAddress NTSC-U/C: 0x001b56f8
     * @ghidraAddress PAL: 0x001be498
     */
    int IsBossJourneyDone();

    /**
     * Report whether the tunnel finished its effects.
     *
     * @return True when the tunnel is idle.
     * @ghidraAddress NTSC-U/C: 0x001b56e8
     * @ghidraAddress PAL: 0x001be488
     */
    bool IsIdle();

    /**
     * Show the energy of the play field of a solo game.
     *
     * A game that is not solo shows nothing. Practice shows a full meter.
     *
     * @param nPlayer The player's index.
     * @param fEnergy The energy, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001b4d68
     * @ghidraAddress PAL: 0x001bdb08
     */
    void SetEnergy(int nPlayer, float fEnergy);

    /**
     * Show whether a player runs out of energy.
     *
     * @param nPlayer The player's index.
     * @param bDying Whether the player runs out of energy.
     * @ghidraAddress NTSC-U/C: 0x001b4f40
     * @ghidraAddress PAL: 0x001bdce0
     */
    void SetDying(int nPlayer, bool bDying);

    /**
     * Show a player's score.
     *
     * @param nPlayer The player's index.
     * @param nScore The new score.
     * @param nMultiplier The streak multiplier the points were scored at. The display does not
     *                    read it.
     * @param nPoints The points just added, or 0. The display does not read them.
     * @ghidraAddress NTSC-U/C: 0x001b4f90
     * @ghidraAddress PAL: 0x001bdd30
     */
    void SetScore(int nPlayer, int nScore, int nMultiplier, int nPoints);

    /**
     * Show a player's streak multiplier and the one the next capture earns.
     *
     * The texts of the multipliers are built only for a player on this console, and only for a
     * multiplier above 1.
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
     * Mark whether a player leads the score.
     *
     * @param nPlayer The player's index.
     * @param bLeader Whether the player leads.
     * @ghidraAddress NTSC-U/C: 0x001b5190
     * @ghidraAddress PAL: 0x001bdf30
     */
    void SetLeader(int nPlayer, bool bLeader);

    /**
     * Show or hide the position in the song.
     *
     * @param bShow Whether the position is shown.
     * @ghidraAddress NTSC-U/C: 0x001b51d8
     * @ghidraAddress PAL: 0x001bdf78
     */
    void ShowSongPos(bool bShow);

    /**
     * Show or hide the labels of the tracks.
     *
     * @param bShow Whether the labels are shown.
     * @ghidraAddress NTSC-U/C: 0x001b51f8
     * @ghidraAddress PAL: 0x001bdf98
     */
    void ShowTrackLabels(bool bShow);

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
     * Show the lyric text.
     *
     * @param pszText The text.
     * @param bFirstLine Show the text on the first line.
     * @ghidraAddress NTSC-U/C: 0x001b5340
     * @ghidraAddress PAL: 0x001be0e0
     */
    void SetLyricText(const char *pszText, bool bFirstLine);

    /**
     * Fly the controller button icon of a lane across the screen.
     *
     * @param nLane The lane.
     * @param pPosition The position of the icon on the screen.
     * @param pTarget The position the icon moves to.
     * @ghidraAddress NTSC-U/C: 0x001b5360
     * @ghidraAddress PAL: 0x001be100
     */
    void ShowButtonIcon(int nLane, const Vector2 *pPosition, const Vector2 *pTarget);

    /**
     * Highlight the controller button icon of a lane, or clear the highlight.
     *
     * @param nLane The lane.
     * @param nHighlight Nonzero to highlight the icon.
     * @ghidraAddress NTSC-U/C: 0x001b5380
     * @ghidraAddress PAL: 0x001be120
     */
    void SetButtonIconHighlight(int nLane, int nHighlight);

    /**
     * Hide the controller button icon of a lane.
     *
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x001b53a0
     * @ghidraAddress PAL: 0x001be140
     */
    void HideButtonIcon(int nLane);

    /**
     * Set the glyph the controller button icon of a lane shows.
     *
     * @param nLane The lane.
     * @param pszGlyph The glyph of the button, or null.
     * @ghidraAddress NTSC-U/C: 0x001b53c0
     * @ghidraAddress PAL: 0x001be160
     */
    void SetButtonIconGlyph(int nLane, const char *pszGlyph);

    /**
     * Show or hide the diagram of the controller.
     *
     * @param bShow Whether the diagram is shown.
     * @param fPosition The position of the diagram, or -1 for the configured one.
     * @ghidraAddress NTSC-U/C: 0x001b53e0
     * @ghidraAddress PAL: 0x001be180
     */
    void ShowController(bool bShow, float fPosition);

    /**
     * Highlight the diagram of the controller, or clear the highlight.
     *
     * @param bHighlight Whether the diagram is highlighted.
     * @ghidraAddress NTSC-U/C: 0x001b5400
     * @ghidraAddress PAL: 0x001be1a0
     */
    void SetControllerHighlight(bool bHighlight);

    /**
     * Show or hide one button of the diagram of the controller.
     *
     * @param nButton The button.
     * @param bShow Whether the button is shown.
     * @ghidraAddress NTSC-U/C: 0x001b5420
     * @ghidraAddress PAL: 0x001be1c0
     */
    void ShowControllerButton(int nButton, bool bShow);

    /**
     * Open the dialog box with a text.
     *
     * @param pszText The text.
     * @param fSize The size of the box.
     * @param nPosition The position of the box, or -1 for the configured one.
     * @ghidraAddress NTSC-U/C: 0x001b5440
     * @ghidraAddress PAL: 0x001be1e0
     */
    void OpenDialog(const char *pszText, float fSize, int nPosition);

    /**
     * Close the dialog box.
     *
     * @ghidraAddress NTSC-U/C: 0x001b5468
     * @ghidraAddress PAL: 0x001be208
     */
    void CloseDialog();

    /**
     * Close or open the letterbox.
     *
     * @param bClosed Whether the letterbox closes.
     * @ghidraAddress NTSC-U/C: 0x001b5488
     * @ghidraAddress PAL: 0x001be228
     */
    void SetLetterbox(bool bClosed);

    /**
     * Show or hide the highlight box.
     *
     * @param bShow Whether the box is shown.
     * @ghidraAddress NTSC-U/C: 0x001b54a8
     * @ghidraAddress PAL: 0x001be248
     */
    void ShowBox(bool bShow);

    /**
     * Show or hide the highlight arrow.
     *
     * @param bShow Whether the arrow is shown.
     * @ghidraAddress NTSC-U/C: 0x001b54c8
     * @ghidraAddress PAL: 0x001be268
     */
    void ShowArrow(bool bShow);

    /**
     * Move the highlight box.
     *
     * @param fX0 The left edge.
     * @param fY0 The top edge.
     * @param fX1 The right edge.
     * @param fY1 The bottom edge.
     * @param fDuration The time the move takes.
     * @ghidraAddress NTSC-U/C: 0x001b54e8
     * @ghidraAddress PAL: 0x001be288
     */
    void SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration);

    /**
     * Move the highlight arrow.
     *
     * @param fX The horizontal position.
     * @param fY The vertical position.
     * @param fAngle The angle of the arrow.
     * @param fDuration The time the move takes.
     * @ghidraAddress NTSC-U/C: 0x001b5508
     * @ghidraAddress PAL: 0x001be2a8
     */
    void SetArrowTarget(float fX, float fY, float fAngle, float fDuration);

    /**
     * Show the diagram of a stick.
     *
     * @param nDirection The direction the diagram shows.
     * @param pPosition The position of the diagram.
     * @ghidraAddress NTSC-U/C: 0x001b5528
     * @ghidraAddress PAL: 0x001be2c8
     */
    void ShowStick(int nDirection, const Vector2 *pPosition);

    /**
     * Hide the diagram of a stick.
     *
     * @ghidraAddress NTSC-U/C: 0x001b5548
     * @ghidraAddress PAL: 0x001be2e8
     */
    void HideStick();

    /**
     * Set the number of sections of the song the remix display lists.
     *
     * @param nCount The number of sections.
     * @ghidraAddress NTSC-U/C: 0x001b5568
     * @ghidraAddress PAL: 0x001be308
     */
    void SetSectionCount(int nCount);

    /**
     * Set the label of a section of the song.
     *
     * @param nIndex The section.
     * @param pszLabel The label.
     * @ghidraAddress NTSC-U/C: 0x001b5588
     * @ghidraAddress PAL: 0x001be328
     */
    void SetSectionLabel(int nIndex, const char *pszLabel);

    /**
     * Show or hide the sections of the song.
     *
     * @param bShow Whether the sections are shown.
     * @ghidraAddress NTSC-U/C: 0x001b55a8
     * @ghidraAddress PAL: 0x001be348
     */
    void ShowSections(bool bShow);

    /**
     * Highlight a section of the song.
     *
     * @param nIndex The section.
     * @ghidraAddress NTSC-U/C: 0x001b55c8
     * @ghidraAddress PAL: 0x001be368
     */
    void HighlightSection(int nIndex);

    /**
     * Set the text of an entry of a remix sub-panel.
     *
     * @param nPlayer The player's index.
     * @param nIndex The entry.
     * @param pszText The text.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001b55e8
     * @ghidraAddress PAL: 0x001be388
     */
    void SetRemixPanelText(int nPlayer, int nIndex, const char *pszText, RemixSubPanel ePanel);

    /**
     * Move the cursor of a remix sub-panel to an entry.
     *
     * @param nPlayer The player's index.
     * @param nIndex The entry, or -1 for none.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001b5608
     * @ghidraAddress PAL: 0x001be3a8
     */
    void SelectRemixPanelEntry(int nPlayer, int nIndex, RemixSubPanel ePanel);

    /**
     * Flash the cursor of a remix sub-panel to confirm a choice.
     *
     * @param nPlayer The player's index.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001b5630
     * @ghidraAddress PAL: 0x001be3d0
     */
    void FlashRemixPanel(int nPlayer, RemixSubPanel ePanel);

    /**
     * Light or darken an entry of a remix sub-panel.
     *
     * @param nPlayer The player's index.
     * @param nIndex The entry.
     * @param nLit Non-zero to light the entry.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001b5650
     * @ghidraAddress PAL: 0x001be3f0
     */
    void SetRemixPanelLit(int nPlayer, int nIndex, int nLit, RemixSubPanel ePanel);

    /**
     * Show one sub-panel of a player's remix menu.
     *
     * @param nPlayer The player's index.
     * @param ePanel The sub-panel.
     * @ghidraAddress NTSC-U/C: 0x001b5670
     * @ghidraAddress PAL: 0x001be410
     */
    void ShowRemixPanel(int nPlayer, RemixSubPanel ePanel);

    /**
     * Show or hide a player's remix menu.
     *
     * @param nPlayer The player's index.
     * @param bShow Whether the menu is shown.
     * @ghidraAddress NTSC-U/C: 0x001b5690
     * @ghidraAddress PAL: 0x001be430
     */
    void ShowRemix(int nPlayer, bool bShow);

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
     * Mark the winner of a song.
     *
     * @param nPlayer The winner's index, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x001b5728
     * @ghidraAddress PAL: 0x001be4c8
     */
    void SetWinner(int nPlayer);

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
     * Move a player to a track.
     *
     * @param nPlayer The player's index.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001b57b0
     * @ghidraAddress PAL: 0x001be550
     */
    void ShowPlayerOnTrack(int nPlayer, int nTrack);

    /**
     * Show the instrument a track changed to, on the track and on every player on it.
     *
     * @param nTrack The track.
     * @param nInstrument The instrument.
     * @ghidraAddress NTSC-U/C: 0x001b5878
     * @ghidraAddress PAL: 0x001be618
     */
    void SetTrackInstrument(int nTrack, int nInstrument);

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
     * Report the track the camera of a player shows.
     *
     * @param nPlayer The player's index.
     * @return The track.
     * @ghidraAddress NTSC-U/C: 0x001b59f8
     * @ghidraAddress PAL: 0x001be798
     */
    int GetViewedTrack(int nPlayer);

    /**
     * Move a player's freestyle effect to a position of the first stick.
     *
     * @param nPlayer The player index.
     * @param position The stick position in the horizontal and depth components.
     * @ghidraAddress NTSC-U/C: 0x001b5a30
     * @ghidraAddress PAL: 0x001be7d0
     */
    void SetFreestylePosition(int nPlayer, const Vector3 &position);

    /**
     * Ignore a position of the second stick for a player's freestyle effect.
     *
     * The body is empty.
     *
     * @param nPlayer The player index.
     * @param position The stick position in the horizontal and depth components.
     * @ghidraAddress NTSC-U/C: 0x001b5a50
     * @ghidraAddress PAL: 0x001be7f0
     */
    void SetFreestyleSecondPosition([[maybe_unused]] int nPlayer,
                                    [[maybe_unused]] const Vector3 &position) {
    }

    /**
     * Show a note a scratch plays on a player's track.
     *
     * @param nPlayer The player's index.
     * @param fTick The song tick of the note.
     * @param fDuration The length of the note in ticks.
     * @ghidraAddress NTSC-U/C: 0x001b5a58
     * @ghidraAddress PAL: 0x001be7f8
     */
    void ShowScratchNote(int nPlayer, float fTick, float fDuration);

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
     * Take a gem off the play field.
     *
     * @param nTrack The track.
     * @param nSlot The gem button the gem lies under.
     * @param fTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001b5a98
     * @ghidraAddress PAL: 0x001be838
     */
    void RemoveGem(int nTrack, int nSlot, float fTick);

    /**
     * Remove the gems of a track in a range of ticks.
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
     * Set the meter that shows how much of a phrase a player has caught.
     *
     * @param nPlayer The player's index.
     * @param fLevel The level, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001b5b20
     * @ghidraAddress PAL: 0x001be8c0
     */
    void SetCatchMeter(int nPlayer, float fLevel);

    /**
     * Show the result of a gem a player played or removed.
     *
     * The triggers fire for the leader, and for every player outside a game.
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
     * Show a player catching a phrase.
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
     * Rank the score bars at the end of a section and, in a solo game, fire the triggers that
     * wait on it.
     *
     * @param nPlayer The player whose completion the triggers receive.
     * @param nSection The section. The display does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b5db0
     * @ghidraAddress PAL: 0x001beb50
     */
    void CompleteStage(int nPlayer, int nSection);

    /**
     * Show the result of a song.
     *
     * @param bWon Whether the song was won.
     * @param nPlayer The player.
     * @param bUnlocked Whether winning the song unlocked a new one.
     * @ghidraAddress NTSC-U/C: 0x001b5e00
     * @ghidraAddress PAL: 0x001beba0
     */
    void ShowResult(bool bWon, int nPlayer, bool bUnlocked);

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
     * Dim one bar of a track.
     *
     * @param nTrack The track.
     * @param fTick The tick the bar starts at.
     * @ghidraAddress NTSC-U/C: 0x001b5f58
     * @ghidraAddress PAL: 0x001becf8
     */
    void DimBar(int nTrack, float fTick);

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
     * Place a checkpoint on the tracks and on the position in the song.
     *
     * The position in the song is the fraction of the song the tick makes, or fOffset when the
     * arena does not know the length of the song.
     *
     * @param pszLabel The label of the checkpoint, or null.
     * @param fTick The tick of the checkpoint.
     * @param fOffset The position in the song used when the length of the song is unknown.
     * @param fScale The scale of the checkpoint.
     * @ghidraAddress NTSC-U/C: 0x001b5f98
     * @ghidraAddress PAL: 0x001bed38
     */
    void AddCheckpoint(const char *pszLabel, float fTick, float fOffset, float fScale);

    /**
     * Remove every checkpoint.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6030
     * @ghidraAddress PAL: 0x001bedd0
     */
    void ClearCheckpoints();

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
     * Set the scroll speed of the lanes.
     *
     * @param fSpeed The speed, 1 for normal.
     * @ghidraAddress NTSC-U/C: 0x001b60a0
     * @ghidraAddress PAL: 0x001bee40
     */
    void SetScrollSpeed(float fSpeed);

    /**
     * Show or hide the ship of a player.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the ship is shown.
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
     * Show or hide the energy meter.
     *
     * @param nPlayer The player's index. The display does not read it.
     * @param bShown Whether the meter is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6130
     * @ghidraAddress PAL: 0x001beed0
     */
    void SetEnergyShown(int nPlayer, bool bShown);

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
     * Show or hide the avatar display of a player.
     *
     * @param nPlayer The player's index.
     * @param bShown Whether the display is shown.
     * @ghidraAddress NTSC-U/C: 0x001b6170
     * @ghidraAddress PAL: 0x001bef10
     */
    void SetLaneShown(int nPlayer, bool bShown);

    /**
     * Show or hide the display of the players' scores and meters over the tracks.
     *
     * @param bShow Whether the display shows.
     * @ghidraAddress NTSC-U/C: 0x001b6190
     * @ghidraAddress PAL: 0x001bef30
     */
    void ShowHud(bool bShow);

    /**
     * Clear the highlights of the cameras and reset the head-up display.
     *
     * @ghidraAddress NTSC-U/C: 0x001b61b0
     * @ghidraAddress PAL: 0x001bef50
     */
    void ClearLanes();

    /**
     * Clear every effect of the tunnel, reset the ships and the panels of the arena, and clear
     * the start of the head-up display.
     *
     * @ghidraAddress NTSC-U/C: 0x001b61e0
     * @ghidraAddress PAL: 0x001bef80
     */
    void ClearAll();

    /**
     * Start or stop the victory lap, which hides the energy and score bars.
     *
     * @param bActive Whether the victory lap runs.
     * @ghidraAddress NTSC-U/C: 0x001b6250
     * @ghidraAddress PAL: 0x001beff0
     */
    void SetActive(bool bActive);

    /**
     * Report the colour of an instrument, `inst_color_<instrument>`.
     *
     * @param nInstrument The instrument, one of Instrument.
     * @return The colour.
     * @ghidraAddress NTSC-U/C: 0x001b4cc0
     * @ghidraAddress PAL: 0x001bda60
     */
    const Color *GetInstrumentColor(int nInstrument) const;

    /**
     * Report the background colour of an instrument, `inst_bg_color_<instrument>`.
     *
     * @param nInstrument The instrument, one of Instrument.
     * @return The colour.
     * @ghidraAddress NTSC-U/C: 0x001b4ce0
     * @ghidraAddress PAL: 0x001bda80
     */
    const Color *GetInstrumentBgColor(int nInstrument) const;

    /**
     * Report the colour of a player, `player_color_<colour>`.
     *
     * @param nPlayer The player.
     * @return The colour of the player's slot.
     * @ghidraAddress NTSC-U/C: 0x001b4cf8
     * @ghidraAddress PAL: 0x001bda98
     */
    const Color *GetPlayerColor(int nPlayer) const;

    /**
     * Report the background colour of a player, `player_bg_color_<colour>`.
     *
     * @param nPlayer The player.
     * @return The colour of the player's slot.
     * @ghidraAddress NTSC-U/C: 0x001b4d30
     * @ghidraAddress PAL: 0x001bdad0
     */
    const Color *GetPlayerBgColor(int nPlayer) const;

    /**
     * The colours of the instruments, indexed by Instrument.
     *
     * @ghidraAddress NTSC-U/C: 0x0043aea0
     */
    static Color sInstrumentColors[kNumInstruments];

    /**
     * The background colours of the instruments, indexed by Instrument.
     *
     * @ghidraAddress NTSC-U/C: 0x0043af00
     */
    static Color sInstrumentBgColors[kNumInstruments];

    /**
     * The colours of the player slots, indexed by PlayerColor.
     *
     * @ghidraAddress NTSC-U/C: 0x0043af60
     */
    static Color sPlayerColors[kNumPlayerColors];

    /**
     * The background colours of the player slots, indexed by PlayerColor.
     *
     * @ghidraAddress NTSC-U/C: 0x0043afc0
     */
    static Color sPlayerBgColors[kNumPlayerColors];

    /**
     * The textures that may be skipped by a load without a report, `merges_always_allowed`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8f4
     */
    static DataArray *sMergesAlwaysAllowed;

    /**
     * The energy at which the top zone of the meter starts.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8f8
     */
    static float sEnergyHighZone;

    /**
     * The energy at which the middle zone of the meter starts.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8fc
     */
    static float sEnergyLowZone;

    /**
     * Whether Init() ran.
     *
     * @ghidraAddress NTSC-U/C: 0x003af900
     */
    static int sInitialized;

    /**
     * Whether Terminate() ran.
     *
     * @ghidraAddress NTSC-U/C: 0x003af904
     */
    static int sTerminated;

private:
    /**
     * Read the colours and the configuration of the head-up display, and with bReload clear
     * define the script macros of the instruments first.
     *
     * @param bReload Whether the macros are already defined.
     * @ghidraAddress NTSC-U/C: 0x001b18c0
     * @ghidraAddress PAL: 0x001ba660
     */
    void LoadConfig(bool bReload);

    /**
     * Destroy everything Load() and BuildTracks() built.
     *
     * @ghidraAddress NTSC-U/C: 0x001b3238
     * @ghidraAddress PAL: 0x001bbfd8
     */
    void Teardown();

    /**
     * Work out the scroll speed of the tunnel from the configuration or from the song.
     *
     * @param fSongTicks The length of the song in ticks, used when the song has no entry.
     * @ghidraAddress NTSC-U/C: 0x001b35d0
     * @ghidraAddress PAL: 0x001bc370
     */
    void UpdateScrollSpeed(float fSongTicks);

    /**
     * Draw the views of every player around the layers of the tunnel and the ships.
     *
     * @param bDrawVortex Whether the vortex is drawn.
     * @ghidraAddress NTSC-U/C: 0x001b3e70
     * @ghidraAddress PAL: 0x001bcc10
     */
    void DrawLocalViews(bool bDrawVortex);

    /**
     * Report whether every file of mLoaders has loaded, finishing each one that has.
     *
     * @return Whether every file has loaded.
     * @ghidraAddress NTSC-U/C: 0x001b4200
     * @ghidraAddress PAL: 0x001bcfa0
     */
    bool CheckLoaders();

    /**
     * Run the `gfx_show` command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x001b4470
     * @ghidraAddress PAL: 0x001bd210
     */
    void Show(DataArray *pCommand);

    /**
     * Run the `gfx_cheat` command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x001b4730
     * @ghidraAddress PAL: 0x001bd4d0
     */
    void Cheat(DataArray *pCommand);

    /**
     * Run the `gfx` debug command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x001b4888
     * @ghidraAddress PAL: 0x001bd628
     */
    void Debug(DataArray *pCommand);

    /**
     * Draw the translucent echo of the frame of the `drugs` cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x001b11f0
     * @ghidraAddress PAL: 0x001b9f90
     */
    static void DrawDrugs();

    /**
     * Handle `gfx_handle_beat`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b08c8
     * @ghidraAddress PAL: 0x001b9668
     */
    static void OnHandleBeat(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b08f0
     * @ghidraAddress PAL: 0x001b9690
     */
    static void OnDebug(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_cheat`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0918
     * @ghidraAddress PAL: 0x001b96b8
     */
    static void OnCheat(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_show`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0940
     * @ghidraAddress PAL: 0x001b96e0
     */
    static void OnShow(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_add_checkpoint`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0968
     * @ghidraAddress PAL: 0x001b9708
     */
    static void OnAddCheckpoint(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_view_pullback`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0a48
     * @ghidraAddress PAL: 0x001b97e8
     */
    static void OnViewPullback(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_button_icon`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0a80
     * @ghidraAddress PAL: 0x001b9820
     */
    static void OnButtonIcon(DataArray *pCommand, void *pUserData);

    /**
     * Handle `place_hlbox`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0c30
     * @ghidraAddress PAL: 0x001b99d0
     */
    static void OnPlaceBox(DataArray *pCommand, void *pUserData);

    /**
     * Handle `place_hlarrow`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0ce8
     * @ghidraAddress PAL: 0x001b9a88
     */
    static void OnPlaceArrow(DataArray *pCommand, void *pUserData);

    /**
     * Handle `set_juice`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0d98
     * @ghidraAddress PAL: 0x001b9b38
     */
    static void OnSetJuice(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_controller`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0dd8
     * @ghidraAddress PAL: 0x001b9b78
     */
    static void OnController(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_dialog`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b0f38
     * @ghidraAddress PAL: 0x001b9cd8
     */
    static void OnDialog(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_show_message`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b1050
     * @ghidraAddress PAL: 0x001b9df0
     */
    static void OnShowMessage(DataArray *pCommand, void *pUserData);

    /**
     * Handle `gfx_stick_diagram`.
     *
     * @param pCommand The command.
     * @param pUserData The user data. The handler does not read it.
     * @ghidraAddress NTSC-U/C: 0x001b10e0
     * @ghidraAddress PAL: 0x001b9e80
     */
    static void OnStickDiagram(DataArray *pCommand, void *pUserData);

public:
    Vortex *mVortex;                    /*!< The vortex of the intro and the outro. */
    int mVortexAfterViews;              /*!< Whether the vortex draws after the head-up display. */
    std::list<GfxLoader> mLoaders;      /*!< The loaders of the files of the `load` list. */
    std::list<AsyncStream *> mPreloads; /*!< The streams of `data_preloads`. */
    int mLoadStage;                     /*!< One of LoadStage. */
    RndLoader *mHudLoader;              /*!< The loader of the head-up display, or null. */
    int mLoaded;                        /*!< Whether the files of the world have loaded. */
    float mLoadStartMs;                 /*!< The system time Load() ran at. */
    int mReserved34;                    // +0x34, cleared by the constructor and never read.
    int mState;                         /*!< One of State. */
    float mLastTick;                    /*!< The song tick of the last poll. */
    float mLastTime;                    /*!< The song time of the last poll. */
    float mScrollSpeed;                 /*!< The scroll speed of the tunnel. */
    float mInvScrollSpeed;              /*!< The reciprocal of mScrollSpeed. */
    float mSongTicks;                   /*!< The length of the song in ticks. */
    float mCamPathScale;                /*!< The length of the camera path for each tick. */
    int mVictoryLap;    /*!< Whether the victory lap runs, which hides the energy and score bars. */
    int mArenaUnlocked; /*!< Whether the path of the arena is unlocked. */
    int mNoArena;       /*!< The `no_arena` cheat. */
    int mMonkeyGems;    /*!< The `monkey` cheat. */
    int mDrugs;         /*!< The `drugs` cheat. */
    int mBlackPanels;   /*!< The `black_panels` cheat. */
    int mNoPanels;      /*!< The `no_panels` cheat. */
    signed char mTunnelShape;            /*!< The shape of the `circular` cheat, from 0 to 2. */
    int mHudFlags;                       /*!< The display of `toggle_hud`. Bit 0 draws the hud. */
    GfxTunnel *mTunnel;                  /*!< The tunnel of tracks, or null. */
    Overlay *mOverlay;                   /*!< The head-up display, or null. */
    GfxArena *mArena;                    /*!< The arena, or null. */
    Rnd::View *mTunnelView;              /*!< The `tunnelview` view, or null. */
    Rnd::View *mMainView;                /*!< The main view of the tunnel, or null. */
    std::vector<LocalViews> mLocalViews; /*!< The views of each player. */
    Rnd::View *mPanelView;               /*!< `panel mat render.view`, or null. */
    Ship *mShips[kMaxPlayers];           /*!< The ship of each player, or null. */
    int mLeader;                         /*!< The leading player. */
    int mWinnerDrawn;                    /*!< Whether the winner's avatar was drawn this frame. */
    int mWinner;                         /*!< The winning player, or -1. */
    const float *mMsPerTick;             /*!< The milliseconds per tick of the song, or null. */
    const char *mMultiplierFormat;       /*!< The localized `MULTIPLIER_FORMAT`. */
    std::list<DebugMsg> mDebugMsgs;      /*!< Messages for debugging. */
    std::list<DebugMsg> mDebugMsgs2;     /*!< More messages for debugging. */
};

/**
 * The display.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b020
 */
extern GfxManager TheGfxManager;
