#include "os/debug.h"

#include <stdio.h>

#include "os/filestream.h"

namespace {

constexpr char kNewline = '\n';

} // namespace

Debug::Debug() {
    mEnabled = 1;
    mReserved08 = 0;
    mReserved0C = 0;
    mLog = nullptr;
    mReserved18 = 0;
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

void Debug::OpenLog(const char *pszFile) {
    CloseLog();
    mLog = new FileStream(pszFile, true, true, 0);
    if (mLog->Fail()) {
        DebugNotify("Couldn't open log %s", pszFile);
        delete mLog;
        mLog = nullptr;
    }
}

void Debug::CloseLog() {
    delete mLog;
    mLog = nullptr;
}

// The unit's static initialiser constructs and destroys TheDebug, and the global constructor and
// the global destructor of the unit run the initialiser.
// NTSC-U/C: 0x002890a8, PAL: 0x002928a0
// NTSC-U/C: 0x002890e8, PAL: 0x002928e0
// NTSC-U/C: 0x00289108, PAL: 0x00292900
Debug TheDebug;
