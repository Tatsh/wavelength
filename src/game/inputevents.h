#pragma once

/**
 * Input event that plays a note on one of the three gem buttons.
 *
 * The RTTI includes the name as the argument of the input command template.
 */
class PlayNoteEvent {
public:
    /**
     * Construct the event.
     *
     * @param nPlayer The player index.
     * @param nButton The gem button, 0 to 2.
     * @param nState The state of the button the controller reported.
     * @param fX The horizontal position of the expression stick of the player.
     * @param fY The vertical position of the expression stick of the player.
     * @ghidraAddress NTSC-U/C: 0x001512c8
     * @ghidraAddress PAL: 0x00152bb8
     */
    PlayNoteEvent(unsigned char nPlayer, unsigned char nButton, int nState, float fX, float fY);

    unsigned char mPlayer; /*!< The player index. */
    unsigned char mButton; /*!< The gem button, 0 to 2. */
    int mState;            /*!< The state of the button the controller reported. */
    float mX;              /*!< The horizontal position of the expression stick. */
    float mY;              /*!< The vertical position of the expression stick. */
};

/**
 * Input event of one button.
 *
 * The RTTI includes the template. The argument is the identifier of the serializable input
 * command that records the event, and it makes each instance a distinct type.
 *
 * @tparam N The command identifier.
 */
template <int N>
struct BtnEvent {
    unsigned char mPlayer; /*!< The player index. */
};

/**
 * Input event of an analogue stick position.
 *
 * The RTTI includes the template with the arguments 2 and 6. The argument is the identifier of
 * the serializable input command that records the event, and it makes each instance a distinct
 * type.
 *
 * @tparam N The command identifier.
 */
template <int N>
struct StickEvent {
    unsigned char mPlayer; /*!< The player index. */
    float mX;              /*!< The horizontal position. */
    float mY;              /*!< The vertical position. */
};

/**
 * Input event that moves a player to a neighbouring track.
 *
 * The RTTI includes the name as the argument of the input command template.
 */
struct RotateEvent {
    /** Values of mDirection. */
    enum Direction {
        kDirectionPrevious = 0, /*!< The previous track. */
        kDirectionNext = 1,     /*!< The next track. */
    };

    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< One of Direction. */
};

/**
 * Input event that changes the section a player is on.
 *
 * The RTTI includes the name as the argument of the input command template.
 */
struct ChangeSectionEvent {
    unsigned char mPlayer; /*!< The player index. */
    int mDirection;        /*!< The direction of the change, 0 or 1. */
};
