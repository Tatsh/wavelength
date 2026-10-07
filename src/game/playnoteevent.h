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
     * @param fX The first coordinate the input router stores for the player.
     * @param fY The second coordinate the input router stores for the player.
     * @ghidraAddress NTSC-U/C: 0x001512c8
     * @ghidraAddress PAL: 0x00152bb8
     */
    PlayNoteEvent(unsigned char nPlayer, unsigned char nButton, int nState, float fX, float fY);

    unsigned char mPlayer; /*!< The player index. */
    unsigned char mButton; /*!< The gem button, 0 to 2. */
    int mState;            /*!< The state of the button the controller reported. */
    float mX;              /*!< The first coordinate the input router stores for the player. */
    float mY;              /*!< The second coordinate the input router stores for the player. */
};
