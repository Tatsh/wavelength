#pragma once

#include "app/msgsink.h"

/** The number of analogue sticks of a controller. */
constexpr int kJoypadNumSticks = 2;

/** The number of buttons of a controller, counting the stick directions. */
constexpr int kJoypadNumButtons = 24;

/** The number of controllers the poll reads, four multitap slots on each of two ports. */
constexpr int kJoypadNumPorts = 8;

/** The number of players the controllers can be given to. */
constexpr int kJoypadNumPads = 4;

/** The pad a port stores while no player uses it. */
constexpr int kJoypadNoPad = -1;

/**
 * Buttons of a controller, the values JoypadInputMsg::mButton takes.
 *
 * The name is inferred.
 */
enum JoypadButton {
    kPadL2 = 0,               /*!< The L2 button. */
    kPadR2 = 1,               /*!< The R2 button. */
    kPadL1 = 2,               /*!< The L1 button. */
    kPadR1 = 3,               /*!< The R1 button. */
    kPadTriangle = 4,         /*!< The triangle button. */
    kPadCircle = 5,           /*!< The circle button. */
    kPadCross = 6,            /*!< The cross button, which chooses in the menus. */
    kPadSquare = 7,           /*!< The square button. */
    kPadSelect = 8,           /*!< The SELECT button. */
    kPadL3 = 9,               /*!< The left stick pressed in. */
    kPadR3 = 10,              /*!< The right stick pressed in. */
    kPadStart = 11,           /*!< The START button. */
    kPadDUp = 12,             /*!< Up on the directional buttons. */
    kPadDRight = 13,          /*!< Right on the directional buttons. */
    kPadDDown = 14,           /*!< Down on the directional buttons. */
    kPadDLeft = 15,           /*!< Left on the directional buttons. */
    kPadLeftStickUp = 16,     /*!< The left stick pushed up past the threshold. */
    kPadLeftStickRight = 17,  /*!< The left stick pushed right past the threshold. */
    kPadLeftStickDown = 18,   /*!< The left stick pushed down past the threshold. */
    kPadLeftStickLeft = 19,   /*!< The left stick pushed left past the threshold. */
    kPadRightStickUp = 20,    /*!< The right stick pushed up past the threshold. */
    kPadRightStickRight = 21, /*!< The right stick pushed right past the threshold. */
    kPadRightStickDown = 22,  /*!< The right stick pushed down past the threshold. */
    kPadRightStickLeft = 23,  /*!< The right stick pushed left past the threshold. */
    kPadNone = 24,            /*!< No button. */
};

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
 * State of one controller port as the last poll read it.
 *
 * The name is inferred. The poll has a table of kJoypadNumPorts at `0x0047fe00`.
 */
class JoypadState {
public:
    /**
     * Clear the buttons, the sticks, and the pressures, and give the port no pad.
     *
     * mThreshold is not set.
     *
     * @ghidraAddress NTSC-U/C: 0x0028a5e8
     * @ghidraAddress PAL: 0x00293de0
     */
    JoypadState();

    /**
     * Set the bits of the stick directions pushed past mThreshold.
     *
     * Bits kPadLeftStickUp to kPadRightStickLeft follow the two sticks. With bMenu set, the left
     * stick also sets the bits of the directional buttons.
     *
     * @param pButtons The button word to add the bits to.
     * @param bMenu Whether the menus receive this controller's input.
     * @ghidraAddress NTSC-U/C: 0x0028ab30
     * @ghidraAddress PAL: 0x00294328
     */
    void AddStickButtons(int *pButtons, bool bMenu);

    int mButtons;                          /*!< The buttons held, one bit for each JoypadButton. */
    JoypadStick mSticks[kJoypadNumSticks]; /*!< The analogue sticks. */
    float mPressures[kJoypadNumButtons];   /*!< Each button's pressure, from 0 to 1. */
    int mPad;                              /*!< The player this port serves, or kJoypadNoPad. */
    int mConnected;                        /*!< Non-zero while the controller is connected. */
    float mThreshold;                      /*!< How far a stick moves before its bit is set. */
};

/**
 * Open every controller port and give the first port to pad 0.
 *
 * Reads the `deadzone` and `threshold` settings of the `joypad` block of the system
 * configuration, 24 and 0.75 by default.
 *
 * @ghidraAddress NTSC-U/C: 0x0028aea0
 * @ghidraAddress PAL: 0x00294698
 */
void JoypadInit();

/**
 * Shut the controller layer down.
 *
 * The body does nothing.
 *
 * @ghidraAddress NTSC-U/C: 0x0028af50
 * @ghidraAddress PAL: 0x00294748
 */
void JoypadTerminate();

/**
 * Read every controller port that serves a pad, and send messages for what changed.
 *
 * Sends a JoypadConnectionMsg when a pad connects or disconnects, a JoypadAnalogStickMsg when a
 * stick moves while JoypadSetStickMessages() is on, and a JoypadInputMsg for each button pressed
 * or released. A digital-only pad is ignored. Before JoypadInit() the call only reports an error.
 *
 * @ghidraAddress NTSC-U/C: 0x0028a780
 * @ghidraAddress PAL: 0x00293f78
 */
void JoypadPoll();

/**
 * Wait for pad 0 to release every button and then press one.
 *
 * Before JoypadInit() the call waits five seconds instead.
 *
 * @return The buttons pad 0 holds, or 0 before JoypadInit().
 * @ghidraAddress NTSC-U/C: 0x0028a6e8
 * @ghidraAddress PAL: 0x00293ee0
 */
int JoypadWait();

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
 * back to the menus. While the menus receive a pad's input, its left stick also presses the
 * directional buttons. The name is inferred.
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

/**
 * Give the controller ports to the pads.
 *
 * With bPort0Multitap set, the four slots of port 0 serve pads 0 to 3. Otherwise slot 0 of port 0
 * serves pad 0, slot 0 of port 1 serves pad 1 unless bPort1Multitap is set, and the other slots
 * serve no pad.
 *
 * @param bPort0Multitap Whether a multitap is connected to port 0.
 * @param bPort1Multitap Whether a multitap is connected to port 1.
 * @ghidraAddress NTSC-U/C: 0x0028b048
 * @ghidraAddress PAL: 0x00294840
 */
void JoypadMapDefault(bool bPort0Multitap, bool bPort1Multitap);

/**
 * Report whether a multitap is connected, as the controller poll last found.
 *
 * The name is inferred from the player count screen. That screen offers three and four players
 * only while the flag is set.
 *
 * @return Non-zero while a multitap is connected.
 * @ghidraAddress NTSC-U/C: 0x0028c4d0
 * @ghidraAddress PAL: 0x00295e20
 */
int JoypadMultitapConnected();
