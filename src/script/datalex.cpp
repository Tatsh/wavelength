// fileno() and isatty() are POSIX.
#define _POSIX_C_SOURCE 200809L

#include "script/datalex.h"

#include <stdlib.h>
#include <unistd.h>

#include "script/datafile.h"

// The scanner is a table-driven automaton of the flex 2.5 skeleton. The tables below are the
// automaton of the data file rules: white space, newlines, comments that run from `;` to the end
// of the line, numbers, quoted text, bare words, parentheses, braces, and the include and merge
// directives.

namespace {

// The scanner's input buffer.
class FlexBuffer {
public:
    // Allocate a buffer of a capacity for a file.
    static FlexBuffer *Create(FILE *pFile, int nSize);

    // Empty a buffer, which may be null, and reload the scanner state when it is the current one.
    static void Flush(FlexBuffer *pBuffer);

    // Empty the buffer and attach it to a file, which may be null.
    void Init(FILE *pFile);

    FILE *mInputFile;   // The file the buffer reads.
    char *mChars;       // The characters, with two terminators after the last.
    char *mPosition;    // The scan position to resume at.
    unsigned int mSize; // The capacity of mChars, terminators excluded.
    int mCharCount;     // The characters in mChars.
    int mIsOurs;        // Non-zero when the scanner allocated mChars and may grow it.
    int mIsInteractive; // Non-zero when the file is a terminal.
    int mAtLineStart;   // Non-zero at the start of a line.
    int mFill;          // Non-zero when the buffer refills from the file.
    int mStatus;        // One of BufferStatus.
};

// FlexBuffer::mStatus values.
enum BufferStatus {
    kBufferNew = 0,
    kBufferNormal = 1,
    kBufferEofPending = 2,
};

// FlexGetNextBuffer() results.
enum EobAction {
    kEobContinueScan = 0,
    kEobEndOfFile = 1,
    kEobLastMatch = 2,
};

// Rule actions, numbered from 1, then the end of the buffer and the end of the input.
enum Action {
    kActionBackUp = 0,
    kActionNewline = 1,
    kActionInt = 2,
    kActionFloat = 3,
    kActionExponent = 4,
    kActionString = 5,
    kActionSymbol = 6,
    kActionComment = 7,
    kActionOpen = 8,
    kActionClose = 9,
    kActionDefine = 10,
    kActionDefineEnd = 11,
    kActionMerge = 12,
    kActionInclude = 13,
    kActionWhitespace = 14,
    kActionEndOfBuffer = 15,
    kActionEndOfInput = 16,
};

constexpr int kFlexBufferSize = 16384;
constexpr int kFlexInitialState = 1;

// The first state past the last real one, where the meta classes take over.
constexpr int kFlexFirstTemplateState = 38;

// The base of the jam state, and the state itself.
constexpr int kFlexJamBase = 80;
constexpr int kFlexJamState = 37;

// The class of a NUL character, which the tables otherwise give class 0.
constexpr int kFlexNulClass = 1;

constexpr int kFlexExitFailure = 2;

// NTSC-U/C: 0x0041e010
constexpr short kFlexAccept[] = {0, 0, 0, 15, 6, 14, 1, 6, 8, 9, 6, 3,  2, 6, 14, 10, 11, 6,  6,
                                 0, 5, 3, 2,  3, 6,  6, 0, 7, 0, 0, 13, 5, 6, 4,  0,  13, 12, 0};

// NTSC-U/C: 0x0041e060
constexpr int kFlexClass[] = {
    0,  1,  1,  1,  1,  1,  1, 1,  1,  2, 3,  1, 1, 2, 1, 1, 1, 1,  1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  2, 1,  4,  1, 1,  1, 1, 1, 5, 6, 1, 7,  1, 8,  9, 1,  10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 1, 11, 12, 1, 13, 1, 1, 1, 1, 1, 1, 14, 1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1,  1,  15, 1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 16, 1, 17, 1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 1,  1,  1, 1,  1, 1, 1, 1, 1, 1, 1,  1, 1,  1, 1};

// NTSC-U/C: 0x0041e460
constexpr int kFlexMeta[] = {0, 1, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2};

// NTSC-U/C: 0x0041e4a8
constexpr short kFlexBase[] = {0,  0,  0,  79, 0,  80, 80, 17, 80, 80, 26, 27, 0,  42, 26,
                               80, 80, 0,  0,  74, 0,  0,  0,  0,  53, 0,  74, 80, 63, 62,
                               80, 80, 64, 63, 59, 49, 80, 80, 39, 63, 65, 67, 69};

// NTSC-U/C: 0x0041e500
constexpr short kFlexDefault[] = {0,  37, 1,  37, 38, 37, 37, 37, 37, 37, 38, 38, 10, 37, 39,
                                  37, 37, 38, 7,  40, 38, 11, 10, 11, 38, 13, 41, 37, 39, 42,
                                  37, 37, 38, 38, 42, 37, 37, 0,  37, 37, 37, 37, 37};

// NTSC-U/C: 0x0041e558
constexpr short kFlexNext[] = {0,  4,  5,  6,  7,  8,  9,  10, 10, 11, 12, 13, 14, 5,  4,  4,  15,
                               16, 18, 19, 19, 20, 19, 19, 18, 18, 18, 18, 18, 19, 19, 18, 18, 19,
                               19, 21, 22, 23, 29, 30, 17, 24, 24, 25, 26, 27, 25, 26, 26, 25, 25,
                               25, 25, 25, 26, 26, 25, 25, 26, 26, 32, 32, 36, 33, 28, 28, 19, 19,
                               26, 26, 34, 34, 35, 33, 33, 35, 30, 27, 31, 37, 3,  37, 37, 37, 37,
                               37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37};

// NTSC-U/C: 0x0041e620
constexpr short kFlexCheck[] = {0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                                1,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
                                7,  10, 10, 11, 14, 14, 38, 11, 11, 13, 13, 13, 13, 13, 13, 13, 13,
                                13, 13, 13, 13, 13, 13, 13, 13, 13, 24, 24, 35, 24, 39, 39, 40, 40,
                                41, 41, 42, 42, 34, 33, 32, 29, 28, 26, 19, 3,  37, 37, 37, 37, 37,
                                37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37};

} // namespace

