#pragma once

#include "app/hideablepanel.h"
#include "app/hudpowerup.h"
#include "app/rampanimator.h"
#include "math/interpolator.h"
#include "math/vector3.h"
#include "rnd/drawable.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * Score of a player on the head-up display, with the bar a shared-screen game shows at each
 * checkpoint.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x94 bytes. At a
 * checkpoint the score moves to its place in the ranking, its bar grows to the player's share,
 * rests, fades, and the score moves back.
 */
class OvyScore : public HideablePanel {
public:
    /** The stages of the checkpoint display, the values of mState. */
    enum State {
        kStateIdle = 0,    /*!< No checkpoint shows. */
        kStateMoveIn = 1,  /*!< The score moves to its place in the ranking. */
        kStateGrow = 2,    /*!< The bar grows. */
        kStateFade = 3,    /*!< The bar rests and then fades. */
        kStateMoveOut = 4, /*!< The score moves back. */
    };

    /** The number of places of the ranking at a checkpoint. */
    static constexpr int kNumCheckpointPositions = 4;

    /**
     * Construct a hidden score of a player.
     *
     * The scene objects are `<hud> score<n>.mesh`, `<hud> score<n>.tnm`, `<hud> score<n>.txt`, and
     * `<hud> pts<n>.mesh`. A shared-screen game adds the bar `<hud> score bar<n>.mesh` and its
     * objects, and a game of local players adds the slide `<hud> pts<n> hide.tnm`. A local player
     * of a game that is neither a remix nor a duel has a power-up icon.
     *
     * @param nPlayer The player.
     * @param nIndex The number of the score, from 0.
     * @param pHudView The view of the head-up display.
     * @ghidraAddress NTSC-U/C: 0x001bd080
     * @ghidraAddress PAL: 0x001c5e20
     */
    OvyScore(int nPlayer, int nIndex, Rnd::View *pHudView);

    /**
     * Destroy the score, its slide, and its power-up icon.
     *
     * @ghidraAddress NTSC-U/C: 0x001bd700
     * @ghidraAddress PAL: 0x001c64a0
     */
    ~OvyScore() override;

    /**
     * Slide the score in or out. The score does not show in the victory lap.
     *
     * @param bShow Whether to show the score.
     * @ghidraAddress NTSC-U/C: 0x001bd8f8
     * @ghidraAddress PAL: 0x001c6698
     */
    void Show(bool bShow) override;

    /**
     * Empty the score and stop the checkpoint display.
     *
     * @ghidraAddress NTSC-U/C: 0x001bd7b0
     * @ghidraAddress PAL: 0x001c6550
     */
    void Reset();

    /**
     * Record a score, which shows after 600 milliseconds.
     *
     * The text waits only in a solo game, in a duel, and for a local player of an online game.
     * Otherwise it shows at the next poll.
     *
     * @param nScore The score.
     * @ghidraAddress NTSC-U/C: 0x001bd860
     * @ghidraAddress PAL: 0x001c6600
     */
    void SetScore(int nScore);

    /**
     * Advance the slide, the checkpoint display, the score text, and the power-up icon.
     *
     * @param fDelta The time since the last poll.
     * @param fUnused Passed to HudPowerup::Poll().
     * @param fDeltaTicks The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001bd928
     * @ghidraAddress PAL: 0x001c66c8
     */
    void Poll(float fDelta, float fUnused, float fDeltaTicks);

    /**
     * Draw the score.
     *
     * @ghidraAddress NTSC-U/C: 0x001bdc98
     * @ghidraAddress PAL: 0x001c6a38
     */
    void Draw();

    /**
     * Start the checkpoint display.
     *
     * @param fShare The player's share of the points, the length of the bar from 0 to 1.
     * @param nPlace The player's place in the ranking.
     * @ghidraAddress NTSC-U/C: 0x001bdcc0
     * @ghidraAddress PAL: 0x001c6a60
     */
    void ShowCheckpoint(float fShare, int nPlace);

    /**
     * The ticks the score takes to move in, `score_checkpoint_move_in_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af928
     */
    static float sMoveInTicks;

    /**
     * The ticks the bar takes to grow, `score_checkpoint_bar_grow_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af92c
     */
    static float sBarGrowTicks;

    /**
     * The ticks the bar rests, `score_checkpoint_stable_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af930
     */
    static float sStableTicks;

    /**
     * The ticks the bar takes to fade, `score_checkpoint_fade_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af934
     */
    static float sFadeTicks;

    /**
     * The ticks the score takes to move back, `score_checkpoint_move_out_ticks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af938
     */
    static float sMoveOutTicks;

    /**
     * The places of the ranking, `score_checkpoint_pos_1` to `score_checkpoint_pos_4`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b160
     */
    static Vector3 sCheckpointPositions[kNumCheckpointPositions];

    HudPowerup *mPowerup;          /*!< The power-up icon, or null. */
    Rnd::Text *mScoreText;         /*!< `<hud> score<n>.txt`. */
    float mScoreTime;              /*!< The song time of the last score, or a sentinel. */
    int mScore;                    /*!< The last score. */
    Rnd::Mat *mBackgroundMat;      /*!< `HUD score<n> bg.mat`, or null. */
    signed char mPlayer;           /*!< The player. */
    RampAnimator *mPointsSlide;    /*!< The slide of `<hud> pts<n>.mesh`, or null. */
    signed char mState;            /*!< The stage of the checkpoint display, one of State. */
    Rnd::Transformable *mTop;      /*!< `<hud> score top<n>.view`, which moves, or null. */
    Rnd::Drawable *mMesh;          /*!< `<hud> score<n>.mesh`, or null. */
    LinearInterpolator mMove;      /*!< The move, and then the fade, against the tick. */
    float mStableEnd;              /*!< The tick at which the bar starts to fade. */
    Rnd::Mesh *mBar;               /*!< `<hud> score bar<n>.mesh`, or null. */
    Rnd::Mat *mBarMat;             /*!< `HUD score bar<n>.mat`, or null. */
    RampAnimator mBarGrow;         /*!< The growth of the bar. */
    Vector3 mTopPos;               /*!< The place of mTop outside a checkpoint. */
    const Vector3 *mCheckpointPos; /*!< The place mTop moves to, or null. */
};
