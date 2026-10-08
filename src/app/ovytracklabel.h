#pragma once

#include "app/hideablepanel.h"
#include "rnd/mat.h"
#include "rnd/text.h"

/**
 * Label of the instrument of the track the local player is on.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x30 bytes. The label
 * follows the first local player of the game.
 */
class OvyTrackLabel : public HideablePanel {
public:
    /**
     * Construct a hidden label from `HUD track label.tnm`, `HUD track label.view`,
     * `HUD track label.txt`, and `HUD track label.mat`.
     *
     * @ghidraAddress NTSC-U/C: 0x001c22a0
     * @ghidraAddress PAL: 0x001cb040
     */
    OvyTrackLabel();

    /**
     * Destroy the label.
     *
     * @ghidraAddress NTSC-U/C: 0x00367778
     * @ghidraAddress PAL: 0x003d5ea8
     */
    ~OvyTrackLabel() override = default;

    /**
     * Record whether the label is wanted, and show it while it has a text.
     *
     * @param bShow Whether to show the label.
     * @ghidraAddress NTSC-U/C: 0x003677d0
     * @ghidraAddress PAL: 0x003d5f00
     */
    void Show(bool bShow) override {
        mWanted = bShow;
        UpdateShown();
    }

    /**
     * Empty the label.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2400
     * @ghidraAddress PAL: 0x001cb1a0
     */
    void Hide();

    /**
     * Label the track a player moved to, when the player is the one the label follows.
     *
     * Tracks of kind 2, 4, and 5 show the name of their instrument. Others empty the label.
     *
     * @param nPlayer The player.
     * @param nInstrument The instrument of the track, one of GfxManager::Instrument.
     * @param nTrackKind The kind of the track.
     * @ghidraAddress NTSC-U/C: 0x001c2448
     * @ghidraAddress PAL: 0x001cb1e8
     */
    void SetTrack(int nPlayer, int nInstrument, int nTrackKind);

    /**
     * Show the label when it is wanted and has a text.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2608
     * @ghidraAddress PAL: 0x001cb3a8
     */
    void UpdateShown();

    int mWanted;      /*!< Whether Show() asked for the label. */
    int mHasText;     /*!< Whether the label has a text. */
    int mPlayer;      /*!< The player the label follows, the first local player. */
    Rnd::Text *mText; /*!< `HUD track label.txt`. */
    Rnd::Mat *mMat;   /*!< `HUD track label.mat`, which takes the instrument's colour. */
};
