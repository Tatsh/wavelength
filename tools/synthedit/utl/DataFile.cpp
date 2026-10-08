#include "utl/DataFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/DataString.h"
#include "utl/FileStream.h"
#include "utl/Str.h"

namespace {

// A parsed array starts with this many nodes and grows by this many when full.
const int kDataGrowNodes = 64;

// The version byte a compiled script starts with.
const char kDataCompiledVersion = 2;

// The longest script name a compiled script stores, with its terminator.
const int kFilenameBufferSize = 256;

// The longest compiled script path, with its terminator.
const int kCompiledPathSize = 128;

} // namespace

// The array being parsed.
// 0x100c3914
static DataArray *gDataArray;

// The next node of the array being parsed.
// 0x100c3918
static int gDataNode;

int gDataLine;

// The interned path of the script being parsed.
// 0x100c3920
static const char *gDataFile;

// The stream the script is read from.
// 0x100c3924
static BinStream *gDataInputStream;

// 0x100c3928
static bool gDataUseCompiled;

BinStream &DataReadFilenames(BinStream &bs, std::vector<const char *> &filenames) {
    int count;
    bs.ReadEndian(&count, sizeof(count));
    filenames.resize(count, NULL);
    for (int i = 0; i < count; ++i) {
        char name[kFilenameBufferSize];
        bs.ReadString(name, sizeof(name));
        filenames[i] = DataInternString(name);
    }
    return bs;
}

int DataLexInput(char *buf, int max) {
    if (gDataInputStream->Fail()) {
        return 0;
    }
    gDataInputStream->Read(buf, max);
    // A read that runs past the end fails, and the bytes it did read are discarded.
    return gDataInputStream->Fail() ? 0 : max;
}

void DataSetUseCompiled() {
    gDataUseCompiled = true;
}

DataArray *DataReadFile(const char *file, BinStream *stream) {
    if (!UsingCD() && !gDataUseCompiled) {
        return DataReadTxtFile(file, stream);
    }
    char path[kCompiledPathSize];
    if (stream == NULL) {
        DataMakeCompiledPath(path, file, true);
        gDataInputStream = new FileStream(path, FileStream::kRead, true, 0);
        if (gDataInputStream->Fail()) {
            TheDebug.Printf("DataReadFile: Can't open %s\n", path);
            delete gDataInputStream;
            gDataInputStream = NULL;
            return NULL;
        }
    } else {
        gDataInputStream = stream;
    }
    gDataFile = DataInternString(file);
    char version;
    gDataInputStream->Read(&version, sizeof(version));
    if (version != kDataCompiledVersion) {
        TheDebug.Fail("DataReadFile. Can't read version %d of file %s\n", version, file);
    }
    std::vector<const char *> filenames;
    DataReadFilenames(*gDataInputStream, filenames);
    DataArray *array = new DataArray(*gDataInputStream, filenames);
    if (stream == NULL) {
        delete gDataInputStream;
        gDataInputStream = NULL;
    }
    return array;
}

DataArray *DataReadTxtFile(const char *file, BinStream *stream) {
    if (stream == NULL) {
        gDataInputStream = new FileStream(file, FileStream::kRead, true, 0);
        if (gDataInputStream->Fail()) {
            TheDebug.Printf("DataReadTxtFile: Can't open %s\n", file);
            delete gDataInputStream;
            gDataInputStream = NULL;
            return NULL;
        }
    } else {
        gDataInputStream = stream;
    }
    gDataFile = DataInternString(file);
    gDataLine = 1;
    DataArray *array = DataParseArray();
    if (stream == NULL) {
        delete gDataInputStream;
        gDataInputStream = NULL;
    }
    return array;
}

DataArray *DataParseArray() {
    DataArray *outerArray = gDataArray;
    const int outerNode = gDataNode;
    gDataArray = NewDataArray(kDataGrowNodes);
    gDataArray->SetFileLine(gDataFile, gDataLine);
    gDataNode = 0;
    while (DataParseNode()) {
    }
    gDataArray->Resize(gDataNode);
    DataArray *array = gDataArray;
    gDataNode = outerNode;
    gDataArray = outerArray;
    return array;
}

