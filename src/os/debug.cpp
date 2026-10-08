#include "os/debug.h"

#include <stdio.h>

#include "os/filestream.h"

namespace {

constexpr char kNewline = '\n';
constexpr char kLogNewline[] = {'\r', '\n'};

} // namespace

Debug::Debug() : mReserved08(0), mReserved0C(0), mEnabled(1), mLog(nullptr), mReserved18(0) {
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
    for (const char *p = pszText; *p != '\0'; ++p) {
        if (*p == kNewline) {
            mLog->Write(kLogNewline, sizeof(kLogNewline));
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
