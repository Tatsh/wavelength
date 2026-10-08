#pragma once

#include <list>
#include <vector>

#include "math/key.h"
#include "os/asyncstream.h"
#include "os/string.h"
#include "rnd/animatable.h"
#include "rnd/environ.h"
#include "rnd/mat.h"
#include "rnd/matanim.h"
#include "rnd/movie.h"
#include "rnd/rndloader.h"
#include "rnd/tex.h"
#include "rnd/transanim.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Arena the play field flies through, with its camera path, its triggers, and the movies and the
 * frequency texture its panels show.
 *
 * The RTTI identifies the class. The object is 0xa0 bytes with the vptr at +0x9c. A registered
 * creator can build a class derived from it for an arena of a matching name.
 */
class GfxArena {
public:
    /** A material stage that shows the texture of a movie, 0xc bytes. */
    struct MovieMat {
        Rnd::Mat *mMat; /*!< The material. */
        Rnd::Tex *mTex; /*!< The texture of the movie the stage shows when nothing replaces it. */
        char mStage;    /*!< The stage of mMat that shows the texture, or -1. */
    };

    /** The bits of mPanelFlags, in order of precedence from the last. */
    enum PanelFlag {
        kPanelLowEnergy = 0x1, /*!< The energy is low, and the panels show `snow.ipu`. */
        kPanelBeat = 0x2,      /*!< A beat runs, and the panels show the fourth movie. */
        kPanelFrequency = 0x4, /*!< The winner is drawn, and the panels show `fbase_freq.tex`. */
    };

    /** The number of movies of an arena, `movie1.mov` to `movie4.mov`. */
    static constexpr int kNumMovies = 4;

    /** The movie a beat plays. */
    static constexpr int kBeatMovie = 3;

    /** A function that builds an arena of a name, or reports null when the name is not its own. */
    typedef GfxArena *(*Creator)(const char *pszName);

    /**
     * Construct the arena of a name from the `arena_paths` entry of the `gfx` configuration.
     *
     * An arena with no file gets a placeholder view of its own.
     *
     * @param pszName The arena.
     * @ghidraAddress NTSC-U/C: 0x001ea4b0
     * @ghidraAddress PAL: 0x001f3250
     */
    explicit GfxArena(const char *pszName);

    /**
     * Create the arena of a name, through the first registered creator that accepts the name or
     * as a plain arena.
     *
     * @param pszName The arena.
     * @return The arena.
     * @ghidraAddress NTSC-U/C: 0x001ea438
     * @ghidraAddress PAL: 0x001f31d8
     */
    static GfxArena *Create(const char *pszName);

    /**
     * Destroy the arena.
     *
     * @ghidraAddress NTSC-U/C: 0x001ebe30
     * @ghidraAddress PAL: 0x001f4bd0
     */
    virtual ~GfxArena();

    /**
     * Read the configuration of the arena again, with its triggers from their file.
     *
     * @ghidraAddress NTSC-U/C: 0x001ebf90
     * @ghidraAddress PAL: 0x001f4d30
     */
    virtual void LoadConfig();

    /**
     * Fit the camera path to the length of the song.
     *
     * @param fSongTicks The length of the song in ticks, or 1 or less for no song.
     * @ghidraAddress NTSC-U/C: 0x001ec710
     * @ghidraAddress PAL: 0x001f54b0
     */
    virtual void SetSongTicks(float fSongTicks);

    /**
     * Move the camera along its path, run the triggers, and animate the panels.
     *
     * @param fTick The tick of the song.
     * @param fTime The time of the display.
     * @ghidraAddress NTSC-U/C: 0x001eca40
     * @ghidraAddress PAL: 0x001f57e0
     */
    virtual void Poll(float fTick, float fTime);

    /**
     * Start to stream the triggers of the arena, or mark them loaded when the arena has none.
     *
     * @ghidraAddress NTSC-U/C: 0x001ec058
     * @ghidraAddress PAL: 0x001f4df8
     */
    void LoadTriggers();

    /**
     * Gather the objects of the arena once its file is read, and load the streamed triggers once
     * they are ready.
     *
     * @param bHudLoaded Whether the head-up display has started to load.
     * @ghidraAddress NTSC-U/C: 0x001ec0d0
     * @ghidraAddress PAL: 0x001f4e70
     */
    void PollLoad(bool bHudLoaded);

    /**
     * Set the camera path unless the arena found one.
     *
     * @param pPath The path.
     * @ghidraAddress NTSC-U/C: 0x001ec6f8
     * @ghidraAddress PAL: 0x001f5498
     */
    void SetCamPath(Rnd::TransAnim *pPath);

    /**
     * Return the panels to their resting look.
     *
     * @ghidraAddress NTSC-U/C: 0x001ec910
     * @ghidraAddress PAL: 0x001f56b0
     */
    void ResetPanels();

    /**
     * Show the energy of the play field on the panels, unless a won result is shown.
     *
     * @param nZone The zone of the energy. The arena does not read it.
     * @param fEnergy The energy, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001ec930
     * @ghidraAddress PAL: 0x001f56d0
     */
    void SetEnergy(int nZone, float fEnergy);

