#ifndef RTIMAGE_H
#define RTIMAGE_H

#include "rt/rtgsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Image and sub-band audio coding of the media layer of the SCE-RT network library.
 *
 * The sub-band audio records are GSM 06.10 full-rate speech frames. The names of the routines and
 * of their parameters are inferred.
 */

/** The bytes one call of rtSubbandEncode() writes. */
#define RT_SUBBAND_PACKED_SIZE 33

/** An image plane or a 32-bit pixel surface, followed by its pixels. */
typedef struct {
    int size;               /*!< The bytes of pixels. */
    int stride;             /*!< The bytes of one row. */
    int width;              /*!< The width in pixels. */
    int height;             /*!< The height in pixels. */
    unsigned char pixels[]; /*!< The pixels. */
} rtImage;

/**
 * Encode one frame of samples with the sub-band coder.
 *
 * @param codec The coder.
 * @param samples The RT_SUBBAND_FRAME_BYTES bytes of samples.
 * @param packed Receives RT_SUBBAND_PACKED_SIZE bytes.
 * @return The bytes written.
 * @ghidraAddress NTSC-U/C: 0x002e9934
 * @ghidraAddress PAL: 0x002f38a4
 */
int rtSubbandEncode(rtSubbandCodec *codec, const short *samples, unsigned char *packed);

/**
 * Decode one frame of the sub-band coder.
 *
 * @param codec The coder.
 * @param packed The RT_SUBBAND_PACKED_SIZE bytes of the frame.
 * @param samples Receives RT_SUBBAND_FRAME_BYTES bytes of samples.
 * @return The bytes of samples written, or -1 for a damaged frame.
 * @ghidraAddress NTSC-U/C: 0x002e9ffc
 * @ghidraAddress PAL: 0x002f3f6c
 */
int rtSubbandDecode(rtSubbandCodec *codec, const unsigned char *packed, short *samples);

/**
 * Create a cleared image plane with its sides rounded up to a multiple of 8.
 *
 * @param width The width in pixels.
 * @param height The height in pixels.
 * @return The plane, or null.
 * @ghidraAddress NTSC-U/C: 0x002ea5d0
 * @ghidraAddress PAL: 0x002f4540
 */
rtImage *rtImageCreate(int width, int height);

/**
 * Release an image plane.
 *
 * @param image The plane, or null.
 * @ghidraAddress NTSC-U/C: 0x002ea678
 * @ghidraAddress PAL: 0x002f45e8
 */
void rtImageDestroy(rtImage *image);

/**
 * Encode the three planes of a YUV 4:2:0 picture.
 *
 * @param width The width of the luma plane.
 * @param height The height of the luma plane.
 * @param y The luma plane.
 * @param u The first chroma plane.
 * @param v The second chroma plane.
 * @param bitstream Receives the encoded picture.
 * @return The bytes written, or -1 when the sizes of the planes do not match.
 * @ghidraAddress NTSC-U/C: 0x002ea8ac
 * @ghidraAddress PAL: 0x002f481c
 */
int rtImageEncode(
    int width, int height, rtImage *y, rtImage *u, rtImage *v, unsigned char *bitstream);

/**
 * Decode an encoded picture into the three planes of a YUV 4:2:0 picture.
 *
 * @param width The width of the luma plane.
 * @param height The height of the luma plane.
 * @param y Receives the luma plane.
 * @param u Receives the first chroma plane.
 * @param v Receives the second chroma plane.
 * @param bitstream The encoded picture.
 * @return 0, or -1 when the sizes of the planes or of the picture do not match.
 * @ghidraAddress NTSC-U/C: 0x002eaac4
 * @ghidraAddress PAL: 0x002f4a34
 */
int rtImageDecode(
    int width, int height, rtImage *y, rtImage *u, rtImage *v, const unsigned char *bitstream);

/**
 * Create a 32-bit pixel surface.
 *
 * @param width The width in pixels.
 * @param height The height in pixels.
 * @return The surface, or null.
 * @ghidraAddress NTSC-U/C: 0x002ead0c
 * @ghidraAddress PAL: 0x002f4c7c
 */
rtImage *rtImageCreateSurface(int width, int height);

/**
 * Release a 32-bit pixel surface.
 *
 * @param surface The surface, or null.
 * @ghidraAddress NTSC-U/C: 0x002ead94
 * @ghidraAddress PAL: 0x002f4d04
 */
void rtImageDestroySurface(rtImage *surface);

/**
 * Convert the three planes of a YUV 4:2:0 picture into a 32-bit pixel surface.
 *
 * The conversion processes rows in steps of 32 pixels and reads the luma plane as if its stride
 * equalled the width of the surface.
 *
 * @param y The luma plane.
 * @param u The first chroma plane, added to the red channel.
 * @param v The second chroma plane, added to the blue channel.
 * @param surface Receives the pixels.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002eadc0
 * @ghidraAddress PAL: 0x002f4d30
 */
int rtImageConvertToRgb(rtImage *y, rtImage *u, rtImage *v, rtImage *surface);

#ifdef __cplusplus
}
#endif

#endif
