#pragma once

#include "app/hideablepanel.h"
#include "app/rampanimator.h"
#include "game/avatarpartset.h"
#include "math/color.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Avatar of a player beside the tracks, with the name of the track or player under it.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x80 bytes. In a
 * shared-screen game the avatar of the leading player shows, and a change of leader swings the
 * display round to swap the avatar.
 */
class OvyAvatar : public HideablePanel {
public:
    /** The value of mPlayer and mNextPlayer for no player. */
    static constexpr signed char kNoPlayer = -1;

    /** The value of mNextPlayer when no change of player is pending. */
    static constexpr signed char kNoChange = -2;

    /**
     * Construct a hidden display.
     *
     * The scene objects are `<hud> freq <n>.tnm`, `<hud> freq <n>.view`,
     * `freq placeholder <n>.mesh`, and, in a duel or a solo game, `<hud> track<n>.txt`. A
     * shared-screen game that is not a remix swings the display with `<hud> freq <n> fx.tnm`.
     *
     * @param nPlayer The player shown, or a negative value for the leader of a shared-screen game.
     * @param nIndex The number of the display, from 0.
     * @param pHudView The view the display's view is removed from.
     * @ghidraAddress NTSC-U/C: 0x001ba388
     * @ghidraAddress PAL: 0x001c3128
     */
    OvyAvatar(int nPlayer, int nIndex, Rnd::View *pHudView);

    /**
     * Stop the animations of the avatars and move the avatar camera back to `bust`.
     *
     * @ghidraAddress NTSC-U/C: 0x001ba8f8
     * @ghidraAddress PAL: 0x001c3698
     */
    ~OvyAvatar() override;

    /**
     * Record whether the display is wanted, and show it while the avatar display is not hidden.
     *
     * @param bShow Whether to show the display.
     * @ghidraAddress NTSC-U/C: 0x00367298
     * @ghidraAddress PAL: 0x003d59c8
     */
    void Show(bool bShow) override {
        mWanted = bShow;
        UpdateShown();
    }

    /**
     * Show the display when it is wanted and the avatar display is not hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001baa08
     * @ghidraAddress PAL: 0x001c37a8
     */
    void UpdateShown();

    /**
     * Render the avatar and draw the display while it is not hidden, or render the winner's
     * avatar while it is.
     *
     * @ghidraAddress NTSC-U/C: 0x001baa50
     * @ghidraAddress PAL: 0x001c37f0
     */
    void Draw();

    /**
     * Advance the slide and the swing, and swap the avatar halfway through a swing.
     *
     * @param fDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001bab68
     * @ghidraAddress PAL: 0x001c3908
     */
    void Poll(float fDelta);

    /**
     * Swing the display round to another player.
     *
     * @param nPlayer The player, or a negative value for none.
     * @ghidraAddress NTSC-U/C: 0x001bad08
     * @ghidraAddress PAL: 0x001c3aa8
     */
    void SetPlayer(int nPlayer);

    /**
     * Choose the size of the avatar display.
     *
     * @param nFreqSize The size, one of GameOptions::FreqSize.
     * @ghidraAddress NTSC-U/C: 0x001bad58
     * @ghidraAddress PAL: 0x001c3af8
     */
    void SetFreqSize(int nFreqSize);

    /**
     * Stop the animations of the avatars, apply the size from the options, and empty a display
     * that swings.
     *
     * @ghidraAddress NTSC-U/C: 0x001bad90
     * @ghidraAddress PAL: 0x001c3b30
     */
    void Reset();

    /**
     * Show the name and the colour of the instrument of a track under the avatar.
     *
     * @param nPlayer The player. The display does not read it.
     * @param nInstrument The instrument, one of GfxManager::Instrument.
     * @param nTrackKind The kind of the track. The display does not read it.
     * @ghidraAddress NTSC-U/C: 0x001baee0
     * @ghidraAddress PAL: 0x001c3c80
     */
    void SetInstrument(int nPlayer, int nInstrument, int nTrackKind);

    /**
     * The time the swing to a new leader takes, `avatar_fx_leader_time`.
     *
     * @ghidraAddress NTSC-U/C: 0x003af924
     */
    static float sLeaderSwingTime;

    /**
     * The depth range the avatars draw into, `freq_z`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b150
     */
    static Vector2 sDepthRange;

    int mWanted;              /*!< Whether Show() asked for the display. */
    signed char mPlayer;      /*!< The player shown, or kNoPlayer. */
    AvatarPartSet *mAvatar;   /*!< The avatar of mPlayer, or null. */
    Rnd::View *mView;         /*!< `<hud> freq <n>.view`. */
    Rnd::Mesh *mPlaceholder;  /*!< `freq placeholder <n>.mesh`, which the avatar renders into. */
    Rnd::Text *mName;         /*!< `<hud> track<n>.txt`, or null. */
    Color mNameColor;         /*!< The colour of `HUD freq track.mat`. */
    Color mBackgroundColor;   /*!< The colour of `HUD freq_background.mat`. */
    Rnd::Mat *mBackgroundMat; /*!< `HUD freq_background.mat`. */
    Rnd::Mat *mNameMat;       /*!< `HUD freq track.mat`. */
    RampAnimator mSwing;      /*!< The swing to a new player, from 0 to 500. */
    signed char mNextPlayer;  /*!< The player the swing swaps to, or kNoChange. */
};
