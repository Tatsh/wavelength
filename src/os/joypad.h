#pragma once

#include "app/msgsink.h"

/** The number of analogue sticks of a controller. */
constexpr int kJoypadNumSticks = 2;

/**
 * Position of one analogue stick.
 *
 * The name is inferred.
 */
struct JoypadStick {
    float mX; /*!< The horizontal position, from -1 to 1. */
    float mY; /*!< The vertical position, from -1 to 1. */
};

/**
 * State of one controller as the last poll read it.
 *
 * The name is inferred. The record is 0x80 bytes. Only the members its callers here read are
 * declared.
 */
struct JoypadState {
    int mReserved00;                       // +0x00, not yet identified.
    JoypadStick mSticks[kJoypadNumSticks]; /*!< The analogue sticks. */
    unsigned char mReserved14[0x64];       // +0x14, not yet identified.
    int mConnected;                        /*!< Non-zero while the controller is connected. +0x78 */
};

/**
 * Report the state of a controller.
 *
 * @param nPad The controller.
 * @return The state.
 * @ghidraAddress NTSC-U/C: 0x0028aca0
 * @ghidraAddress PAL: 0x00294498
 */
JoypadState *JoypadGetState(int nPad);

/**
 * Add a sink to the receivers of the controller messages.
 *
 * @param pSink The sink.
 * @ghidraAddress NTSC-U/C: 0x0028acf0
 * @ghidraAddress PAL: 0x002944e8
 */
void JoypadAddSink(MsgSink *pSink);

/**
 * Remove a sink from the receivers of the controller messages.
 *
 * @param pSink The sink.
 * @ghidraAddress NTSC-U/C: 0x0028ad18
 * @ghidraAddress PAL: 0x00294510
 */
void JoypadRemoveSink(MsgSink *pSink);

/**
 * Turn the analog stick messages of the controller poll on or off.
 *
 * While the setting is off, the poll does not compare stick positions and sends no message for a
 * moved stick. The setting starts off. The name is inferred from the behaviour.
 *
 * @param bEnable Whether moved sticks send messages.
 * @ghidraAddress NTSC-U/C: 0x0028ace0
 * @ghidraAddress PAL: 0x002944d8
 */
void JoypadSetStickMessages(bool bEnable);

/**
 * Give the input of a controller to the menus or to the running world.
 *
 * WorldLogic gives every controller to the world while it exists, and a paused game gives them
 * back to the menus. The name is inferred.
 *
 * @param nPad The controller.
 * @param bMenu Whether the menus receive the input.
 * @ghidraAddress NTSC-U/C: 0x0028acc8
 * @ghidraAddress PAL: 0x002944c0
 */
void JoypadSetMenuControl(int nPad, bool bMenu);

/**
 * Drive the vibration motors of the controller on a port.
 *
 * The name is inferred.
 *
 * @param nPad The controller port.
 * @param nSmallMotor The small motor's state, 0 or 1.
 * @param nBigMotor The big motor's level.
 * @ghidraAddress NTSC-U/C: 0x0028aff8
 * @ghidraAddress PAL: 0x002947f0
 */
void JoypadSetVibration(int nPad, int nSmallMotor, int nBigMotor);
