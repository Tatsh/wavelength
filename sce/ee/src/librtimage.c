#include <limits.h>
#include <stddef.h>
#include <string.h>

#include <librtdmedev.h>
#include <librtimage.h>
#include <librtvlc.h>

enum {
    // The upper four bits of the first byte of a sub-band frame.
    kSubbandMagic = 13,
    kSubbandMagicShift = 4,
};

enum {
    kBlockSide = 8,
    kBlockShift = 3,
    kBlockSamples = kBlockSide * kBlockSide,
    // The offset that centres the unsigned samples of a plane on 0.
    kSampleBias = 128,
    kSampleMax = 255,
    // The luma blocks that follow each pair of chroma blocks in an encoded picture.
    kLumaBlocksPerChroma = 4,
    // The value of the two 16-bit words that follow the size in the header of an encoded picture.
    kPictureHeaderMarker = 1,
    kByteShift = 8,
};

// Selectors of the quantisation tables.
enum {
    kTableLuma = 0,
    kTableChroma = 1,
};

enum {
    kShortMin = SHRT_MIN,
    kShortMax = SHRT_MAX,
    // The rounding term and the shift of the quantiser.
    kQuantizeRounding = 0x4000,
    kQuantizeShift = 15,
    // The fixed-point factors of the transforms and their shifts.
    kFactorHalfRootTwo = 23170,
    kFactorCosineEighth = 17734,
    kFactorSineEighth = 25080,
    kFactorRootTwoCosineEighth = 21407,
    kTransformShift14 = 14,
    kTransformShift15 = 15,
    kTransformShift16 = 16,
    kInverseOutputShift = 4,
};

enum {
    kPixelRed = 0,
    kPixelGreen = 1,
    kPixelBlue = 2,
    kPixelAlpha = 3,
    kPixelBytes = 4,
    kPixelAlphaValue = 128,
    // The luma samples, and the chroma samples of each plane, of one step of the conversion.
    kConvertLumaStep = 32,
    kConvertChromaStep = 16,
    kGreenScaleShift = 2,
};

// The quantisation factors of luma blocks, in units of 2^-15.
// NTSC-U/C: 0x0042de00
static const short g_anQuantizeLuma[kBlockSamples] __attribute__((aligned(16))) = {
    512, 492, 448, 498, 455, 434, 309, 412, 537, 355, 348, 295, 268, 215, 171, 233,
    627, 323, 300, 242, 169, 145, 149, 239, 435, 264, 222, 204, 124, 139, 148, 258,
    341, 227, 157, 137, 120, 129, 147, 265, 261, 130, 140, 102, 96,  128, 159, 378,
    297, 182, 168, 161, 147, 170, 233, 533, 487, 389, 406, 407, 386, 411, 543, 1087,
};

// The quantisation factors of chroma blocks, in units of 2^-15.
// NTSC-U/C: 0x0042de80
static const short g_anQuantizeChroma[kBlockSamples] __attribute__((aligned(16))) = {
    482, 328, 261, 148, 83,  105, 153, 300, 328, 203, 174, 76,  60,  76,  110, 216,
    261, 174, 86,  54,  63,  81,  117, 230, 148, 76,  54,  60,  70,  90,  130, 255,
    83,  60,  63,  70,  83,  105, 153, 300, 105, 76,  81,  90,  105, 134, 195, 382,
    153, 110, 117, 130, 153, 195, 283, 554, 300, 216, 230, 255, 300, 382, 554, 1087,
};

// The dequantisation factors of luma blocks.
// NTSC-U/C: 0x0042dd00
static const short g_anDequantizeLuma[kBlockSamples] __attribute__((aligned(16))) = {
    64,  67,  73,  66,  72,  75,  106, 79,  61,  92,  94,  111, 122, 153, 192, 141,
    52,  101, 109, 135, 193, 226, 221, 137, 75,  124, 147, 160, 263, 237, 221, 127,
    96,  144, 209, 240, 272, 255, 223, 124, 126, 253, 234, 322, 343, 257, 206, 87,
    110, 180, 195, 204, 223, 192, 141, 62,  67,  84,  81,  80,  85,  80,  60,  30,
};

