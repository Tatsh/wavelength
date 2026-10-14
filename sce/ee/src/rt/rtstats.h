#ifndef RTSTATS_H
#define RTSTATS_H

#include "rt/rttime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Traffic statistics of the SCE-RT transport, counted per kind of record and turned into rates each
 * second. The routines report 0 for success, 1 when the clock failed, and 3 for a null argument.
 */

/** The kinds of record counted. */
#define RT_STATS_KINDS 6

/** A count, and its rate over the last second. */
typedef struct {
    unsigned int uCount; /*!< The count since the last update. */
    float fRate;         /*!< The count per second over the last update. */
    float fPeak;         /*!< The largest rate. */
} RtStatsCounter;

/** The traffic of one kind of record. */
typedef struct {
    RtTime lastUpdate;              /*!< When the rates were last computed. */
    RtStatsCounter packetsSent;     /*!< The records sent. */
    RtStatsCounter bytesSent;       /*!< The bytes sent. */
    RtStatsCounter packetsReceived; /*!< The records received. */
    RtStatsCounter bytesReceived;   /*!< The bytes received. */
} RtStatsEntry;

/**
 * Clear the statistics.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9a70
 * @ghidraAddress PAL: 0x002d39e0
 */
int RtStatsInit(void);

/**
 * Count a record sent or received, and update the rates when a second passed.
 *
 * @param nFlags The flags of the record, which pick its kind.
 * @param nSent The bytes sent, or 0.
 * @param nReceived The bytes received, or 0.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9b10
 * @ghidraAddress PAL: 0x002d3a80
 */
int RtStatsRecordTraffic(int nFlags, int nSent, int nReceived);

/**
 * Update the rates when more than a second passed since the last update.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9be0
 * @ghidraAddress PAL: 0x002d3b50
 */
int RtStatsUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
