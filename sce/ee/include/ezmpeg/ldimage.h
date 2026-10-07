#ifndef EZMPEG_LDIMAGE_H
#define EZMPEG_LDIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

// The image loader of Sony's ezmpegstr sample, ldimage.c. The unit builds the DMA chains that move
// decoded macroblocks into the frame buffer over PATH3. The entry points are declared in
// <ezmpeg.h>, which this header includes so that consumers of the unit gain the declarations with
// a single include.

#include <ezmpeg.h>

#ifdef __cplusplus
}
#endif

#endif