// The dequantisation factors of chroma blocks.
// NTSC-U/C: 0x0042dd80
static const short g_anDequantizeChroma[kBlockSamples] __attribute__((aligned(16))) = {
    68,  100, 125, 221, 396, 311, 214, 109, 100, 162, 188, 431, 549, 432, 297, 152,
    125, 188, 382, 608, 517, 407, 280, 143, 221, 431, 608, 548, 466, 366, 252, 128,
    396, 549, 517, 466, 396, 311, 214, 109, 311, 432, 407, 366, 311, 244, 168, 86,
    214, 297, 280, 252, 214, 168, 116, 59,  109, 152, 143, 128, 109, 86,  59,  30,
};

int rtSubbandEncode(rtSubbandCodec *codec, const short *samples, unsigned char *packed) {
    int logAreaRatios[RT_SUBBAND_LAR_COUNT];
    int lags[RT_SUBBAND_SUBFRAMES];
    int gains[RT_SUBBAND_SUBFRAMES];
    int gridPositions[RT_SUBBAND_SUBFRAMES];
    int amplitudes[RT_SUBBAND_SUBFRAMES];
    int pulses[RT_SUBBAND_SUBFRAMES * RT_SUBBAND_PULSES];
    const int *pulse;
    int subframe;

    rtSubbandCoder(codec, samples, logAreaRatios, lags, gains, gridPositions, amplitudes, pulses);
    *packed++ =
        (unsigned char)((kSubbandMagic << kSubbandMagicShift) | ((logAreaRatios[0] >> 2) & 0xf));
    *packed++ = (unsigned char)(((logAreaRatios[0] & 0x3) << 6) | (logAreaRatios[1] & 0x3f));
    *packed++ = (unsigned char)(((logAreaRatios[2] & 0x1f) << 3) | ((logAreaRatios[3] >> 2) & 0x7));
    *packed++ = (unsigned char)(((logAreaRatios[3] & 0x3) << 6) | ((logAreaRatios[4] & 0xf) << 2) |
                                ((logAreaRatios[5] >> 2) & 0x3));
    *packed++ = (unsigned char)(((logAreaRatios[5] & 0x3) << 6) | ((logAreaRatios[6] & 0x7) << 3) |
                                (logAreaRatios[7] & 0x7));
    for (subframe = 0; subframe < RT_SUBBAND_SUBFRAMES; ++subframe) {
        pulse = &pulses[subframe * RT_SUBBAND_PULSES];
        *packed++ =
            (unsigned char)(((lags[subframe] & 0x7f) << 1) | ((gains[subframe] >> 1) & 0x1));
        *packed++ = (unsigned char)(((gains[subframe] & 0x1) << 7) |
                                    ((gridPositions[subframe] & 0x3) << 5) |
                                    ((amplitudes[subframe] >> 1) & 0x1f));
        *packed++ = (unsigned char)(((amplitudes[subframe] & 0x1) << 7) | ((pulse[0] & 0x7) << 4) |
                                    ((pulse[1] & 0x7) << 1) | ((pulse[2] >> 2) & 0x1));
        *packed++ =
            (unsigned char)(((pulse[2] & 0x3) << 6) | ((pulse[3] & 0x7) << 3) | (pulse[4] & 0x7));
        *packed++ = (unsigned char)(((pulse[5] & 0x7) << 5) | ((pulse[6] & 0x7) << 2) |
                                    ((pulse[7] >> 1) & 0x3));
        *packed++ = (unsigned char)(((pulse[7] & 0x1) << 7) | ((pulse[8] & 0x7) << 4) |
                                    ((pulse[9] & 0x7) << 1) | ((pulse[10] >> 2) & 0x1));
        *packed++ = (unsigned char)(((pulse[10] & 0x3) << 6) | ((pulse[11] & 0x7) << 3) |
                                    (pulse[12] & 0x7));
    }
    return RT_SUBBAND_PACKED_SIZE;
}

