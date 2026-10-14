#include <librtpitch.h>

enum {
    // Values at or below this limit normalise to 0.
    kNormNegativeLimit = -1073741824,
    kNormHighByteShift = 24,
    kNormThirdByteShift = 16,
    kNormSecondByteShift = 8,
    kNormHighByteBias = -1,
    kNormThirdByteBias = 7,
    kNormSecondByteBias = 15,
    kNormLowByteBias = 23,
    // The LPC-10 analysis window of each difference sum, in samples, less one.
    kDifmagWindowSpan = 155,
    kDifmagStride = 4,
};

// The number of zero bits above the highest set bit of each byte value.
// NTSC-U/C: 0x0042fc10
static const unsigned char bitoff[] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int rtGsmNorm(int value) {
    if (value == 0) {
        return 0;
    }
    if (value < 0) {
        if (value <= kNormNegativeLimit) {
            return 0;
        }
        value = ~value;
    }
    if ((value & 0xffff0000) != 0) {
        if ((value & 0xff000000) != 0) {
            return bitoff[(unsigned int)value >> kNormHighByteShift] + kNormHighByteBias;
        }
        return bitoff[(value >> kNormThirdByteShift) & 0xff] + kNormThirdByteBias;
    }
    if ((value & 0xff00) != 0) {
        return bitoff[(value >> kNormSecondByteShift) & 0xff] + kNormSecondByteBias;
    }
    return bitoff[value & 0xff] + kNormLowByteBias;
}

void rtLpc10Difmag(const float *speech,
                   const int *tau,
                   int lagCount,
                   int maxLag,
                   float *amdf,
                   int *minIndex,
                   int *maxIndex) {
    int i;
    int j;
    int first;
    int last;
    float sum;
    double difference;

    *minIndex = 1;
    *maxIndex = 1;
    for (i = 1; i <= lagCount; ++i) {
        first = (int)((double)(maxLag - tau[i]) * 0.5 + 1.0);
        last = first + kDifmagWindowSpan;
        sum = 0.0f;
        for (j = first; j <= last; j += kDifmagStride) {
            difference = speech[j] - speech[j + tau[i]];
            if (difference < 0.0) {
                difference = 0.0 - difference;
            }
            sum += (float)difference;
        }
        amdf[i] = sum;
        if (sum < amdf[*minIndex]) {
            *minIndex = i;
        }
        if (amdf[i] > amdf[*maxIndex]) {
            *maxIndex = i;
        }
    }
}
