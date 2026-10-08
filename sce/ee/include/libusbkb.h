#ifndef LIBUSBKB_H
#define LIBUSBKB_H

#ifdef __cplusplus
extern "C" {
#endif

/** USB keyboard access through the usbkbd server on the IOP. */

/** Key codes one sceUsbKbRead() report can include. */
#define USBKB_MAX_KEYCODES 62

/** Keyboards one sceUsbKbGetInfo() report can describe. */
#define USBKB_MAX_STATUS 127

/** Bit of a key code that marks a key with no character. */
#define USBKB_RAWDAT 0x8000

/** One report of key presses. */
typedef struct {
    unsigned int led;                           /*!< The lit indicators. */
    unsigned int mkey;                          /*!< The modifier keys held. */
    int len;                                    /*!< Entries of keycode. */
    unsigned short keycode[USBKB_MAX_KEYCODES]; /*!< The keys pressed. */
} USBKBDATA_t;

/** Report of the keyboards attached. */
typedef struct {
    int max_connect;                        /*!< Keyboards the server supports. */
    int now_connect;                        /*!< Keyboards attached. */
    unsigned char status[USBKB_MAX_STATUS]; /*!< Non-zero for each attached keyboard. */
} USBKBINFO_t;

/**
 * Bind the usbkbd server.
 *
 * @param pMaxConnect Receives the keyboards the server supports.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310618
 * @ghidraAddress PAL: 0x0037cbc8
 */
int sceUsbKbInit(int *pMaxConnect);

/**
 * Release the usbkbd server.
 *
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310810
 * @ghidraAddress PAL: 0x0037cdc0
 */
int sceUsbKbEnd(void);

/**
 * Start reading the keyboard report. sceUsbKbSync() finishes the request.
 *
 * @param pInfo Receives the report.
 * @return Zero when the request started.
 * @ghidraAddress NTSC-U/C: 0x003108d8
 * @ghidraAddress PAL: 0x0037ce88
 */
int sceUsbKbGetInfo(USBKBINFO_t *pInfo);

/**
 * Start reading the key presses of a keyboard. sceUsbKbSync() finishes the request.
 *
 * @param nNo The keyboard.
 * @param pData Receives the key presses.
 * @return Zero when the request started.
 * @ghidraAddress NTSC-U/C: 0x00310980
 * @ghidraAddress PAL: 0x0037cf30
 */
int sceUsbKbRead(unsigned int nNo, USBKBDATA_t *pData);

/**
 * Choose how a keyboard's indicators follow the lock keys.
 *
 * @param nNo The keyboard.
 * @param nMode The mode.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310a68
 * @ghidraAddress PAL: 0x0037d018
 */
int sceUsbKbSetLEDMode(unsigned int nNo, int nMode);

/**
 * Set a keyboard's key repeat.
 *
 * @param nNo The keyboard.
 * @param nStart The delay before the first repeat.
 * @param nInterval The delay between repeats.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310b68
 * @ghidraAddress PAL: 0x0037d118
 */
int sceUsbKbSetRepeat(unsigned int nNo, int nStart, int nInterval);

/**
 * Choose the form of a keyboard's key codes.
 *
 * @param nNo The keyboard.
 * @param nType The form.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310bb0
 * @ghidraAddress PAL: 0x0037d160
 */
int sceUsbKbSetCodeType(unsigned int nNo, int nType);

/**
 * Choose a keyboard's layout.
 *
 * @param nNo The keyboard.
 * @param nArrangement The layout.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00310c00
 * @ghidraAddress PAL: 0x0037d1b0
 */
int sceUsbKbSetArrangement(unsigned int nNo, int nArrangement);

/**
 * Wait for, or poll, the latest request.
 *
 * @param nMode 0 to wait, 1 to poll.
 * @param pResult Receives the request's result.
 * @return Zero when the request finished.
 * @ghidraAddress NTSC-U/C: 0x00310c50
 * @ghidraAddress PAL: 0x0037d200
 */
int sceUsbKbSync(int nMode, int *pResult);

/**
 * Choose whether a keyboard reports every key change or only presses.
 *
 * @param nNo The keyboard.
 * @param nMode The mode.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x00311870
 * @ghidraAddress PAL: 0x0037de20
 */
int sceUsbKbSetReadMode(unsigned int nNo, int nMode);

/**
 * Discard a keyboard's buffered key presses.
 *
 * @param nNo The keyboard.
 * @return Zero on success.
 * @ghidraAddress NTSC-U/C: 0x003118e0
 * @ghidraAddress PAL: 0x0037de90
 */
int sceUsbKbClearRbuf(unsigned int nNo);

#ifdef __cplusplus
}
#endif

#endif