int rtSubbandDecode(rtSubbandCodec *codec, const unsigned char *packed, short *samples) {
    int logAreaRatios[RT_SUBBAND_LAR_COUNT];
    int lags[RT_SUBBAND_SUBFRAMES];
    int gains[RT_SUBBAND_SUBFRAMES];
    int gridPositions[RT_SUBBAND_SUBFRAMES];
    int amplitudes[RT_SUBBAND_SUBFRAMES];
    int pulses[RT_SUBBAND_SUBFRAMES * RT_SUBBAND_PULSES];
    int *pulse;
    int subframe;

    if ((packed[0] >> kSubbandMagicShift) != kSubbandMagic) {
        return -1;
    }
    logAreaRatios[0] = ((packed[0] & 0xf) << 2) | (packed[1] >> 6);
    logAreaRatios[1] = packed[1] & 0x3f;
    logAreaRatios[2] = packed[2] >> 3;
    logAreaRatios[3] = ((packed[2] & 0x7) << 2) | (packed[3] >> 6);
    logAreaRatios[4] = (packed[3] >> 2) & 0xf;
    logAreaRatios[5] = ((packed[3] & 0x3) << 2) | (packed[4] >> 6);
    logAreaRatios[6] = (packed[4] >> 3) & 0x7;
    logAreaRatios[7] = packed[4] & 0x7;
    packed = &packed[5];
    for (subframe = 0; subframe < RT_SUBBAND_SUBFRAMES; ++subframe) {
        pulse = &pulses[subframe * RT_SUBBAND_PULSES];
        lags[subframe] = packed[0] >> 1;
        gains[subframe] = ((packed[0] & 0x1) << 1) | (packed[1] >> 7);
        gridPositions[subframe] = (packed[1] >> 5) & 0x3;
        amplitudes[subframe] = ((packed[1] & 0x1f) << 1) | (packed[2] >> 7);
        pulse[0] = (packed[2] >> 4) & 0x7;
        pulse[1] = (packed[2] >> 1) & 0x7;
        pulse[2] = ((packed[2] & 0x1) << 2) | (packed[3] >> 6);
        pulse[3] = (packed[3] >> 3) & 0x7;
        pulse[4] = packed[3] & 0x7;
        pulse[5] = packed[4] >> 5;
        pulse[6] = (packed[4] >> 2) & 0x7;
        pulse[7] = ((packed[4] & 0x3) << 1) | (packed[5] >> 7);
        pulse[8] = (packed[5] >> 4) & 0x7;
        pulse[9] = (packed[5] >> 1) & 0x7;
        pulse[10] = ((packed[5] & 0x1) << 2) | (packed[6] >> 6);
        pulse[11] = (packed[6] >> 3) & 0x7;
        pulse[12] = packed[6] & 0x7;
        packed = &packed[7];
    }
    rtSubbandDecoder(codec, logAreaRatios, lags, gains, gridPositions, amplitudes, pulses, samples);
    return RT_SUBBAND_FRAME_BYTES;
}

rtImage *rtImageCreate(int width, int height) {
    void *block = NULL;
    rtImage *image;
    int stride = (width + kBlockSide - 1) & -kBlockSide;
    int rows = (height + kBlockSide - 1) & -kBlockSide;
    int size = rows * stride;

    rtDmeAllocZeroed(&block, size + (int)sizeof(rtImage));
    image = block;
    if (image != NULL) {
        image->size = size;
        image->stride = stride;
        image->width = width;
        image->height = height;
        memset(image->pixels, 0, (size_t)size);
    }
    return image;
}

void rtImageDestroy(rtImage *image) {
    void *block = image;

    if (image != NULL) {
        rtDmeFree(&block);
    }
}

// Returns the blocks of 8 by 8 pixels that cover a plane.
// NTSC-U/C: 0x002ea6a0, PAL: 0x002f4610
static int rtImageGetBlockCount(const rtImage *image) {
    if (image == NULL) {
        return 0;
    }
    return ((image->width + kBlockSide - 1) >> kBlockShift) *
           ((image->height + kBlockSide - 1) >> kBlockShift);
}

// Returns the index of the first pixel of a block. The blocks run along the stride of the plane,
// 8 rows at a time.
static inline int rtImageBlockOffset(const rtImage *image, int index) {
    int position = index * kBlockSide;

    return position / image->stride * image->stride * kBlockSide + position % image->stride;
}

