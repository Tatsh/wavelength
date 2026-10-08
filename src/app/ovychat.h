#pragma once

#include <list>
#include <vector>

#include "app/editfield.h"
#include "app/msgsink.h"
#include "app/rampanimator.h"
#include "math/interpolator.h"
#include "msg/chatmsg.h"
#include "msg/keyboardkeymsg.h"
#include "os/string.h"
#include "rnd/drawable.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Chat of an online game on the head-up display, with the line a player types.
 *
 * The RTTI records the class as deriving from MsgSink. The object is 0x80 bytes. The panel slides
 * in when a line arrives or a key is pressed, and slides out once no line has moved for
 * `chat_timeout` milliseconds. The front of mLines is the bottom line, which the player types in.
 */
class OvyChat : public MsgSink {
public:
    /**
     * One line of the chat, which slides between the rows of the panel.
     *
     * The RTTI names the type. The object is 8 bytes.
     */
    class Line {
    public:
        /**
         * Construct a line on a text, with the slide of `chat_line_slide`.
         *
         * @param pConfig The configuration section of the game mode.
         * @param pDefaults The section of defaults.
         * @param pText The text of the line.
         * @ghidraAddress NTSC-U/C: 0x001c1e98
         * @ghidraAddress PAL: 0x001cac38
         */
        Line(DataArray *pConfig, DataArray *pDefaults, Rnd::Text *pText);

        /**
         * Destroy the slide, and forget the length of the slides.
         *
         * @ghidraAddress NTSC-U/C: 0x001c1f20
         * @ghidraAddress PAL: 0x001cacc0
         */
        ~Line();

        /**
         * Move the line to a row, sliding from where it is or jumping there.
         *
         * @param bJump Whether the line jumps to the row.
         * @param fTime The time of the move, in milliseconds.
         * @param fRow The position of the row.
         * @ghidraAddress NTSC-U/C: 0x001c1f98
         * @ghidraAddress PAL: 0x001cad38
         */
        void SetPosition(bool bJump, float fTime, float fRow);

        /**
         * Colour the line for the player who wrote it.
         *
         * @param nPlayer The player, or a negative value for white.
         * @ghidraAddress NTSC-U/C: 0x001c2060
         * @ghidraAddress PAL: 0x001cae00
         */
        void SetColor(int nPlayer);

        /**
         * Move the text to the position the slide has at a time.
         *
         * @param fTime The time, in milliseconds.
         * @return Whether the text moved.
         * @ghidraAddress NTSC-U/C: 0x001c2180
         * @ghidraAddress PAL: 0x001caf20
         */
        bool Update(float fTime);

        /**
         * The length of a slide, the end of the input range of the first slide built, or -1e9.
         *
         * @ghidraAddress NTSC-U/C: 0x003af95c
         */
        static float sSlideTime;

        Rnd::Text *mText;     /*!< The text. */
        Interpolator *mSlide; /*!< The position of the text over time. */
    };

    /** A line that arrived and waits for the top of the panel. */
    struct PendingMsg {
        String mText; /*!< The text. */
        int mPlayer;  /*!< The player who wrote it. */
    };

    /**
     * Construct an empty chat, and register it for the keyboard and for channel 3 of the chat.
     *
     * The lines are `HUDnr_chat_<nn>.txt` from 01 for as long as they exist, and the rows are the
     * resting positions of the lines plus one row past the last.
     *
     * @param pConfig The configuration section of the game mode.
     * @param pDefaults The section of defaults.
     * @param pHudView The view of the head-up display, which animates `HUD cursor.mnm`.
     * @ghidraAddress NTSC-U/C: 0x001c0ac8
     * @ghidraAddress PAL: 0x001c9868
     */
    OvyChat(DataArray *pConfig, DataArray *pDefaults, Rnd::View *pHudView);

    /**
     * Unregister the chat and destroy its lines.
     *
     * @ghidraAddress NTSC-U/C: 0x001c1200
     * @ghidraAddress PAL: 0x001c9fa0
     */
    ~OvyChat() override;

    /**
     * Empty the lines, hide the panel, and stop the editing.
     *
     * @ghidraAddress NTSC-U/C: 0x001c1358
     * @ghidraAddress PAL: 0x001ca0f8
     */
    void Reset();

    /**
     * Advance the panel and the lines, and show the next waiting line.
     *
     * @ghidraAddress NTSC-U/C: 0x001c14f0
     * @ghidraAddress PAL: 0x001ca290
     */
    void Poll();

    /**
     * Queue a line that arrived from another console.
     *
     * @param pMsg The line.
     * @return Whether the line was queued.
     * @ghidraAddress NTSC-U/C: 0x001c1770
     * @ghidraAddress PAL: 0x001ca510
     */
    bool AddLine(ChatMsg *pMsg);

    /**
     * Apply a key to the line the player types, starting or ending the editing.
     *
     * @param pMsg The key.
     * @return Always false.
     * @ghidraAddress NTSC-U/C: 0x001c18f0
     * @ghidraAddress PAL: 0x001ca690
     */
    bool HandleKey(KeyboardKeyMsg *pMsg);

    /**
     * Move every line one row up, the top line wrapping round to the bottom.
     *
     * @param bEditing Whether the bottom line is being typed in, which keeps it at the bottom.
     * @ghidraAddress NTSC-U/C: 0x001c1c08
     * @ghidraAddress PAL: 0x001ca9a8
     */
    void Layout(bool bEditing);

    /**
     * Drop the line the player typed in, moving every other line one row down.
     *
     * @ghidraAddress NTSC-U/C: 0x001c1d90
     * @ghidraAddress PAL: 0x001cab30
     */
    void Collapse();

    /**
     * Act on a line of chat or a key.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001c2208
     * @ghidraAddress PAL: 0x001cafa8
     */
    bool DispatchPriv(Message *pMsg) override;

    std::list<Line *> mLines;       /*!< The lines, from the bottom row up. */
    std::vector<float> mRows;       /*!< The position of each row. */
    std::list<PendingMsg> mPending; /*!< The lines that wait for the top of the panel. */
    Rnd::Drawable *mDrawable;       /*!< `HUDnr_chat_mask.mesh`, the drawing of the chat. */
    Rnd::Drawable *mTypeBox;        /*!< `HUDnr_chat_04.mesh`, shown while typing. */
    Rnd::Drawable *mRule;           /*!< `HUDnr_chat_hr.mesh`, shown while typing. */
    Rnd::Mesh *mCursor;             /*!< `HUDnr_chat_cursor.mesh`. */
    RampAnimator mSlide;            /*!< The slide of the panel, `HUDnr_chat.tnm`. */
    float mHiddenPosition;          /*!< The frame of mSlide with the panel hidden. */
    float mShownPosition;           /*!< The frame of mSlide with the panel shown. */
    float mLastActivity;            /*!< The time a line last moved, or -1e9. */
    Line *mEditLine;                /*!< The line the player types in, or null. */
    EditField mEdit;                /*!< The text the player types. */
    float mLastTime;                /*!< The time of the last poll, or -1e9. */
    float mTimeout;                 /*!< `chat_timeout`, in milliseconds. */
    int mLocalPlayer;               /*!< The first player on this console. */
};
