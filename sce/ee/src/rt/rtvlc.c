#include "rt/rtvlc.h"

#include <string.h>

enum {
    kBlockSamples = 64,
    // The blocks of one macroblock of a picture: Cb, Cr, and four luma blocks.
    kBlocksPerMacroblock = 6,
    kBlockCb = 0,
    kBlockCr = 1,

    kByteBits = 8,
    kAccumulatorBits = 32,
    kTopBitShift = 31,

    // A symbol of the AC code is the run of zeros in the upper four bits and the size of the
    // value in the lower four.
    kRunShift = 4,
    kSizeMask = 15,
    kZeroRunLength = 16,
    kEndOfBlockSymbol = 0x00,
    kZeroRunSymbol = 0xf0,

    kDcCodeCount = 16,
    kAcCodeCount = 256,
    kDcTreeNodes = 25,
    kAcTreeNodes = 325,

    // The value of a node with children, and of the leaf node of an invalid code.
    kNodeBranch = -1,
    kNodeInvalid = -2,
    kNodeFirstChild = 1,
    kNodeSecondChild = 2,
};

// The state of the encoder or of the decoder.
typedef struct {
    int mode;
    int blockIndex;
    int reserved; // +0x08
    // The encoder counts the pending bits at the bottom of the accumulator. The decoder counts the
    // consumed bits there, and the value 32 marks an empty accumulator.
    int bitCount;
    int dcPredictorLuma;
    int dcPredictorCb;
    int dcPredictorCr;
    unsigned int bits;
} rtVlcState;

// One code of the encoder.
typedef struct {
    int length;
    unsigned int code;
} rtVlcCode;

// One node of a decoding tree.
typedef struct rtVlcHuffNode {
    const struct rtVlcHuffNode *child[2];
    int value;
} rtVlcHuffNode;

// NTSC-U/C: 0x00549830
static rtVlcState g_rtVlcEncoder;
// NTSC-U/C: 0x00549850
static rtVlcState g_rtVlcDecoder;

// The block index of each coefficient in zigzag order.
// NTSC-U/C: 0x0042db00, PAL: 0x004c5800
static const int g_anZigzag[kBlockSamples] = {
    0,  8,  1,  2,  9,  16, 24, 17, 10, 3,  4,  11, 18, 25, 32, 40, 33, 26, 19, 12, 5,  6,
    13, 20, 27, 34, 41, 48, 56, 49, 42, 35, 28, 21, 14, 7,  15, 22, 29, 36, 43, 50, 57, 58,
    51, 44, 37, 30, 23, 31, 38, 45, 52, 59, 60, 53, 46, 39, 47, 54, 61, 62, 55, 63,
};

// NTSC-U/C: 0x0042ea30, PAL: 0x004c6730
static const rtVlcCode g_aDcCodesLuma[kDcCodeCount] = {
    {2, 0x0},
    {3, 0x2},
    {3, 0x3},
    {3, 0x4},
    {3, 0x5},
    {3, 0x6},
    {4, 0xe},
    {5, 0x1e},
    {6, 0x3e},
    {7, 0x7e},
    {8, 0xfe},
    {9, 0x1fe},
    {10, 0x3fe},
    {11, 0x7fe},
    {12, 0xffe},
    {13, 0x1ffe},
};

// NTSC-U/C: 0x0042eab0, PAL: 0x004c67b0
static const rtVlcCode g_aDcCodesChroma[kDcCodeCount] = {
    {2, 0x0},
    {2, 0x1},
    {2, 0x2},
    {3, 0x6},
    {4, 0xe},
    {5, 0x1e},
    {6, 0x3e},
    {7, 0x7e},
    {8, 0xfe},
    {9, 0x1fe},
    {10, 0x3fe},
    {11, 0x7fe},
    {12, 0xffe},
    {13, 0x1ffe},
    {14, 0x3ffe},
    {15, 0x7ffe},
};

