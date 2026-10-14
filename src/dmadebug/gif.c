#include "dmadebug/gif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmadebug/debug.h"

// Most fields one GS register has.
enum { MAX_REGISTER_FIELDS = 12 };

// Capacity of the label of one register field.
enum { FIELD_LABEL_SIZE = 96 };

// Capacity of the report line of one quadword.
enum { QWORD_LINE_SIZE = 2048 };

// Capacity of the description FormatPrimitive() builds.
enum { PRIMITIVE_DESCRIPTION_SIZE = 200 };

// Attribute bits of PRIM after its three type bits.
enum { PRIMITIVE_ATTRIBUTES = 8 };

// GS register identifiers that the decoder treats specially.
enum {
    GS_PRIM = 0x00,
    GS_RGBAQ = 0x01,
    GS_ST = 0x02,
    GS_UV = 0x03,
    GS_XYZF2 = 0x04,
    GS_XYZ2 = 0x05,
    GS_TEX0_1 = 0x06,
    GS_TEX0_2 = 0x07,
    GS_CLAMP_1 = 0x08,
    GS_CLAMP_2 = 0x09,
    GS_FOG = 0x0a,
    GS_XYZF3 = 0x0c,
    GS_XYZ3 = 0x0d,
    GS_TEX1_1 = 0x14,
    GS_TEX1_2 = 0x15,
    GS_TEX2_1 = 0x16,
    GS_TEX2_2 = 0x17,
    GS_XYOFFSET_1 = 0x18,
    GS_XYOFFSET_2 = 0x19,
    GS_PRMODECONT = 0x1a,
    GS_PRMODE = 0x1b,
    GS_TEXCLUT = 0x1c,
    GS_SCANMSK = 0x22,
    GS_MIPTBP1_1 = 0x34,
    GS_MIPTBP1_2 = 0x35,
    GS_MIPTBP2_1 = 0x36,
    GS_MIPTBP2_2 = 0x37,
    GS_TEXA = 0x3b,
    GS_FOGCOL = 0x3d,
    GS_TEXFLUSH = 0x3f,
    GS_SCISSOR_1 = 0x40,
    GS_SCISSOR_2 = 0x41,
    GS_ALPHA_1 = 0x42,
    GS_ALPHA_2 = 0x43,
    GS_DIMX = 0x44,
    GS_DTHE = 0x45,
    GS_COLCLAMP = 0x46,
    GS_TEST_1 = 0x47,
    GS_TEST_2 = 0x48,
    GS_PABE = 0x49,
    GS_FBA_1 = 0x4a,
    GS_FBA_2 = 0x4b,
    GS_FRAME_1 = 0x4c,
    GS_FRAME_2 = 0x4d,
    GS_ZBUF_1 = 0x4e,
    GS_ZBUF_2 = 0x4f,
    GS_BITBLTBUF = 0x50,
    GS_TRXPOS = 0x51,
    GS_TRXREG = 0x52,
    GS_TRXDIR = 0x53,
    GS_HWREG = 0x54,
    GS_SIGNAL = 0x60,
    GS_FINISH = 0x61,
    GS_LABEL = 0x62,
    GS_NOP = 0x7f,
    GS_PMODE = 0x80,
    GS_SMODE2 = 0x82,
    GS_DISPFB1 = 0x87,
    GS_DISPLAY1 = 0x88,
    GS_DISPFB2 = 0x89,
    GS_DISPLAY2 = 0x8a,
    GS_EXTBUF = 0x8b,
    GS_EXTDATA = 0x8c,
    GS_EXTWRITE = 0x8d,
    GS_BGCOLOR = 0x8e,
    GS_CSR = 0xc0,
    GS_IMR = 0xc1,
    GS_BUSDIR = 0xc4,
    GS_SIGLBLID = 0xc8,
    GS_SKIPPED = 0xfd,
    GS_RESERVED = 0xfe,
    GS_NO_OPERATION = 0xff,
};

// The identifier bit that selects TEX2 rather than TEX0.
enum { GS_TEX2_BIT = 0x10 };

// The identifier bits that select a Z buffer pixel format.
enum { PSM_Z_BITS = 0x30 };

// Pixel storage formats.
enum {
    PSMCT32 = 0,
    PSMCT24 = 1,
    PSMCT16 = 2,
    PSMCT16S = 10,
    PS_GPU24 = 18,
    PSMT8 = 19,
    PSMT4 = 20,
    PSMT8H = 27,
    PSMT4HL = 36,
    PSMT4HH = 44,
    PSMZ32 = 48,
    PSMZ24 = 49,
    PSMZ16 = 50,
    PSMZ16S = 58,
};

// Values of the REGS field of a GIF tag.
enum {
    GIF_REG_XYZF2 = 4,
    GIF_REG_XYZ2 = 5,
    GIF_REG_FOG = 10,
    GIF_REG_RESERVED = 11,
    GIF_REG_XYZF3 = 12,
    GIF_REG_XYZ3 = 13,
    GIF_REG_A_D = 14,
    GIF_REG_NOP = 15,
    GIF_REG_COUNT = 16,
};

// The distance from XYZF2 and XYZ2 to the identifiers without a drawing kick.
enum { GIF_REG_NO_KICK_OFFSET = 8 };

// The ADC bit of packed XYZF2 and XYZ2 data, in its fourth word.
enum { GIF_PACKED_ADC = 0x8000 };

// The register identifier of packed A+D data, in its third word.
enum { GIF_A_D_REGISTER_MASK = 0x7f };

// Fields of CSR the decoder reports.
enum { CSR_FIELDS = 6 };

// A FRAME pixel format the decoder accepts without a name for it.
enum { PSM_UNNAMED = 6 };

// Values of the FLG field of a GIF tag.
enum {
    GIF_FLG_PACKED = 0,
    GIF_FLG_REGLIST = 1,
    GIF_FLG_IMAGE = 2,
    GIF_FLG_DISABLED = 3,
};

// Message numbers.
enum {
    MESSAGE_ILLEGAL_PRIMITIVE = 700,
    MESSAGE_RESERVED_REGISTER = 710,
    MESSAGE_UNKNOWN_REGISTER = 730,
    MESSAGE_MISALIGNED = 740,
    MESSAGE_WRONG_DIRECTION = 750,
    MESSAGE_NO_DATA = 760,
    MESSAGE_PRE_IGNORED = 5000,
    MESSAGE_SURPLUS_REGS = 5010,
    MESSAGE_BAD_FIELD = 5030,
    MESSAGE_REGLIST_A_D = 5060,
};

// Values of the CLD field that are reserved.
enum { CLD_FIRST_RESERVED = 6 };

// Values of MXL and MMIN that are reserved.
enum {
    MXL_FIRST_RESERVED = 7,
    MMIN_FIRST_RESERVED = 6,
};

// The sign bit of a DIMX matrix element, and the bits that extend it.
enum {
    DIMX_ELEMENT_MASK = 7,
    DIMX_ELEMENT_SIGN = 4,
    DIMX_ELEMENT_SIZE = 4,
};

// The dither matrix is four rows by four columns.
enum { DIMX_ROWS = 4 };

// One field of a GS register.
struct gs_register_field {
    const char *name;        // Field name.
    const char *description; // What the field controls.
    int shift;               // The first bit of the field in the 64-bit value.
    int bits;                // The width of the field.
};

// One GS register the decoder knows.
struct gs_register {
    unsigned char id;                                     // Register identifier.
    const char *name;                                     // Register name.
    const char *description;                              // What the register controls.
    int fieldCount;                                       // Entries of fields in use.
    struct gs_register_field fields[MAX_REGISTER_FIELDS]; // The fields.
};

// The current GIF tag.
struct gif_tag {
    const unsigned int *address;     // The tag.
    unsigned int nloop;              // NLOOP.
    unsigned int eop;                // EOP.
    unsigned int pre;                // PRE.
    unsigned int prim;               // PRIM.
    unsigned int flg;                // FLG.
    unsigned int nregs;              // NREGS, 16 when the tag stores 0.
    unsigned int regsLow;            // REGS, its low word.
    unsigned int regsHigh;           // REGS, its high word.
    signed char regs[GIF_REG_COUNT]; // REGS, one nibble each.
};

// The position of the decoder in the data of the current tag.
struct gif_progress {
    unsigned int nloop : 15;      // Loops remaining.
    unsigned int reserved15 : 1;  // Not yet identified.
    unsigned int reg : 4;         // The next REGS entry, from 1.
    unsigned int reserved20 : 10; // Cleared with the loop count; not yet identified.
    unsigned int reserved30 : 2;  // Not yet identified.
};

// The GIF_STAT bit that directs the transfer from the GS to the EE.
enum { GIF_STAT_DIR_SHIFT = 12 };

const unsigned int *g_pGifData;

// NTSC-U/C: 0x00519a20
// The GIF_STAT value the transfer runs with.
static unsigned int g_dwGifStat;

// NTSC-U/C: 0x00519a24
// The words of the current GIF tag.
static unsigned int g_adwGifTag[4];

// NTSC-U/C: 0x00519a38
// The position of the decoder in the current tag.
static struct gif_progress g_gifProgress;

// NTSC-U/C: 0x00519a48
// The fields of the current GIF tag.
static struct gif_tag g_gifTag;

// NTSC-U/C: 0x00519950
// The description FormatPrimitive() builds.
static char g_szGifPrimDescription[PRIMITIVE_DESCRIPTION_SIZE];

