#pragma once

#include <libpad.h>

/**
 * One controller's pad-library state: the DMA area libpad fills and the decoder's working state.
 *
 * The class is not polymorphic and emits no RTTI. The title comes from the prefix of its messages.
 * The joypad layer has a table of eight at `0x00480240`, one per port and multitap slot. The record
 * is 0x180 bytes. The first 0x100 bytes are the area scePadPortOpen() receives, and the record
 * therefore starts on a 64-byte boundary.
 */
class BreugPad {
public:
    /** Bytes of the actuator buffers scePadSetActDirect() and scePadSetActAlign() receive. */
    static constexpr int kActuatorByteCount = 6;

    /** Bytes of the pressure baseline Read() subtracts from each pressure reading. */
    static constexpr int kPressureByteCount = 12;

    /** Frames of the DMA area scePadPortOpen() receives. */
    static constexpr int kDmaFrameCount = 2;

    /** Entries of mLastAxes. */
    static constexpr int kLastAxisCount = 4;

    /** Entries of mAxisDeltas. */
    static constexpr int kAxisDeltaCount = 4;

    /**
     * Drive the two vibration motors.
     *
     * Does nothing until mReadyLevel arrives at 2. Otherwise the small motor runs whenever
     * nSmallMotor is positive, and the big motor takes nBigMotor as its level.
     *
     * @param nSmallMotor Positive to run the small motor.
     * @param nBigMotor The big motor's level.
     * @ghidraAddress NTSC-U/C: 0x0028d4a0
     * @ghidraAddress PAL: 0x00296e80
     */
    void SetActuators(int nSmallMotor, unsigned char nBigMotor);

    /**
     * Reset the record and open the pad at nPort and nSlot.
     *
     * Clears the actuator levels and the pressure baseline, aligns actuator 0 to byte 0 and
     * actuator 1 to byte 1 with the rest unused, starts libpad through scePadInit(0) the first time
     * any record opens, and opens the port with mDmaArea. Also sets the scale of ApplyDeadzone()
     * from nDeadZone, which every record shares.
     *
     * @param nPort The port, from 0.
     * @param nSlot The multitap slot, from 0.
     * @param nDeadZone Stored in mDeadZone.
     * @ghidraAddress NTSC-U/C: 0x0028d4e8
     * @ghidraAddress PAL: 0x00296ec8
     */
    void Init(int nPort, int nSlot, int nDeadZone);

    /**
     * Rescale an analog axis centred on zero to remove the dead zone.
     *
     * A magnitude below nDeadZone reads 0. A larger one maps linearly from the dead zone's edge to
     * 127, rounded, with the sign of nAxis.
     *
     * @param nAxis The axis, from -128 to 127.
     * @param nDeadZone The dead zone's magnitude.
     * @return The rescaled axis.
     * @ghidraAddress NTSC-U/C: 0x0028d650
     * @ghidraAddress PAL: 0x00297030
     */
    static short ApplyDeadzone(short nAxis, int nDeadZone);

    /**
     * Advance the pad's setup state machine and decode the latest report.
     *
     * mPhase indexes a 78-entry jump table at `0x0041c5e0` that walks the pad through analog mode,
     * actuator alignment, and pressure-sensitive mode, and mReadyLevel records how far it has got.
     * Each non-null output receives one value of the decoded report, the analog values centred on
     * zero and passed through ApplyDeadzone(). A pad that is not stable reports only mButtons and
     * zero axes.
     *
     * @param pButtons Receives the button word at mButtons, or null.
     * @param pAxis0 Receives the left stick's horizontal axis, or null.
     * @param pAxis1 Receives the left stick's vertical axis, or null.
     * @param pAxis2 Receives the right stick's horizontal axis, or null.
     * @param pAxis3 Receives the right stick's vertical axis, or null.
     * @param pPressures Receives the twelve pressure bytes, or null.
     * @param pPressureDeltas Receives each pressure less its baseline, or null.
     * @return mReadyLevel, or 0 when the read fails.
     * @ghidraAddress NTSC-U/C: 0x0028d6f8
     * @ghidraAddress PAL: 0x002970d8
     */
    int Read(int *pButtons,
             unsigned char *pAxis0,
             unsigned char *pAxis1,
             unsigned char *pAxis2,
             unsigned char *pAxis3,
             unsigned char *pPressures,
             short *pPressureDeltas);

    /** The area libpad writes reports into. +0x000 */
    alignas(64) scePadDmaFrame mDmaArea[kDmaFrameCount];

    /** The decoded button word Read() reports. +0x100 */
    int mButtons;

    int mHeldButtonsSeen; // +0x104, every button held since Init()
    int mToggledButtons;  // +0x108, each new press toggles its bit
    int mPreviousButtons; // +0x10c, mButtons before the latest report

    /** The port Init() received. +0x110 */
    int mPort;

    /** The multitap slot Init() received. +0x114 */
    int mSlot;

    unsigned short mRawButtons; // +0x118, the latest report's button word

    /** Index into Read()'s setup state machine. +0x11c */
    int mPhase;

    int mReportMode; // +0x120, the latest report's mode byte

    /**
     * Setup progress Read() records, from 0 (no pad) through 1 (digital) and 2 (analog) to 3
     * (pressure-sensitive). +0x124
     */
    int mReadyLevel;

    int mDeadZone;                           // +0x128, Init()'s fourth argument
    int mReadCount;                          // +0x12c, counted by Read()
    unsigned char mLastAxes[kLastAxisCount]; // +0x130, cleared by Init() and otherwise unused
    short mAxisDeltas[kAxisDeltaCount];      // +0x134, cleared by Init() and otherwise unused

    /** The pressure baseline Read() subtracts. +0x13c */
    unsigned char mPressureBaseline[kPressureByteCount];

    /** The actuator levels SetActuators() sends. +0x148 */
    unsigned char mActDirect[kActuatorByteCount];

    /** The actuator alignment Read() sends during setup. +0x14e */
    unsigned char mActAlign[kActuatorByteCount];
};