char *g_pszDataLexText = nullptr;
int g_nDataLexLength = 0;
FILE *g_pDataLexIn = nullptr;
FILE *g_pDataLexOut = nullptr;

// NTSC-U/C: 0x003b2380
static FlexBuffer *g_pFlexBuffer = nullptr;

// NTSC-U/C: 0x003b2384
static char *g_pFlexCursor = nullptr;

// NTSC-U/C: 0x003b2388
static int g_nFlexNeedsInit = 1;

// NTSC-U/C: 0x003b238c
static int g_nFlexStartState = 0;

// NTSC-U/C: 0x005152fc
static char g_cFlexHoldChar;

// NTSC-U/C: 0x00515300
static int g_nFlexCharCount;

// NTSC-U/C: 0x00515304
static int g_nFlexSwitchedOnEof;

// NTSC-U/C: 0x0051530c
static int g_nFlexLastAcceptState;

// NTSC-U/C: 0x00515310
static char *g_pFlexLastAcceptPos;

// NTSC-U/C: 0x00299340, PAL: 0x002a2f50
static void FlexFatalError(const char *pszMessage) {
    fprintf(stderr, "%s\n", pszMessage);
    exit(kFlexExitFailure);
}

// NTSC-U/C: 0x00299370, PAL: 0x002a2f80
static void *FlexAlloc(unsigned int nSize) {
    return malloc(nSize);
}

// NTSC-U/C: 0x00299390, PAL: 0x002a2fa0
static void *FlexRealloc(void *pBlock, unsigned int nSize) {
    return realloc(pBlock, nSize);
}

