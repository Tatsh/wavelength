#ifndef LIBPAD_H
#define LIBPAD_H

#ifdef __cplusplus
extern "C" {
#endif

/** Controller access through the padman server on the IOP. */

/** Size of the DMA area of one controller, in 16-byte units. */
#define scePadDmaBufferMax 16

/** States scePadGetState() reports. */
#define scePadStateDiscon 0   /*!< No controller is connected. */
#define scePadStateFindPad 1  /*!< The IOP is looking for a controller. */
#define scePadStateFindCTP1 2 /*!< The IOP is identifying the controller. */
#define scePadStateExecCmd 5  /*!< A command to the controller is running. */
#define scePadStateStable 6   /*!< The controller is ready. */
#define scePadStateError 7    /*!< The controller failed. */
#define scePadStateClosed 99  /*!< The port is not open. */

/** Progress scePadGetReqState() reports. */
#define scePadReqStateComplete 0 /*!< The last request finished. */
#define scePadReqStateFaild 1    /*!< The last request failed. */
#define scePadReqStateBusy 2     /*!< A request is running. */

/** Terms of scePadInfoMode(). */
#define InfoModeCurID 1     /*!< Identifier of the current mode. */
#define InfoModeCurExID 2   /*!< Extended identifier of the current mode. */
#define InfoModeCurExOffs 3 /*!< Offset of the current mode in the identifier table. */
#define InfoModeIdTable 4   /*!< An entry of the identifier table. */

/** Terms of scePadInfoAct(). */
#define InfoActFunc 1 /*!< Function of the actuator. */
#define InfoActSub 2  /*!< Sub-function of the actuator. */
#define InfoActSize 3 /*!< Size of the actuator's data. */
#define InfoActCurr 4 /*!< Current the actuator draws. */

/**
 * One of the two frames of a controller's DMA area. The IOP writes the two frames in turn.
 *
 * The frame with the larger count is the newer.
 */
typedef struct {
    unsigned char abData[32];         /*!< The latest report. */
    unsigned char abReserved32[16];   /*!< Undetermined. */
    unsigned char aabActInfo[4][4];   /*!< The four scePadInfoAct() terms of each actuator. */
    unsigned char aabCombInfo[4][4];  /*!< Count and members of each actuator combination. */
    unsigned short anModeTable[4];    /*!< Extended identifiers of the modes. */
    unsigned int nFrame;              /*!< Count of the frame. */
    unsigned int nReserved92;         /*!< Undetermined. */
    unsigned int nLength;             /*!< Size of the report in bytes. */
    unsigned char nModeConfig;        /*!< 1 when the mode tables are absent. */
    unsigned char nModeCurrentId;     /*!< Identifier of the current mode, in the high nibble. */
    unsigned char nModel;             /*!< Controller model. */
    unsigned char nReportReady;       /*!< Nonzero once a report was read. */
    unsigned char nModeCount;         /*!< Entries of anModeTable. */
    unsigned char nModeCurrentOffset; /*!< Index of the current mode in anModeTable. */
    unsigned char nActuatorCount;     /*!< Entries of aabActInfo. */
    unsigned char nCombinationCount;  /*!< Entries of aabCombInfo. */
    unsigned char abReserved108[4];   /*!< Undetermined. */
    unsigned char nState;             /*!< scePadState value. */
    unsigned char nReqState;          /*!< scePadReqState value. */
    unsigned char nCurrentTask;       /*!< 1 while the mode and actuator tables are valid. */
    unsigned char abReserved115[6];   /*!< Undetermined. */
    unsigned char abButtonMask[4];    /*!< Pressure-sensitive buttons, least significant first. */
    unsigned char abReserved125[3];   /*!< Undetermined. */
} scePadDmaFrame;

/**
 * Bind the pad servers, check the padman version, and reset the ports.
 *
 * @param nMode Passed to scePadInit2().
 * @return The scePadInit2() result, or zero on a version mismatch.
 * @ghidraAddress NTSC-U/C: 0x00311e08
 * @ghidraAddress PAL: 0x0037e3b8
 */
int scePadInit(int nMode);

/**
 * Close every port record and reset padman.
 *
 * @param nMode Not used.
 * @return padman's result, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00311f48
 * @ghidraAddress PAL: 0x0037e4f8
 */
int scePadInit2(int nMode);

/**
 * Start reading a controller into a DMA area.
 *
 * @param nPort Port.
 * @param nSlot Slot of a multitap, or zero.
 * @param pFrames Two frames aligned to 64 bytes. They must remain valid until the port closes.
 * @return padman's result, or zero when the area is misaligned, the port is open, or the call
 * failed.
 * @ghidraAddress NTSC-U/C: 0x00311fe8
 * @ghidraAddress PAL: 0x0037e598
 */
int scePadPortOpen(int nPort, int nSlot, scePadDmaFrame *pFrames);

/**
 * Pick the newer frame of a port's DMA area.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return The frame.
 * @ghidraAddress NTSC-U/C: 0x003121d0
 * @ghidraAddress PAL: 0x0037e780
 */
scePadDmaFrame *scePadGetDmaStr(int nPort, int nSlot);

/**
 * Copy the latest report of a controller.
 *
 * Bytes 2 and 3 of a report are the button bits, active low.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pData Receives the report, 32 bytes at most.
 * @return The report size, or zero when the port is closed.
 * @ghidraAddress NTSC-U/C: 0x00312230
 */
int scePadRead(int nPort, int nSlot, unsigned char *pData);

/**
 * Report the state of a controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return A scePadState value. scePadStateExecCmd replaces scePadStateStable while a request
 * runs.
 * @ghidraAddress NTSC-U/C: 0x003122b0
 * @ghidraAddress PAL: 0x0037e948
 */
int scePadGetState(int nPort, int nSlot);

/**
 * Overwrite the request progress of a controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param nState scePadReqState value.
 * @return 1, or zero when the port is closed.
 * @ghidraAddress NTSC-U/C: 0x00312330
 * @ghidraAddress PAL: 0x0037e9c8
 */
int scePadSetReqState(int nPort, int nSlot, int nState);

/**
 * Report the request progress of a controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return A scePadReqState value, or zero when the port is closed.
 * @ghidraAddress NTSC-U/C: 0x00312398
 * @ghidraAddress PAL: 0x0037ea30
 */
int scePadGetReqState(int nPort, int nSlot);

/**
 * Describe an actuator of a controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param nActuator Actuator index, or -1 for the actuator count.
 * @param nTerm InfoAct term.
 * @return The value, or zero when the tables are not valid.
 * @ghidraAddress NTSC-U/C: 0x003123f0
 * @ghidraAddress PAL: 0x0037ea88
 */
int scePadInfoAct(int nPort, int nSlot, int nActuator, int nTerm);

/**
 * Describe the modes of a controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param nTerm InfoMode term.
 * @param nOffset Table index for InfoModeIdTable, or -1 for the table size.
 * @return The value, or zero when the tables are not valid or a request runs.
 * @ghidraAddress NTSC-U/C: 0x00312518
 * @ghidraAddress PAL: 0x0037ebb0
 */
int scePadInfoMode(int nPort, int nSlot, int nTerm, int nOffset);

/**
 * Request a mode change.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param nOffset Index of the mode in the identifier table.
 * @param nLock 3 to lock the mode against the controller's button.
 * @return padman's result, 1 once the request started, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312650
 * @ghidraAddress PAL: 0x0037ece8
 */
int scePadSetMainMode(int nPort, int nSlot, int nOffset, int nLock);

/**
 * Send actuator values straight to the controller.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pData Six actuator bytes.
 * @return 1, or zero when the tables are not valid or the transfer to the IOP failed.
 * @ghidraAddress NTSC-U/C: 0x00312708
 * @ghidraAddress PAL: 0x0037eda0
 */
int scePadSetActDirect(int nPort, int nSlot, const unsigned char *pData);

/**
 * Assign each of the six actuator bytes an actuator.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param pData Six actuator indices, 0xff for none.
 * @return padman's result, 1 once the request started, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x003127c8
 * @ghidraAddress PAL: 0x0037ee60
 */
int scePadSetActAlign(int nPort, int nSlot, const unsigned char *pData);

/**
 * Report which buttons are pressure-sensitive.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return The button mask, or zero when the port is closed or the controller has none.
 * @ghidraAddress NTSC-U/C: 0x003128a0
 * @ghidraAddress PAL: 0x0037ef38
 */
int scePadGetButtonMask(int nPort, int nSlot);

/**
 * Request the pressure-sensitive buttons.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @param nMask Buttons to report as pressures.
 * @return padman's result, 1 once the request started, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312958
 * @ghidraAddress PAL: 0x0037eff0
 */
int scePadSetButtonInfo(int nPort, int nSlot, int nMask);

/**
 * Report whether a controller can report pressures.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return 1 when every button is pressure-sensitive, or zero.
 * @ghidraAddress NTSC-U/C: 0x00312a08
 * @ghidraAddress PAL: 0x0037f0a0
 */
int scePadInfoPressMode(int nPort, int nSlot);

/**
 * Request pressure reports for every button.
 *
 * @param nPort Port.
 * @param nSlot Slot.
 * @return padman's result, 1 once the request started, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312a68
 * @ghidraAddress PAL: 0x0037f100
 */
int scePadEnterPressMode(int nPort, int nSlot);

/**
 * Report the padman version.
 *
 * @return The major version in bits 8 to 15 and the minor in bits 0 to 7, or zero when the call
 * failed.
 * @ghidraAddress NTSC-U/C: 0x00312ad8
 * @ghidraAddress PAL: 0x0037f170
 */
int scePadGetModVersion(void);

#ifdef __cplusplus
}
#endif

#endif