// NTSC-U/C: 0x003b2580
// The GS registers the decoder knows.
static const struct gs_register g_aGsRegisters[] = {
    {0x42,
     "ALPHA_1",
     "Alpha Blending settings",
     5,
     {{"A", "A", 0, 2},
      {"B", "B", 2, 2},
      {"C", "C", 4, 2},
      {"D", "D", 6, 2},
      {"FIX", "FIX", 32, 8}}},
    {0x43,
     "ALPHA_2",
     "Alpha Blending settings",
     5,
     {{"A", "A", 0, 2},
      {"B", "B", 2, 2},
      {"C", "C", 4, 2},
      {"D", "D", 6, 2},
      {"FIX", "FIX", 32, 8}}},
    {0x50,
     "BITBLTBUF",
     "Intra-Buffer Transmission settings",
     6,
     {{"SBP", "Source Buffer Base Pointer", 0, 14},
      {"SBW", "Source Buffer Width", 16, 6},
      {"SPSM", "Source Pixel Storage Format", 24, 6},
      {"DBP", "Destination Buffer Base Pointer", 32, 14},
      {"DBW", "Destination Buffer Width", 48, 6},
      {"DPSM", "Destination Pixel Storage Format", 56, 6}}},
    {0x08,
     "CLAMP_1",
     "Texture Wrap mode",
     6,
     {{"WMS", "Wrap Mode (Horizontal)", 0, 2},
      {"WMT", "Wrap Mode (Vertical)", 2, 2},
      {"MINU", "Clamp Parameter in U dir. (MIN)", 4, 10},
      {"MAXU", "Clamp Parameter in U dir. (MAX)", 14, 10},
      {"MINV", "Clamp Parameter in V dir. (MIN)", 24, 10},
      {"MAXV", "Clamp Parameter in V dir. (MAX)", 34, 10}}},
    {0x09,
     "CLAMP_2",
     "Texture Wrap mode",
     6,
     {{"WMS", "Wrap Mode (Horizontal)", 0, 2},
      {"WMT", "Wrap Mode (Vertical)", 2, 2},
      {"MINU", "Clamp Parameter in U dir. (MIN)", 4, 10},
      {"MAXU", "Clamp Parameter in U dir. (MAX)", 14, 10},
      {"MINV", "Clamp Parameter in V dir. (MIN)", 24, 10},
      {"MAXV", "Clamp Parameter in V dir. (MAX)", 34, 10}}},
    {0x46, "COLCLAMP", "Colour Clamp control", 1, {{"CLAMP", "Colour Clamping Method", 0, 1}}},
    {0x44, "DIMX", "Dither Matrix setting", 1, {{"DIMX", "Dither Matrix", 0, 1}}},
    {0x45, "DTHE", "Dither Control", 0, {{NULL, NULL, 0, 0}}},
    {0x4a,
     "FBA_1",
     "Alpha Correction value",
     1,
     {{"FBA", "MSB of Alpha value for drawing to Frame Buffer", 0, 1}}},
    {0x4b,
     "FBA_2",
     "Alpha Correction value",
     1,
     {{"FBA", "MSB of Alpha value for drawing to Frame Buffer", 0, 1}}},
    {0x61, "FINISH", "FINISH Event Occurence Request", 0, {{NULL, NULL, 0, 0}}},
    {0x0a, "FOG", "Vertex Fog Value setting", 1, {{"F", "Fog", 56, 8}}},
    {0x3d,
     "FOGCOL",
     "Distant Fog Colour settings",
     3,
     {{"FCR", "Fog Colour Red", 0, 8},
      {"FCG", "Fog Colour Green", 8, 8},
      {"FCB", "Fog Colour Blue", 16, 8}}},
    {0x4c,
     "FRAME_1",
     "Frame Buffer setting",
     4,
     {{"FBP", "Frame Buffer Pointer", 0, 9},
      {"FBW", "Frame Buffer Width", 16, 6},
      {"PSM", "Pixel Storage Format", 24, 6},
      {"FBMSK", "Frame Buffer Drawing Mask", 32, 32}}},
    {0x4d,
     "FRAME_2",
     "Frame Buffer setting",
     4,
     {{"FBP", "Frame Buffer Pointer", 0, 9},
      {"FBW", "Frame Buffer Width", 16, 6},
      {"PSM", "Pixel Storage Format", 24, 6},
      {"FBMSK", "Frame Buffer Drawing Mask", 32, 32}}},
    {0x54,
     "HWREG",
     "Intra-Buffer Transmission Port",
     2,
     {{"DATA(L)", "DATA (Low 32 bits)", 0, 32}, {"DATA(H)", "DATA (High 32 bits)", 32, 32}}},
    {0x62,
     "LABEL",
     "LABEL Event Occurence Request",
     2,
     {{"ID", "Value written to SIGLBLID register", 0, 32},
      {"IDMSK", "Whether or not the corresponding SIGBLID bit is updated", 32, 32}}},
    {0x34,
     "MIPTBP1_1",
     "MIPMAP settings (Level 1 to 3)",
     6,
     {{"TBP1", "MIPMAP Level 1 Texture Base Pointer", 0, 14},
      {"TBW1", "MIPMAP Level 1 Texture Buffer Width", 14, 6},
      {"TBP2", "MIPMAP Level 2 Texture Base Pointer", 20, 14},
      {"TBW2", "MIPMAP Level 2 Texture Buffer Width", 34, 6},
      {"TBP3", "MIPMAP Level 3 Texture Base Pointer", 40, 14},
      {"TBW3", "MIPMAP Level 3 Texture Buffer Width", 54, 6}}},
    {0x35,
     "MIPTBP1_2",
     "MIPMAP settings (Level 1 to 3)",
     6,
     {{"TBP1", "MIPMAP Level 1 Texture Base Pointer", 0, 14},
      {"TBW1", "MIPMAP Level 1 Texture Buffer Width", 14, 6},
      {"TBP2", "MIPMAP Level 2 Texture Base Pointer", 20, 14},
      {"TBW2", "MIPMAP Level 2 Texture Buffer Width", 34, 6},
      {"TBP3", "MIPMAP Level 3 Texture Base Pointer", 40, 14},
      {"TBW3", "MIPMAP Level 3 Texture Buffer Width", 54, 6}}},
    {0x36,
     "MIPTBP2_1",
     "MIPMAP settings (Level 4 to 6)",
     6,
     {{"TBP4", "MIPMAP Level 4 Texture Base Pointer", 0, 14},
      {"TBW4", "MIPMAP Level 4 Texture Buffer Width", 14, 6},
      {"TBP5", "MIPMAP Level 5 Texture Base Pointer", 20, 14},
      {"TBW5", "MIPMAP Level 5 Texture Buffer Width", 34, 6},
      {"TBP6", "MIPMAP Level 6 Texture Base Pointer", 40, 14},
      {"TBW6", "MIPMAP Level 6 Texture Buffer Width", 54, 6}}},
    {0x37,
     "MIPTBP2_2",
     "MIPMAP settings (Level 4 to 6)",
     6,
     {{"TBP4", "MIPMAP Level 4 Texture Base Pointer", 0, 14},
      {"TBW4", "MIPMAP Level 4 Texture Buffer Width", 14, 6},
      {"TBP5", "MIPMAP Level 5 Texture Base Pointer", 20, 14},
      {"TBW5", "MIPMAP Level 5 Texture Buffer Width", 34, 6},
      {"TBP6", "MIPMAP Level 6 Texture Base Pointer", 40, 14},
      {"TBW6", "MIPMAP Level 6 Texture Buffer Width", 54, 6}}},
    {0x7f, "NOP", "No Operation", 0, {{NULL, NULL, 0, 0}}},
    {0x49,
     "PABE",
     "Pixel Level Alpha Blending control",
     1,
     {{"PABE", "Alpha Blending Control Performed in Units of Pixels", 0, 1}}},
    {0x00,
     "PRIM",
     "Drawing Primitive settings",
     9,
     {{"PRIM", "Drawing Primitive Type", 0, 3},
      {"IIP", "Shading Method (Flat/Gouraud)", 3, 1},
      {"TME", "Texture Mapping", 4, 1},
      {"FGE", "Fogging", 5, 1},
      {"ABE", "Alpha Blending", 6, 1},
      {"AA1", "Anti-Aliasing", 7, 1},
      {"FST", "Method of specifying texture coords (STQ/UV)", 8, 1},
      {"CTXT", "Context", 9, 1},
      {"FIX", "Fragment Value Control", 10, 1}}},
    {0x1b,
     "PRMODE",
     "Drawing Primitive Attribute settings",
     8,
     {{"IIP", "Shading Method (Flat/Gouraud)", 3, 1},
      {"TME", "Texture Mapping", 4, 1},
      {"FGE", "Fogging", 5, 1},
      {"ABE", "Alpha Blending", 6, 1},
      {"AA1", "Anti-Aliasing", 7, 1},
      {"FST", "Method of specifying texture coords (STQ/UV)", 8, 1},
      {"CTXT", "Context", 9, 1},
      {"FIX", "Fragment Value Control", 10, 1}}},
    {0x1a,
     "PRMODECONT",
     "Enable Primitive Attribute setting",
     1,
     {{"AC", "Attributes of primitives specified by PRMODE (0) or PRIM (1)", 0, 1}}},
    {0x01,
     "RGBAQ",
     "Vertex Colour setting",
     5,
     {{"R", "Red", 0, 8},
      {"G", "Green", 8, 8},
      {"B", "Blue", 16, 8},
      {"A", "Alpha", 24, 8},
      {"Q", "Q", 32, 32}}},
    {0x22,
     "SCANMSK",
     "Raster Address Mask setting",
     1,
     {{"MSK", "Even/odd row drawing control", 0, 2}}},
    {0x40,
     "SCISSOR_1",
     "Scissoring Area settings",
     4,
     {{"SCAX0", "Drawing area upper left X coord", 0, 11},
      {"SCAX1", "Drawing area lower right X coord", 16, 11},
      {"SCAY0", "Drawing area upper left Y coord", 32, 11},
      {"SCAY1", "Drawing area lower right Y coord", 48, 11}}},
    {0x41,
     "SCISSOR_2",
     "Scissoring Area settings",
     4,
     {{"SCAX0", "Drawing area upper left X coord", 0, 11},
      {"SCAX1", "Drawing area lower right X coord", 16, 11},
      {"SCAY0", "Drawing area upper left Y coord", 32, 11},
      {"SCAY1", "Drawing area lower right Y coord", 48, 11}}},
    {0x60,
     "SIGNAL",
     "SIGNAL Event Occurence Request",
     2,
     {{"ID", "Value written to SIGLBLID register", 0, 32},
      {"IDMSK", "Whether or not the corresponding SIGBLID bit is updated", 32, 32}}},
    {0x02,
     "ST",
     "Vertex Texture Coordinate setting",
     2,
     {{"S", "S Texture Coord", 0, 32}, {"T", "T Texture Coord", 32, 32}}},
    {0x47,
     "TEST_1",
     "Pixel Test Control",
     8,
     {{"ATE", "Alpha Test", 0, 1},
      {"ATST", "Alpha Test Method", 1, 3},
      {"AREF", "Alpha Value to be compared/referred to", 4, 8},
      {"AFAIL", "Processing method when Alpha Test fails", 12, 2},
      {"DATE", "Destination Alpha Test", 14, 1},
      {"DATM", "Destination Alpha Test Mode", 15, 1},
      {"ZTE", "Depth Test", 16, 1},
      {"ZTST", "Depth Test Method", 17, 2}}},
    {0x48,
     "TEST_2",
     "Pixel Test Control",
     8,
     {{"ATE", "Alpha Test", 0, 1},
      {"ATST", "Alpha Test Method", 1, 3},
      {"AREF", "Alpha Value to be compared/referred to", 4, 8},
      {"AFAIL", "Processing method when Alpha Test fails", 12, 2},
      {"DATE", "Destination Alpha Test", 14, 1},
      {"DATM", "Destination Alpha Test Mode", 15, 1},
      {"ZTE", "Depth Test", 16, 1},
      {"ZTST", "Depth Test Method", 17, 2}}},
    {0x06,
     "TEX0_1",
     "Texture information setting",
     12,
     {{"TBP0", "Texture Base Pointer", 0, 14},
      {"TBW", "Texture Buffer Width", 14, 6},
      {"PSM", "Pixel Storage Method", 20, 6},
      {"TW", "Texture width", 26, 4},
      {"TH", "Texture height", 30, 4},
      {"TCC", "Texture Colour Component", 34, 1},
      {"TFX", "Texture Function", 35, 2},
      {"CBP", "CLUT Buffer Base Pointer", 37, 14},
      {"CPSM", "CLUT Pixel Storage Format", 51, 4},
      {"CSM", "CLUT Storage Mode", 55, 1},
      {"CSA", "CLUT Entry Offset", 56, 5},
      {"CLD", "CLUT Buffer Load Control", 61, 3}}},
    {0x07,
     "TEX0_2",
     "Texture information setting",
     12,
     {{"TBP0", "Texture Base Pointer", 0, 14},
      {"TBW", "Texture Buffer Width", 14, 6},
      {"PSM", "Pixel Storage Method", 20, 6},
      {"TW", "Texture width", 26, 4},
      {"TH", "Texture height", 30, 4},
      {"TCC", "Texture Colour Component", 34, 1},
      {"TFX", "Texture Function", 35, 2},
      {"CBP", "CLUT Buffer Base Pointer", 37, 14},
      {"CPSM", "CLUT Pixel Storage Format", 51, 4},
      {"CSM", "CLUT Storage Mode", 55, 1},
      {"CSA", "CLUT Entry Offset", 56, 5},
      {"CLD", "CLUT Buffer Load Control", 61, 3}}},
    {0x14,
     "TEX1_1",
     "Texture information setting",
     7,
     {{"LCM", "LOD Calculation Method", 0, 1},
      {"MXL", "Maximum MIP Level (0-6)", 2, 3},
      {"MMAG", "Filter when Texture is expanded (LOD < 0)", 5, 1},
      {"MMIN", "Filter when Texture is reduced (LOD >= 0)", 6, 3},
      {"MTBA", "Base address specification of MIPMAP Texture 1", 9, 1},
      {"L", "LOD Parameter L", 19, 2},
      {"K", "LOD Parameter K", 32, 12}}},
    {0x15,
     "TEX1_2",
     "Texture information setting",
     7,
     {{"LCM", "LOD Calculation Method", 0, 1},
      {"MXL", "Maximum MIP Level (0-6)", 2, 3},
      {"MMAG", "Filter when Texture is expanded (LOD < 0)", 5, 1},
      {"MMIN", "Filter when Texture is reduced (LOD >= 0)", 6, 3},
      {"MTBA", "Base address specification of MIPMAP Texture 1", 9, 1},
      {"L", "LOD Parameter L", 19, 2},
      {"K", "LOD Parameter K", 32, 12}}},
    {0x16,
     "TEX2_1",
     "Texture information setting",
     6,
     {{"PSM", "Pixel Storage Method", 20, 6},
      {"CBP", "CLUT Buffer Base Pointer", 37, 14},
      {"CPSM", "CLUT Pixel Storage Format", 51, 4},
      {"CSM", "CLUT Storage Mode", 55, 4},
      {"CSA", "CLUT Entry Offset", 56, 5},
      {"CLD", "CLUT Buffer Load Control", 61, 3}}},
    {0x17,
     "TEX2_2",
     "Texture information setting",
     6,
     {{"PSM", "Pixel Storage Method", 20, 6},
      {"CBP", "CLUT Buffer Base Pointer", 37, 14},
      {"CPSM", "CLUT Pixel Storage Format", 51, 4},
      {"CSM", "CLUT Storage Mode", 55, 4},
      {"CSA", "CLUT Entry Offset", 56, 5},
      {"CLD", "CLUT Buffer Load Control", 61, 3}}},
    {0x3b,
     "TEXA",
     "Texture Alpha Value setting",
     3,
     {{"TA0", "'As' value referred to when A field is 0 in RGBA16 or RGB24 format", 0, 8},
      {"TA1", "'As' value referred to when A field is 1 in RGBA16 format", 32, 8},
      {"AEM", "Method of expanding Texture Alpha", 15, 1}}},
    {0x1c,
     "TEXCLUT",
     "CLUT Position specication",
     3,
     {{"CBW", "CLUT Buffer Width", 0, 6},
      {"COU", "CLUT Offset U", 6, 6},
      {"COV", "CLUT Offset V", 12, 10}}},
    {0x3f, "TEXFLUSH", "Texture Page Buffer disabling", 0, {{NULL, NULL, 0, 0}}},
    {0x53,
     "TRXDIR",
     "Activate Intra-Buffer Transmission",
     1,
     {{"XDIR", "Transmission direction", 0, 2}}},
    {0x51,
     "TRXPOS",
     "Intra-Buffer Transmission Area settings",
     5,
     {{"SSAX", "X Coord of Upper Left Source Rectangluar Area", 0, 11},
      {"SSAY", "Y Coord of Upper Left Source Rectangluar Area", 16, 11},
      {"DSAX", "X Coord of Upper Left Dest Rectangluar Area", 32, 11},
      {"DSAY", "Y Coord of Upper Left Dest Rectangluar Area", 48, 11},
      {"DIR", "Pixel Transmission Order", 59, 2}}},
    {0x52,
     "TRXREG",
     "Intra-Buffer Transmission Area settings",
     2,
     {{"RRW", "Width of Transmission Area", 0, 12},
      {"RRH", "Height of Transmission Area", 32, 12}}},
    {0x03,
     "UV",
     "Vertex Texture Coordinate setting",
     2,
     {{"U", "Texel Coordinate U", 0, 16}, {"V", "Texel Coordinate V", 16, 16}}},
    {0x18,
     "XYOFFSET_1",
     "Offset value settings",
     2,
     {{"OFX", "Offset X", 0, 16}, {"OFY", "Offset Y", 32, 16}}},
    {0x19,
     "XYOFFSET_2",
     "Offset value settings",
     2,
     {{"OFX", "Offset X", 0, 16}, {"OFY", "Offset Y", 32, 16}}},
    {0x05,
     "XYZ2",
     "Vertex Coordinate Value settings",
     3,
     {{"X", "X", 0, 16}, {"Y", "Y", 16, 16}, {"Z", "Z", 32, 32}}},
    {0x0d,
     "XYZ3",
     "Vertex Coordinate Value settings (ADC=1, no Kick)",
     3,
     {{"X", "X", 0, 16}, {"Y", "Y", 16, 16}, {"Z", "Z", 32, 32}}},
    {0x04,
     "XYZF2",
     "Vertex Coordinate Value settings",
     4,
     {{"X", "X", 0, 16}, {"Y", "Y", 16, 16}, {"Z", "Z", 32, 24}, {"F", "F", 56, 8}}},
    {0x0c,
     "XYZF3",
     "Vertex Coordinate Value settings (ADC=1, no Kick)",
     4,
     {{"X", "X", 0, 16}, {"Y", "Y", 16, 16}, {"Z", "Z", 32, 24}, {"F", "F", 56, 8}}},
    {0x4e,
     "ZBUF_1",
     "Z-Buffer settings",
     3,
     {{"ZBP", "Z-Buffer Base Pointer", 0, 8},
      {"PSM", "Pixel Storage Format", 24, 4},
      {"ZMSK", "Z Value Drawing Mask", 32, 1}}},
    {0x4f,
     "ZBUF_2",
     "Z-Buffer settings",
     3,
     {{"ZBP", "Z-Buffer Base Pointer", 0, 8},
      {"PSM", "Pixel Storage Format", 24, 4},
      {"ZMSK", "Z Value Drawing Mask", 32, 1}}},
    {0x8e,
     "BGCOLOR",
     "Background Colour setting",
     3,
     {{"R", "R", 0, 8}, {"G", "G", 8, 8}, {"B", "B", 16, 8}}},
    {0xc4,
     "BUSDIR",
     "Hist I/F Bus Switching",
     1,
     {{"DIR", "Interface Transmission Direction", 0, 1}}},
    {0xc0,
     "CSR",
     "System Status",
     6,
     {{"SIGNAL", "Signal Event Control", 0, 1},
      {"FINISH", "Finish Event Control", 1, 1},
      {"HSINT", "HSync Interrupt Control", 2, 1},
      {"VSINT", "VSync Interrupt Control", 3, 1},
      {"FLUSH", "Drawing Suspend and FIFO clear", 8, 1},
      {"RESET", "GS System Reset", 9, 1}}},
    {0x87,
     "DISPFB1",
     "Rectangular Area Read Output Circuit 1",
     5,
     {{"FBP", "Frame Buffer Base Pointer", 0, 9},
      {"FBW", "Frame Buffer Width", 9, 6},
      {"PSM", "Pixel Storage Format", 15, 5},
      {"DBX", "X Position in Rectangular Area in Buffer (pixels)", 32, 11},
      {"DBY", "Y Position in Rectangular Area in Buffer (pixels)", 43, 11}}},
    {0x89,
     "DISPFB2",
     "Rectangular Area Read Output Circuit 2",
     5,
     {{"FBP", "Frame Buffer Base Pointer", 0, 9},
      {"FBW", "Frame Buffer Width", 9, 6},
      {"PSM", "Pixel Storage Format", 15, 5},
      {"DBX", "X Position in Rectangular Area in Buffer (pixels)", 32, 11},
      {"DBY", "Y Position in Rectangular Area in Buffer (pixels)", 43, 11}}},
    {0x88,
     "DISPLAY1",
     "Rectangular Area Read Output Circuit 1",
     6,
     {{"DX", "Display Area X Position (VCK Units)", 0, 12},
      {"DY", "Display Area Y Position (Raster Units)", 12, 11},
      {"DW", "Display Area Width (VCK Units)", 32, 12},
      {"DH", "Display Area Height (Pixel Units)", 44, 11},
      {"MAGH", "Magnification in H direction", 23, 4},
      {"MAGV", "Magnification in V direction", 27, 2}}},
    {0x8a,
     "DISPLAY2",
     "Rectangular Area Read Output Circuit 2",
     6,
     {{"DX", "Display Area X Position (VCK Units)", 0, 12},
      {"DY", "Display Area Y Position (Raster Units)", 12, 11},
      {"DW", "Display Area Width (VCK Units)", 32, 12},
      {"DH", "Display Area Height (Pixel Units)", 44, 11},
      {"MAGH", "Magnification in H direction", 23, 4},
      {"MAGV", "Magnification in V direction", 27, 2}}},
    {0x8b,
     "EXTBUF",
     "Feedback Write Buffer setting",
     8,
     {{"EXBP", "Base Pointer of buffer where data is written", 0, 14},
      {"EXBW", "Width of buffer where data is written", 14, 6},
      {"FBIN", "Input Source Selection", 20, 2},
      {"WFFMD", "Interlace mode", 22, 1},
      {"EMODA", "Input Alpha Processing Method", 23, 2},
      {"EMODC", "Input Colour Processing Method", 25, 2},
      {"WDX", "X Coordinate in rectangular area where input image data is written", 32, 11},
      {"WDY", "Y Coordinate in rectangular area where input image data is written", 43, 11}}},
    {0x8c,
     "EXTDATA",
     "Feedback Write setting",
     6,
     {{"SX", "X Coordinate of rectangular area where input image is written", 0, 12},
      {"SY", "Y Coordinate of rectangular area where input image is written", 12, 11},
      {"SMPH", "H direction sampling rate (VCK Units)", 23, 4},
      {"SMPV", "V direction sampling rate", 27, 2},
      {"WW", "Rectangular Area Width", 32, 12},
      {"WH", "Rectangular Area Height", 44, 11}}},
    {0x8d,
     "EXTWRITE",
     "Feedback Write Function control",
     1,
     {{"WRITE", "Activation/Deactivation of write", 0, 1}}},
    {0xc1,
     "IMR",
     "Interrupt Mask control",
     4,
     {{"SIGMSK", "SIGNAL Event Interrupt Mask", 8, 1},
      {"FINISHMSK", "FINISH Event Interrupt Mask", 9, 1},
      {"HSMSK", "HSync Interrupt Mask", 10, 1},
      {"VSMSK", "VSync Interrupt Mask", 11, 1}}},
    {0x80,
     "PMODE",
     "PCRTC Mode setting",
     7,
     {{"EN1", "Read Circuit 1 enable", 0, 1},
      {"EN2", "Read Circuit 2 enable", 1, 1},
      {"CRTMD", "CRT Output Switching", 2, 3},
      {"MMOD", "Alpha Value Selection for Alpha Blending", 5, 1},
      {"AMOD", "OUT1 Alpha Output Selection", 6, 1},
      {"SLBG", "Alpha Blending Method Selection", 7, 1},
      {"ALP", "Fixed Alpha Buffer Value (0xff = 1.0)", 8, 8}}},
    {0xc8,
     "SIGLBLID",
     "Signal ID Value read",
     2,
     {{"SIGID", "ID Value set by SIGNAL Register", 0, 32},
      {"LABID", "ID Value set by LABEL Register", 32, 32}}},
    {0x82,
     "SMODE2",
     "Video Synchronisation mode settings",
     3,
     {{"INT", "Interlace Mode Setting", 0, 1},
      {"FFMD", "Field/Frame setting", 1, 1},
      {"DPMS", "VESA DPMS Mode Setting", 2, 2}}},
    {0xfd, "NOP", "(this isn't a real GS register)", 0, {{NULL, NULL, 0, 0}}},
    {0xfe, "RESERVED", "*RESERVED* - 0xb is in a REGS field in GIFTAG.", 0, {{NULL, NULL, 0, 0}}},
    {0xff, "NOP", "No operation", 0, {{NULL, NULL, 0, 0}}},
};

