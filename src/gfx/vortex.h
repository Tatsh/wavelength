#pragma once

#include "math/interpolator.h"

/**
 * Animation of the vortex the play field flies through on the way into and out of a song.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x78 bytes, and its configuration
 * is the entry of `gfx vortices` with the name given to the constructor. The name is inferred.
 * Only the members the display reads are declared.
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

    // +0x00 to +0x1f are not yet identified.
    int mMode; /*!< The run in progress, one of Mode. +0x20 */
    // +0x24 is not yet identified.
    LinearInterpolator mInterp; /*!< The frame of the run over time. +0x28 */
    float mFrame;               /*!< The frame of the run. +0x44 */
    // +0x48 to +0x77 are not yet identified.
};
