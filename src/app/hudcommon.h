#pragma once

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
#include "rnd/animatable.h"
#include "rnd/view.h"
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
    HudCommon(Rnd::View *pHudView, DataArray *pConfig, DataArray *pDefaults);

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

    OvyJuice mJuice;                      /*!< The energy bar. */
    HudTextNode mMessage;                 /*!< `HUD genmsg.txt`. */
    HudTextNode mMessage2;                /*!< `HUD genmsg2.txt`. */
    HudTextMessage mTextMessage;          /*!< The message that flies across. */
    HudHiliteBox mHilite;                 /*!< The highlight and the arrow of the tutorial. */
    OvySongPos *mSongPos;                 /*!< The song position bar, or null. */
    HudLetterbox mLetterbox;              /*!< The letterbox bars. */
    HudFlyingButton *mButtons[kNumLanes]; /*!< The button icons of the lanes. */
    OvyController *mController;           /*!< The picture of the controller, or null. */
    OvyDialog *mDialog;                   /*!< The panel of text, or null. */
    OvyAvatar *mLeaderAvatar;             /*!< The avatar of the leader, or null. */
    OvySectionPanel *mSections;           /*!< The sections of a remix, or null. */
    HudStick *mStick;                     /*!< The stick marker of a remix, or null. */
    OvyChat *mChat;                       /*!< The chat of an online remix, or null. */
    HudLetterExit *mLetterExit;           /*!< The letter banner of a duel, or null. */
    OvyTrackLabel *mTrackLabel;           /*!< The track label, or null. */
    Rnd::Animatable *mMaterialAnim;       /*!< `mat_2d_always.anim`, posed on the song time. */
    unsigned char mPlaying;               /*!< A bit per player who plays. */
};