// Entries of g_aGsRegisters.
enum { GS_REGISTER_COUNT = sizeof(g_aGsRegisters) / sizeof(g_aGsRegisters[0]) };

// NTSC-U/C: 0x003b6000
// The Q value of the last ST written in PACKED mode, as float bits.
static unsigned int g_dwGifQ = 0;

// NTSC-U/C: 0x003b6004
// Non-zero when reports give field descriptions and raw words.
static int g_nGifVerboseAttrs = 1;

// NTSC-U/C: 0x003b6008
// Names of the attribute bits of PRIM. The ninth entry repeats what the binary reads past the
// eight names, the first entry of g_apszGifPrimAttrValues.
static const char *g_apszGifPrimAttrNames[PRIMITIVE_ATTRIBUTES + 1] = {
    "IIP", "TME", "FGE", "ABE", "AA1", "FST", "CTXT", "FIX", "Gouraud"};

// NTSC-U/C: 0x003b6028
// Meanings of the attribute bits of PRIM. The ninth entry repeats what the binary reads past the
// eight meanings, the first entry of g_apszGifRegNames.
static const char *g_apszGifPrimAttrValues[PRIMITIVE_ATTRIBUTES + 1] = {"Gouraud",
                                                                        "Textured",
                                                                        "Fog",
                                                                        "Alpha Blending",
                                                                        "AntiAliasing",
                                                                        "UV is used",
                                                                        "Context 2",
                                                                        "Fragment control fixed",
                                                                        "PRIM"};