// Reads a block of a plane as signed samples, or clears the block when the index is out of range.
// NTSC-U/C: 0x002ea6cc, PAL: 0x002f463c
static void rtImageExtractBlock(int index, const rtImage *image, short *block) {
    const unsigned char *row;
    int y;
    int x;

    if (block == NULL) {
        return;
    }
    if (image == NULL || index < 0 || index >= rtImageGetBlockCount(image)) {
        memset(block, 0, kBlockSamples * sizeof(*block));
        return;
    }
    row = &image->pixels[rtImageBlockOffset(image, index)];
    for (y = 0; y < kBlockSide; ++y) {
        for (x = 0; x < kBlockSide; ++x) {
            block[y * kBlockSide + x] = (signed char)(row[x] - kSampleBias);
        }
        row = &row[image->stride];
    }
}

// Writes the low byte of each sample of a block to a plane.
// NTSC-U/C: 0x002ea7cc, PAL: 0x002f473c
static void rtImageStoreBlock(int index, rtImage *image, const short *block) {
    unsigned char *row;
    int y;
    int x;

    if (block == NULL || image == NULL || index < 0 || index >= rtImageGetBlockCount(image)) {
        return;
    }
    row = &image->pixels[rtImageBlockOffset(image, index)];
    for (y = 0; y < kBlockSide; ++y) {
        for (x = 0; x < kBlockSide; ++x) {
            row[x] = (unsigned char)block[y * kBlockSide + x];
        }
        row = &row[image->stride];
    }
}

// Returns a sum or a difference clamped to the range of a short.
static inline short rtImageSaturate(int value) {
    if (value > kShortMax) {
        return kShortMax;
    }
    if (value < kShortMin) {
        return kShortMin;
    }
    return (short)value;
}

static inline short rtImageAdd(short a, short b) {
    return rtImageSaturate(a + b);
}

static inline short rtImageSubtract(short a, short b) {
    return rtImageSaturate(a - b);
}

// Returns the low half of a fixed-point product.
static inline short rtImageScale(short value, int factor, int shift) {
    return (short)((value * factor) >> shift);
}

// Returns a sample doubled without saturation.
static inline short rtImageDouble(short value) {
    return (short)(value * 2);
}

static inline void rtImageTransposeBlock(short *block) {
    short swap;
    int y;
    int x;

    for (y = 0; y < kBlockSide; ++y) {
        for (x = y + 1; x < kBlockSide; ++x) {
            swap = block[y * kBlockSide + x];
            block[y * kBlockSide + x] = block[x * kBlockSide + y];
            block[x * kBlockSide + y] = swap;
        }
    }
}

// Applies the one-dimensional forward transform to the column of a block that starts at sample.
static inline void rtImageForwardDctColumn(short *sample) {
    short sum07 = rtImageAdd(sample[0 * kBlockSide], sample[7 * kBlockSide]);
    short difference07 = rtImageSubtract(sample[0 * kBlockSide], sample[7 * kBlockSide]);
    short sum16 = rtImageAdd(sample[1 * kBlockSide], sample[6 * kBlockSide]);
    short difference16 = rtImageSubtract(sample[1 * kBlockSide], sample[6 * kBlockSide]);
    short sum25 = rtImageAdd(sample[2 * kBlockSide], sample[5 * kBlockSide]);
    short difference25 = rtImageSubtract(sample[2 * kBlockSide], sample[5 * kBlockSide]);
    short sum34 = rtImageAdd(sample[3 * kBlockSide], sample[4 * kBlockSide]);
    short difference34 = rtImageSubtract(sample[3 * kBlockSide], sample[4 * kBlockSide]);
    short evenSum = rtImageAdd(sum07, sum34);
    short evenDifference = rtImageSubtract(sum07, sum34);
    short innerSum = rtImageAdd(sum16, sum25);
    short innerDifference = rtImageSubtract(sum16, sum25);
    short oddFirst = rtImageAdd(difference34, difference25);
    short oddSecond = rtImageAdd(difference25, difference16);
    short oddThird = rtImageAdd(difference16, difference07);
    short rotation = rtImageScale(
        rtImageAdd(innerDifference, evenDifference), kFactorHalfRootTwo, kTransformShift15);
    short middle = rtImageScale(oddSecond, kFactorHalfRootTwo, kTransformShift15);
    short common =
        rtImageScale(rtImageSubtract(oddFirst, oddThird), kFactorSineEighth, kTransformShift16);
    short first =
        rtImageAdd(rtImageScale(oddFirst, kFactorCosineEighth, kTransformShift15), common);
    short third =
        rtImageAdd(rtImageScale(oddThird, kFactorRootTwoCosineEighth, kTransformShift14), common);
    short upper = rtImageAdd(difference07, middle);
    short lower = rtImageSubtract(difference07, middle);

    sample[0 * kBlockSide] = rtImageAdd(evenSum, innerSum);
    sample[1 * kBlockSide] = rtImageAdd(upper, third);
    sample[2 * kBlockSide] = rtImageAdd(evenDifference, rotation);
    sample[3 * kBlockSide] = rtImageSubtract(lower, first);
    sample[4 * kBlockSide] = rtImageSubtract(evenSum, innerSum);
    sample[5 * kBlockSide] = rtImageAdd(lower, first);
    sample[6 * kBlockSide] = rtImageSubtract(evenDifference, rotation);
    sample[7 * kBlockSide] = rtImageSubtract(upper, third);
}

