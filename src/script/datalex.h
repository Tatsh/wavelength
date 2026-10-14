#pragma once

#include <stdio.h>

/** Tokens DataLex() reports. The names are inferred. */
enum DataToken {
    kDataTokenEnd = 0,       /*!< The end of the input. */
    kDataTokenFloat = 1,     /*!< A number with a fraction or an exponent. */
    kDataTokenInt = 2,       /*!< A whole number. */
    kDataTokenString = 3,    /*!< Text in double quotes, quotes included. */
    kDataTokenSymbol = 4,    /*!< A bare word. */
    kDataTokenOpen = 5,      /*!< `(`, the start of a child array. */
    kDataTokenClose = 6,     /*!< `)`, the end of a child array. */
    kDataTokenDefine = 7,    /*!< `{`, the start of the value of the macro just read. */
    kDataTokenDefineEnd = 8, /*!< `}`, the end of a macro value. */
    kDataTokenInclude = 9,   /*!< A file name in `<` and `>`, read in place. */
    kDataTokenMerge = 10,    /*!< A file name in `<<` and `>>`, merged by tag. */
};

/**
 * Scan the next token of the data file the parser reads.
 *
 * The scanner reads through DataInput() and advances g_nDataParseLineNumber at each newline. It
 * skips white space and comments.
 *
 * @return One of DataToken.
 * @ghidraAddress NTSC-U/C: 0x002987a0
 * @ghidraAddress PAL: 0x002a23b0
 */
int DataLex();

/**
 * The text of the token DataLex() reported last, terminated.
 *
 * @ghidraAddress NTSC-U/C: 0x00515308
 */
extern char *g_pszDataLexText;

/**
 * The length of g_pszDataLexText.
 *
 * @ghidraAddress NTSC-U/C: 0x005152f8
 */
extern int g_nDataLexLength;

/**
 * The file the scanner reads when it starts, standard input.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2390
 */
extern FILE *g_pDataLexIn;

/**
 * The file the scanner would echo to, standard output.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2394
 */
extern FILE *g_pDataLexOut;