// NTSC-U/C: 0x003b6048
// Names of the REGS values of a GIF tag.
static const char *g_apszGifRegNames[GIF_REG_COUNT] = {"PRIM",
                                                       "RGBAQ",
                                                       "ST",
                                                       "UV",
                                                       "XYZF2",
                                                       "XYZ2",
                                                       "TEX0_1",
                                                       "TEX0_2",
                                                       "CLAMP_1",
                                                       "CLAMP_2",
                                                       "FOG",
                                                       DEBUG_ANSI_ALERT
                                                       "*RESERVED*" DEBUG_ANSI_RESET,
                                                       "XYZF3",
                                                       "XYZ3",
                                                       "A+D",
                                                       "NOP"};

// NTSC-U/C: 0x003b6088
// Names of the FLG values of a GIF tag.
static const char *g_apszGifFlagNames[] = {"PACKED", "REGLIST", "IMAGE  ", "Disable"};

// Returns the float with the bits of a word.
static inline float wordToFloat(unsigned int word) {
    union {
        unsigned int word;
        float value;
    } bits;
    bits.word = word;
    return bits.value;
}

// NTSC-U/C: 0x002a26b8, PAL: 0x002ac628
// Returns a fixed-point field as a float.
static float fixedToFloat(unsigned int value, int isSigned, int integerBits, int fractionBits) {
    float result = (float)(value & ((1u << (integerBits + fractionBits)) - 1));
    result /= (float)(1 << fractionBits);
    if (isSigned && ((value >> (integerBits + fractionBits)) & 1)) {
        result = -result;
    }
    return result;
}

int GIF_isTagDone(void) {
    return g_gifProgress.nloop == 0;
}

// NTSC-U/C: 0x002a2750, PAL: 0x002ac6c0
// Reports a message of the decoder.
static int report(int number, int severity, const char *text) {
    return DEBUG_message(number, severity, text);
}

// NTSC-U/C: 0x002a2770, PAL: 0x002ac6e0
// Describes the primitive type of PRIM when withType is set, and each attribute bit that is set.
static const char *describePrimitive(unsigned int prim, int withType, int withMeanings) {
    g_szGifPrimDescription[0] = '\0';
    if (withType) {
        strcat(g_szGifPrimDescription, DEBUG_ANSI_GREEN);
        switch (prim & 7) {
        case 0:
            strcat(g_szGifPrimDescription, "Point");
            break;
        case 1:
            strcat(g_szGifPrimDescription, "Line");
            break;
        case 2:
            strcat(g_szGifPrimDescription, "Line Strip");
            break;
        case 3:
            strcat(g_szGifPrimDescription, "Tri");
            break;
        case 4:
            strcat(g_szGifPrimDescription, "Tri Strip");
            break;
        case 5:
            strcat(g_szGifPrimDescription, "Tri Fan");
            break;
        case 6:
            strcat(g_szGifPrimDescription, "Sprite");
            break;
        case 7:
            strcat(g_szGifPrimDescription, DEBUG_ANSI_ALERT "*ILLEGAL*" DEBUG_ANSI_RESET);
            break;
        }
        strcat(g_szGifPrimDescription, DEBUG_ANSI_RESET ": ");
    }
    const unsigned int attributes = prim >> 3;
    // Yes, the binary tests nine attribute bits against eight names.
    for (int i = 0; i < PRIMITIVE_ATTRIBUTES + 1; ++i) {
        if (attributes & (1 << i)) {
            if (withMeanings) {
                DEBUG_addField(DEBUG_FIELD_INLINE,
                               g_szGifPrimDescription,
                               DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET "(" DEBUG_ANSI_CYAN
                                                "%s" DEBUG_ANSI_RESET ")",
                               g_apszGifPrimAttrNames[i],
                               g_apszGifPrimAttrValues[i]);
            } else {
                DEBUG_addField(DEBUG_FIELD_INLINE,
                               g_szGifPrimDescription,
                               DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET,
                               g_apszGifPrimAttrNames[i]);
            }
        }
    }
    return g_szGifPrimDescription;
}

// NTSC-U/C: 0x002a2968, PAL: 0x002ac8d8
// Returns the name of a pixel storage format, or null for an unknown one.
static const char *pixelFormatName(unsigned int psm) {
    switch (psm) {
    case PSMCT32:
        return "PSMCT32";
    case PSMCT24:
        return "PSMCT24";
    case PSMCT16:
        return "PSMCT16";
    case PSMCT16S:
        return "PCMCT16S";
    case PS_GPU24:
        return "PS-GPU24";
    case PSMT8:
        return "PSMT8";
    case PSMT4:
        return "PSMT4";
    case PSMT8H:
        return "PSMT8H";
    case PSMT4HL:
        return "PSMT4HL";
    case PSMT4HH:
        return "PSMT4HH";
    case PSMZ32:
        return "PSMZ32";
    case PSMZ24:
        return "PSMZ24";
    case PSMZ16:
        return "PSMZ16";
    case PSMZ16S:
        return "PSMZ16S";
    default:
        return NULL;
    }
}

// NTSC-U/C: 0x002a2a40, PAL: 0x002ac9b0
// Returns bits of a 64-bit value stored as two words, low word first.
static unsigned int extractBits(const unsigned int *value, int shift, int bits) {
    unsigned int result;
    if (shift < 32) {
        if (shift + bits < 33) {
            result = value[0] >> shift;
        } else {
            result =
                ((value[0] >> shift) & ((1u << (32 - shift)) - 1)) | (value[1] << (32 - shift));
        }
    } else {
        result = value[1] >> (shift - 32);
    }
    if (bits < 32) {
        result &= (1u << bits) - 1;
    }
    return result;
}

