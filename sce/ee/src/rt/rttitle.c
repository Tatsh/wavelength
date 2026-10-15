#include "rt/rttitle.h"

// NTSC-U/C: 0x003c5e58, PAL: 0x00458d68
#ifdef VIDEO_STANDARD_PAL
static unsigned int g_uTitleApplicationId = 10314;
#else
static unsigned int g_uTitleApplicationId = 10164;
#endif

// NTSC-U/C: 0x003c5e60, PAL: 0x00458d70
// The original holds a Sony-issued signing key, which is not included.
static const RtKey g_titleSigningModulus __attribute__((used));

// NTSC-U/C: 0x003c5ea0, PAL: 0x00458db0
// The original holds a Sony-issued signing key, which is not included.
static const RtKey g_titleSigningExponent __attribute__((used));

unsigned int RtTitleGetApplicationId(void) {
    return g_uTitleApplicationId;
}

void RtTitleGetSigningKey(RtKey *pModulus, RtKey *pExponent) {
    *pModulus = g_titleSigningModulus;
    *pExponent = g_titleSigningExponent;
}
