#include "rt/rtclock.h"

#include <stdio.h>

#include <libcdvd.h>
#include <libscf.h>

#include "rt/rttime.h"

enum {
    kClockResultFailed = 1,
    kClockResultInvalid = 2,
    kSecondsPerMinute = 60,
    kMinutesPerHour = 60,
    kHoursPerDay = 24,
    kBcdBase = 10,
    kBcdDigitBits = 4,
    kBcdDigitMask = 0xf,
};

// The CPU counts 294912000 cycles a second.
#define CYCLES_PER_SECOND 294912000U
// The factor is 1 / 294.912 rounded down to a float.
#define MICROSECONDS_PER_CYCLE 0x1.bc71c6p-9f

// The uptime clock.
typedef struct {
    unsigned int uSeconds;
    unsigned int uCycles; // Cycles within the second.
    unsigned int uMicroseconds;
    unsigned int uLastCount;
} RtUptime;

// The real-time clock as it was read once, with the uptime it was read at.
typedef struct {
    int nSecond;
    int nMinute;
    int nHour;
    int nDay;
    int nMonth;
    int nYear;
    unsigned int uBaseSeconds;
} RtWallClock;

// NTSC-U/C: 0x003b6b78, PAL: 0x00435a38
static unsigned char g_bUptimeStarted;
// NTSC-U/C: 0x003b6b79, PAL: 0x00435a39
static unsigned char g_bWallClockRead;
// NTSC-U/C: 0x00519ee0
static RtUptime g_uptime;
// NTSC-U/C: 0x00519ef0
static RtWallClock g_wallClock;

static inline int RtFromBcd(unsigned char uValue) {
    return (uValue >> kBcdDigitBits) * kBcdBase + (uValue & kBcdDigitMask);
}

int RtGetUptime(RtTime *pTime) {
    if (pTime == NULL) {
        return kClockResultInvalid;
    }
    if (RtUpdateUptime() != 0) {
        return kClockResultFailed;
    }
    pTime->uSeconds = g_uptime.uSeconds;
    pTime->uMicroseconds = g_uptime.uMicroseconds;
    return 0;
}

int RtFormatWallClock(char *pszOut) {
    sceCdCLOCK clock;
    RtWallClock wallClock;
    unsigned int uNow;

    if (pszOut == NULL) {
        return kClockResultInvalid;
    }
    if (!g_bWallClockRead) {
        sceCdReadClock(&clock);
        sceScfGetLocalTimefromRTC(&clock);
        g_wallClock.nSecond = RtFromBcd(clock.second);
        g_wallClock.nMinute = RtFromBcd(clock.minute);
        g_wallClock.nHour = RtFromBcd(clock.hour);
        g_wallClock.nDay = RtFromBcd(clock.day);
        g_wallClock.nMonth = RtFromBcd(clock.month);
        g_wallClock.nYear = RtFromBcd(clock.year);
        if (RtGetTimeSeconds(&g_wallClock.uBaseSeconds) != 0) {
            return kClockResultFailed;
        }
        g_bWallClockRead = 1;
    }
    wallClock = g_wallClock;
    if (RtGetTimeSeconds(&uNow) != 0) {
        return kClockResultFailed;
    }
    wallClock.nSecond += uNow - g_wallClock.uBaseSeconds;
    wallClock.nMinute += (unsigned int)wallClock.nSecond / kSecondsPerMinute;
    wallClock.nSecond = (unsigned int)wallClock.nSecond % kSecondsPerMinute;
    wallClock.nHour += (unsigned int)wallClock.nMinute / kMinutesPerHour;
    wallClock.nMinute = (unsigned int)wallClock.nMinute % kMinutesPerHour;
    wallClock.nHour = (unsigned int)wallClock.nHour % kHoursPerDay;
    sprintf(pszOut, "%02d:%02d:%02d", wallClock.nHour, wallClock.nMinute, wallClock.nSecond);
    return 0;
}

unsigned int RtReadCycleCounter(void) {
    unsigned int uCount;

    __asm__ volatile("mfc0 %0, $9\n\tsync.p" : "=r"(uCount));
    return uCount;
}

int RtStartUptime(void) {
    if (!g_bUptimeStarted) {
        g_uptime.uSeconds = 0;
        g_uptime.uCycles = 0;
        g_uptime.uMicroseconds = 0;
        g_uptime.uLastCount = RtReadCycleCounter();
        g_bUptimeStarted = 1;
    }
    return 0;
}

int RtUpdateUptime(void) {
    unsigned int uCount;
    unsigned int uElapsed;

    if (!g_bUptimeStarted && RtStartUptime() != 0) {
        return kClockResultFailed;
    }
    uCount = RtReadCycleCounter();
    uElapsed = uCount - g_uptime.uLastCount;
    g_uptime.uLastCount = uCount;
    if (uElapsed != 0) {
        g_uptime.uCycles += uElapsed;
        if (g_uptime.uCycles > CYCLES_PER_SECOND) {
            do {
                g_uptime.uCycles -= CYCLES_PER_SECOND;
                ++g_uptime.uSeconds;
            } while (g_uptime.uCycles > CYCLES_PER_SECOND);
        }
        g_uptime.uMicroseconds = (unsigned int)((float)g_uptime.uCycles * MICROSECONDS_PER_CYCLE);
    }
    return 0;
}
