#pragma once

#include <memory>
#include <utility>

#include "app/hudflyingbutton.h"
#include "app/hudhilitebox.h"
#include "app/hudletterbox.h"
#include "app/hudletterexit.h"
#include "app/hudstick.h"
#include "app/hudtextmessage.h"
#include "app/hudtextnode.h"
#include "app/ovyavatar.h"
#include "app/ovychat.h"
#include "app/ovycontroller.h"
#include "app/ovydialog.h"
#include "app/ovyjuice.h"
#include "app/ovysectionpanel.h"
#include "app/ovysongpos.h"
#include "app/ovytracklabel.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

/**
 * Parts of the head-up display that belong to the whole game rather than to one player.
 *
 * The class is not polymorphic and emits no RTTI. The object is 0x250 bytes. The title is
 * inferred. The overlay builds one and owns it.
 */
class HudCommon {
public:
    /** The number of lanes, one flying button icon each. */
    static constexpr int kNumLanes = 6;

    /**
     * Construct the parts the rule set and the community of the game call for.
     *
     * @param pHudView The view of the head-up display layout.
     * @param pConfig The configuration section of the game mode.
     * @param pDefaults The section of defaults.
     * @ghidraAddress NTSC-U/C: 0x001c2ba8
     * @ghidraAddress PAL: 0x001cb948
     */
    HudCommon(RndView *pHudView, DataArray *pConfig, DataArray *pDefaults);

    /**
     * Destroy the parts.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2fa8
     * @ghidraAddress PAL: 0x001cbd48
     */
    ~HudCommon();

    /**
     * Advance the parts.
     *
     * @param fSongDelta The song time since the last poll.
     * @param fRealDelta The time since the last poll.
     * @param fTickDelta The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001c31e8
     * @ghidraAddress PAL: 0x001cbf88
     */
    void Poll(float fSongDelta, float fRealDelta, float fTickDelta);

    /**
     * Draw the song position, the button icons, the sections, and the stick marker.
     *
     * @ghidraAddress NTSC-U/C: 0x001c3360
     * @ghidraAddress PAL: 0x001cc100
     */
    void Draw();

    /**
     * Draw the chat, the text message, the letterbox, and the two lines of text.
     *
     * @ghidraAddress NTSC-U/C: 0x001c33f8
     * @ghidraAddress PAL: 0x001cc198
     */
    void DrawText();

    /**
     * Draw the avatar of the leader.
     *
     * @ghidraAddress NTSC-U/C: 0x001c3450
     * @ghidraAddress PAL: 0x001cc1f0
     */
    void DrawAvatar();

    /**
     * Mark a player as playing or not, and show the leader avatar of the one player who plays.
     *
     * @param nPlayer The player, or a negative value for none.
     * @param bPlaying Whether the player plays.
     * @ghidraAddress NTSC-U/C: 0x001c3478
     * @ghidraAddress PAL: 0x001cc218
     */
    void SetPlaying(int nPlayer, bool bPlaying);

    /**
     * Return every part to its state at the start of a song.
     *
     * @ghidraAddress NTSC-U/C: 0x001c3558
     * @ghidraAddress PAL: 0x001cc2f8
     */
    void Reset();

    /** @return The energy bar. */
    OvyJuice &GetJuice() {
        return mJuice;
    }

    /** @return `HUD genmsg.txt`. */
    HudTextNode &GetMessage() {
        return mMessage;
    }

    /** @return `HUD genmsg2.txt`. */
    HudTextNode &GetMessage2() {
        return mMessage2;
    }

    /** @return The message that flies across. */
    HudTextMessage &GetTextMessage() {
        return mTextMessage;
    }

    /** @return The highlight and the arrow of the tutorial. */
    HudHiliteBox &GetHilite() {
        return mHilite;
    }

    /** @return The letterbox bars. */
    HudLetterbox &GetLetterbox() {
        return mLetterbox;
    }

    /** @return The song position bar, or null. */
    OvySongPos *GetSongPos() const {
        return mSongPos.get();
    }

    /**
     * @param nLane The lane, below kNumLanes.
     * @return The button icon of the lane.
     */
    HudFlyingButton &GetButton(int nLane) const {
        return *mButtons[nLane];
    }

    /** @return The picture of the controller, or null. */
    OvyController *GetController() const {
        return mController.get();
    }

    /**
     * Replace the picture of the controller.
     *
     * @param pController The new picture, which the head-up display then owns.
     */
    void SetController(std::unique_ptr<OvyController> pController) {
        mController = std::move(pController);
    }

    /** @return The panel of text, or null. */
    OvyDialog *GetDialog() const {
        return mDialog.get();
    }

    /**
     * Replace the panel of text.
     *
     * @param pDialog The new panel, which the head-up display then owns.
     */
    void SetDialog(std::unique_ptr<OvyDialog> pDialog) {
        mDialog = std::move(pDialog);
    }

    /** @return The avatar of the leader, or null. */
    OvyAvatar *GetLeaderAvatar() const {
        return mLeaderAvatar.get();
    }

    /** @return The sections of a remix, or null. */
    OvySectionPanel *GetSections() const {
        return mSections.get();
    }

    /** @return The stick marker of a remix, or null. */
    HudStick *GetStick() const {
        return mStick.get();
    }

    /** @return The letter banner of a duel, or null. */
    HudLetterExit *GetLetterExit() const {
        return mLetterExit.get();
    }

    /** @return The track label, or null. */
    OvyTrackLabel *GetTrackLabel() const {
        return mTrackLabel.get();
    }

private:
    OvyJuice mJuice;                      /*!< The energy bar. */
    HudTextNode mMessage;                 /*!< `HUD genmsg.txt`. */
    HudTextNode mMessage2;                /*!< `HUD genmsg2.txt`. */
    HudTextMessage mTextMessage;          /*!< The message that flies across. */
    HudHiliteBox mHilite;                 /*!< The highlight and the arrow of the tutorial. */
    std::unique_ptr<OvySongPos> mSongPos; /*!< The song position bar, or null. */
    HudLetterbox mLetterbox;              /*!< The letterbox bars. */
    std::unique_ptr<HudFlyingButton> mButtons[kNumLanes]; /*!< The button icons of the lanes. */
    std::unique_ptr<OvyController> mController; /*!< The picture of the controller, or null. */
    std::unique_ptr<OvyDialog> mDialog;         /*!< The panel of text, or null. */
    std::unique_ptr<OvyAvatar> mLeaderAvatar;   /*!< The avatar of the leader, or null. */
    std::unique_ptr<OvySectionPanel> mSections; /*!< The sections of a remix, or null. */
    std::unique_ptr<HudStick> mStick;           /*!< The stick marker of a remix, or null. */
    std::unique_ptr<OvyChat> mChat;             /*!< The chat of an online remix, or null. */
    std::unique_ptr<HudLetterExit> mLetterExit; /*!< The letter banner of a duel, or null. */
    std::unique_ptr<OvyTrackLabel> mTrackLabel; /*!< The track label, or null. */
    RndAnimatable *mMaterialAnim; /*!< `mat_2d_always.anim`, posed on the song time. */
    unsigned char mPlaying;       /*!< A bit per player who plays. */
};