// Field formats the register reports share.
#define FIELD_HEX2                                                                                 \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET
#define FIELD_HEX4                                                                                 \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET
#define FIELD_HEX8                                                                                 \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET
#define FIELD_HEX                                                                                  \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%x" DEBUG_ANSI_RESET
#define FIELD_DEC DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET
#define FIELD_TEXT DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET
#define FIELD_NAMED_SUFFIX "(" DEBUG_ANSI_CYAN "%s" DEBUG_ANSI_RESET ")"
#define FIELD_HEX_NAMED FIELD_HEX FIELD_NAMED_SUFFIX
#define FIELD_HEX2_NAMED FIELD_HEX2 FIELD_NAMED_SUFFIX
#define FIELD_DEC_NAMED FIELD_DEC FIELD_NAMED_SUFFIX
#define FIELD_NOTE "(" DEBUG_ANSI_CYAN "%s" DEBUG_ANSI_RESET ")"
#define FIELD_CSM "%s=" DEBUG_ANSI_GREEN "CSM%d" DEBUG_ANSI_RESET
#define FIELD_CSA                                                                                  \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET          \
                     " [CSA*16=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET "]"
#define FIELD_CBP                                                                                  \
    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET          \
                     " [CBP*64=" DEBUG_ANSI_GREEN "0x%06x" DEBUG_ANSI_RESET "]"
#define FIELD_BASE_2048                                                                            \
    "%s=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET " [*2048=" DEBUG_ANSI_GREEN                   \
    "0x%06x" DEBUG_ANSI_RESET "]"
#define FIELD_PAIR                                                                                 \
    "=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN                          \
    "0x%04x" DEBUG_ANSI_RESET ")"
#define FIELD_FLOAT_PAIR                                                                           \
    DEBUG_ANSI_GREEN "%f" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "%f" DEBUG_ANSI_RESET
#define FIELD_FLOAT_PAIR_TIGHT                                                                     \
    DEBUG_ANSI_GREEN "%f" DEBUG_ANSI_RESET "," DEBUG_ANSI_GREEN "%f" DEBUG_ANSI_RESET
#define FIELD_NO_KICK DEBUG_ANSI_BOLD "(No Kick) " DEBUG_ANSI_RESET
#define UNKNOWN_FORMAT DEBUG_ANSI_ALERT "*UNKNOWN*" DEBUG_ANSI_RESET

