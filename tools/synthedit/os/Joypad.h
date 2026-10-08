#pragma once

#include "os/JoypadData.h"
#include "os/JoypadMsgSource.h"
#include "utl/Data.h"
#include "utl/MsgSink.h"

/** Number of controllers. */
const int kNumJoypads = 8;

/** Number of players. */
const int kNumPlayers = 4;

/**
 * State of each controller.
 *
 * @ghidraAddress 0x10040cf0
 */
extern JoypadData gJoypadData[kNumJoypads];

/**
 * Controller of each player.
 *
 * @ghidraAddress 0x10040ce0
 */
extern int gPlayerNumToPadNum[kNumPlayers];

/**
 * Source of the controller messages.
 *
 * @ghidraAddress 0x100410f0
 */
extern JoypadMsgSource gJoypadMsgSource;

/**
 * Apply the `joypad` configuration array, the press threshold of every controller.
 *
 * @param config The array.
 * @ghidraAddress 0x1000ed50
 */
void JoypadConfigInit(DataArray *config);

/**
 * Report the state of a player's controller.
 *
 * @param iPlayerNum The player.
 * @return The state.
 * @ghidraAddress 0x1000ed90
 */
JoypadData *JoypadGetPlayerData(int iPlayerNum);

/**
 * Send the controller messages to a sink.
 *
 * @param sink The sink.
 * @ghidraAddress 0x1000ee10
 */
void JoypadSubscribe(MsgSink *sink);

/**
 * Stop sending the controller messages to a sink.
 *
 * @param sink The sink.
 * @ghidraAddress 0x1000ee20
 */
void JoypadUnsubscribe(MsgSink *sink);

/**
 * Give a controller to a player, or take it from its player.
 *
 * @param iPlayerNum The player, or -1 for none.
 * @param iPadNum The controller.
 * @ghidraAddress 0x1000ee30
 */
void JoypadAssignPadToPlayer(int iPlayerNum, int iPadNum);

/**
 * Create DirectInput and a DIJoypad for each attached joystick, giving joystick n to player n.
 *
 * @ghidraAddress 0x1000b8a0
 */
void JoypadInit();

/**
 * Release the joysticks and DirectInput.
 *
 * @ghidraAddress 0x1000b930
 */
void JoypadTerminate();
