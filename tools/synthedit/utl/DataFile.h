#pragma once

#include <vector>

#include "utl/BinStream.h"
#include "utl/Data.h"

/** A token the script lexer returns. */
enum DataToken {
    kDataTokenEnd = 0,          /*!< The end of the input. */
    kDataTokenFloat = 1,        /*!< A real number. */
    kDataTokenInt = 2,          /*!< An integer. */
    kDataTokenString = 3,       /*!< Text in double quotes. */
    kDataTokenSymbol = 4,       /*!< A bare word. */
    kDataTokenArrayOpen = 5,    /*!< `(`, which opens a nested array. */
    kDataTokenArrayClose = 6,   /*!< `)`, which closes an array. */
    kDataTokenMacroOpen = 7,    /*!< `{`, which opens the value of the macro named before it. */
    kDataTokenMacroClose = 8,   /*!< `}`, which closes a macro's value. */
    kDataTokenInclude = 9,      /*!< `<file>`, whose nodes are inserted in place. */
    kDataTokenMergeInclude = 10 /*!< `<<file>>`, whose nested arrays are merged by tag. */
};

/**
 * Read the next token of the script being parsed.
 *
 * @return A DataToken. The text of the token is in #yytext.
 * @ghidraAddress 0x1001a210
 */
int yylex();

/**
 * Text of the token yylex() returned last.
 *
 * @ghidraAddress 0x100cbe58
 */
extern char *yytext;

/**
 * Length of #yytext.
 *
 * @ghidraAddress 0x100cbe54
 */
extern int yyleng;

/**
 * Line of the script being parsed, counted by the lexer from one.
 *
 * @ghidraAddress 0x100c391c
 */
extern int gDataLine;

/**
 * Read the script names a compiled script begins with into the shared string table.
 *
 * @param bs The stream.
 * @param filenames Receives the names.
 * @return The stream.
 * @ghidraAddress 0x10015340
 */
BinStream &DataReadFilenames(BinStream &bs, std::vector<const char *> &filenames);

/**
 * Fill the lexer's buffer from the stream being parsed.
 *
 * @param buf The buffer.
 * @param max The size of the buffer.
 * @return `max`, or zero when the stream has failed before or during the read.
 * @ghidraAddress 0x10015410
 */
int DataLexInput(char *buf, int max);

/**
 * Read every script compiled, even from the host.
 *
 * @ghidraAddress 0x10015450
 */
void DataSetUseCompiled();

/**
 * Read a script, as text from the host or compiled from the disc or when DataSetUseCompiled() was
 * called.
 *
 * A compiled script is read from the path DataMakeCompiledPath() makes. A compiled script of the
 * wrong version is a failure.
 *
 * @param file The path of the script source.
 * @param stream The stream to read from, or null to open the file.
 * @return The array, with one reference, or null when the file does not open.
 * @ghidraAddress 0x10015460
 */
DataArray *DataReadFile(const char *file, BinStream *stream);

/**
 * Parse a script source.
 *
 * @param file The path of the script, recorded as the file of each array.
 * @param stream The stream to read from, or null to open the file.
 * @return The array, with one reference, or null when the file does not open.
 * @ghidraAddress 0x100156e0
 */
DataArray *DataReadTxtFile(const char *file, BinStream *stream);

/**
 * Parse nodes up to the end of the current array into a new array.
 *
 * @return The array, with one reference.
 * @ghidraAddress 0x10015800
 */
DataArray *DataParseArray();

/**
 * Parse one token into the current array.
 *
 * @return Whether the array continues.
 * @ghidraAddress 0x10015880
 */
bool DataParseNode();

/**
 * Parse the script #yytext refers to, relative to the directory of the script being parsed.
 *
 * A file that does not open is a failure.
 *
 * @param delimiterLength The number of angle brackets on each side of the name in #yytext.
 * @return The array, with one reference.
 * @ghidraAddress 0x10015bd0
 */
DataArray *DataReadEmbeddedFile(int delimiterLength);

/**
 * Make the path a script is read from: the source itself, or `dir/gen/base.ext.bin` when reading
 * compiled scripts.
 *
 * @param out Receives the path.
 * @param file The path of the script source.
 * @param compiled Whether to make the compiled path even from the host.
 * @ghidraAddress 0x10015cc0
 */
void DataMakeCompiledPath(char *out, const char *file, bool compiled);