// Applies the one-dimensional inverse transform to the column of a block that starts at sample.
static inline void rtImageInverseDctColumn(short *sample) {
    short evenSum = rtImageAdd(sample[2 * kBlockSide], sample[6 * kBlockSide]);
    short evenRotation = rtImageSubtract(
        rtImageScale(rtImageSubtract(sample[2 * kBlockSide], sample[6 * kBlockSide]),
                     kFactorHalfRootTwo,
                     kTransformShift16),
        (short)(evenSum >> 2));
    short dcDifference = (short)(rtImageSubtract(sample[0], sample[4 * kBlockSide]) >> 2);
    short dcSum = (short)(rtImageAdd(sample[0], sample[4 * kBlockSide]) >> 1);
    short evenHalf = (short)(evenSum >> 1);
    // Yes, the binary doubles these without saturation.
    short evenFirst = rtImageDouble(rtImageAdd(dcDifference, evenRotation));
    short evenSecond = rtImageDouble(rtImageSubtract(dcDifference, evenRotation));
    short evenZero = rtImageAdd(dcSum, evenHalf);
    short evenThird = rtImageSubtract(dcSum, evenHalf);
    short difference17 = rtImageSubtract(sample[1 * kBlockSide], sample[7 * kBlockSide]);
    short difference53 = rtImageSubtract(sample[5 * kBlockSide], sample[3 * kBlockSide]);
    short sum17 = (short)(rtImageAdd(sample[1 * kBlockSide], sample[7 * kBlockSide]) >> 1);
    short sum35 = (short)(rtImageAdd(sample[3 * kBlockSide], sample[5 * kBlockSide]) >> 1);
    short oddRotation =
        rtImageScale(rtImageSubtract(sum17, sum35), kFactorHalfRootTwo, kTransformShift14);
    short oddSum = rtImageAdd(sum17, sum35);
    short common =
        rtImageScale(rtImageSubtract((short)(difference53 >> 1), (short)(difference17 >> 1)),
                     kFactorSineEighth,
                     kTransformShift16);
    short third = rtImageSubtract(
        common, rtImageScale(difference53, kFactorRootTwoCosineEighth, kTransformShift15));
    short first = rtImageSubtract(
        rtImageDouble(rtImageSubtract(
            rtImageScale(difference17, kFactorCosineEighth, kTransformShift16), common)),
        oddSum);
    short second = rtImageSubtract(oddRotation, first);

    third = rtImageAdd(rtImageDouble(third), second);
    sample[0 * kBlockSide] = rtImageAdd(evenZero, oddSum);
    sample[1 * kBlockSide] = rtImageAdd(evenFirst, first);
    sample[2 * kBlockSide] = rtImageAdd(evenSecond, second);
    sample[3 * kBlockSide] = rtImageSubtract(evenThird, third);
    sample[4 * kBlockSide] = rtImageAdd(evenThird, third);
    sample[5 * kBlockSide] = rtImageSubtract(evenSecond, second);
    sample[6 * kBlockSide] = rtImageSubtract(evenFirst, first);
    sample[7 * kBlockSide] = rtImageSubtract(evenZero, oddSum);
}

// Transforms a block of samples into its transposed coefficients.
// NTSC-U/C: 0x0042e190
static void rtImageForwardDct(short *block) {
    int column;

    for (column = 0; column < kBlockSide; ++column) {
        rtImageForwardDctColumn(&block[column]);
    }
    rtImageTransposeBlock(block);
    for (column = 0; column < kBlockSide; ++column) {
        rtImageForwardDctColumn(&block[column]);
    }
}

