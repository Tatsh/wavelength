#pragma once

#include "app/msgsink.h"

/**
 * Analogue stick of one controller that bends the notes a player plays.
 *
 * The RTTI includes the class name and records the MsgSink base. Only the members its callers
 * here use are declared.
 */
class ExpressionStick : public MsgSink {
public:
    /**
     * Construct the reader of one stick and add it to the receivers of the controller messages.
     *
     * @param nPad The controller.
     * @param nStick The stick.
     * @ghidraAddress NTSC-U/C: 0x0014e9e8
     * @ghidraAddress PAL: 0x00150368
     */
    ExpressionStick(int nPad, int nStick);

    /**
     * Remove the reader from the receivers of the controller messages.
     *
     * @ghidraAddress NTSC-U/C: 0x0014ea40
     * @ghidraAddress PAL: 0x001503c0
     */
    ~ExpressionStick() override;

    /**
     * Act on a controller message.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x0014ed88
     * @ghidraAddress PAL: 0x00150708
     */
    void DispatchPriv(Message *pMsg) override;

    /**
     * Move the position toward the stick for the time since the last poll.
     *
     * @return Whether the position moved.
     * @ghidraAddress NTSC-U/C: 0x0014ea98
     * @ghidraAddress PAL: 0x00150418
     */
    bool Poll();

    /**
     * Centre the position and stop its movement.
     *
     * @ghidraAddress NTSC-U/C: 0x0014ec78
     * @ghidraAddress PAL: 0x001505f8
     */
    void Reset();

    int mPad;   /*!< The controller. */
    int mStick; /*!< The stick. */
    float mX;   /*!< The horizontal position, from -1 to 1. */
    float mY;   /*!< The vertical position, from -1 to 1. */
};
