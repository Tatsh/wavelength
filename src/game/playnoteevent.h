#pragma once

#include "os/binstream.h"

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

    /**
     * Construct the event from a recording.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001512a0
     * @ghidraAddress PAL: 0x00152b90
     */
    explicit PlayNoteEvent(BinStream &stream);

    /**
     * Record the event.
     *
     * The player, the button, and the low byte of the state take one byte each, and the
     * coordinates four bytes each.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x001512e8
     */
    void Save(BinStream &stream) const;

    /**
     * Read the event Save() records.
     *
     * The state becomes 1 when its byte is not zero.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x001513b0
     */
    void Load(BinStream &stream);

    unsigned char mPlayer; /*!< The player index. */
    unsigned char mButton; /*!< The gem button, 0 to 2. */
    int mState;            /*!< The state of the button the controller reported. */
    float mX;              /*!< The first coordinate the input router stores for the player. */
    float mY;              /*!< The second coordinate the input router stores for the player. */
};