// NTSC-U/C: 0x002a2ac0, PAL: 0x002aca30
// Reports a write of a GS register and checks the values of its fields.
static int describeRegister(int id, const unsigned int *value, char *line) {
    unsigned int values[MAX_REGISTER_FIELDS];
    char labels[MAX_REGISTER_FIELDS][FIELD_LABEL_SIZE];
    int result = 0;
    int index;
    for (index = 0; index < GS_REGISTER_COUNT; ++index) {
        if (g_aGsRegisters[index].id == id) {
            break;
        }
    }
    if (index == GS_REGISTER_COUNT) {
        sprintf(g_szDebugScratch,
                DEBUG_ANSI_ALERT "Invalid GS register encountered : %02x" DEBUG_ANSI_RESET,
                id);
        return report(MESSAGE_UNKNOWN_REGISTER, DEBUG_ERROR, g_szDebugScratch);
    }
    const struct gs_register *reg = &g_aGsRegisters[index];
    DEBUG_addField(DEBUG_FIELD_COLUMN,
                   line,
                   DEBUG_ANSI_ON_MAGENTA "GS %-10s" DEBUG_ANSI_RESET " [" DEBUG_ANSI_CYAN
                                         "%08x" DEBUG_ANSI_RESET "]",
                   reg->name,
                   g_pGifData);
    if (g_nGifVerboseAttrs) {
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "[" DEBUG_ANSI_MAGENTA "%08x.%08x" DEBUG_ANSI_RESET "]",
                       value[1],
                       value[0]);
    }
    DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, "Reg", reg->id);
    if (g_nGifVerboseAttrs) {
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, DEBUG_ANSI_CYAN "%s" DEBUG_ANSI_RESET, reg->description);
    }
    for (int field = 0; field < reg->fieldCount; ++field) {
        values[field] = extractBits(value, reg->fields[field].shift, reg->fields[field].bits);
        if (g_nGifVerboseAttrs) {
            sprintf(labels[field],
                    DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET ":" DEBUG_ANSI_CYAN
                                     "%s" DEBUG_ANSI_RESET,
                    reg->fields[field].name,
                    reg->fields[field].description);
        } else {
            sprintf(labels[field], DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET, reg->fields[field].name);
        }
    }
    switch (reg->id) {
    case GS_PRIM:
    case GS_PRMODE:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "%s=%s",
                       reg->id != GS_PRIM ? "PRMODE" : "PRIM",
                       describePrimitive(value[0], reg->id == GS_PRIM, g_nGifVerboseAttrs));
        break;
    case GS_RGBAQ: {
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "RGBA=(" DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET
                       ", " DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1],
                       values[2],
                       values[3]);
        const float q = wordToFloat(value[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=(" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET
                       ", " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1],
                       values[2],
                       values[3]);
        sprintf(g_szDebugScratch, "%f", (double)q);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "Q=" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET " [" DEBUG_ANSI_GREEN
                       "0x%08x" DEBUG_ANSI_RESET "]",
                       g_szDebugScratch,
                       value[1]);
        break;
    }
    case GS_ST: {
        const float s = wordToFloat(values[0]);
        const float t = wordToFloat(values[1]);
        sprintf(g_szDebugScratch, FIELD_FLOAT_PAIR, (double)s, (double)t);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, "ST=(%s)", g_szDebugScratch);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=[" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%08x" DEBUG_ANSI_RESET "]",
                       value[0],
                       value[1]);
        break;
    }
    case GS_UV: {
        const float u = fixedToFloat(values[0], 0, 12, 4);
        const float v = fixedToFloat(values[1], 0, 12, 4);
        sprintf(g_szDebugScratch, FIELD_FLOAT_PAIR_TIGHT, (double)u, (double)v);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, "UV=(%s)", g_szDebugScratch);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET "," DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1]);
        break;
    }
    case GS_XYZF2:
    case GS_XYZF3: {
        const float x = fixedToFloat(values[0], 0, 12, 4);
        const float y = fixedToFloat(values[1], 0, 12, 4);
        sprintf(g_szDebugScratch, FIELD_FLOAT_PAIR_TIGHT, (double)x, (double)y);
        if (reg->id == GS_XYZF3) {
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_NO_KICK);
        }
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "XYZF=(%s, " DEBUG_ANSI_GREEN "0x%x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%x" DEBUG_ANSI_RESET ")",
                       g_szDebugScratch,
                       values[2],
                       values[3]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "0x%06x" DEBUG_ANSI_RESET
                       ", " DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1],
                       values[2],
                       values[3]);
        break;
    }
    case GS_XYZ2:
    case GS_XYZ3: {
        const float x = fixedToFloat(values[0], 0, 12, 4);
        const float y = fixedToFloat(values[1], 0, 12, 4);
        sprintf(g_szDebugScratch, FIELD_FLOAT_PAIR_TIGHT, (double)x, (double)y);
        if (reg->id == GS_XYZ3) {
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_NO_KICK);
        }
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "XYZ=(%s, " DEBUG_ANSI_GREEN "0x%x" DEBUG_ANSI_RESET ")",
                       g_szDebugScratch,
                       values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET
                       ")",
                       values[0],
                       values[1],
                       values[2]);
        break;
    }
    case GS_TEX0_1:
    case GS_TEX0_2: {
        const char *functionNames[] = {"MODULATE", "DECAL", "HIGHLIGHT", "HIGHLIGHT2"};
        const char *loadNames[] = {"Temp buff not changed",
                                   "Load to CSA position",
                                   "Load to CSA position, CBP0<-CBP",
                                   "Load to CSA position, CBP1<-CBP",
                                   "If CBP0<>CBP, then load and CBP0<-CBP",
                                   "If CBP1<>CBP, then load and CBP1<-CBP",
                                   "*RESERVED*",
                                   "*RESERVED*"};
        const char *clutFormat = pixelFormatName(values[8]);
        const char *format = pixelFormatName(values[2]);
        if (format == NULL) {
            format = UNKNOWN_FORMAT;
        }
        if (clutFormat == NULL) {
            clutFormat = UNKNOWN_FORMAT;
        }
        // Yes, the binary tests for TEX2 in a case that only TEX0 uses.
        if (!(reg->id & GS_TEX2_BIT)) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "0x%04x" DEBUG_ANSI_RESET " [TBP*64=" DEBUG_ANSI_GREEN
                                            "0x%06x" DEBUG_ANSI_RESET "]",
                           labels[0],
                           values[0],
                           values[0] << 6);
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[1], values[1]);
        }
        DEBUG_addField(DEBUG_FIELD_LINE, line, FIELD_HEX_NAMED, labels[2], values[2], format);
        if (!(reg->id & GS_TEX2_BIT)) {
            DEBUG_addField(
                DEBUG_FIELD_LINE, line, FIELD_TEXT, labels[5], values[5] ? "RGBA" : "RGB+TEXA");
            DEBUG_addField(
                DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[6], functionNames[values[6]]);
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "0x%x" DEBUG_ANSI_RESET " (=" DEBUG_ANSI_GREEN
                                            "%d" DEBUG_ANSI_RESET " txls)",
                           labels[3],
                           values[3],
                           1 << values[3]);
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "0x%x" DEBUG_ANSI_RESET " (=" DEBUG_ANSI_GREEN
                                            "%d" DEBUG_ANSI_RESET " txls)",
                           labels[4],
                           values[4],
                           1 << values[4]);
        }
        DEBUG_addField(DEBUG_FIELD_LINE, line, FIELD_CBP, labels[7], values[7], values[7] << 6);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[8], values[8], clutFormat);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_CSM, labels[9], values[9]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_CSA, labels[10], values[10], values[10] << 4);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_HEX_NAMED,
                       labels[11],
                       values[11],
                       loadNames[values[11]]);
        if (values[11] - CLD_FIRST_RESERVED < 2) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid load control parameter");
        }
        // Yes, the binary tests CSM and reports CSA.
        if (values[9] == 1) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "CSM is 1, but CSA is not zero!!");
        }
        if (pixelFormatName(values[2]) == NULL || values[2] == PS_GPU24) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid pixel storage format");
        }
        if (values[8] != PSMCT32 && values[8] != PSMCT16 && values[8] != PSMCT16S) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid clut storage format");
        }
        break;
    }
    case GS_CLAMP_1:
    case GS_CLAMP_2: {
        const char *wrapNames[] = {"REPEAT", "CLAMP", "REGION_CLAMP", "REGION_REPEAT"};
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[0], values[0], wrapNames[values[0]]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[1], values[1], wrapNames[values[1]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[2], values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[3], values[3]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[4], values[4]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[5], values[5]);
        break;
    }
    case GS_FOG:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[0], values[0]);
        break;
    case GS_TEX1_1:
    case GS_TEX1_2: {
        const char *filterNames[] = {"NEAREST",
                                     "LINEAR",
                                     "NEAREST_MIPMAP_NEAREST",
                                     "NEAREST_MIPMAP_LINEAR",
                                     "LINEAR_MIPMAP_NEAREST",
                                     "LINEAR_MIPMAP_LINEAR",
                                     DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET,
                                     DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET};
        const char *baseNames[] = {"Value specified by TBP1 & TBP2 used",
                                   "Base address of TBP1-3 automatically set"};
        const float k = fixedToFloat(values[6], 1, 7, 4);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_TEXT,
                       labels[0],
                       values[0] ? "K" : "(log2(1/|Q|)<<L)+K");
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[1], values[1]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[2], values[2] ? "LINEAR" : "NEAREST");
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[3], filterNames[values[3]]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_DEC_NAMED, labels[4], values[4], baseNames[values[4]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX, labels[5], values[5]);
        sprintf(g_szDebugScratch, DEBUG_ANSI_GREEN "%f" DEBUG_ANSI_RESET, (double)k);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[6], g_szDebugScratch);
        if (values[1] >= MXL_FIRST_RESERVED) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "MXL is > 6");
        }
        if (values[3] >= MMIN_FIRST_RESERVED) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "MMIN is set to a reserved value");
        }
        break;
    }
    case GS_TEX2_1:
    case GS_TEX2_2: {
        const char *loadNames[] = {"Temp buff not changed",
                                   "Load to CSA position",
                                   "Load to CSA position, CBP0<-CBP",
                                   "Load to CSA position, CBP1<-CBP",
                                   "If CBP0<>CBP, then load and CBP0<-CBP",
                                   "If CBP1<>CBP, then load and CBP1<-CBP",
                                   DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET,
                                   DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET};
        const char *clutFormat = pixelFormatName(values[2]);
        const char *format = pixelFormatName(values[0]);
        if (format == NULL) {
            format = UNKNOWN_FORMAT;
        }
        if (clutFormat == NULL) {
            clutFormat = UNKNOWN_FORMAT;
        }
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[0], values[0], format);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_CBP, labels[1], values[1], values[1] << 6);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[2], values[2], clutFormat);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_CSM, labels[3], values[3]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_CSA, labels[4], values[4], values[4] << 4);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_HEX_NAMED, labels[5], values[5], loadNames[values[5]]);
        if (values[5] - CLD_FIRST_RESERVED < 2) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid load control parameter");
        }
        if (values[4] == 1) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "CSM is 1, but CSA is not zero!!");
        }
        if (pixelFormatName(values[0]) == NULL || values[0] == PS_GPU24) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid pixel storage format");
        }
        if (values[2] != PSMCT32 && values[2] != PSMCT16 && values[2] != PSMCT16S) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid clut storage format");
        }
        break;
    }
    case GS_XYOFFSET_1:
    case GS_XYOFFSET_2: {
        const float x = fixedToFloat(values[0], 0, 12, 4);
        const float y = fixedToFloat(values[1], 0, 12, 4);
        sprintf(g_szDebugScratch, FIELD_FLOAT_PAIR, (double)x, (double)y);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, "OFX/Y=(%s)", g_szDebugScratch);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_PAIR, values[0], values[1]);
        break;
    }
    case GS_PRMODECONT:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[0], values[0]);
        if (g_nGifVerboseAttrs) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_CYAN "%s used" DEBUG_ANSI_RESET,
                           values[0] ? "Enabled - PRIM" : "Disabled - PRMODE");
        }
        break;
    case GS_TEXCLUT:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET " [*64=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET "]",
                       labels[0],
                       values[0],
                       values[0] << 6);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET " [*16=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET "]",
                       labels[1],
                       values[1],
                       values[1] << 4);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[2], values[2]);
        break;
    case GS_SCANMSK: {
        const char *maskNames[] = {"Not masked",
                                   "*RESERVED*",
                                   "Drawing with even Y coord is prohibited",
                                   "Drawing with odd Y coord is prohibited"};
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[0], values[0]);
        if (g_nGifVerboseAttrs) {
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_NOTE, maskNames[values[0]]);
        }
        if (values[0] == 1) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid Field Mask value");
        }
        break;
    }
    case GS_MIPTBP1_1:
    case GS_MIPTBP1_2:
    case GS_MIPTBP2_1:
    case GS_MIPTBP2_2: {
        int level = (unsigned char)(reg->id - GS_MIPTBP2_1) > 1 ? 1 : 4;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBP%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBW%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[1]);
        ++level;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBP%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBW%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[3]);
        ++level;
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBP%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[4]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "TBW%d=" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET,
                       level,
                       values[5]);
        break;
    }
    case GS_TEXA:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[2], values[2]);
        if (g_nGifVerboseAttrs) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           FIELD_NOTE,
                           values[2] ? "A=0 when R=G=B=0" : "Processed normally when R=G=B=0");
        }
        break;
    case GS_FOGCOL:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "RGB=(" DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET
                       ")",
                       values[0],
                       values[1],
                       values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "=(" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1],
                       values[2]);
        break;
    case GS_SCISSOR_1:
    case GS_SCISSOR_2:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[2], values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[3], values[3]);
        break;
    case GS_ALPHA_1:
    case GS_ALPHA_2: {
        const char *colourNames[] = {
            "Cs", "Cd", "0", DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET};
        const char *alphaNames[] = {
            "As", "Ad", "FIX", DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET};
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[0], colourNames[values[0]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[1], colourNames[values[1]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[2], alphaNames[values[2]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[3], colourNames[values[3]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[4], values[4]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "Cv=(%s-%s)*%s>>7+%s",
                       colourNames[values[0]],
                       colourNames[values[1]],
                       alphaNames[values[2]],
                       colourNames[values[3]]);
        if (values[0] == 3 || values[1] == 3 || values[2] == 3 || values[3] == 3) {
            result =
                report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Reserved value used in alpha formula!");
        }
        break;
    }
    case GS_DIMX:
        for (unsigned int row = 0; row < DIMX_ROWS; ++row) {
            for (unsigned int column = 0; column < DIMX_ROWS; ++column) {
                int word = (int)(row < 2 ? value[0] : value[1]);
                if (row & 1) {
                    word >>= 16;
                }
                int element = (word >> (column * DIMX_ELEMENT_SIZE)) & DIMX_ELEMENT_MASK;
                if (element & DIMX_ELEMENT_SIGN) {
                    element |= ~DIMX_ELEMENT_MASK;
                }
                sprintf(g_szDebugScratch,
                        DEBUG_ANSI_GREEN "%2.3f" DEBUG_ANSI_RESET,
                        (double)(float)element);
                DEBUG_addField(DEBUG_FIELD_INLINE, line, g_szDebugScratch);
            }
        }
        break;
    case GS_DTHE:
        // Yes, the binary reads a field of a register whose table entry lists none.
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[0], values[0] ? "ON" : "OFF");
        break;
    case GS_COLCLAMP:
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[0], values[0] ? "CLAMP" : "MASK");
        break;
    case GS_TEST_1:
    case GS_TEST_2: {
        const char *alphaTestNames[] = {"NEVER: all fail",
                                        "ALWAYS: all pass",
                                        "LESS: A < AREF pass",
                                        "LEQUAL: A <= AREF pass",
                                        "EQUAL: A == AREF pass",
                                        "GEQUAL: A >= AREF pass",
                                        "GREATER: A > AREF pass",
                                        "NOTEQUAL: A <> AREF pass"};
        const char *failNames[] = {"KEEP: FB & Z-Buff NOT updated",
                                   "FB_ONLY: Only F-Buff updated",
                                   "ZB_ONLY: Only Z-Buff updated",
                                   "RGB_ONLY: Only F-Buff RGB updated"};
        const char *depthTestNames[] = {"NEVER: All fail",
                                        "ALWAYS: all pass",
                                        "GEQUAL: Z >= Z-Buff pass",
                                        "GREATER: Z > Z-Buff pass"};
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[0], values[0] ? "ON" : "OFF");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[1],
                       values[1],
                       alphaTestNames[values[1]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[2], values[2]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_DEC_NAMED, labels[3], values[3], failNames[values[3]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[4], values[4] ? "ON" : "OFF");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "%d" DEBUG_ANSI_RESET " (" DEBUG_ANSI_CYAN
                                        "pixels w/dest A=%d pass" DEBUG_ANSI_RESET ")",
                       labels[5],
                       values[5],
                       values[5]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[6], values[6] ? "ON" : "OFF");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[7],
                       values[7],
                       depthTestNames[values[7]]);
        break;
    }
    case GS_PABE:
    case GS_FBA_1:
    case GS_FBA_2:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[0], values[0]);
        break;
    case GS_FRAME_1:
    case GS_FRAME_2: {
        const char *format = pixelFormatName(values[2]);
        if (format == NULL) {
            format = UNKNOWN_FORMAT;
        }
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%04x" DEBUG_ANSI_RESET " [*2048=" DEBUG_ANSI_GREEN
                                        "0x%08x" DEBUG_ANSI_RESET "]",
                       labels[0],
                       values[0],
                       values[0] << 11);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                        "0x%02x" DEBUG_ANSI_RESET " (=" DEBUG_ANSI_GREEN
                                        "%d" DEBUG_ANSI_RESET " pixels)",
                       labels[1],
                       values[1],
                       values[1] << 6);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC_NAMED, labels[2], values[2], format);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX8, labels[3], values[3]);
        if (values[2] > PSMCT16 && values[2] != PSM_UNNAMED && values[2] != PSMZ32 &&
            values[2] != PSMZ24 && values[2] != PSMZ16 && values[2] != PSMZ16S) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Unknown pixel storage format");
        }
        break;
    }
    case GS_ZBUF_1:
    case GS_ZBUF_2: {
        const char *format = pixelFormatName(values[1] | PSM_Z_BITS);
        if (format == NULL) {
            format = UNKNOWN_FORMAT;
        }
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_BASE_2048, labels[0], values[0], values[0] << 11);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2_NAMED, labels[1], values[1], format);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[2], values[2]);
        if (g_nGifVerboseAttrs) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           FIELD_NOTE,
                           values[2] ? "ZBuffer is NOT updated" : "ZBuffer is updated");
        }
        if (pixelFormatName(values[1] | PSM_Z_BITS) == NULL) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid Z pixel storage format");
        }
        break;
    }
    case GS_BITBLTBUF: {
        const char *source = pixelFormatName(values[2]);
        const char *destination = pixelFormatName(values[5]);
        if (source == NULL) {
            source = UNKNOWN_FORMAT;
        }
        if (destination == NULL) {
            destination = UNKNOWN_FORMAT;
        }
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2_NAMED, labels[2], values[2], source);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[3], values[3]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[4], values[4]);
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_HEX2_NAMED, labels[5], values[5], destination);
        if (pixelFormatName(values[2]) == NULL || values[2] == PS_GPU24) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid source pixel storage format");
        }
        if (pixelFormatName(values[5]) == NULL || values[5] == PS_GPU24) {
            result |= report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid dest pixel storage format");
        }
        break;
    }
    case GS_TRXPOS: {
        const char *orderNames[] = {"Upper Left -> Lower Right",
                                    "Lower Left -> Upper Right",
                                    "Upper Right -> Lower Left",
                                    "Lower Right -> Upper Left"};
        // Yes, the binary reports the first field's value for each of the four coordinates.
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[1], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[2], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[3], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, labels[4], orderNames[values[4]]);
        break;
    }
    case GS_TRXREG:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[1], values[1]);
        break;
    case GS_TRXDIR: {
        const char *directionNames[] = {
            "Host -> Local", "Local -> Host", "Local -> Local", "Transmission disabled"};
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[0],
                       values[0],
                       directionNames[values[0]]);
        break;
    }
    case GS_HWREG:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "DATA=" DEBUG_ANSI_GREEN "%08x.%08x" DEBUG_ANSI_RESET,
                       value[1],
                       value[0]);
        break;
    case GS_PMODE:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[0],
                       values[0],
                       values[0] ? "ON" : "OFF");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[1],
                       values[1],
                       values[1] ? "ON" : "OFF");
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX, labels[2], values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[3],
                       values[3],
                       values[3] ? "ALP Register value" : "Alpha Value of Rd Circuit1");
        // Yes, the binary gives the same meaning for both values of AMOD.
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[4],
                       values[4],
                       "Alpha Value of Read Circuit 2");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[5],
                       values[5],
                       values[5] ? "Blended with backgrnd col" :
                                   "Blended with output of Rd Circuit 2");
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[6], values[6]);
        if (values[2] != 0) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid CRTMD value (should be 0)");
        }
        break;
    case GS_SMODE2: {
        const char *powerNames[] = {"ON", "STAND-BY", "SUSPEND", "OFF"};
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[0],
                       values[0],
                       values[0] ? "INTERLACE" : "NON-INTERLACE");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[1],
                       values[1],
                       values[1] ? "FRAME" : "FIELD");
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_DEC_NAMED, labels[2], values[2], powerNames[values[2]]);
        break;
    }
    case GS_DISPFB1:
    case GS_DISPFB2: {
        const char *format = pixelFormatName(values[2]);
        if (format == NULL) {
            format = UNKNOWN_FORMAT;
        }
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_BASE_2048, labels[0], values[0], values[0] << 11);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2_NAMED, labels[2], values[2], format);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "DBX/Y=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ") = (" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET
                       ", " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ")",
                       values[3],
                       values[4],
                       values[3],
                       values[4]);
        // Yes, the binary compares the identifier with that of TEX0_2 instead of a DISPFB register.
        if (values[2] > PSMCT16 && values[2] != PSMCT16S &&
            (reg->id != GS_TEX0_2 || values[2] != PS_GPU24)) {
            result = report(MESSAGE_BAD_FIELD, DEBUG_ERROR, "Invalid dest pixel storage format");
        }
        break;
    }
    case GS_DISPLAY1:
    case GS_DISPLAY2:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "DX/Y=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "DW/H=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[2],
                       values[3]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[4], values[4]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[5], values[5]);
        break;
    case GS_EXTBUF: {
        const char *alphaNames[] = {"Input alpha written as is",
                                    "Value from RGB->LumY written",
                                    "Value from RGB->LumY/2 written",
                                    "Always 0"};
        const char *colourNames[] = {"Input alpha written as is",
                                     "Value from RGB->LumY written to RGB",
                                     "Value from RGB->YCbCr written",
                                     "Input Alpha written to RGB resp."};
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX4, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX2, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[2],
                       values[2],
                       values[2] ? "OUT2" : "OUT1");
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[3],
                       values[3],
                       values[3] ? "FRAME" : "FIELD");
        DEBUG_addField(
            DEBUG_FIELD_INLINE, line, FIELD_DEC_NAMED, labels[4], values[4], alphaNames[values[4]]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[5],
                       values[5],
                       colourNames[values[5]]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "WDX/Y=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[6],
                       values[7]);
        break;
    }
    case GS_EXTDATA:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "SX/Y=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "WW/H=(" DEBUG_ANSI_GREEN "0x%04x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%04x" DEBUG_ANSI_RESET ")",
                       values[4],
                       values[5]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX, labels[2], values[2]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX, labels[3], values[3]);
        break;
    case GS_EXTWRITE:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[0],
                       values[0],
                       values[0] ? "Write to mem is started from next frame" :
                                   "Write to mem is completed in the current frame");
        break;
    case GS_BGCOLOR:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "RGB = (" DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "0x%02x" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "0x%02x" DEBUG_ANSI_RESET
                       ") = (" DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN
                       "%d" DEBUG_ANSI_RESET ", " DEBUG_ANSI_GREEN "%d" DEBUG_ANSI_RESET ")",
                       values[0],
                       values[1],
                       values[2],
                       values[0],
                       values[1],
                       values[2]);
        break;
    case GS_CSR:
        for (int field = 0; field < CSR_FIELDS; ++field) {
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, labels[field], values[field]);
        }
        break;
    case GS_IMR: {
        const char *maskNames[] = {"Not masked", "Masked"};
        // Yes, the binary passes one argument for the two conversions of each field.
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, maskNames[values[0]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, maskNames[values[1]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, maskNames[values[2]]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, maskNames[values[3]]);
        break;
    }
    case GS_BUSDIR:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       FIELD_DEC_NAMED,
                       labels[0],
                       values[0],
                       values[0] ? "Local -> Host" : "Host->Local");
        break;
    case GS_SIGNAL:
    case GS_LABEL:
    case GS_SIGLBLID:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX8, labels[0], values[0]);
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_HEX8, labels[1], values[1]);
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "(ID & IDMSK)=" DEBUG_ANSI_GREEN "0x%08x" DEBUG_ANSI_RESET,
                       values[0] & values[1]);
        break;
    case GS_SKIPPED:
        DEBUG_addField(DEBUG_FIELD_INLINE, line, "(This field of data was skipped by the GIF)");
        break;
    case GS_RESERVED:
        DEBUG_addField(DEBUG_FIELD_INLINE,
                       line,
                       "*GIFTAG* REGS field = 0xb (" DEBUG_ANSI_ALERT "*RESERVED*" DEBUG_ANSI_RESET
                       "). Skipping this field.");
        break;
    case GS_TEXFLUSH:
    case GS_FINISH:
    case GS_NOP:
    case GS_NO_OPERATION:
        break;
    default:
        printf("Logic error, this case (0x%x) in disassembleRegister() should be caught.\n",
               reg->id);
        DEBUG_ABORT("Invalid GS register\n");
        break;
    }
    return result;
}