// Transforms the transposed coefficients of a block into samples, centred on kSampleBias and
// limited to kSampleMax from above only.
// NTSC-U/C: 0x0042e490
static void rtImageInverseDct(short *block) {
    int column;
    int i;

    for (column = 0; column < kBlockSide; ++column) {
        rtImageInverseDctColumn(&block[column]);
    }
    rtImageTransposeBlock(block);
    for (column = 0; column < kBlockSide; ++column) {
        rtImageInverseDctColumn(&block[column]);
    }
    for (i = 0; i < kBlockSamples; ++i) {
        block[i] = rtImageAdd((short)(block[i] >> kInverseOutputShift), kSampleBias);
        if (block[i] > kSampleMax) {
            block[i] = kSampleMax;
        }
    }
}

// Quantises the coefficients of a block with rounding.
// NTSC-U/C: 0x0042df00
static void rtImageQuantize(int table, short *block) {
    const short *factors = g_anQuantizeChroma;
    int i;

    if (table == kTableLuma) {
        factors = g_anQuantizeLuma;
    }
    for (i = 0; i < kBlockSamples; ++i) {
        block[i] = (short)((block[i] * factors[i] + kQuantizeRounding) >> kQuantizeShift);
    }
}

// Scales quantised coefficients back to coefficients.
// NTSC-U/C: 0x0042e090
static void rtImageDequantize(int table, short *block) {
    const short *factors = g_anDequantizeChroma;
    int i;

    if (table == kTableLuma) {
        factors = g_anDequantizeLuma;
    }
    for (i = 0; i < kBlockSamples; ++i) {
        block[i] = (short)(block[i] * factors[i]);
    }
}

int rtImageEncode(
    int width, int height, rtImage *y, rtImage *u, rtImage *v, unsigned char *bitstream) {
    short block[kBlockSamples] __attribute__((aligned(16)));
    unsigned char *out = bitstream;
    unsigned int count;
    unsigned int index;
    unsigned int part;

    if (width != y->width || height != y->height || u->width != v->width ||
        u->height != v->height || u->width * 2 != width || u->height * 2 != height) {
        return -1;
    }
    rtVlcEncoderInit(RT_VLC_MODE_IMAGE);
    *out++ = (unsigned char)width;
    *out++ = (unsigned char)(width >> kByteShift);
    *out++ = (unsigned char)height;
    *out++ = (unsigned char)(height >> kByteShift);
    *out++ = kPictureHeaderMarker;
    *out++ = 0;
    *out++ = kPictureHeaderMarker;
    *out++ = 0;
    count = (unsigned int)rtImageGetBlockCount(u);
    for (index = 0; index < count; ++index) {
        rtImageExtractBlock((int)index, u, block);
        rtImageForwardDct(block);
        rtImageQuantize(kTableChroma, block);
        out += rtVlcEncodeBlock(block, out);
        rtImageExtractBlock((int)index, v, block);
        rtImageForwardDct(block);
        rtImageQuantize(kTableChroma, block);
        out += rtVlcEncodeBlock(block, out);
        // Yes, the binary takes the luma blocks in raster order, not as the 2 by 2 square under
        // the chroma block.
        for (part = 0; part < kLumaBlocksPerChroma; ++part) {
            rtImageExtractBlock((int)(index * kLumaBlocksPerChroma + part), y, block);
            rtImageForwardDct(block);
            rtImageQuantize(kTableLuma, block);
            out += rtVlcEncodeBlock(block, out);
        }
    }
    out += rtVlcEncoderFlush(out);
    return (int)(out - bitstream);
}

