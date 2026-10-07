#pragma once

#include "app/msgsink.h"
#include "msg/joypadinputmsg.h"
#include "os/joypad.h"

/**
 * Analogue stick of one controller that bends the notes a player plays.
 *
 * The RTTI includes the class name and records the MsgSink base. The directional buttons also move
 * the position, at a constant speed while they are held.
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
     * @return The result of the button handler for a controller button, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x0014ed88
     * @ghidraAddress PAL: 0x00150708
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take the stick position when the stick moved, otherwise move the position by the speed the
     * directional buttons set for the time since the last poll.
     *
     * @return Whether the stick moved or a directional button moves the position.
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

    /**
     * Start or stop the movement of the position for a directional button of the controller.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0014ec90
     * @ghidraAddress PAL: 0x00150610
     */
    bool HandleButton(JoypadInputMsg *pMsg);

    int mPad;               /*!< The controller. */
    int mStick;             /*!< The stick. */
    float mX;               /*!< The horizontal position, from -1 to 1. */
    float mY;               /*!< The vertical position, from -1 to 1. */
    float mSpeedX;          /*!< The horizontal movement, in units per millisecond. */
    float mSpeedY;          /*!< The vertical movement, in units per millisecond. */
    JoypadStick mLastStick; /*!< The stick position at the last poll. */
    float mLastPollMs;      /*!< The system time of the last poll, in milliseconds. */
};