// NTSC-U/C: 0x002a4e30, PAL: 0x002aeda0
// Returns the GS register a REGS value writes. In PACKED mode the register of A+D comes from the
// data, and XYZF2 and XYZ2 become XYZF3 and XYZ3 when ADC is set.
static int resolveRegister(int reg, const unsigned int *data, int packed) {
    if (reg < GIF_REG_RESERVED || reg == GIF_REG_XYZF3 || reg == GIF_REG_XYZ3) {
        if (packed && (unsigned int)(reg - GIF_REG_XYZF2) < 2 && (data[3] & GIF_PACKED_ADC)) {
            return reg + GIF_REG_NO_KICK_OFFSET;
        }
        return reg;
    }
    if (reg == GIF_REG_A_D) {
        if (packed) {
            return data[2] & GIF_A_D_REGISTER_MASK;
        }
        return GS_NO_OPERATION;
    }
    return reg != GIF_REG_NOP ? GIF_REG_RESERVED : GS_NO_OPERATION;
}

// NTSC-U/C: 0x002a4eb8, PAL: 0x002aee28
// Reports one register of REGLIST data.
static int describeListRegister(int reg, const unsigned int *data, char *line) {
    int result = 0;
    if (reg == GIF_REG_A_D) {
        result = report(MESSAGE_REGLIST_A_D,
                        DEBUG_WARNING,
                        "REGLIST mode GIF tag has a A+D setting in the REGS field. Treating as a "
                        "NOP.");
    }
    const int id = resolveRegister(reg, data, 0);
    if (reg == GIF_REG_RESERVED) {
        result |= report(MESSAGE_RESERVED_REGISTER,
                         DEBUG_ERROR,
                         "Encountered *Reserved* (0xb) in GIFTAG REGS field. Treating as a NOP.");
    }
    return result | describeRegister(id, data, line);
}

