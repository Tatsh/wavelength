#include "rt/rtstats.h"

#include <stddef.h>

enum {
    kStatsResultFailed = 1,
    kStatsResultInvalid = 3,
    // The kinds a record counts under, by its flags.
    kStatsKindPlain = 1,
    kStatsKindOrdered = 2,
    kStatsKindGuaranteed = 4,
    kStatsKindGuaranteedOrdered = 5,
    kStatsFlagOrdered = 0x10,
    kStatsFlagGuaranteed = 0x40,
    kStatsUpdateMilliseconds = 1000,
};

#define STATS_MILLISECONDS_PER_SECOND 1000.0f

// NTSC-U/C: 0x00521860
static RtTime g_statsStart;
// NTSC-U/C: 0x00521868
static RtStatsEntry g_aStats[RT_STATS_KINDS];

// NTSC-U/C: 0x002c9cf0, PAL: 0x002d3c60
static int RtStatsComputeRate(RtStatsCounter *pCounter, unsigned int uElapsed) {
    if (pCounter == NULL) {
        return kStatsResultInvalid;
    }
    if (uElapsed == 0) {
        pCounter->fRate = 0.0f;
    } else {
        pCounter->fRate =
            (float)pCounter->uCount / ((float)uElapsed / STATS_MILLISECONDS_PER_SECOND);
    }
    if (pCounter->fPeak < pCounter->fRate) {
        pCounter->fPeak = pCounter->fRate;
    }
    pCounter->uCount = 0;
    return 0;
}

int RtStatsInit(void) {
    RtTime now;
    int i;

    if (RtReadTime(&now) != 0) {
        return kStatsResultFailed;
    }
    for (i = 0; i < RT_STATS_KINDS; ++i) {
        g_aStats[i].packetsSent.uCount = 0;
        g_aStats[i].bytesSent.uCount = 0;
        g_aStats[i].packetsReceived.uCount = 0;
        g_aStats[i].bytesReceived.uCount = 0;
        g_aStats[i].lastUpdate = now;
        g_aStats[i].packetsSent.fRate = 0.0f;
        g_aStats[i].packetsSent.fPeak = 0.0f;
        g_aStats[i].bytesSent.fRate = 0.0f;
        g_aStats[i].bytesSent.fPeak = 0.0f;
        g_aStats[i].packetsReceived.fRate = 0.0f;
        g_aStats[i].packetsReceived.fPeak = 0.0f;
        g_aStats[i].bytesReceived.fRate = 0.0f;
        g_aStats[i].bytesReceived.fPeak = 0.0f;
    }
    g_statsStart = now;
    return 0;
}

int RtStatsRecordTraffic(int nFlags, int nSent, int nReceived) {
    int nKind;

    if ((nFlags & kStatsFlagGuaranteed) != 0) {
        nKind =
            (nFlags & kStatsFlagOrdered) != 0 ? kStatsKindGuaranteedOrdered : kStatsKindGuaranteed;
    } else {
        nKind = (nFlags & kStatsFlagOrdered) != 0 ? kStatsKindOrdered : kStatsKindPlain;
    }
    if (nSent != 0) {
        ++g_aStats[nKind].packetsSent.uCount;
        g_aStats[nKind].bytesSent.uCount += nSent;
    }
    if (nReceived != 0) {
        ++g_aStats[nKind].packetsReceived.uCount;
        g_aStats[nKind].bytesReceived.uCount += nReceived;
    }
    return RtStatsUpdate();
}

int RtStatsUpdate(void) {
    RtTime now;
    unsigned int uElapsed;
    int nResult;
    int i;

    if (RtMeasureMillisecondsSince(&g_aStats[0].lastUpdate, &uElapsed) != 0) {
        return kStatsResultFailed;
    }
    if (uElapsed <= kStatsUpdateMilliseconds) {
        return 0;
    }
    if (RtReadTime(&now) != 0) {
        return kStatsResultFailed;
    }
    for (i = 0; i < RT_STATS_KINDS; ++i) {
        if ((nResult = RtStatsComputeRate(&g_aStats[i].packetsSent, uElapsed)) != 0 ||
            (nResult = RtStatsComputeRate(&g_aStats[i].bytesSent, uElapsed)) != 0 ||
            (nResult = RtStatsComputeRate(&g_aStats[i].packetsReceived, uElapsed)) != 0 ||
            (nResult = RtStatsComputeRate(&g_aStats[i].bytesReceived, uElapsed)) != 0) {
            return nResult;
        }
        g_aStats[i].lastUpdate = now;
    }
    return 0;
}
