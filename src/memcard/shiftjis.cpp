#include "memcard/shiftjis.h"

#include <string.h>

#include "os/debug.h"

namespace {

// Indices into g_aShiftJisRanges.
enum ShiftJisRangeIndex { kRangeDigits = 0, kRangeCapitals = 1, kRangeSmallLetters = 2 };

// Marks a character that maps through g_aShiftJisRanges rather than g_aShiftJisSymbols.
constexpr int kNoSymbol = -1;

// Symbols in g_aShiftJisSymbols ahead of each run of ASCII punctuation.
constexpr int kSymbolsBeforeColon = 16;
constexpr int kSymbolsBeforeBracket = 23;
constexpr int kSymbolsBeforeBrace = 29;

constexpr int kShiftJisLeadShift = 8;
constexpr unsigned int kByteMask = 0xff;

inline bool InRange(int nValue, int nFirst, int nLast) {
    return nValue >= nFirst && nValue <= nLast;
}

} // namespace

const ShiftJisSymbol g_aShiftJisSymbols[kShiftJisSymbolCount + 1] = {
    {0x8140, ' '},  {0x8149, '!'},  {0x8168, '"'}, {0x8194, '#'}, {0x8190, '$'}, {0x8193, '%'},
    {0x8195, '&'},  {0x8166, '\''}, {0x8169, '('}, {0x816a, ')'}, {0x8196, '*'}, {0x817b, '+'},
    {0x8143, ','},  {0x817c, '-'},  {0x8144, '.'}, {0x815e, '/'}, {0x8146, ':'}, {0x8147, ';'},
    {0x8171, '<'},  {0x8181, '='},  {0x8172, '>'}, {0x8148, '?'}, {0x8197, '@'}, {0x816d, '['},
    {0x818f, '\\'}, {0x816e, ']'},  {0x814f, '^'}, {0x8151, '_'}, {0x8165, '`'}, {0x816f, '{'},
    {0x8162, '|'},  {0x8170, '}'},  {0x8150, '~'}, {0, '\0'}};

const ShiftJisRange g_aShiftJisRanges[kShiftJisRangeCount] = {
    {0x824f, '0'}, {0x8260, 'A'}, {0x8281, 'a'}};

unsigned short EncodeShiftJisCharacter(unsigned char cAscii) {
    int nSymbol = kNoSymbol;
    int nRange = kRangeDigits;
    if (InRange(cAscii, ' ', '/')) {
        nSymbol = cAscii - ' ';
    } else if (InRange(cAscii, '0', '9')) {
        nRange = kRangeDigits;
    } else if (InRange(cAscii, ':', '@')) {
        nSymbol = cAscii - ':' + kSymbolsBeforeColon;
    } else if (InRange(cAscii, 'A', 'Z')) {
        nRange = kRangeCapitals;
    } else if (InRange(cAscii, '[', '`')) {
        nSymbol = cAscii - '[' + kSymbolsBeforeBracket;
    } else if (InRange(cAscii, 'a', 'z')) {
        nRange = kRangeSmallLetters;
    } else if (InRange(cAscii, '{', '~')) {
        nSymbol = cAscii - '{' + kSymbolsBeforeBrace;
    } else {
        DebugPrint("bad ASCII code 0x%x\n", cAscii);
        return 0;
    }

    if (nSymbol != kNoSymbol) {
        return g_aShiftJisSymbols[nSymbol].mShiftJis;
    }
    const ShiftJisRange &range = g_aShiftJisRanges[nRange];
    return static_cast<unsigned short>(range.mShiftJisBase + cAscii - range.mAsciiBase);
}

void AsciiToShiftJis(const char *pszAscii, char *pszDest) {
    const int nLength = strlen(pszAscii);
    int i = 0;
    for (; i < nLength; ++i) {
        const unsigned short nCode = EncodeShiftJisCharacter(pszAscii[i]);
        pszDest[i * 2] = static_cast<char>(nCode >> kShiftJisLeadShift);
        pszDest[(i * 2) + 1] = static_cast<char>(nCode & kByteMask);
    }
    pszDest[i * 2] = '\0';
    pszDest[(i * 2) + 1] = '\0';
}