// NTSC-U/C: 0x002991a8, PAL: 0x002a2db8
static void FlexLoadBufferState() {
    g_nFlexCharCount = g_pFlexBuffer->mCharCount;
    g_pDataLexIn = g_pFlexBuffer->mInputFile;
    g_pFlexCursor = g_pFlexBuffer->mPosition;
    g_pszDataLexText = g_pFlexCursor;
    g_cFlexHoldChar = *g_pFlexCursor;
}

// NTSC-U/C: 0x002992e0, PAL: 0x002a2ef0
void FlexBuffer::Flush(FlexBuffer *pBuffer) {
    if (pBuffer == nullptr) {
        return;
    }
    pBuffer->mCharCount = 0;
    pBuffer->mChars[0] = '\0';
    pBuffer->mChars[1] = '\0';
    pBuffer->mAtLineStart = 1;
    pBuffer->mPosition = pBuffer->mChars;
    pBuffer->mStatus = kBufferNew;
    if (pBuffer == g_pFlexBuffer) {
        FlexLoadBufferState();
    }
}

// NTSC-U/C: 0x00299280, PAL: 0x002a2e90
void FlexBuffer::Init(FILE *pFile) {
    Flush(this);
    mInputFile = pFile;
    mFill = 1;
    mIsInteractive = pFile != nullptr ? isatty(fileno(pFile)) > 0 : 0;
}

// NTSC-U/C: 0x002991f0, PAL: 0x002a2e00
FlexBuffer *FlexBuffer::Create(FILE *pFile, int nSize) {
    FlexBuffer *pBuffer = static_cast<FlexBuffer *>(FlexAlloc(sizeof(FlexBuffer)));
    if (pBuffer == nullptr) {
        FlexFatalError("out of dynamic memory in yy_create_buffer()");
    }
    pBuffer->mSize = nSize;
    pBuffer->mChars = static_cast<char *>(FlexAlloc(pBuffer->mSize + 2));
    if (pBuffer->mChars == nullptr) {
        FlexFatalError("out of dynamic memory in yy_create_buffer()");
    }
    pBuffer->mIsOurs = 1;
    pBuffer->Init(pFile);
    return pBuffer;
}

// NTSC-U/C: 0x00299148, PAL: 0x002a2d58
static void FlexRestart(FILE *pFile) {
    if (g_pFlexBuffer == nullptr) {
        g_pFlexBuffer = FlexBuffer::Create(g_pDataLexIn, kFlexBufferSize);
    }
    g_pFlexBuffer->Init(pFile);
    FlexLoadBufferState();
}

// Follow the automaton from a state on a character class.
inline int FlexStep(int nState, int nClass) {
    while (kFlexCheck[kFlexBase[nState] + nClass] != nState) {
        nState = kFlexDefault[nState];
        if (nState >= kFlexFirstTemplateState) {
            nClass = kFlexMeta[nClass];
        }
    }
    return kFlexNext[kFlexBase[nState] + nClass];
}

// NTSC-U/C: 0x00298eb8, PAL: 0x002a2ac8
static int FlexGetPreviousState() {
    int nState = g_nFlexStartState;
    for (char *pCursor = g_pszDataLexText; pCursor < g_pFlexCursor; ++pCursor) {
        const int nClass =
            *pCursor != '\0' ? kFlexClass[static_cast<unsigned char>(*pCursor)] : kFlexNulClass;
        if (kFlexAccept[nState] != 0) {
            g_nFlexLastAcceptState = nState;
            g_pFlexLastAcceptPos = pCursor;
        }
        nState = FlexStep(nState, nClass);
    }
    return nState;
}

// NTSC-U/C: 0x00299040, PAL: 0x002a2c50
static int FlexTryNulTransition(int nState) {
    char *pCursor = g_pFlexCursor;
    if (kFlexAccept[nState] != 0) {
        g_nFlexLastAcceptState = nState;
        g_pFlexLastAcceptPos = pCursor;
    }
    nState = FlexStep(nState, kFlexNulClass);
    return nState == kFlexJamState ? 0 : nState;
}

