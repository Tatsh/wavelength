#ifndef LIBSCF_H
#define LIBSCF_H

#include <libcdvd.h>

#ifdef __cplusplus
extern "C" {
#endif

/** System configuration library: the console's language and time settings. */

/** Language codes sceScfGetLanguage() reports. */
#define SCE_JAPANESE_LANGUAGE 0 /*!< Japanese. */
#define SCE_ENGLISH_LANGUAGE 1  /*!< English. */
#define SCE_FRENCH_LANGUAGE 2   /*!< French. */
#define SCE_SPANISH_LANGUAGE 3  /*!< Spanish. */
#define SCE_GERMAN_LANGUAGE 4   /*!< German. */
#define SCE_ITALIAN_LANGUAGE 5  /*!< Italian. */

/** Settings a tool console reports in place of the console configuration. */
typedef struct {
    short nTimezone;              /*!< The timezone, in minutes east of UTC. */
    unsigned char abReserved2[2]; /*!< Undetermined. */
    unsigned char nLanguage;      /*!< One of the language codes. */
    unsigned char nReserved5;     /*!< Undetermined. */
    unsigned char nSummerTime;    /*!< Non-zero while summer time applies. */
    unsigned char nReserved7;     /*!< Undetermined. */
} sceScfT10kConfig;

/**
 * Replace the settings a tool console reports.
 *
 * @param pConfig The settings.
 * @ghidraAddress NTSC-U/C: 0x00315af0
 * @ghidraAddress PAL: 0x003823a8
 */
void sceScfSetT10kConfig(const sceScfT10kConfig *pConfig);

/**
 * Report the console's language setting.
 *
 * A tool console reports a fixed default instead.
 *
 * @return One of the language codes.
 * @ghidraAddress NTSC-U/C: 0x00315a90
 * @ghidraAddress PAL: 0x00382348
 */
int sceScfGetLanguage(void);

/**
 * Report the console's time zone.
 *
 * A tool console reports a fixed default instead, and a version 0 configuration Tokyo time.
 *
 * @return The offset from UTC in minutes.
 * @ghidraAddress NTSC-U/C: 0x00315b18
 * @ghidraAddress PAL: 0x003823d0
 */
int sceScfGetTimeZone(void);

/**
 * Report whether summer time applies.
 *
 * A tool console reports a fixed default instead, and a version 0 configuration zero.
 *
 * @return 1 while summer time applies, otherwise 0.
 * @ghidraAddress NTSC-U/C: 0x00315b70
 * @ghidraAddress PAL: 0x00382428
 */
int sceScfGetSummerTime(void);

/**
 * Convert a clock from the console's Japan-time RTC to local time, in place.
 *
 * Applies the configured time-zone offset and summer-time setting.
 *
 * @param pClock A clock sceCdReadClock() read.
 * @ghidraAddress NTSC-U/C: 0x00315f40
 * @ghidraAddress PAL: 0x003827f8
 */
void sceScfGetLocalTimefromRTC(sceCdCLOCK *pClock);

#ifdef __cplusplus
}
#endif

#endif