// NTSC-U/C: 0x002a4f70, PAL: 0x002aeee0
// Reports one register of PACKED data, unpacking the quadword into the register's 64 bits.
static int describePackedRegister(int reg, const unsigned int *data, char *line) {
    const int id = resolveRegister(reg, data, 1);
    unsigned int packed[2];
    int result = 0;
    switch (reg) {
    case 1: // RGBAQ.
        packed[0] = 0;
        for (int i = 0; i < 4; ++i) {
            packed[0] |= (data[i] & 0xff) << (i * 8);
        }
        packed[1] = g_dwGifQ;
        result = describeRegister(id, packed, line);
        break;
    case 2: { // ST.
        g_dwGifQ = data[2];
        const int length = sprintf(line,
                                   "  STQ encountered - setting internal GIF Q to " DEBUG_ANSI_GREEN
                                   "%f" DEBUG_ANSI_RESET "\n",
                                   (double)wordToFloat(data[2]));
        result = describeRegister(id, data, &line[length]);
        break;
    }
    case 3: // UV.
        packed[1] = 0;
        packed[0] = (data[0] & 0xffff) | ((data[1] & 0xffff) << 16);
        result = describeRegister(id, packed, line);
        break;
    case GIF_REG_XYZF2:
    case GIF_REG_XYZ2:
        packed[0] = (data[0] & 0xffff) | ((data[1] & 0xffff) << 16);
        if (reg == GIF_REG_XYZF2) {
            // Yes, the binary places F in the low byte of Z instead of above it.
            packed[1] = ((data[2] >> 4) & 0xffffff) | ((data[3] >> 4) & 0xff);
        } else {
            packed[1] = data[2];
        }
        result = describeRegister(id, packed, line);
        break;
    case GIF_REG_FOG:
        packed[0] = 0;
        packed[1] = (data[3] & 0xff0) << 20;
        result = describeRegister(id, packed, line);
        break;
    case GIF_REG_RESERVED:
        result = report(MESSAGE_RESERVED_REGISTER,
                        DEBUG_ERROR,
                        "Encountered *Reserved* (0xb) in GIFTAG REGS field. Treating as a NOP.");
        result |= describeRegister(id, data, line);
        break;
    case GIF_REG_A_D:
        result = describeRegister(data[2] & GIF_A_D_REGISTER_MASK, data, line);
        break;
    default:
        result = describeRegister(id, data, line);
        break;
    }
    return result;
}

// Moves the decoder to the next REGS entry, or to the next loop after the last entry.
static inline void advanceRegister(void) {
    if (g_gifProgress.nloop != 0) {
        if (g_gifProgress.reg == (g_gifTag.nregs & 0xf)) {
            g_gifProgress.reg = 1;
            g_gifProgress.nloop = g_gifProgress.nloop - 1;
        } else {
            g_gifProgress.reg = g_gifProgress.reg + 1;
        }
    }
}

// NTSC-U/C: 0x002a5188, PAL: 0x002af0f8
// Reports the quadword of data at g_pGifData under the current tag.
static int describeData(char *line) {
    int result = 0;
    line[0] = '\0';
    switch (g_gifTag.flg) {
    case GIF_FLG_REGLIST:
        for (int half = 0; half < 3; half += 2) {
            line[0] = '\0';
            if (g_gifProgress.nloop != 0) {
                result |= describeListRegister(
                    g_gifTag.regs[(g_gifProgress.reg - 1) & 0xf], &g_pGifData[half], line);
            } else {
                if (half == 0) {
                    DEBUG_ABORT("Logic error - came across a NOP in the first half of a "
                                "quadword.\n");
                }
                result |= describeRegister(GS_SKIPPED, &g_pGifData[2], line);
            }
            result |= DEBUG_print(line);
            advanceRegister();
        }
        break;
    case GIF_FLG_PACKED:
        result =
            describePackedRegister(g_gifTag.regs[(g_gifProgress.reg - 1) & 0xf], g_pGifData, line);
        result |= DEBUG_print(line);
        advanceRegister();
        break;
    case GIF_FLG_IMAGE:
    case GIF_FLG_DISABLED:
        g_gifProgress.nloop = g_gifProgress.nloop - 1;
        break;
    }
    return result;
}

// NTSC-U/C: 0x002a5420, PAL: 0x002af390
// Decodes and reports the GIF tag in g_adwGifTag.
static int describeTag(char *line) {
    int result = 0;
    line[0] = '\0';
    g_gifTag.address = g_pGifData;
    g_gifTag.nloop = g_adwGifTag[0] & 0x7fff;
    g_gifTag.eop = (g_adwGifTag[0] >> 15) & 1;
    g_gifTag.pre = (g_adwGifTag[1] >> 14) & 1;
    g_gifTag.prim = (g_adwGifTag[1] >> 15) & 0x7ff;
    g_gifTag.flg = (g_adwGifTag[1] >> 26) & 3;
    g_gifTag.nregs = g_adwGifTag[1] >> 28;
    if (g_gifTag.nregs == 0) {
        g_gifTag.nregs = GIF_REG_COUNT;
    }
    g_gifTag.regsLow = g_adwGifTag[2];
    g_gifTag.regsHigh = g_adwGifTag[3];
    for (unsigned int i = 0; i < GIF_REG_COUNT; ++i) {
        const unsigned int word =
            i < 8 ? g_gifTag.regsLow >> (i * 4) : g_gifTag.regsHigh >> (i * 4 - 32);
        g_gifTag.regs[i] = word & 0xf;
    }
    unsigned int quadwords;
    if (g_gifTag.flg < GIF_FLG_IMAGE) {
        quadwords = g_gifTag.nregs * g_gifTag.nloop;
    } else {
        quadwords = g_gifTag.nloop;
    }
    DEBUG_addField(DEBUG_FIELD_COLUMN,
                   line,
                   DEBUG_ANSI_ON_CYAN "GIFTAG" DEBUG_ANSI_RESET "%7s [" DEBUG_ANSI_CYAN
                                      "%08x" DEBUG_ANSI_RESET "]",
                   "",
                   g_gifTag.address);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   line,
                   "[" DEBUG_ANSI_MAGENTA "%08x.%08x.%08x.%08x" DEBUG_ANSI_RESET "]",
                   g_adwGifTag[3],
                   g_adwGifTag[2],
                   g_adwGifTag[1],
                   g_adwGifTag[0]);
    DEBUG_addField(DEBUG_FIELD_INLINE,
                   line,
                   "Expects " DEBUG_ANSI_GREEN "0x%x" DEBUG_ANSI_RESET " regs",
                   quadwords);
    DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, "FLG", g_apszGifFlagNames[g_gifTag.flg]);
    DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, "EOP", g_gifTag.eop);
    DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, "NLOOP", g_gifTag.nloop);
    if (g_gifTag.flg < GIF_FLG_IMAGE) {
        if (g_gifTag.nregs == GIF_REG_COUNT) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           DEBUG_ANSI_WHITE "%s" DEBUG_ANSI_RESET "=" DEBUG_ANSI_GREEN
                                            "%d" DEBUG_ANSI_RESET " (" DEBUG_ANSI_GREEN
                                            "%d" DEBUG_ANSI_RESET " regs)",
                           "NREG",
                           0,
                           GIF_REG_COUNT);
        } else {
            DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_DEC, "NREG", g_gifTag.nregs);
        }
    }
    if (g_gifTag.flg == GIF_FLG_PACKED) {
        DEBUG_addField(DEBUG_FIELD_INLINE, line, FIELD_TEXT, "PRE", g_gifTag.pre ? "ON" : "OFF");
        if (g_gifTag.pre) {
            DEBUG_addField(DEBUG_FIELD_LINE,
                           line,
                           FIELD_TEXT,
                           "PRIM",
                           describePrimitive(g_gifTag.prim, 1, g_nGifVerboseAttrs));
        }
    }
    if (g_gifTag.flg < GIF_FLG_IMAGE) {
        DEBUG_addField(DEBUG_FIELD_LINE, line, "Regs:");
        for (unsigned int i = 0; i < g_gifTag.nregs; ++i) {
            DEBUG_addField(DEBUG_FIELD_INLINE,
                           line,
                           "%d:" DEBUG_ANSI_GREEN "%s" DEBUG_ANSI_RESET "  ",
                           i,
                           g_apszGifRegNames[(int)g_gifTag.regs[i]]);
        }
    }
    if (g_gifTag.flg != GIF_FLG_PACKED) {
        if (g_gifTag.pre) {
            sprintf(g_szDebugScratch,
                    "Warning, PRE bit is set, but FLG is not PACKED. PRIM is ignored.");
            result = report(MESSAGE_PRE_IGNORED, DEBUG_WARNING, g_szDebugScratch);
        }
        return result;
    }
    if (g_gifTag.pre && (g_gifTag.prim & 7) == 7) {
        sprintf(g_szDebugScratch, "Illegal primitive in GIFTAG prim field.");
        result = report(MESSAGE_ILLEGAL_PRIMITIVE, DEBUG_ERROR, g_szDebugScratch);
        if (result & (DEBUG_ERROR | DEBUG_FATAL)) {
            return result;
        }
    }
    for (unsigned int i = 0; i < GIF_REG_COUNT; ++i) {
        if (i < g_gifTag.nregs) {
            if (g_gifTag.regs[i] == GIF_REG_RESERVED) {
                sprintf(g_szDebugScratch, "Illegal register in GIFTAG REGS field %d", i);
                result |= report(MESSAGE_RESERVED_REGISTER, DEBUG_ERROR, g_szDebugScratch);
                if (result & DEBUG_FATAL) {
                    return result;
                }
            }
        } else if (g_gifTag.regs[i] != 0) {
            sprintf(g_szDebugScratch,
                    "Regs fields are set, but NREGS is not large enough to use them.");
            result |= report(MESSAGE_SURPLUS_REGS, DEBUG_WARNING, g_szDebugScratch);
        }
    }
    return result;
}

// NTSC-U/C: 0x002a5918, PAL: 0x002af888
// Starts the data of a new tag at its first REGS entry.
static void resetProgress(void) {
    g_gifProgress.nloop = 0;
    g_gifProgress.reg = 1;
    g_gifProgress.reserved20 = 0;
}

int GIF_processQword(void) {
    char line[QWORD_LINE_SIZE];
    line[0] = '\0';
    if (((g_dwGifStat >> GIF_STAT_DIR_SHIFT) & 1) == 1) {
        return report(MESSAGE_WRONG_DIRECTION,
                      DEBUG_FATAL,
                      "The DIR bit in GIF_STAT is set to GS->EE, the DMA debugger can only handle "
                      "EE->GS");
    }
    const unsigned int *data = g_pGifData;
    if (data == NULL) {
        sprintf(g_szDebugScratch, "GIF tag/register address is not valid. (%08x)", 0);
        return report(MESSAGE_NO_DATA, DEBUG_FATAL, g_szDebugScratch);
    }
    if ((unsigned int)data & 0xf) {
        // Yes, the binary tests quadword alignment and reports a 4 byte boundary.
        sprintf(g_szDebugScratch,
                "DMA List is not aligned on a 4 byte boundary. (Address = %08x)",
                (unsigned int)data);
        return report(MESSAGE_MISALIGNED, DEBUG_FATAL, g_szDebugScratch);
    }
    if (g_gifProgress.nloop == 0) {
        g_adwGifTag[0] = data[0];
        g_adwGifTag[1] = data[1];
        g_adwGifTag[2] = data[2];
        g_adwGifTag[3] = data[3];
        resetProgress();
        g_gifProgress.nloop = g_adwGifTag[0] & 0x7fff;
        const int result = describeTag(line);
        return result | DEBUG_print(line);
    }
    return describeData(line);
}
