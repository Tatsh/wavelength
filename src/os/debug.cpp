#include "os/debug.h"

#include <stdio.h>

#ifdef VIDEO_STANDARD_PAL
#include <sifdev.h>
#else
#include "os/filestream.h"
#endif

namespace {

constexpr char kNewline = '\n';

#ifdef VIDEO_STANDARD_PAL
// The File::New() modes of a log file that is added to and of one that is emptied first.
constexpr int kAppendLogMode = SCE_WRONLY | SCE_CREAT | SCE_APPEND;
constexpr int kNewLogMode = SCE_WRONLY | SCE_CREAT | SCE_TRUNC;
#endif

} // namespace

Debug::Debug() {
    mEnabled = 1;
    mReserved08 = 0;
    mReserved0C = 0;
    mLog = nullptr;
    mModalCallback = nullptr;
}

Debug::~Debug() {
    CloseLog();
}

void Debug::Print(const char *pszText) {
    if (mEnabled == 0) {
        return;
    }
    if (mLog == nullptr) {
        printf("%s", pszText);
        return;
    }
    const char crlf[] = {'\r', '\n'};
    for (const char *p = pszText; *p != '\0'; ++p) {
        if (*p == kNewline) {
            mLog->Write(crlf, sizeof(crlf));
        } else {
            mLog->Write(p, 1);
        }
    }
}

#ifdef VIDEO_STANDARD_PAL
void Debug::OpenLog(const char *pszFile, bool bAppend) {
    CloseLog();
    mLog = File::New(pszFile, bAppend ? kAppendLogMode : kNewLogMode, 0);
    if (mLog == nullptr) {
        DebugNotify("Couldn't open log %s", pszFile);
    }
}
#else
void Debug::OpenLog(const char *pszFile) {
    CloseLog();
    mLog = new FileStream(pszFile, true, true, 0);
    if (mLog->Fail()) {
        DebugNotify("Couldn't open log %s", pszFile);
        delete mLog;
        mLog = nullptr;
    }
}
#endif

void Debug::CloseLog() {
    delete mLog;
    mLog = nullptr;
}

void DebugPrint([[maybe_unused]] const char *pszFormat, ...) {
}

void DebugWarn([[maybe_unused]] const char *pszFormat, ...) {
}

void DebugNotify([[maybe_unused]] const char *pszFormat, ...) {
}

void DebugError([[maybe_unused]] const char *pszFormat, ...) {
}

// The unit's static initialiser constructs and destroys TheDebug, and the global constructor and
// the global destructor of the unit run the initialiser.
// NTSC-U/C: 0x002890a8, PAL: 0x002928a0
// NTSC-U/C: 0x002890e8, PAL: 0x002928e0
// NTSC-U/C: 0x00289108, PAL: 0x00292900
Debug TheDebug;