// NTSC-U/C: 0x00298c60, PAL: 0x002a2870
static int FlexGetNextBuffer() {
    char *pDest = g_pFlexBuffer->mChars;
    char *pSource = g_pszDataLexText;
    if (g_pFlexCursor > &g_pFlexBuffer->mChars[g_nFlexCharCount + 1]) {
        FlexFatalError("fatal flex scanner internal error--end of buffer missed");
    }
    if (g_pFlexBuffer->mFill == 0) {
        return g_pFlexCursor - g_pszDataLexText == 1 ? kEobEndOfFile : kEobLastMatch;
    }

    const int nToMove = static_cast<int>(g_pFlexCursor - g_pszDataLexText) - 1;
    for (int i = 0; i < nToMove; ++i) {
        *pDest++ = *pSource++;
    }
    if (g_pFlexBuffer->mStatus == kBufferEofPending) {
        g_nFlexCharCount = 0;
        g_pFlexBuffer->mCharCount = 0;
    } else {
        int nToRead = g_pFlexBuffer->mSize - nToMove - 1;
        while (nToRead <= 0) {
            FlexBuffer *pBuffer = g_pFlexBuffer;
            const int nCursorOffset = static_cast<int>(g_pFlexCursor - pBuffer->mChars);
            if (pBuffer->mIsOurs != 0) {
                const int nNewSize = pBuffer->mSize * 2;
                if (nNewSize <= 0) {
                    pBuffer->mSize += pBuffer->mSize / 8;
                } else {
                    pBuffer->mSize *= 2;
                }
                pBuffer->mChars =
                    static_cast<char *>(FlexRealloc(pBuffer->mChars, pBuffer->mSize + 2));
            } else {
                pBuffer->mChars = nullptr;
            }
            if (pBuffer->mChars == nullptr) {
                FlexFatalError("fatal error - scanner input buffer overflow");
            }
            g_pFlexCursor = &pBuffer->mChars[nCursorOffset];
            nToRead = g_pFlexBuffer->mSize - nToMove - 1;
        }
        // The read is one character at a time, whatever room the buffer has.
        g_nFlexCharCount = DataInput(&g_pFlexBuffer->mChars[nToMove], 1) != 0;
        g_pFlexBuffer->mCharCount = g_nFlexCharCount;
    }

    int nResult;
    if (g_nFlexCharCount == 0) {
        if (nToMove == 0) {
            nResult = kEobEndOfFile;
            FlexRestart(g_pDataLexIn);
        } else {
            nResult = kEobLastMatch;
            g_pFlexBuffer->mStatus = kBufferEofPending;
        }
    } else {
        nResult = kEobContinueScan;
    }
    g_nFlexCharCount += nToMove;
    g_pFlexBuffer->mChars[g_nFlexCharCount] = '\0';
    g_pFlexBuffer->mChars[g_nFlexCharCount + 1] = '\0';
    g_pszDataLexText = g_pFlexBuffer->mChars;
    return nResult;
}

// Mark the matched text as the token: terminate it and remember the character it replaces.
inline void FlexSetToken(char *pBegin, char *pEnd) {
    g_pszDataLexText = pBegin;
    g_nDataLexLength = static_cast<int>(pEnd - pBegin);
    g_cFlexHoldChar = *pEnd;
    *pEnd = '\0';
    g_pFlexCursor = pEnd;
}

