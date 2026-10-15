#include <eekernel.h>
#include <libcdvd.h>
#include <libscf.h>
#include <sifdev.h>

enum {
    // Minutes in one hour.
    kMinutesPerHour = 60,
    // Minutes in nine hours of Tokyo time, the baseline the RTC keeps.
    kTokyoMinutes = 540,
    // The lowest minute total that carries into the hour field. A total of exactly 60 stays in the
    // minute field, as in the binary.
    kHourThreshold = 61,
    kHoursPerDay = 24,
    kMonthsPerYear = 12,
    kLastYear = 99,
    kFebruary = 1,
    kLeapFebruaryDays = 29,
    kLeapYearMask = 3,
    kDecimalBase = 10,
    // A packed BCD byte holds the tens in its high nibble.
    kBcdTensShift = 4,
    kBcdNibbleGap = 6,
    // The version field starts after this many bits in the configuration word.
    kVersionShift = 13,
    // The version field uses this mask after the shift.
    kVersionMask = 7,
    // The language field starts after this many bits in the configuration word.
    kLanguageShift = 16,
    // The language field uses this mask after the shift.
    kLanguageMask = 0x1f,
    // A version 0 configuration word has a one-bit language at this position instead.
    kLegacyLanguageShift = 4,
    // The timezone field starts after this many bits in the configuration word.
    kTimezoneShift = 21,
    // The daylight flag starts after this many bits in the detail byte.
    kDaylightShift = 4,
    // The detail byte of the extended configuration.
    kDetailOffset = 1,
    kDetailSize = 1,
};

// The ROM region letter of a tool console. A tool console reports the defaults below rather than
// the console configuration.
enum {
    kRomRegionIndex = 4,
    kRomRegionTool = 'T',
    kRomVersionReadSize = 14,
};

// NTSC-U/C: 0x003c4730, PAL: 0x0045ab78, the identification tag of the library.
#ifdef VIDEO_STANDARD_PAL
static char g_acLibraryId[16] __attribute__((used, nonstring)) = "PsIIlibscf  2700";
#else
static char g_acLibraryId[16] __attribute__((used, nonstring)) = "PsIIlibscf  2600";
#endif

// NTSC-U/C: 0x003c4740, PAL: 0x0045ab88, the settings a tool console reports, which
// sceScfSetT10kConfig() replaces.
static sceScfT10kConfig g_scfT10kConfig = {540, {0, 0}, SCE_JAPANESE_LANGUAGE, 0, 0, 0};

// NTSC-U/C: 0x003c4748, PAL: 0x0045ab90, the contents of rom0:ROMVER, empty until the first read.
static char g_szScfRomVersion[16];

// NTSC-U/C: 0x00432458, PAL: 0x004cf120, month lengths for the day arithmetic below.
static const signed char kMonthLengths[kMonthsPerYear] = {
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31,
};

// NTSC-U/C: 0x003159e0, PAL: 0x00382298
static char *GetRomName(void) {
    int fd;

    if (g_szScfRomVersion[0] == '\0') {
        fd = sceOpen("rom0:ROMVER", SCE_RDONLY);
        if (fd >= 0) {
            sceRead(fd, g_szScfRomVersion, kRomVersionReadSize);
            sceClose(fd);
        }
    }
    return g_szScfRomVersion;
}

// NTSC-U/C: 0x00315a50, PAL: 0x00382308
// Reports whether the console runs a tool ROM, reading the ROM version first if needed.
static int IsT10K(void) {
    if (g_szScfRomVersion[0] == '\0') {
        GetRomName();
    }
    return g_szScfRomVersion[kRomRegionIndex] == kRomRegionTool;
}

int sceScfGetLanguage(void) {
    unsigned int nConfig;
    unsigned int nVersion;

    GetOsdConfigParam(&nConfig); // Yes, the binary reads the word before the tool check too.
    if (IsT10K()) {
        return g_scfT10kConfig.nLanguage;
    }
    GetOsdConfigParam(&nConfig);
    nVersion = (nConfig >> kVersionShift) & (unsigned int)kVersionMask;
    if (nVersion == 0u) {
        return (int)((nConfig >> kLegacyLanguageShift) & 1u);
    }
    return (int)((nConfig >> kLanguageShift) & (unsigned int)kLanguageMask);
}

void sceScfSetT10kConfig(const sceScfT10kConfig *pConfig) {
    g_scfT10kConfig = *pConfig;
}

int sceScfGetTimeZone(void) {
    unsigned int nConfig;

    if (IsT10K()) {
        return g_scfT10kConfig.nTimezone;
    }
    GetOsdConfigParam(&nConfig);
    if (((nConfig >> kVersionShift) & (unsigned int)kVersionMask) == 0u) {
        return kTokyoMinutes;
    }
    return (int)nConfig >> kTimezoneShift;
}

