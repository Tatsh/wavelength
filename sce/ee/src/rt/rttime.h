#ifndef RTTIME_H
#define RTTIME_H

#include "rt/rtclock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Time since the SCE-RT library first read its clock. The routines report 0 for success, 2 for a
 * null argument, and 3 when a time is too long to report in milliseconds.
 */

/**
 * Report the milliseconds since the first reading.
 *
 * @param puMilliseconds Receives the milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9db4
 * @ghidraAddress PAL: 0x002d3d24
 */
int RtGetMilliseconds(unsigned int *puMilliseconds);

/**
 * Report the milliseconds since the first reading.
 *
 * @param puMilliseconds Receives the milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9dd0
 * @ghidraAddress PAL: 0x002d3d40
 */
int RtGetElapsedMilliseconds(unsigned int *puMilliseconds);

/**
 * Report the whole seconds since the first reading.
 *
 * @param puSeconds Receives the seconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9e14
 * @ghidraAddress PAL: 0x002d3d84
 */
int RtGetTimeSeconds(unsigned int *puSeconds);

/**
 * Read the clock.
 *
 * @param pTime Receives the reading.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9e58
 * @ghidraAddress PAL: 0x002d3dc8
 */
int RtReadTime(RtTime *pTime);

/**
 * Read the clock.
 *
 * @param pTime Receives the reading.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9e74
 * @ghidraAddress PAL: 0x002d3de4
 */
int RtReadClock(RtTime *pTime);

/**
 * Report the milliseconds since a reading.
 *
 * @param pSince The reading.
 * @param puMilliseconds Receives the milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9e90
 * @ghidraAddress PAL: 0x002d3e00
 */
int RtGetMillisecondsSince(const RtTime *pSince, unsigned int *puMilliseconds);

/**
 * Report the milliseconds since a reading.
 *
 * @param pSince The reading.
 * @param puMilliseconds Receives the milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9eac
 * @ghidraAddress PAL: 0x002d3e1c
 */
int RtMeasureMillisecondsSince(const RtTime *pSince, unsigned int *puMilliseconds);

/**
 * Spin until a number of milliseconds passed.
 *
 * @param uMilliseconds The milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9ef8
 * @ghidraAddress PAL: 0x002d3e68
 */
int RtSpinMilliseconds(unsigned int uMilliseconds);

/**
 * Take the first reading of the clock, once.
 *
 * @return 0, 1 when the reading was already taken, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9f50
 * @ghidraAddress PAL: 0x002d3ec0
 */
int RtReadBaseTime(void);

/**
 * Subtract a reading from a later one. A later reading before the first gives zero.
 *
 * @param pFrom The earlier reading.
 * @param pTo The later reading.
 * @param pDifference Receives the difference.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9f9c
 * @ghidraAddress PAL: 0x002d3f0c
 */
int RtSubtractTime(const RtTime *pFrom, const RtTime *pTo, RtTime *pDifference);

/**
 * Convert a time to whole seconds.
 *
 * @param pTime The time.
 * @param puSeconds Receives the seconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ca060
 * @ghidraAddress PAL: 0x002d3fd0
 */
int RtTimeToSeconds(const RtTime *pTime, unsigned int *puSeconds);

/**
 * Convert a time to milliseconds.
 *
 * @param pTime The time.
 * @param puMilliseconds Receives the milliseconds.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ca0ac
 * @ghidraAddress PAL: 0x002d401c
 */
int RtTimeToMilliseconds(const RtTime *pTime, unsigned int *puMilliseconds);

/**
 * Report the time since the first reading.
 *
 * @param pTime Receives the time.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ca114
 * @ghidraAddress PAL: 0x002d4084
 */
int RtGetElapsedTime(RtTime *pTime);

/**
 * Report the time since a reading.
 *
 * @param pSince The reading.
 * @param pTime Receives the time.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ca16c
 * @ghidraAddress PAL: 0x002d40dc
 */
int RtGetTimeSince(const RtTime *pSince, RtTime *pTime);

#ifdef __cplusplus
}
#endif

#endif
