#pragma once

#include <list>

#include "app/hideablepanel.h"
#include "math/vector3.h"
#include "rnd/animatable.h"
#include "rnd/drawable.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * Bar of the song's progress on the head-up display, with a marker per checkpoint.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x3c bytes.
 */
class OvySongPos : public HideablePanel {
public:
    /** One checkpoint marker. */
    struct Section {
        /**
         * Order a checkpoint before a place on the bar.
         *
         * @param fPos The place.
         * @return Whether mPos is before fPos.
         * @ghidraAddress NTSC-U/C: 0x003670c8
         * @ghidraAddress PAL: 0x003d57f8
         */
        bool operator<(float fPos) const {
            return mPos < fPos;
        }

        float mPos;       /*!< The place on the bar, from 0 (start) to 1 (end). */
        Rnd::Text *mText; /*!< The label of the checkpoint, which the bar owns, or null. */
    };

    /**
     * Construct a hidden bar and find its scene objects.
     *
     * The objects are `<hud> songpos.view`, `<hud> songpos chpt.mesh`,
     * `<hud> songpos future.mesh`, `<hud> songpos ship.mesh`, `<hud> songpos future.msnm`, and
     * the label template `HUDr songpos chpt.txt`. The head-up display prefix is `HUD1` in an online
     * game that is not a remix, and the overlay's prefix otherwise.
     *
     * @param pHudView The view the bar's view is removed from.
     * @ghidraAddress NTSC-U/C: 0x001b9230
     * @ghidraAddress PAL: 0x001c1fd0
     */
    explicit OvySongPos(Rnd::View *pHudView);

    /**
     * Destroy the bar and its checkpoint labels.
     *
     * @ghidraAddress NTSC-U/C: 0x00367040
     * @ghidraAddress PAL: 0x003d5770
     */
    ~OvySongPos() override {
        ClearCheckpoints();
    }

    /**
     * Add a checkpoint marker, unless one is already at the place.
     *
     * @param fPos The place on the bar, from 0 to 1.
     * @param pszLabel The label, or null for none.
     * @ghidraAddress NTSC-U/C: 0x001b96e8
     * @ghidraAddress PAL: 0x001c2488
     */
    void AddCheckpoint(float fPos, const char *pszLabel);

    /**
     * Remove every checkpoint marker and its label.
     *
     * @ghidraAddress NTSC-U/C: 0x001b98f8
     * @ghidraAddress PAL: 0x001c2698
     */
    void ClearCheckpoints();

    /**
     * Remove the checkpoints and empty the future part of the bar.
     *
     * @ghidraAddress NTSC-U/C: 0x001b99b8
     * @ghidraAddress PAL: 0x001c2758
     */
    void Reset();

    /**
     * Pose the future part of the bar.
     *
     * @param fFuture The length of the future part, in seconds.
     * @ghidraAddress NTSC-U/C: 0x001b99e8
     * @ghidraAddress PAL: 0x001c2788
     */
    void SetFuture(float fFuture);

    /**
     * Advance the slide.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9a10
     * @ghidraAddress PAL: 0x001c27b0
     */
    void Poll();

    /**
     * Draw the bar, the checkpoints, and the ship while the bar is not hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001b9a30
     * @ghidraAddress PAL: 0x001c27d0
     */
    void Draw();

    /**
     * The place of the start of the bar, `song_pos_start_pos`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b130
     */
    static Vector3 sStartPos;

    /**
     * The place of the end of the bar, `song_pos_end_pos`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b140
     */
    static Vector3 sEndPos;

    /**
     * The number of checkpoint labels created, which numbers their names.
     *
     * @ghidraAddress NTSC-U/C: 0x003af948
     */
    static int sNumLabels;

    Rnd::View *mView;                /*!< The view of the bar. */
    Rnd::Mesh *mCheckpointMesh;      /*!< The marker drawn at every checkpoint. */
    Rnd::Transformable *mFuture;     /*!< The future part, which the markers are placed under. */
    Rnd::Drawable *mShip;            /*!< The ship on the bar. */
    Rnd::Animatable *mFutureAnim;    /*!< The animation of the future part. */
    Rnd::Text *mLabelTemplate;       /*!< The label the checkpoint labels copy, or null. */
    std::list<Section> mCheckpoints; /*!< The checkpoints, in order of place. */
};
