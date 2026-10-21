#pragma once

#include "gfx/gfxtunnel.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

/**
 * The shell of a won song, a view that plays once at the result and then unlocks the path the win
 * earned.
 *
 * The class is not polymorphic, and the name is inferred from the `win_shell` entry and the
 * `win_shell.view` view. The object is 0x14 bytes.
 */
class TnlWinShell {
public:
    /** The value of mStartTick while the shell does not play. */
    static constexpr float kNoTick = -1e9f;

    /** The value of mPath of a win that unlocks no path. */
    static constexpr int kNoPath = -1;

    /**
     * Find the view of the shell, draw it with `win_arms.view`, and hide it.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6430
     * @ghidraAddress PAL: 0x001df1d0
     */
    TnlWinShell();

    /**
     * Destroy the shell.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6548
     * @ghidraAddress PAL: 0x001df2e8
     */
    ~TnlWinShell();

    /**
     * Load the duration and the length of the animation from the `win_shell` entry.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. The routine does not
     *                read it.
     * @ghidraAddress NTSC-U/C: 0x001d6570
     * @ghidraAddress PAL: 0x001df310
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Hide the shell and stop it.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6620
     * @ghidraAddress PAL: 0x001df3c0
     */
    void Reset();

    /**
     * Play the shell from the song tick.
     *
     * @param nPath The path the win unlocks at the end of the shell, or kNoPath.
     * @ghidraAddress NTSC-U/C: 0x001d6678
     * @ghidraAddress PAL: 0x001df418
     */
    void Start(int nPath);

    /**
     * Move the view of the shell to the song tick and show it, or at the end of the duration
     * unlock the path, play the unlock sound, hide the head-up display, and stop the shell.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6690
     * @ghidraAddress PAL: 0x001df430
     */
    void Update();

private:
    RndView *mView;   /*!< The view, `win_shell.view`. */
    float mStartTick; /*!< The tick the shell started at, or kNoTick. */
    float mDuration;  /*!< The ticks the shell plays, `duration`. */
    float mFrameRate; /*!< The frames of the view per tick, `anim_length` over mDuration. */
    int mPath;        /*!< The path the win unlocks, or kNoPath. */
};