bool DataParseNode() {
    const int token = yylex();
    if (token == kDataTokenEnd || token == kDataTokenArrayClose || token == kDataTokenMacroClose) {
        return false;
    }

    DataArray::DataNode node;
    int i;
    if (token == kDataTokenMergeInclude) {
        DataArray *merged = DataReadEmbeddedFile(2);
        gDataArray->Resize(gDataNode);
        DataMergeTags(gDataArray, merged);
        gDataNode = gDataArray->Size();
        merged->Release();
        return true;
    }
    if (token == kDataTokenInclude) {
        DataArray *included = DataReadEmbeddedFile(1);
        const int needed = included->Size() + gDataNode;
        if (needed > gDataArray->Size()) {
            gDataArray->Resize(needed + kDataGrowNodes);
        }
        for (i = 0; i < included->Size(); ++i) {
            gDataArray->SetNode(gDataNode++, included->Node(i), included->Type(i));
        }
        included->Release();
        return true;
    }
    if (token == kDataTokenMacroOpen) {
        // The symbol before the brace is the macro's name, which the macro replaces.
        DataArray *macro = DataParseArray();
        DataSetMacro(gDataArray->Sym(--gDataNode), macro);
        macro->Release();
        return true;
    }

    // The array grows by one block here, even when a macro below inserts more nodes than that.
    if (gDataNode == gDataArray->Size()) {
        gDataArray->Resize(gDataNode + kDataGrowNodes);
    }
    switch (token) {
    case kDataTokenArrayOpen: {
        DataArray *array = DataParseArray();
        node.array = array;
        gDataArray->SetNode(gDataNode++, node, DataArray::kDataArray);
        array->Release();
        return true;
    }
    case kDataTokenInt:
        node.i = atoi(yytext);
        gDataArray->SetNode(gDataNode++, node, DataArray::kDataInt);
        return true;
    case kDataTokenFloat:
        node.f = static_cast<float>(atof(yytext));
        gDataArray->SetNode(gDataNode++, node, DataArray::kDataFloat);
        return true;
    case kDataTokenSymbol:
    case kDataTokenString:
        break;
    default:
        TheDebug.Fail(
            "DataReadFile: Unrecognized token %d (file %s, line %d)", token, gDataFile, gDataLine);
        return false;
    }

    char *text = yytext;
    if (token == kDataTokenString) {
        text[yyleng - 1] = '\0';
        ++text;
    }
    for (char *p = text; *p != '\0'; ++p) {
        if (p[0] == '\\' && p[1] == 'n') {
            *p = '\n';
            for (char *q = p + 1; *q != '\0'; ++q) {
                *q = q[1];
            }
        }
    }
    const char *sym = DataFindString(text);
    if (sym == NULL) {
        node.sym = DataAddString(text);
        gDataArray->SetNode(gDataNode++, node, DataArray::kDataSymbol);
        return true;
    }
    if (token == kDataTokenSymbol) {
        DataArray *macro = DataGetMacro(sym);
        if (macro != NULL) {
            for (i = 0; i < macro->Size(); ++i) {
                gDataArray->SetNode(gDataNode++, macro->Node(i), macro->Type(i));
            }
            return true;
        }
    }
    node.sym = sym;
    gDataArray->SetNode(gDataNode++, node, DataArray::kDataSymbol);
    return true;
}

DataArray *DataReadEmbeddedFile(int delimiterLength) {
    DataArray *outerArray = gDataArray;
    const int outerLine = gDataLine;
    const char *outerFile = gDataFile;
    const int outerNode = gDataNode;
    BinStream *outerStream = gDataInputStream;

    yytext[strlen(yytext) - delimiterLength] = '\0';
    const char *name = yytext + delimiterLength;
    const char *path;
    if (FileIsAbsolute(name)) {
        path = FileNormalizePath(name);
    } else {
        path = FileNormalizePath(FormatString("%s/%s", FileGetPath(gDataFile), name));
    }
    DataArray *array = DataReadTxtFile(path, NULL);
    if (array == NULL) {
        TheDebug.Fail("Could not open embedded file: %s", path);
    }

    gDataLine = outerLine;
    gDataInputStream = outerStream;
    gDataNode = outerNode;
    gDataFile = outerFile;
    gDataArray = outerArray;
    return array;
}

void DataMakeCompiledPath(char *out, const char *file, bool compiled) {
    if (gDataUseCompiled) {
        compiled = true;
    }
    if (!UsingCD() && !compiled) {
        strcpy(out, file);
        return;
    }
    sprintf(out, "%s/gen/%s.%s.bin", FileGetPath(file), FileGetBase(file), FileGetExt(file));
}
