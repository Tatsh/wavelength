#pragma once

/** Punctuation pairs in g_aShiftJisSymbols, one for each printable ASCII symbol. */
constexpr int kShiftJisSymbolCount = 33;

/** Runs in g_aShiftJisRanges: the digits, the capital letters, and the small letters. */
constexpr int kShiftJisRangeCount = 3;

/** One ASCII symbol and its full-width Shift-JIS code. The title is inferred. */
struct ShiftJisSymbol {
    unsigned short mShiftJis; /*!< The two-byte code, lead byte high. */
    char mAscii;              /*!< The ASCII character. */
};

/** One run of consecutive characters that maps linearly. The title is inferred. */
struct ShiftJisRange {
    unsigned short mShiftJisBase; /*!< The code of the first character of the run. */
    unsigned short mAsciiBase;    /*!< The first ASCII character of the run. */
};

/**
 * The ASCII punctuation from space to tilde in ASCII order, followed by one zero entry.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2430
 */
extern const ShiftJisSymbol g_aShiftJisSymbols[kShiftJisSymbolCount + 1];

/**
 * The digit, capital, and small-letter runs, based at `0x824f`, `0x8260`, and `0x8281`.
 *
 * @ghidraAddress NTSC-U/C: 0x003b24b8
 */
extern const ShiftJisRange g_aShiftJisRanges[kShiftJisRangeCount];

/**
 * Convert one ASCII character to its full-width Shift-JIS code.
 *
 * A code outside space to tilde is reported through the log as `bad ASCII code 0x%x`. The title
 * is inferred.
 *
 * @param cAscii The character.
 * @return The two-byte code, lead byte high, or zero for a character outside the tables.
 * @ghidraAddress NTSC-U/C: 0x0029e518
 * @ghidraAddress PAL: 0x002a81e0
 */
unsigned short EncodeShiftJisCharacter(unsigned char cAscii);

/**
 * Convert ASCII text to the Shift-JIS bytes `icon.sys` stores a title as.
 *
 * Every character becomes two bytes, lead byte first, and two zero bytes end the result.
 * MCCreateSaveDirTask is its only caller, and the title is inferred from that use.
 *
 * @param pszAscii The text to convert.
 * @param pszDest The destination, of at least twice the text length plus two bytes.
 * @ghidraAddress NTSC-U/C: 0x0029e628
 * @ghidraAddress PAL: 0x002a82f0
 */
void AsciiToShiftJis(const char *pszAscii, char *pszDest);