// NTSC-U/C: 0x0042eb30, PAL: 0x004c6830
static const rtVlcCode g_aAcCodesLuma[kAcCodeCount] = {
    {4, 0xa},     {2, 0x0},     {2, 0x1},     {3, 0x4},     {4, 0xb},     {5, 0x1a},
    {7, 0x78},    {8, 0xf8},    {10, 0x3f6},  {16, 0xff82}, {16, 0xff83}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {4, 0xc},
    {5, 0x1b},    {7, 0x79},    {9, 0x1f6},   {11, 0x7f6},  {16, 0xff84}, {16, 0xff85},
    {16, 0xff86}, {16, 0xff87}, {16, 0xff88}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {5, 0x1c},    {8, 0xf9},    {10, 0x3f7},
    {12, 0xff4},  {16, 0xff89}, {16, 0xff8a}, {16, 0xff8b}, {16, 0xff8c}, {16, 0xff8d},
    {16, 0xff8e}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {6, 0x3a},    {9, 0x1f7},   {12, 0xff5},  {16, 0xff8f}, {16, 0xff90},
    {16, 0xff91}, {16, 0xff92}, {16, 0xff93}, {16, 0xff94}, {16, 0xff95}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {6, 0x3b},
    {10, 0x3f8},  {16, 0xff96}, {16, 0xff97}, {16, 0xff98}, {16, 0xff99}, {16, 0xff9a},
    {16, 0xff9b}, {16, 0xff9c}, {16, 0xff9d}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {7, 0x7a},    {11, 0x7f7},  {16, 0xff9e},
    {16, 0xff9f}, {16, 0xffa0}, {16, 0xffa1}, {16, 0xffa2}, {16, 0xffa3}, {16, 0xffa4},
    {16, 0xffa5}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {7, 0x7b},    {12, 0xff6},  {16, 0xffa6}, {16, 0xffa7}, {16, 0xffa8},
    {16, 0xffa9}, {16, 0xffaa}, {16, 0xffab}, {16, 0xffac}, {16, 0xffad}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {8, 0xfa},
    {12, 0xff7},  {16, 0xffae}, {16, 0xffaf}, {16, 0xffb0}, {16, 0xffb1}, {16, 0xffb2},
    {16, 0xffb3}, {16, 0xffb4}, {16, 0xffb5}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {9, 0x1f8},   {15, 0x7fc0}, {16, 0xffb6},
    {16, 0xffb7}, {16, 0xffb8}, {16, 0xffb9}, {16, 0xffba}, {16, 0xffbb}, {16, 0xffbc},
    {16, 0xffbd}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {9, 0x1f9},   {16, 0xffbe}, {16, 0xffbf}, {16, 0xffc0}, {16, 0xffc1},
    {16, 0xffc2}, {16, 0xffc3}, {16, 0xffc4}, {16, 0xffc5}, {16, 0xffc6}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {9, 0x1fa},
    {16, 0xffc7}, {16, 0xffc8}, {16, 0xffc9}, {16, 0xffca}, {16, 0xffcb}, {16, 0xffcc},
    {16, 0xffcd}, {16, 0xffce}, {16, 0xffcf}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {10, 0x3f9},  {16, 0xffd0}, {16, 0xffd1},
    {16, 0xffd2}, {16, 0xffd3}, {16, 0xffd4}, {16, 0xffd5}, {16, 0xffd6}, {16, 0xffd7},
    {16, 0xffd8}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {10, 0x3fa},  {16, 0xffd9}, {16, 0xffda}, {16, 0xffdb}, {16, 0xffdc},
    {16, 0xffdd}, {16, 0xffde}, {16, 0xffdf}, {16, 0xffe0}, {16, 0xffe1}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {11, 0x7f8},
    {16, 0xffe2}, {16, 0xffe3}, {16, 0xffe4}, {16, 0xffe5}, {16, 0xffe6}, {16, 0xffe7},
    {16, 0xffe8}, {16, 0xffe9}, {16, 0xffea}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {16, 0xffeb}, {16, 0xffec}, {16, 0xffed},
    {16, 0xffee}, {16, 0xffef}, {16, 0xfff0}, {16, 0xfff1}, {16, 0xfff2}, {16, 0xfff3},
    {16, 0xfff4}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {11, 0x7f9},  {16, 0xfff5}, {16, 0xfff6}, {16, 0xfff7}, {16, 0xfff8}, {16, 0xfff9},
    {16, 0xfffa}, {16, 0xfffb}, {16, 0xfffc}, {16, 0xfffd}, {16, 0xfffe}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
};

