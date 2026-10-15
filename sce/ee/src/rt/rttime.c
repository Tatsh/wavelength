#include "rt/rttime.h"

#include <stddef.h>

enum {
    kTimeResultAlreadyRead = 1,
    kTimeResultInvalid = 2,
    kTimeResultOverflow = 3,
    kMillisecondsPerSecond = 1000,
    kMicrosecondsPerMillisecond = 1000,
    kMicrosecondsPerSecond = 1000000,
    // The most seconds a count of milliseconds can have.
    kMaxMillisecondSeconds = 4294967,
};

// NTSC-U/C: 0x003b6d1c, PAL: 0x00435bdc
static unsigned char g_bBaseTimeRead;
// NTSC-U/C: 0x005219f8
static RtTime g_baseTime;

int RtGetMilliseconds(unsigned int *puMilliseconds) {
    return RtGetElapsedMilliseconds(puMilliseconds);
}

int RtGetElapsedMilliseconds(unsigned int *puMilliseconds) {
    RtTime time;
    int nResult;

    if (puMilliseconds == NULL) {
        return kTimeResultInvalid;
    }
    nResult = RtGetElapsedTime(&time);
    if (nResult != 0) {
        return nResult;
    }
    return RtTimeToMilliseconds(&time, puMilliseconds);
}

int RtGetTimeSeconds(unsigned int *puSeconds) {
    RtTime time;
    int nResult;

    if (puSeconds == NULL) {
        return kTimeResultInvalid;
    }
    nResult = RtGetElapsedTime(&time);
    if (nResult != 0) {
        return nResult;
    }
    return RtTimeToSeconds(&time, puSeconds);
}

int RtReadTime(RtTime *pTime) {
    return RtReadClock(pTime);
}

int RtReadClock(RtTime *pTime) {
    return RtGetUptime(pTime);
}

int RtGetMillisecondsSince(const RtTime *pSince, unsigned int *puMilliseconds) {
    return RtMeasureMillisecondsSince(pSince, puMilliseconds);
}

int RtMeasureMillisecondsSince(const RtTime *pSince, unsigned int *puMilliseconds) {
    RtTime time;
    int nResult;

    if (pSince == NULL || puMilliseconds == NULL) {
        return kTimeResultInvalid;
    }
    nResult = RtGetTimeSince(pSince, &time);
    if (nResult != 0) {
        return nResult;
    }
    return RtTimeToMilliseconds(&time, puMilliseconds);
}

int RtSpinMilliseconds(unsigned int uMilliseconds) {
    RtTime start;
    unsigned int uElapsed;
    int nResult;

    uElapsed = 0;
    nResult = RtReadTime(&start);
    while (nResult == 0) {
        if (uElapsed >= uMilliseconds) {
            return 0;
        }
        nResult = RtGetMillisecondsSince(&start, &uElapsed);
    }
    return nResult;
}

int RtReadBaseTime(void) {
    int nResult;

    if (g_bBaseTimeRead == 1) {
        return kTimeResultAlreadyRead;
    }
    nResult = RtReadClock(&g_baseTime);
    if (nResult != 0) {
        return nResult;
    }
    g_bBaseTimeRead = 1;
    return 0;
}

int RtSubtractTime(const RtTime *pFrom, const RtTime *pTo, RtTime *pDifference) {
    if (pFrom == NULL || pTo == NULL || pDifference == NULL) {
        return kTimeResultInvalid;
    }
    if (pFrom->uSeconds == pTo->uSeconds) {
        pDifference->uSeconds = 0;
        if (pTo->uMicroseconds < pFrom->uMicroseconds) {
            pDifference->uMicroseconds = 0;
            return 0;
        }
        pDifference->uMicroseconds = pTo->uMicroseconds - pFrom->uMicroseconds;
        return 0;
    }
    if (pFrom->uSeconds >= pTo->uSeconds) {
        pDifference->uSeconds = 0;
        pDifference->uMicroseconds = 0;
        return 0;
    }
    if (pTo->uMicroseconds < pFrom->uMicroseconds) {
        pDifference->uSeconds = pTo->uSeconds - pFrom->uSeconds - 1;
        pDifference->uMicroseconds =
            kMicrosecondsPerSecond - (pFrom->uMicroseconds - pTo->uMicroseconds);
        return 0;
    }
    pDifference->uSeconds = pTo->uSeconds - pFrom->uSeconds;
    pDifference->uMicroseconds = pTo->uMicroseconds - pFrom->uMicroseconds;
    return 0;
}

int RtTimeToSeconds(const RtTime *pTime, unsigned int *puSeconds) {
    if (pTime == NULL || puSeconds == NULL) {
        return kTimeResultInvalid;
    }
    *puSeconds = pTime->uSeconds;
    *puSeconds = pTime->uMicroseconds / kMicrosecondsPerSecond + pTime->uSeconds;
    return 0;
}

int RtTimeToMilliseconds(const RtTime *pTime, unsigned int *puMilliseconds) {
    if (pTime == NULL || puMilliseconds == NULL) {
        return kTimeResultInvalid;
    }
    if (pTime->uSeconds > kMaxMillisecondSeconds) {
        return kTimeResultOverflow;
    }
    *puMilliseconds = pTime->uSeconds * kMillisecondsPerSecond +
                      pTime->uMicroseconds / kMicrosecondsPerMillisecond;
    return 0;
}

int RtGetElapsedTime(RtTime *pTime) {
    int nResult;

    if (pTime == NULL) {
        return kTimeResultInvalid;
    }
    if (!g_bBaseTimeRead) {
        nResult = RtReadBaseTime();
        if (nResult != 0) {
            return nResult;
        }
    }
    return RtGetTimeSince(&g_baseTime, pTime);
}

int RtGetTimeSince(const RtTime *pSince, RtTime *pTime) {
    RtTime now;
    int nResult;

    if (pSince == NULL || pTime == NULL) {
        return kTimeResultInvalid;
    }
    nResult = RtReadClock(&now);
    if (nResult != 0) {
        return nResult;
    }
    return RtSubtractTime(pSince, &now, pTime);
}