int sceScfGetSummerTime(void) {
    unsigned int nConfig;
    unsigned char nDetail;

    if (IsT10K()) {
        return g_scfT10kConfig.nSummerTime;
    }
    GetOsdConfigParam(&nConfig);
    if (((nConfig >> kVersionShift) & (unsigned int)kVersionMask) == 0u) {
        return 0;
    }
    GetOsdConfigParam2(&nDetail, kDetailSize, kDetailOffset);
    return (nDetail >> kDaylightShift) & 1;
}

// NTSC-U/C: 0x00315bd8, PAL: 0x00382490
static unsigned char tobcd(unsigned char nValue) {
    return (unsigned char)((nValue / kDecimalBase) * kBcdNibbleGap + nValue);
}

// NTSC-U/C: 0x00315c08, PAL: 0x003824c0
static unsigned char frombcd(unsigned char nValue) {
    return (unsigned char)(nValue - (nValue >> kBcdTensShift) * kBcdNibbleGap);
}

// NTSC-U/C: 0x00315c28, PAL: 0x003824e0
static void convertfrombcd(sceCdCLOCK *pClock) {
    pClock->year = frombcd(pClock->year);
    pClock->month = frombcd(pClock->month);
    pClock->day = frombcd(pClock->day);
    pClock->hour = frombcd(pClock->hour);
    pClock->minute = frombcd(pClock->minute);
    pClock->second = frombcd(pClock->second);
}

// NTSC-U/C: 0x00315c90, PAL: 0x00382548
static void converttobcd(sceCdCLOCK *pClock) {
    pClock->year = tobcd(pClock->year);
    pClock->month = tobcd(pClock->month);
    pClock->day = tobcd(pClock->day);
    pClock->hour = tobcd(pClock->hour);
    pClock->minute = tobcd(pClock->minute);
    pClock->second = tobcd(pClock->second);
}

// NTSC-U/C: 0x00315cf8, PAL: 0x003825b0
static void adddate(sceCdCLOCK *pClock) {
    signed char anMonthLengths[kMonthsPerYear];
    int i;

    ++pClock->day;
    for (i = 0; i < kMonthsPerYear; ++i) {
        anMonthLengths[i] = kMonthLengths[i];
    }
    if ((pClock->year & kLeapYearMask) == 0) {
        anMonthLengths[kFebruary] = kLeapFebruaryDays;
    }
    if (anMonthLengths[pClock->month - 1] < pClock->day) {
        pClock->day = 1;
        ++pClock->month;
        if (pClock->month == kMonthsPerYear + 1) {
            if (pClock->year == kLastYear) {
                pClock->year = 0;
            } else {
                ++pClock->year;
            }
            pClock->month = 1;
        }
    }
}

// NTSC-U/C: 0x00315db0, PAL: 0x00382668
static void subdate(sceCdCLOCK *pClock) {
    signed char anMonthLengths[kMonthsPerYear];
    int i;

    --pClock->day;
    for (i = 0; i < kMonthsPerYear; ++i) {
        anMonthLengths[i] = kMonthLengths[i];
    }
    if ((pClock->year & kLeapYearMask) == 0) {
        anMonthLengths[kFebruary] = kLeapFebruaryDays;
    }
    if (pClock->day != 0) {
        return;
    }
    --pClock->month;
    if (pClock->month == 0) {
        pClock->year = pClock->year == 0 ? kLastYear : pClock->year - 1;
        pClock->month = kMonthsPerYear;
    }
    pClock->day = (unsigned char)anMonthLengths[pClock->month - 1];
}

// NTSC-U/C: 0x00315e58, PAL: 0x00382710
static void addhour(sceCdCLOCK *pClock) {
    ++pClock->hour;
    if (pClock->hour == kHoursPerDay) {
        pClock->hour = 0;
        adddate(pClock);
    }
}

// NTSC-U/C: 0x00315e88, PAL: 0x00382740
static void subhour(sceCdCLOCK *pClock) {
    if (pClock->hour == 0) {
        pClock->hour = kHoursPerDay - 1;
        subdate(pClock);
        return;
    }
    --pClock->hour;
}

// NTSC-U/C: 0x00315eb0, PAL: 0x00382768
static void AdjustTime(sceCdCLOCK *pClock, int nMinutes) {
    int nTotal;

    convertfrombcd(pClock);
    nTotal = pClock->minute + nMinutes;
    if (nTotal < 0) {
        do {
            nTotal += kMinutesPerHour;
            subhour(pClock);
        } while (nTotal < 0);
    } else if (nTotal >= kHourThreshold) {
        do {
            addhour(pClock);
            nTotal -= kMinutesPerHour;
        } while (nTotal >= kHourThreshold);
    }
    pClock->minute = (unsigned char)nTotal;
    converttobcd(pClock);
}

void sceScfGetLocalTimefromRTC(sceCdCLOCK *pClock) {
    int nTimezone = sceScfGetTimeZone();
    int nSummer = sceScfGetSummerTime();

    AdjustTime(pClock, nTimezone + nSummer * kMinutesPerHour - kTokyoMinutes);
}
