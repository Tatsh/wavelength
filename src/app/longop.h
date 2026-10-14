#pragma once

/** Callback that a long operation invokes while it blocks the frame loop. */
typedef void (*LongOperationProc)();

/**
 * Install the callback that a long operation pumps its timers through.
 *
 * A null callback removes the previous one.
 *
 * @param pfnPoll The callback to install.
 */
void SetLongOperationPollProc(LongOperationProc pfnPoll);

/**
 * Call the installed poll callback, if there is one.
 *
 * GfxDevice::FlushGifPacket() calls it after every packet it submits.
 */
void RunLongOperationPollProc();

/**
 * Install the callback that a long operation refreshes the display through.
 *
 * A null callback removes the previous one.
 *
 * @param pfnDraw The callback to install.
 */
void SetLongOperationDrawProc(LongOperationProc pfnDraw);

/**
 * Call the installed draw callback, if there is one.
 */
void RunLongOperationDrawProc();

/**
 * The installed poll callback, or null.
 */
extern LongOperationProc g_pfnLongOperationPollProc;

/**
 * The installed draw callback, or null.
 */
extern LongOperationProc g_pfnLongOperationDrawProc;
