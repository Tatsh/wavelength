#ifndef RTCLOCK_H
#define RTCLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/** Clocks of the SCE-RT library. The routines report 0 for success. */

/** A span of time. */
typedef struct {
    unsigned int uSeconds;      /*!< Whole seconds. */
    unsigned int uMicroseconds; /*!< Microseconds within the second. */
} RtTime;

/**
 * Report the time since the uptime clock started.
 *
 * @param pTime Receives the time.
 * @return 0, 1 when the clock failed, or 2 when pTime is null.
 * @ghidraAddress NTSC-U/C: 0x002aad48
 * @ghidraAddress PAL: 0x002b4cb8
 */
int RtGetUptime(RtTime *pTime);

/**
 * Write the time of day as `hh:mm:ss`, from the real-time clock read once and the uptime since.
 *
 * @param pszOut Receives the text, 9 bytes.
 * @return 0, 1 when a clock failed, or 2 when pszOut is null.
 * @ghidraAddress NTSC-U/C: 0x002aad9c
 * @ghidraAddress PAL: 0x002b4d0c
 */
int RtFormatWallClock(char *pszOut);

/**
 * Read the cycle counter of the CPU.
 *
 * @return The counter.
 * @ghidraAddress NTSC-U/C: 0x002aafd0
 * @ghidraAddress PAL: 0x002b4f40
 */
unsigned int RtReadCycleCounter(void);

/**
 * Start the uptime clock at zero, once.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002aafe0
 * @ghidraAddress PAL: 0x002b4f50
 */
int RtStartUptime(void);

/**
 * Advance the uptime clock by the cycles counted since the last call.
 *
 * @return 0, or 1 when the clock could not start.
 * @ghidraAddress NTSC-U/C: 0x002ab040
 * @ghidraAddress PAL: 0x002b4fb0
 */
int RtUpdateUptime(void);

#ifdef __cplusplus
}
#endif

#endif
