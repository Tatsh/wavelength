#pragma once

#include <windows.h>

#include <dinput.h>

/**
 * Joystick of DirectInput.
 *
 * The RTTI includes the class name. The object is 0x14 bytes.
 */
class DIJoypad {
public:
    /**
     * Create the device of a joystick, set its data format, axis ranges, dead zone, and buffer,
     * and acquire it.
     *
     * @param instance The joystick DirectInput enumerated.
     * @ghidraAddress 0x1000be00
     */
    explicit DIJoypad(const DIDEVICEINSTANCE *instance);

    /**
     * Release the devices.
     *
     * @ghidraAddress 0x1000c0e0
     */
    ~DIJoypad();

    /**
     * Acquire the device, reporting a failure.
     *
     * @return Whether the device is acquired.
     * @ghidraAddress 0x1000c040
     */
    bool Acquire();

    /**
     * Set a property of the X, Y, Z, and Z rotation axes.
     *
     * @param property The property.
     * @param header The header of the property's value. The routine sets its object.
     * @ghidraAddress 0x1000c080
     */
    void SetAxesProperty(REFGUID property, LPDIPROPHEADER header);

    DWORD mJoystickId;           /*!< The joystick's identifier. The joysticks are sorted by it. */
    LPDIRECTINPUTDEVICE mDevice; /*!< The device. */
    LPDIRECTINPUTDEVICE2 mDevice2; /*!< The device's second interface. */
    int mReserved0c;               // +0x0c, cleared by the constructor and not read.
    bool mReserved10;              // +0x10, cleared by the constructor and not read.
};

/**
 * Print the name of a DirectInput error.
 *
 * @param hr The error.
 * @ghidraAddress 0x1000c140
 */
void PrintDIError(HRESULT hr);

/**
 * Dead zone of the joystick axes, from `deadzone` of the `joypad` configuration array, where 128
 * is the whole range.
 *
 * @ghidraAddress 0x10038850
 */
extern int gDeadzone;

/**
 * The DirectInput object.
 *
 * @ghidraAddress 0x10040c68
 */
extern LPDIRECTINPUT gLpdi;