int DataLex() {
    if (g_nFlexNeedsInit != 0) {
        g_nFlexNeedsInit = 0;
        if (g_nFlexStartState == 0) {
            g_nFlexStartState = kFlexInitialState;
        }
        if (g_pDataLexIn == nullptr) {
            g_pDataLexIn = stdin;
        }
        if (g_pDataLexOut == nullptr) {
            g_pDataLexOut = stdout;
        }
        if (g_pFlexBuffer == nullptr) {
            g_pFlexBuffer = FlexBuffer::Create(g_pDataLexIn, kFlexBufferSize);
        }
        FlexLoadBufferState();
    }

    while (true) {
        char *pCursor = g_pFlexCursor;
        *pCursor = g_cFlexHoldChar;
        char *pBegin = pCursor;
        int nState = g_nFlexStartState;
        int nAction;

    match:
        do {
            const int nClass = kFlexClass[static_cast<unsigned char>(*pCursor)];
            if (kFlexAccept[nState] != 0) {
                g_nFlexLastAcceptState = nState;
                g_pFlexLastAcceptPos = pCursor;
            }
            nState = FlexStep(nState, nClass);
            ++pCursor;
        } while (kFlexBase[nState] != kFlexJamBase);

    findAction:
        nAction = kFlexAccept[nState];
        if (nAction == 0) {
            pCursor = g_pFlexLastAcceptPos;
            nState = g_nFlexLastAcceptState;
            nAction = kFlexAccept[nState];
        }
        FlexSetToken(pBegin, pCursor);

    doAction:
        switch (nAction) {
        case kActionBackUp:
            *pCursor = g_cFlexHoldChar;
            pCursor = g_pFlexLastAcceptPos;
            nState = g_nFlexLastAcceptState;
            goto findAction;
        case kActionNewline:
            ++g_nDataParseLineNumber;
            break;
        case kActionInt:
            return kDataTokenInt;
        case kActionFloat:
        case kActionExponent:
            return kDataTokenFloat;
        case kActionString:
            return kDataTokenString;
        case kActionSymbol:
            return kDataTokenSymbol;
        case kActionComment:
            // The last character matched goes back to the input.
            *pCursor = g_cFlexHoldChar;
            FlexSetToken(pBegin, &pCursor[-1]);
            break;
        case kActionOpen:
            return kDataTokenOpen;
        case kActionClose:
            return kDataTokenClose;
        case kActionDefine:
            return kDataTokenDefine;
        case kActionDefineEnd:
            return kDataTokenDefineEnd;
        case kActionMerge:
            return kDataTokenMerge;
        case kActionInclude:
            return kDataTokenInclude;
        case kActionWhitespace:
            break;
        case kActionEndOfInput:
            return kDataTokenEnd;
        case kActionEndOfBuffer: {
            const int nMatched = static_cast<int>(pCursor - g_pszDataLexText) - 1;
            *pCursor = g_cFlexHoldChar;
            if (g_pFlexBuffer->mStatus == kBufferNew) {
                g_nFlexCharCount = g_pFlexBuffer->mCharCount;
                g_pFlexBuffer->mInputFile = g_pDataLexIn;
                g_pFlexBuffer->mStatus = kBufferNormal;
            }
            if (g_pFlexCursor <= &g_pFlexBuffer->mChars[g_nFlexCharCount]) {
                g_pFlexCursor = &g_pszDataLexText[nMatched];
                nState = FlexGetPreviousState();
                const int nNextState = FlexTryNulTransition(nState);
                pBegin = g_pszDataLexText;
                if (nNextState != 0) {
                    pCursor = ++g_pFlexCursor;
                    nState = nNextState;
                    goto match;
                }
                pCursor = g_pFlexCursor;
                goto findAction;
            }
            switch (FlexGetNextBuffer()) {
            case kEobEndOfFile:
                g_nFlexSwitchedOnEof = 0;
                g_pFlexCursor = g_pszDataLexText;
                nAction = (g_nFlexStartState - 1) / 2 + kActionEndOfInput;
                goto doAction;
            case kEobContinueScan:
                g_pFlexCursor = &g_pszDataLexText[nMatched];
                nState = FlexGetPreviousState();
                pCursor = g_pFlexCursor;
                pBegin = g_pszDataLexText;
                goto match;
            case kEobLastMatch:
                g_pFlexCursor = &g_pFlexBuffer->mChars[g_nFlexCharCount];
                nState = FlexGetPreviousState();
                pCursor = g_pFlexCursor;
                pBegin = g_pszDataLexText;
                goto findAction;
            default:
                break;
            }
            break;
        }
        default:
            FlexFatalError("fatal flex scanner internal error--no action found");
            break;
        }
    }
}
