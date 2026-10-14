#pragma once

#include "math/interpolator.h"
#include "math/vector2.h"
#include "rnd/rndloader.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Animation of the vortex the play field flies through on the way into and out of a song.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x78 bytes, and its configuration
 * is the entry of `gfx vortices` with the name given to the constructor. The name is inferred.
 */
class Vortex {
public:
    /** The values of mMode. */
    enum Mode {
        kModeIdle = 0,  /*!< The vortex does not run. */
        kModeOutro = 1, /*!< The run at the end of a song. */
        kModeIn = 2,    /*!< The run into the play field, `vortex_in`. */
        kModeHold = 3,  /*!< A mode the display tests for and never starts. */
        kModeOut = 4,   /*!< The run out of the play field, `vortex_out`. */
    };

    /**
     * Construct an idle vortex from its configuration.
     *
     * @param pszName The name of the entry of `gfx vortices`.
     * @ghidraAddress NTSC-U/C: 0x001e4c10
     * @ghidraAddress PAL: 0x001ed9b0
     */
    explicit Vortex(const char *pszName);

    /**
     * Release the configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x001e4d98
     * @ghidraAddress PAL: 0x001edb38
     */
    ~Vortex();

    /**
     * Report the length of the run at the end of a song.
     *
     * @return The length in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001e5120
     * @ghidraAddress PAL: 0x001edec0
     */
    float GetOutroLength() const;

    /**
     * Report the length of the run into the play field.
     *
     * @return The length in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001e5138
     * @ghidraAddress PAL: 0x001eded8
     */
    float GetInLength() const;

    /**
     * Report the length of the run out of the play field.
     *
     * @return The length in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001e5150
     * @ghidraAddress PAL: 0x001edef0
     */
    float GetOutLength() const;

    /**
     * Start a run.
     *
     * @param fLength The length of the run in milliseconds, or a negative value for the length the
     *                configuration gives.
     * @param eMode The run, one of Mode.
     * @ghidraAddress NTSC-U/C: 0x001e5168
     * @ghidraAddress PAL: 0x001edf08
     */
    void Start(float fLength, int eMode);

    /**
     * Advance the run to the time of the system clock.
     *
     * @ghidraAddress NTSC-U/C: 0x001e52f8
     * @ghidraAddress PAL: 0x001ee098
     */
    void Poll();

    /**
     * Draw the vortex while a run is in progress.
     *
     * @ghidraAddress NTSC-U/C: 0x001e5478
     * @ghidraAddress PAL: 0x001ee218
     */
    void Draw();

    /**
     * Queue the load of the file of the configuration and find its views.
     *
     * @ghidraAddress NTSC-U/C: 0x001e4df8
     * @ghidraAddress PAL: 0x001edb98
     */
    void Load();

    /**
     * Stop the run, forget the views, and delete the loader with its objects.
     *
     * @ghidraAddress NTSC-U/C: 0x001e50d8
     * @ghidraAddress PAL: 0x001ede78
     */
    void Unload();

    DataArray *mConfig;         /*!< The entry of `gfx vortices`. */
    RndLoader *mLoader;         /*!< The loader of the file of the configuration, or null. */
    Rnd::View *mTopView;        /*!< The view Draw() draws, `top_view`. */
    Rnd::View *mTransitionView; /*!< The view the frame of the run drives, `transition_view`. */
    Rnd::View *mAmbientView;    /*!< The view the time of the run drives, `ambient_view`. */
    Rnd::View *mWiggle;         /*!< The view the left stick tilts, `wiggle`. */
    Rnd::View *mWiggleXAnim;    /*!< The view the left stick drives, `wiggle_x_anim`. */
    Rnd::View *mWiggleYAnim;    /*!< `wiggle_y_anim`, found and never driven. */
    int mMode;                  /*!< The run in progress, one of Mode. */
    int mPrevMode;              /*!< The run at the start of the last Poll(). */
    LinearInterpolator mInterp; /*!< The frame of the run over the system clock. */
    float mFrame;               /*!< The frame of the run. */
    float mAmbientFrame;        /*!< The milliseconds since the run started. */
    float mStartMs;             /*!< The system clock at the start of the run, or kNotStarted. */
    Vector2 mOutroFrames;       /*!< The first and last frames of kModeOutro, `black_frames`. */
    Vector2 mInFrames;          /*!< The first and last frames of kModeIn, `start_frames`. */
    Vector2 mOutFrames;         /*!< The first and last frames of kModeOut, `end_frames`. */
    Vector2 mMotionRange;       /*!< The tilt of mWiggle at a full stick, `motion_range`. */
    Vector2 mStick;             /*!< The position of the left stick at the last Draw(). */

    /** The value of mStartMs before the first Poll() of a run. */
    static constexpr float kNotStarted = -1e9f;
};