    /**
     * Show the result of the song on the panels.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x001ec980
     * @ghidraAddress PAL: 0x001f5720
     */
    void ShowResult(bool bWon);

    /**
     * Run the effect of a beat event of the song.
     *
     * @param nEvent The event, `A` to start a beat or `B` to end it.
     * @ghidraAddress NTSC-U/C: 0x001ec9b0
     * @ghidraAddress PAL: 0x001f5750
     */
    void HandleBeat(signed char nEvent);

    /**
     * The creators of derived arenas, up to a null entry.
     *
     * @ghidraAddress NTSC-U/C: 0x003afac8
     */
    static Creator sCreators[];

    /**
     * The energy at or below which the panels show snow, the second number of
     * `juice_blink_range`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afad0
     */
    static float sLowEnergy;

    String mName;                /*!< The name of the arena. */
    int mLoaded;                 /*!< Whether PollLoad() has gathered the objects. */
    int mTriggersLoaded;         /*!< Whether the triggers are loaded or the arena has none. */
    DataArray *mConfig;          /*!< The `arena_paths` entry of the arena. */
    AsyncStream *mTriggerStream; /*!< The stream of the triggers, or null. */
    RndLoader *mLoader;          /*!< The loader of the file of the arena, or null. */
    Rnd::View *mView;            /*!< The top view of the arena, or null. */
    int mOwnsView;               /*!< Whether mView is the placeholder the arena created. */
    Rnd::TransAnim *mCamPath;    /*!< The path of the camera. */
    Rnd::Tex *mFrequencyTex;     /*!< `fbase_freq.tex`. */
    float mFrameScale;           /*!< The frames of the camera path per tick. */
    float mFrameOffset;          /*!< The frame of the camera path at tick 0. */
    float mLength;               /*!< The ticks the camera path covers, `length`. */
    float mSongTicks;            /*!< The length of the song SetSongTicks() fitted. */
    float mCamPathLength;        /*!< The length of the camera path the data expects. */
    Rnd::Environ *mEnviron;      /*!< The first environment of the file of the arena. */
    int mUnlockIndex;            /*!< The index of the arena among the unlocked arenas. */
    std::vector<Key<float>> mFreestyleKeys; /*!< `freestyle_constraints` over camera frames. */
    float mFreestyle; /*!< The value of mFreestyleKeys at the frame of the camera. */
    char mPanelFlags; /*!< The look of the panels, a set of PanelFlag. */
    int mResultShown; /*!< Whether ShowResult() ran. */
    int mWon;         /*!< Whether the song was won. */
    std::list<Rnd::MatAnim *> mMatAnims; /*!< The material animations of the movie textures. */
    std::list<MovieMat> mMovieMats;      /*!< The material stages that show the movies. */
    std::vector<Rnd::Movie *> mMovies;   /*!< `movie1.mov` to `movie4.mov`. */
    float mBeatTick;                     /*!< The tick the last beat started. */
    Rnd::Movie *mSnow;                   /*!< `snow.ipu`. */

private:
    /**
     * Record every material stage that shows a texture.
     *
     * @param pTex The texture.
     * @param pMats Receives the stages of the materials it does not list yet.
     * @ghidraAddress NTSC-U/C: 0x001ea080
     * @ghidraAddress PAL: 0x001f2e20
     */
    static void CollectMovieMats(Rnd::Tex *pTex, std::list<MovieMat> *pMats);

    /**
     * Record every material animation that switches a stage to a texture.
     *
     * An animation is recorded once for each of its stages that uses the texture.
     *
     * @param pTex The texture.
     * @param pAnims Receives the animations it does not list yet.
     * @ghidraAddress NTSC-U/C: 0x001ea240
     * @ghidraAddress PAL: 0x001f2fe0
     */
    static void CollectMatAnims(Rnd::Tex *pTex, std::list<Rnd::MatAnim *> *pAnims);

    /**
     * Take an animation out of the animation lists of everything that drives it.
     *
     * @param pAnim The animation.
     * @ghidraAddress NTSC-U/C: 0x001e9f98
     * @ghidraAddress PAL: 0x001f2d38
     */
    static void DetachAnim(Rnd::Animatable *pAnim);

    /**
     * Stretch the keys of a camera path from the length it was made for to the length of the
     * song.
     *
     * @param pPath The path.
     * @ghidraAddress NTSC-U/C: 0x001ec668
     * @ghidraAddress PAL: 0x001f5408
     */
    void FitCamPath(Rnd::TransAnim *pPath);

    /**
     * Turn panel flags on or off, and show the texture of the flags on every movie stage.
     *
     * @param nFlags The flags, a set of PanelFlag.
     * @param bOn Whether the flags turn on.
     * @ghidraAddress NTSC-U/C: 0x001ec830
     * @ghidraAddress PAL: 0x001f55d0
     */
    void SetPanelFlags(char nFlags, bool bOn);

    /**
     * Show a texture on a movie stage.
     *
     * @param pMat The stage.
     * @param pTex The texture, or null for the texture of the movie.
     * @ghidraAddress NTSC-U/C: 0x001ece18
     * @ghidraAddress PAL: 0x001f5bb8
     */
    static void ShowTexture(const MovieMat *pMat, Rnd::Tex *pTex);
};