// NTSC-U/C: 0x0042f330, PAL: 0x004c7030
static const rtVlcCode g_aAcCodesChroma[kAcCodeCount] = {
    {2, 0x0},     {2, 0x1},     {3, 0x4},     {4, 0xa},     {5, 0x18},    {5, 0x19},
    {6, 0x38},    {7, 0x78},    {9, 0x1f4},   {10, 0x3f6},  {12, 0xff4},  {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {4, 0xb},
    {6, 0x39},    {8, 0xf6},    {9, 0x1f5},   {11, 0x7f6},  {12, 0xff5},  {16, 0xff88},
    {16, 0xff89}, {16, 0xff8a}, {16, 0xff8b}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {5, 0x1a},    {8, 0xf7},    {10, 0x3f7},
    {12, 0xff6},  {15, 0x7fc2}, {16, 0xff8c}, {16, 0xff8d}, {16, 0xff8e}, {16, 0xff8f},
    {16, 0xff90}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {5, 0x1b},    {8, 0xf8},    {10, 0x3f8},  {12, 0xff7},  {16, 0xff91},
    {16, 0xff92}, {16, 0xff93}, {16, 0xff94}, {16, 0xff95}, {16, 0xff96}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {6, 0x3a},
    {9, 0x1f6},   {16, 0xff97}, {16, 0xff98}, {16, 0xff99}, {16, 0xff9a}, {16, 0xff9b},
    {16, 0xff9c}, {16, 0xff9d}, {16, 0xff9e}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {6, 0x3b},    {10, 0x3f9},  {16, 0xff9f},
    {16, 0xffa0}, {16, 0xffa1}, {16, 0xffa2}, {16, 0xffa3}, {16, 0xffa4}, {16, 0xffa5},
    {16, 0xffa6}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {7, 0x79},    {11, 0x7f7},  {16, 0xffa7}, {16, 0xffa8}, {16, 0xffa9},
    {16, 0xffaa}, {16, 0xffab}, {16, 0xffac}, {16, 0xffad}, {16, 0xffae}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {7, 0x7a},
    {11, 0x7f8},  {16, 0xffaf}, {16, 0xffb0}, {16, 0xffb1}, {16, 0xffb2}, {16, 0xffb3},
    {16, 0xffb4}, {16, 0xffb5}, {16, 0xffb6}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {8, 0xf9},    {16, 0xffb7}, {16, 0xffb8},
    {16, 0xffb9}, {16, 0xffba}, {16, 0xffbb}, {16, 0xffbc}, {16, 0xffbd}, {16, 0xffbe},
    {16, 0xffbf}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {9, 0x1f7},   {16, 0xffc0}, {16, 0xffc1}, {16, 0xffc2}, {16, 0xffc3},
    {16, 0xffc4}, {16, 0xffc5}, {16, 0xffc6}, {16, 0xffc7}, {16, 0xffc8}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {9, 0x1f8},
    {16, 0xffc9}, {16, 0xffca}, {16, 0xffcb}, {16, 0xffcc}, {16, 0xffcd}, {16, 0xffce},
    {16, 0xffcf}, {16, 0xffd0}, {16, 0xffd1}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {9, 0x1f9},   {16, 0xffd2}, {16, 0xffd3},
    {16, 0xffd4}, {16, 0xffd5}, {16, 0xffd6}, {16, 0xffd7}, {16, 0xffd8}, {16, 0xffd9},
    {16, 0xffda}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {9, 0x1fa},   {16, 0xffdb}, {16, 0xffdc}, {16, 0xffdd}, {16, 0xffde},
    {16, 0xffdf}, {16, 0xffe0}, {16, 0xffe1}, {16, 0xffe2}, {16, 0xffe3}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {11, 0x7f9},
    {16, 0xffe4}, {16, 0xffe5}, {16, 0xffe6}, {16, 0xffe7}, {16, 0xffe8}, {16, 0xffe9},
    {16, 0xffea}, {16, 0xffeb}, {16, 0xffec}, {0, 0x0},     {0, 0x0},     {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {14, 0x3fe0}, {16, 0xffed}, {16, 0xffee},
    {16, 0xffef}, {16, 0xfff0}, {16, 0xfff1}, {16, 0xfff2}, {16, 0xfff3}, {16, 0xfff4},
    {16, 0xfff5}, {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
    {10, 0x3fa},  {15, 0x7fc3}, {16, 0xfff6}, {16, 0xfff7}, {16, 0xfff8}, {16, 0xfff9},
    {16, 0xfffa}, {16, 0xfffb}, {16, 0xfffc}, {16, 0xfffd}, {16, 0xfffe}, {0, 0x0},
    {0, 0x0},     {0, 0x0},     {0, 0x0},     {0, 0x0},
};

// Every leaf node links to the children of the root. The walk of the next AC symbol continues from
// the leaf node of the previous one instead of from the root.
#define BRANCH(zero, one) {{&TREE[zero], &TREE[one]}, kNodeBranch}
#define LEAF(value) {{&TREE[kNodeFirstChild], &TREE[kNodeSecondChild]}, value}

#define TREE g_aDcTreeLuma
// NTSC-U/C: 0x003bfe80, PAL: 0x0043ed40
static const rtVlcHuffNode g_aDcTreeLuma[kDcTreeNodes] = {
    BRANCH(1, 2),   BRANCH(3, 4),   BRANCH(5, 6),   LEAF(0),        BRANCH(7, 8),
    BRANCH(9, 10),  BRANCH(11, 12), LEAF(1),        LEAF(2),        LEAF(3),
    LEAF(4),        LEAF(5),        BRANCH(13, 14), LEAF(6),        BRANCH(15, 16),
    LEAF(7),        BRANCH(17, 18), LEAF(8),        BRANCH(19, 20), LEAF(9),
    BRANCH(21, 22), LEAF(10),       BRANCH(23, 24), LEAF(11),       LEAF(kNodeInvalid),
};
#undef TREE

#define TREE g_aDcTreeChroma
// NTSC-U/C: 0x003bffb0, PAL: 0x0043ee70
static const rtVlcHuffNode g_aDcTreeChroma[kDcTreeNodes] = {
    BRANCH(1, 2),   BRANCH(3, 4),   BRANCH(5, 6),   LEAF(0),        LEAF(1),
    LEAF(2),        BRANCH(7, 8),   LEAF(3),        BRANCH(9, 10),  LEAF(4),
    BRANCH(11, 12), LEAF(5),        BRANCH(13, 14), LEAF(6),        BRANCH(15, 16),
    LEAF(7),        BRANCH(17, 18), LEAF(8),        BRANCH(19, 20), LEAF(9),
    BRANCH(21, 22), LEAF(10),       BRANCH(23, 24), LEAF(11),       LEAF(kNodeInvalid),
};
#undef TREE

#define TREE g_aAcTreeLuma
// NTSC-U/C: 0x003c00e0, PAL: 0x0043efa0
static const rtVlcHuffNode g_aAcTreeLuma[kAcTreeNodes] = {
    BRANCH(1, 2),     BRANCH(3, 4),     BRANCH(5, 6),     LEAF(0x01),       LEAF(0x02),
    BRANCH(7, 8),     BRANCH(9, 10),    LEAF(0x03),       BRANCH(11, 12),   BRANCH(13, 14),
    BRANCH(15, 16),   LEAF(0x00),       LEAF(0x04),       LEAF(0x11),       BRANCH(17, 18),
    BRANCH(19, 20),   BRANCH(21, 22),   LEAF(0x05),       LEAF(0x12),       LEAF(0x21),
    BRANCH(23, 24),   BRANCH(25, 26),   BRANCH(27, 28),   LEAF(0x31),       LEAF(0x41),
    BRANCH(29, 30),   BRANCH(31, 32),   BRANCH(33, 34),   BRANCH(35, 36),   LEAF(0x06),
    LEAF(0x13),       LEAF(0x51),       LEAF(0x61),       BRANCH(37, 38),   BRANCH(39, 40),
    BRANCH(41, 42),   BRANCH(43, 44),   LEAF(0x07),       LEAF(0x22),       LEAF(0x71),
    BRANCH(45, 46),   BRANCH(47, 48),   BRANCH(49, 50),   BRANCH(51, 52),   BRANCH(53, 54),
    LEAF(0x14),       LEAF(0x32),       LEAF(0x81),       LEAF(0x91),       LEAF(0xa1),
    BRANCH(55, 56),   BRANCH(57, 58),   BRANCH(59, 60),   BRANCH(61, 62),   BRANCH(63, 64),
    LEAF(0x08),       LEAF(0x23),       LEAF(0x42),       LEAF(0xb1),       LEAF(0xc1),
    BRANCH(65, 66),   BRANCH(67, 68),   BRANCH(69, 70),   BRANCH(71, 72),   BRANCH(73, 74),
    LEAF(0x15),       LEAF(0x52),       LEAF(0xd1),       LEAF(0xf0),       BRANCH(75, 76),
    BRANCH(77, 78),   BRANCH(79, 80),   BRANCH(81, 82),   BRANCH(83, 84),   BRANCH(85, 86),
    LEAF(0x24),       LEAF(0x33),       LEAF(0x62),       LEAF(0x72),       BRANCH(87, 88),
    BRANCH(89, 90),   BRANCH(91, 92),   BRANCH(93, 94),   BRANCH(95, 96),   BRANCH(97, 98),
    BRANCH(99, 100),  BRANCH(101, 102), BRANCH(103, 104), BRANCH(105, 106), BRANCH(107, 108),
    BRANCH(109, 110), BRANCH(111, 112), BRANCH(113, 114), BRANCH(115, 116), BRANCH(117, 118),
    BRANCH(119, 120), BRANCH(121, 122), BRANCH(123, 124), BRANCH(125, 126), BRANCH(127, 128),
    BRANCH(129, 130), BRANCH(131, 132), BRANCH(133, 134), BRANCH(135, 136), BRANCH(137, 138),
    BRANCH(139, 140), BRANCH(141, 142), BRANCH(143, 144), BRANCH(145, 146), BRANCH(147, 148),
    BRANCH(149, 150), BRANCH(151, 152), BRANCH(153, 154), BRANCH(155, 156), BRANCH(157, 158),
    BRANCH(159, 160), BRANCH(161, 162), BRANCH(163, 164), BRANCH(165, 166), BRANCH(167, 168),
    BRANCH(169, 170), BRANCH(171, 172), BRANCH(173, 174), BRANCH(175, 176), BRANCH(177, 178),
    BRANCH(179, 180), BRANCH(181, 182), BRANCH(183, 184), BRANCH(185, 186), BRANCH(187, 188),
    BRANCH(189, 190), BRANCH(191, 192), BRANCH(193, 194), BRANCH(195, 196), BRANCH(197, 198),
    LEAF(0x82),       BRANCH(199, 200), BRANCH(201, 202), BRANCH(203, 204), BRANCH(205, 206),
    BRANCH(207, 208), BRANCH(209, 210), BRANCH(211, 212), BRANCH(213, 214), BRANCH(215, 216),
    BRANCH(217, 218), BRANCH(219, 220), BRANCH(221, 222), BRANCH(223, 224), BRANCH(225, 226),
    BRANCH(227, 228), BRANCH(229, 230), BRANCH(231, 232), BRANCH(233, 234), BRANCH(235, 236),
    BRANCH(237, 238), BRANCH(239, 240), BRANCH(241, 242), BRANCH(243, 244), BRANCH(245, 246),
    BRANCH(247, 248), BRANCH(249, 250), BRANCH(251, 252), BRANCH(253, 254), BRANCH(255, 256),
    BRANCH(257, 258), BRANCH(259, 260), BRANCH(261, 262), BRANCH(263, 264), BRANCH(265, 266),
    BRANCH(267, 268), BRANCH(269, 270), BRANCH(271, 272), BRANCH(273, 274), BRANCH(275, 276),
    BRANCH(277, 278), BRANCH(279, 280), BRANCH(281, 282), BRANCH(283, 284), BRANCH(285, 286),
    BRANCH(287, 288), BRANCH(289, 290), BRANCH(291, 292), BRANCH(293, 294), BRANCH(295, 296),
    BRANCH(297, 298), BRANCH(299, 300), BRANCH(301, 302), BRANCH(303, 304), BRANCH(305, 306),
    BRANCH(307, 308), BRANCH(309, 310), BRANCH(311, 312), BRANCH(313, 314), BRANCH(315, 316),
    BRANCH(317, 318), BRANCH(319, 320), BRANCH(321, 322), BRANCH(323, 324), LEAF(0x09),
    LEAF(0x0a),       LEAF(0x16),       LEAF(0x17),       LEAF(0x18),       LEAF(0x19),
    LEAF(0x1a),       LEAF(0x25),       LEAF(0x26),       LEAF(0x27),       LEAF(0x28),
    LEAF(0x29),       LEAF(0x2a),       LEAF(0x34),       LEAF(0x35),       LEAF(0x36),
    LEAF(0x37),       LEAF(0x38),       LEAF(0x39),       LEAF(0x3a),       LEAF(0x43),
    LEAF(0x44),       LEAF(0x45),       LEAF(0x46),       LEAF(0x47),       LEAF(0x48),
    LEAF(0x49),       LEAF(0x4a),       LEAF(0x53),       LEAF(0x54),       LEAF(0x55),
    LEAF(0x56),       LEAF(0x57),       LEAF(0x58),       LEAF(0x59),       LEAF(0x5a),
    LEAF(0x63),       LEAF(0x64),       LEAF(0x65),       LEAF(0x66),       LEAF(0x67),
    LEAF(0x68),       LEAF(0x69),       LEAF(0x6a),       LEAF(0x73),       LEAF(0x74),
    LEAF(0x75),       LEAF(0x76),       LEAF(0x77),       LEAF(0x78),       LEAF(0x79),
    LEAF(0x7a),       LEAF(0x83),       LEAF(0x84),       LEAF(0x85),       LEAF(0x86),
    LEAF(0x87),       LEAF(0x88),       LEAF(0x89),       LEAF(0x8a),       LEAF(0x92),
    LEAF(0x93),       LEAF(0x94),       LEAF(0x95),       LEAF(0x96),       LEAF(0x97),
    LEAF(0x98),       LEAF(0x99),       LEAF(0x9a),       LEAF(0xa2),       LEAF(0xa3),
    LEAF(0xa4),       LEAF(0xa5),       LEAF(0xa6),       LEAF(0xa7),       LEAF(0xa8),
    LEAF(0xa9),       LEAF(0xaa),       LEAF(0xb2),       LEAF(0xb3),       LEAF(0xb4),
    LEAF(0xb5),       LEAF(0xb6),       LEAF(0xb7),       LEAF(0xb8),       LEAF(0xb9),
    LEAF(0xba),       LEAF(0xc2),       LEAF(0xc3),       LEAF(0xc4),       LEAF(0xc5),
    LEAF(0xc6),       LEAF(0xc7),       LEAF(0xc8),       LEAF(0xc9),       LEAF(0xca),
    LEAF(0xd2),       LEAF(0xd3),       LEAF(0xd4),       LEAF(0xd5),       LEAF(0xd6),
    LEAF(0xd7),       LEAF(0xd8),       LEAF(0xd9),       LEAF(0xda),       LEAF(0xe1),
    LEAF(0xe2),       LEAF(0xe3),       LEAF(0xe4),       LEAF(0xe5),       LEAF(0xe6),
    LEAF(0xe7),       LEAF(0xe8),       LEAF(0xe9),       LEAF(0xea),       LEAF(0xf1),
    LEAF(0xf2),       LEAF(0xf3),       LEAF(0xf4),       LEAF(0xf5),       LEAF(0xf6),
    LEAF(0xf7),       LEAF(0xf8),       LEAF(0xf9),       LEAF(0xfa),       LEAF(kNodeInvalid),
};
#undef TREE

#define TREE g_aAcTreeChroma
// NTSC-U/C: 0x003c1020, PAL: 0x0043fee0
static const rtVlcHuffNode g_aAcTreeChroma[kAcTreeNodes] = {
    BRANCH(1, 2),     BRANCH(3, 4),     BRANCH(5, 6),     LEAF(0x00),       LEAF(0x01),
    BRANCH(7, 8),     BRANCH(9, 10),    LEAF(0x02),       BRANCH(11, 12),   BRANCH(13, 14),
    BRANCH(15, 16),   LEAF(0x03),       LEAF(0x11),       BRANCH(17, 18),   BRANCH(19, 20),
    BRANCH(21, 22),   BRANCH(23, 24),   LEAF(0x04),       LEAF(0x05),       LEAF(0x21),
    LEAF(0x31),       BRANCH(25, 26),   BRANCH(27, 28),   BRANCH(29, 30),   BRANCH(31, 32),
    LEAF(0x06),       LEAF(0x12),       LEAF(0x41),       LEAF(0x51),       BRANCH(33, 34),
    BRANCH(35, 36),   BRANCH(37, 38),   BRANCH(39, 40),   LEAF(0x07),       LEAF(0x61),
    LEAF(0x71),       BRANCH(41, 42),   BRANCH(43, 44),   BRANCH(45, 46),   BRANCH(47, 48),
    BRANCH(49, 50),   LEAF(0x13),       LEAF(0x22),       LEAF(0x32),       LEAF(0x81),
    BRANCH(51, 52),   BRANCH(53, 54),   BRANCH(55, 56),   BRANCH(57, 58),   BRANCH(59, 60),
    BRANCH(61, 62),   LEAF(0x08),       LEAF(0x14),       LEAF(0x42),       LEAF(0x91),
    LEAF(0xa1),       LEAF(0xb1),       LEAF(0xc1),       BRANCH(63, 64),   BRANCH(65, 66),
    BRANCH(67, 68),   BRANCH(69, 70),   BRANCH(71, 72),   LEAF(0x09),       LEAF(0x23),
    LEAF(0x33),       LEAF(0x52),       LEAF(0xf0),       BRANCH(73, 74),   BRANCH(75, 76),
    BRANCH(77, 78),   BRANCH(79, 80),   BRANCH(81, 82),   LEAF(0x15),       LEAF(0x62),
    LEAF(0x72),       LEAF(0xd1),       BRANCH(83, 84),   BRANCH(85, 86),   BRANCH(87, 88),
    BRANCH(89, 90),   BRANCH(91, 92),   BRANCH(93, 94),   LEAF(0x0a),       LEAF(0x16),
    LEAF(0x24),       LEAF(0x34),       BRANCH(95, 96),   BRANCH(97, 98),   BRANCH(99, 100),
    BRANCH(101, 102), BRANCH(103, 104), BRANCH(105, 106), BRANCH(107, 108), BRANCH(109, 110),
    BRANCH(111, 112), BRANCH(113, 114), BRANCH(115, 116), BRANCH(117, 118), BRANCH(119, 120),
    BRANCH(121, 122), BRANCH(123, 124), BRANCH(125, 126), BRANCH(127, 128), BRANCH(129, 130),
    BRANCH(131, 132), BRANCH(133, 134), BRANCH(135, 136), BRANCH(137, 138), BRANCH(139, 140),
    BRANCH(141, 142), LEAF(0xe1),       BRANCH(143, 144), BRANCH(145, 146), BRANCH(147, 148),
    BRANCH(149, 150), BRANCH(151, 152), BRANCH(153, 154), BRANCH(155, 156), BRANCH(157, 158),
    BRANCH(159, 160), BRANCH(161, 162), BRANCH(163, 164), BRANCH(165, 166), BRANCH(167, 168),
    BRANCH(169, 170), BRANCH(171, 172), BRANCH(173, 174), BRANCH(175, 176), BRANCH(177, 178),
    BRANCH(179, 180), BRANCH(181, 182), BRANCH(183, 184), BRANCH(185, 186), BRANCH(187, 188),
    BRANCH(189, 190), BRANCH(191, 192), BRANCH(193, 194), BRANCH(195, 196), BRANCH(197, 198),
    BRANCH(199, 200), BRANCH(201, 202), BRANCH(203, 204), LEAF(0x25),       LEAF(0xf1),
    BRANCH(205, 206), BRANCH(207, 208), BRANCH(209, 210), BRANCH(211, 212), BRANCH(213, 214),
    BRANCH(215, 216), BRANCH(217, 218), BRANCH(219, 220), BRANCH(221, 222), BRANCH(223, 224),
    BRANCH(225, 226), BRANCH(227, 228), BRANCH(229, 230), BRANCH(231, 232), BRANCH(233, 234),
    BRANCH(235, 236), BRANCH(237, 238), BRANCH(239, 240), BRANCH(241, 242), BRANCH(243, 244),
    BRANCH(245, 246), BRANCH(247, 248), BRANCH(249, 250), BRANCH(251, 252), BRANCH(253, 254),
    BRANCH(255, 256), BRANCH(257, 258), BRANCH(259, 260), BRANCH(261, 262), BRANCH(263, 264),
    BRANCH(265, 266), BRANCH(267, 268), BRANCH(269, 270), BRANCH(271, 272), BRANCH(273, 274),
    BRANCH(275, 276), BRANCH(277, 278), BRANCH(279, 280), BRANCH(281, 282), BRANCH(283, 284),
    BRANCH(285, 286), BRANCH(287, 288), BRANCH(289, 290), BRANCH(291, 292), BRANCH(293, 294),
    BRANCH(295, 296), BRANCH(297, 298), BRANCH(299, 300), BRANCH(301, 302), BRANCH(303, 304),
    BRANCH(305, 306), BRANCH(307, 308), BRANCH(309, 310), BRANCH(311, 312), BRANCH(313, 314),
    BRANCH(315, 316), BRANCH(317, 318), BRANCH(319, 320), BRANCH(321, 322), BRANCH(323, 324),
    LEAF(0x17),       LEAF(0x18),       LEAF(0x19),       LEAF(0x1a),       LEAF(0x26),
    LEAF(0x27),       LEAF(0x28),       LEAF(0x29),       LEAF(0x2a),       LEAF(0x35),
    LEAF(0x36),       LEAF(0x37),       LEAF(0x38),       LEAF(0x39),       LEAF(0x3a),
    LEAF(0x43),       LEAF(0x44),       LEAF(0x45),       LEAF(0x46),       LEAF(0x47),
    LEAF(0x48),       LEAF(0x49),       LEAF(0x4a),       LEAF(0x53),       LEAF(0x54),
    LEAF(0x55),       LEAF(0x56),       LEAF(0x57),       LEAF(0x58),       LEAF(0x59),
    LEAF(0x5a),       LEAF(0x63),       LEAF(0x64),       LEAF(0x65),       LEAF(0x66),
    LEAF(0x67),       LEAF(0x68),       LEAF(0x69),       LEAF(0x6a),       LEAF(0x73),
    LEAF(0x74),       LEAF(0x75),       LEAF(0x76),       LEAF(0x77),       LEAF(0x78),
    LEAF(0x79),       LEAF(0x7a),       LEAF(0x82),       LEAF(0x83),       LEAF(0x84),
    LEAF(0x85),       LEAF(0x86),       LEAF(0x87),       LEAF(0x88),       LEAF(0x89),
    LEAF(0x8a),       LEAF(0x92),       LEAF(0x93),       LEAF(0x94),       LEAF(0x95),
    LEAF(0x96),       LEAF(0x97),       LEAF(0x98),       LEAF(0x99),       LEAF(0x9a),
    LEAF(0xa2),       LEAF(0xa3),       LEAF(0xa4),       LEAF(0xa5),       LEAF(0xa6),
    LEAF(0xa7),       LEAF(0xa8),       LEAF(0xa9),       LEAF(0xaa),       LEAF(0xb2),
    LEAF(0xb3),       LEAF(0xb4),       LEAF(0xb5),       LEAF(0xb6),       LEAF(0xb7),
    LEAF(0xb8),       LEAF(0xb9),       LEAF(0xba),       LEAF(0xc2),       LEAF(0xc3),
    LEAF(0xc4),       LEAF(0xc5),       LEAF(0xc6),       LEAF(0xc7),       LEAF(0xc8),
    LEAF(0xc9),       LEAF(0xca),       LEAF(0xd2),       LEAF(0xd3),       LEAF(0xd4),
    LEAF(0xd5),       LEAF(0xd6),       LEAF(0xd7),       LEAF(0xd8),       LEAF(0xd9),
    LEAF(0xda),       LEAF(0xe2),       LEAF(0xe3),       LEAF(0xe4),       LEAF(0xe5),
    LEAF(0xe6),       LEAF(0xe7),       LEAF(0xe8),       LEAF(0xe9),       LEAF(0xea),
    LEAF(0xf2),       LEAF(0xf3),       LEAF(0xf4),       LEAF(0xf5),       LEAF(0xf6),
    LEAF(0xf7),       LEAF(0xf8),       LEAF(0xf9),       LEAF(0xfa),       LEAF(kNodeInvalid),
};
#undef TREE

#undef BRANCH
#undef LEAF

// Returns the bits that encode a value and stores their count in size. A negative value is
// encoded as its one's complement.
static inline unsigned int rtVlcSplitValue(int value, int *size) {
    unsigned int bits = (unsigned int)value;
    unsigned int limit = 1;
    int count = 0;

    if (value < 0) {
        bits = 0u - (unsigned int)value;
        while (bits >= limit) {
            limit <<= 1;
            ++count;
        }
        bits = ~bits + limit;
    } else {
        while (bits >= limit) {
            limit <<= 1;
            ++count;
        }
    }
    *size = count;
    return bits;
}

// Writes the complete bytes of the encoder, up to three, and returns the advanced output.
static inline unsigned char *rtVlcEncoderWriteBytes(unsigned char *out) {
    unsigned int bits = g_rtVlcEncoder.bits >> (g_rtVlcEncoder.bitCount % kByteBits);

    if (g_rtVlcEncoder.bitCount >= 3 * kByteBits) {
        *out++ = (unsigned char)(bits >> (2 * kByteBits));
    }
    if (g_rtVlcEncoder.bitCount >= 2 * kByteBits) {
        *out++ = (unsigned char)(bits >> kByteBits);
    }
    *out++ = (unsigned char)bits;
    g_rtVlcEncoder.bitCount %= kByteBits;
    return out;
}

// Reads input bytes into the bits of the accumulator that the decoder consumed.
static inline const unsigned char *rtVlcDecoderRefill(const unsigned char *in) {
    unsigned int word = 0;
    int shift;

    if (g_rtVlcDecoder.bitCount >= kByteBits) {
        if (g_rtVlcDecoder.bitCount >= 4 * kByteBits) {
            word |= (unsigned int)*in++ << (3 * kByteBits);
        }
        if (g_rtVlcDecoder.bitCount >= 3 * kByteBits) {
            word |= (unsigned int)*in++ << (2 * kByteBits);
        }
        if (g_rtVlcDecoder.bitCount >= 2 * kByteBits) {
            word |= (unsigned int)*in++ << kByteBits;
        }
        word |= *in++;
        shift = g_rtVlcDecoder.bitCount % kByteBits;
        g_rtVlcDecoder.bits |= word << shift;
        g_rtVlcDecoder.bitCount = shift;
    }
    return in;
}

// Walks a decoding tree bit by bit from the node, stores the leaf node it arrives at as the next
// start node, and returns the value of the leaf node.
static inline int rtVlcDecoderReadSymbol(const rtVlcHuffNode **node, const unsigned char **in) {
    const rtVlcHuffNode *current = *node;

    do {
        current = current->child[g_rtVlcDecoder.bits >> kTopBitShift];
        g_rtVlcDecoder.bits <<= 1;
        if (++g_rtVlcDecoder.bitCount >= kByteBits) {
            g_rtVlcDecoder.bits |= *(*in)++;
            g_rtVlcDecoder.bitCount %= kByteBits;
        }
    } while (current->value == kNodeBranch);
    *node = current;
    return current->value;
}

// Returns the value of the size top bits of the accumulator.
static inline int rtVlcExtendValue(unsigned int bits, int size) {
    if (size == 0) {
        return 0;
    }
    if ((int)bits < 0) {
        return (int)(bits >> (kAccumulatorBits - size));
    }
    return -(int)(~bits >> (kAccumulatorBits - size));
}

void rtVlcEncoderInit(int mode) {
    memset(&g_rtVlcEncoder, 0, sizeof(g_rtVlcEncoder));
    g_rtVlcEncoder.mode = mode;
}

void rtVlcDecoderInit(int mode) {
    memset(&g_rtVlcDecoder, 0, sizeof(g_rtVlcDecoder));
    g_rtVlcDecoder.mode = mode;
    g_rtVlcDecoder.bitCount = kAccumulatorBits;
}

int rtVlcEncodeBlock(const short *block, unsigned char *out) {
    unsigned char *start = out;
    const rtVlcCode *dcCodes;
    const rtVlcCode *acCodes;
    const rtVlcCode *code;
    unsigned int bits;
    int component;
    int value;
    int size;
    int run;
    int i;

    if (g_rtVlcEncoder.mode != RT_VLC_MODE_IMAGE) {
        return 0;
    }
    component = g_rtVlcEncoder.blockIndex % kBlocksPerMacroblock;
    if (component == kBlockCb) {
        value = block[0] - g_rtVlcEncoder.dcPredictorCb;
        g_rtVlcEncoder.dcPredictorCb = block[0];
        dcCodes = g_aDcCodesChroma;
        acCodes = g_aAcCodesChroma;
    } else if (component == kBlockCr) {
        value = block[0] - g_rtVlcEncoder.dcPredictorCr;
        g_rtVlcEncoder.dcPredictorCr = block[0];
        dcCodes = g_aDcCodesChroma;
        acCodes = g_aAcCodesChroma;
    } else {
        value = block[0] - g_rtVlcEncoder.dcPredictorLuma;
        g_rtVlcEncoder.dcPredictorLuma = block[0];
        dcCodes = g_aDcCodesLuma;
        acCodes = g_aAcCodesLuma;
    }

    bits = rtVlcSplitValue(value, &size);
    code = &dcCodes[size];
    g_rtVlcEncoder.bitCount += code->length;
    g_rtVlcEncoder.bits <<= code->length;
    g_rtVlcEncoder.bitCount += size;
    g_rtVlcEncoder.bits = ((g_rtVlcEncoder.bits | code->code) << size) | bits;
    if (g_rtVlcEncoder.bitCount >= kAccumulatorBits) {
        return 0;
    }
    if (g_rtVlcEncoder.bitCount >= kByteBits) {
        out = rtVlcEncoderWriteBytes(out);
        if (g_rtVlcEncoder.bitCount >= kByteBits) {
            return 0; // Yes, the binary tests the remainder of the division by 8 again.
        }
    }

    run = 0;
    for (i = 1; i < kBlockSamples; ++i) {
        value = block[g_anZigzag[i]];
        if (value == 0) {
            ++run;
            continue;
        }
        while (run >= kZeroRunLength) {
            code = &acCodes[kZeroRunSymbol];
            g_rtVlcEncoder.bits <<= code->length;
            g_rtVlcEncoder.bitCount += code->length;
            g_rtVlcEncoder.bits |= code->code;
            run -= kZeroRunLength;
            if (g_rtVlcEncoder.bitCount >= kByteBits) {
                g_rtVlcEncoder.bitCount -= kByteBits;
                *out++ = (unsigned char)(g_rtVlcEncoder.bits >> g_rtVlcEncoder.bitCount);
            }
        }

        bits = rtVlcSplitValue(value, &size);
        code = &acCodes[(run << kRunShift) | size];
        g_rtVlcEncoder.bitCount += code->length;
        g_rtVlcEncoder.bits <<= code->length;
        g_rtVlcEncoder.bits |= code->code;
        if (g_rtVlcEncoder.bitCount >= kAccumulatorBits) {
            return 0;
        }
        if (g_rtVlcEncoder.bitCount >= kByteBits) {
            out = rtVlcEncoderWriteBytes(out);
        }

        g_rtVlcEncoder.bitCount += size;
        g_rtVlcEncoder.bits = (g_rtVlcEncoder.bits << size) | bits;
        if (g_rtVlcEncoder.bitCount >= 3 * kByteBits) {
            return 0;
        }
        if (g_rtVlcEncoder.bitCount >= kByteBits) {
            bits = g_rtVlcEncoder.bits >> (g_rtVlcEncoder.bitCount % kByteBits);
            if (g_rtVlcEncoder.bitCount >= 2 * kByteBits) {
                *out++ = (unsigned char)(bits >> kByteBits);
            }
            *out++ = (unsigned char)bits;
            g_rtVlcEncoder.bitCount %= kByteBits;
        }
        run = 0;
    }

    if (run > 0) {
        code = &acCodes[kEndOfBlockSymbol];
        g_rtVlcEncoder.bitCount += code->length;
        g_rtVlcEncoder.bits <<= code->length;
        g_rtVlcEncoder.bits |= code->code;
        if (g_rtVlcEncoder.bitCount >= kByteBits) {
            g_rtVlcEncoder.bitCount -= kByteBits;
            *out++ = (unsigned char)(g_rtVlcEncoder.bits >> g_rtVlcEncoder.bitCount);
        }
    }
    ++g_rtVlcEncoder.blockIndex;
    return (int)(out - start);
}

int rtVlcEncoderFlush(unsigned char *out) {
    unsigned char *p = out;
    int count = g_rtVlcEncoder.bitCount;
    unsigned int bits = g_rtVlcEncoder.bits << ((kAccumulatorBits - count) % kByteBits);

    if (count > 0) {
        if (count > 3 * kByteBits) {
            *p++ = (unsigned char)(bits >> (3 * kByteBits));
        }
        if (count > 2 * kByteBits) {
            *p++ = (unsigned char)(bits >> (2 * kByteBits));
        }
        if (count > kByteBits) {
            *p++ = (unsigned char)(bits >> kByteBits);
        }
        *p++ = (unsigned char)bits;
    }
    return (int)(p - out);
}

int rtVlcDecodeBlock(short *block, const unsigned char *in) {
    const unsigned char *start = in;
    const rtVlcHuffNode *dcNode;
    const rtVlcHuffNode *acNode;
    unsigned int bits;
    int *predictor;
    int component;
    int symbol;
    int size;
    int i;

    if (g_rtVlcDecoder.mode != RT_VLC_MODE_IMAGE) {
        return 0;
    }
    component = g_rtVlcDecoder.blockIndex % kBlocksPerMacroblock;
    if (component == kBlockCb) {
        predictor = &g_rtVlcDecoder.dcPredictorCb;
        dcNode = g_aDcTreeChroma;
        acNode = g_aAcTreeChroma;
    } else if (component == kBlockCr) {
        predictor = &g_rtVlcDecoder.dcPredictorCr;
        dcNode = g_aDcTreeChroma;
        acNode = g_aAcTreeChroma;
    } else {
        predictor = &g_rtVlcDecoder.dcPredictorLuma;
        dcNode = g_aDcTreeLuma;
        acNode = g_aAcTreeLuma;
    }
    memset(block, 0, kBlockSamples * sizeof(*block));

    in = rtVlcDecoderRefill(in);
    size = rtVlcDecoderReadSymbol(&dcNode, &in);
    if (size == kNodeInvalid) {
        return -1;
    }
    bits = g_rtVlcDecoder.bits;
    g_rtVlcDecoder.bits <<= size;
    g_rtVlcDecoder.bitCount += size;
    *predictor += rtVlcExtendValue(bits, size);
    block[0] = (short)*predictor;
    in = rtVlcDecoderRefill(in);

    for (i = 1; i < kBlockSamples; ++i) {
        symbol = rtVlcDecoderReadSymbol(&acNode, &in);
        if (symbol == kNodeInvalid) {
            return -1;
        }
        if (symbol == kEndOfBlockSymbol) {
            break;
        }
        i += (unsigned int)symbol >> kRunShift;
        if (i >= kBlockSamples) {
            return -1;
        }
        size = symbol & kSizeMask;
        bits = g_rtVlcDecoder.bits;
        g_rtVlcDecoder.bits <<= size;
        g_rtVlcDecoder.bitCount += size;
        in = rtVlcDecoderRefill(in);
        block[g_anZigzag[i]] = (short)rtVlcExtendValue(bits, size);
    }

    ++g_rtVlcDecoder.blockIndex;
    // Rewind the input over the bytes of the accumulator that the block did not consume.
    while (g_rtVlcDecoder.bitCount < kAccumulatorBits) {
        g_rtVlcDecoder.bitCount += kByteBits;
        --in;
    }
    g_rtVlcDecoder.bits = 0;
    return (int)(in - start);
}