int rtImageDecode(
    int width, int height, rtImage *y, rtImage *u, rtImage *v, const unsigned char *bitstream) {
    short block[kBlockSamples] __attribute__((aligned(16)));
    const unsigned char *in = bitstream;
    unsigned int count;
    unsigned int index;
    unsigned int part;

    if (width != y->width || height != y->height || u->width != v->width ||
        u->height != v->height || u->width * 2 != width || u->height * 2 != height) {
        return -1;
    }
    if (*in++ != (unsigned char)width || *in++ != (unsigned char)(width >> kByteShift) ||
        *in++ != (unsigned char)height || *in++ != (unsigned char)(height >> kByteShift) ||
        *in++ != kPictureHeaderMarker || *in++ != 0 || *in++ != kPictureHeaderMarker ||
        *in++ != 0) {
        return -1;
    }
    rtVlcDecoderInit(RT_VLC_MODE_IMAGE);
    count = (unsigned int)rtImageGetBlockCount(u);
    for (index = 0; index < count; ++index) {
        in += rtVlcDecodeBlock(block, in);
        rtImageDequantize(kTableChroma, block);
        rtImageInverseDct(block);
        rtImageStoreBlock((int)index, u, block);
        in += rtVlcDecodeBlock(block, in);
        rtImageDequantize(kTableChroma, block);
        rtImageInverseDct(block);
        rtImageStoreBlock((int)index, v, block);
        for (part = 0; part < kLumaBlocksPerChroma; ++part) {
            in += rtVlcDecodeBlock(block, in);
            rtImageDequantize(kTableLuma, block);
            rtImageInverseDct(block);
            rtImageStoreBlock((int)(index * kLumaBlocksPerChroma + part), y, block);
        }
    }
    return 0;
}

rtImage *rtImageCreateSurface(int width, int height) {
    void *block = NULL;
    rtImage *surface;
    int stride = width * kPixelBytes;
    int size = height * stride;

    rtDmeAllocZeroed(&block, size + (int)sizeof(rtImage));
    surface = block;
    if (surface != NULL) {
        surface->size = size;
        surface->stride = stride;
        surface->width = width;
        surface->height = height;
    }
    return surface;
}

void rtImageDestroySurface(rtImage *surface) {
    void *block = surface;

    if (surface != NULL) {
        rtDmeFree(&block);
    }
}

// Returns a difference that stops at 0.
static inline unsigned int rtImageSubtractClamped(unsigned int a, unsigned int b) {
    return a > b ? a - b : 0;
}

static inline unsigned int rtImageLimitSample(unsigned int value) {
    return value > kSampleMax ? kSampleMax : value;
}

static inline void rtImageConvertPixel(unsigned int luma,
                                       unsigned int first,
                                       unsigned int second,
                                       unsigned char *pixel) {
    unsigned int green = (luma + kSampleBias) << kGreenScaleShift;

    green = rtImageSubtractClamped(green, kSampleBias);
    green = rtImageSubtractClamped(green, first);
    green = rtImageSubtractClamped(green, first);
    green = rtImageSubtractClamped(green, second);
    pixel[kPixelRed] =
        (unsigned char)rtImageLimitSample(rtImageSubtractClamped(first + luma, kSampleBias));
    pixel[kPixelGreen] = (unsigned char)rtImageLimitSample(green >> kGreenScaleShift);
    pixel[kPixelBlue] =
        (unsigned char)rtImageLimitSample(rtImageSubtractClamped(luma + second, kSampleBias));
    pixel[kPixelAlpha] = kPixelAlphaValue;
}

int rtImageConvertToRgb(rtImage *y, rtImage *u, rtImage *v, rtImage *surface) {
    const unsigned char *luma = y->pixels;
    const unsigned char *first = u->pixels;
    const unsigned char *second = v->pixels;
    unsigned char *pixel = surface->pixels;
    int rows = surface->height;
    int remaining;
    int x;

    // Yes, the binary runs each loop at least once, and processes whole steps of 32 pixels.
    do {
        remaining = (int)((unsigned int)surface->width >> 1);
        // Yes, the binary rewinds the chroma rows by half the width, not by their stride.
        if ((rows & 1) != 0) {
            first -= remaining;
            second -= remaining;
        }
        do {
            for (x = 0; x < kConvertLumaStep; ++x) {
                rtImageConvertPixel(luma[x], first[x / 2], second[x / 2], &pixel[x * kPixelBytes]);
            }
            luma = &luma[kConvertLumaStep];
            first = &first[kConvertChromaStep];
            second = &second[kConvertChromaStep];
            pixel = &pixel[kConvertLumaStep * kPixelBytes];
            remaining -= kConvertChromaStep;
        } while (remaining > 0);
        --rows;
    } while (rows > 0);
    return 0;
}
