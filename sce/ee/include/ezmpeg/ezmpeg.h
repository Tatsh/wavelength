#ifndef EZMPEG_EZMPEG_H
#define EZMPEG_EZMPEG_H

#ifdef __cplusplus
extern "C" {
#endif

// The reconstructed Sony movie sample units. Each unit header declares what its translation unit
// provides beyond <ezmpeg.h>. Including this header gains every unit with a single include.

#include "ezmpeg/audiodec.h"
#include "ezmpeg/ldimage.h"
#include "ezmpeg/vibuf.h"

#ifdef __cplusplus
}
#endif

#endif
