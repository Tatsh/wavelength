#include "os/Debug.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <windows.h>

namespace {

// 0x1003f12c
char gFormatBuffer[2048];

// 0x1003e92c
char gModalMessage[2048];

const int kFormatBufferChars = 2047;

// 0x1000a2c0
char *FormatStringV(const char *fmt, va_list args) {
    _vsnprintf(gFormatBuffer, kFormatBufferChars, fmt, args);
    return gFormatBuffer;
}

} // namespace

Debug TheDebug;

void DebugPrint(const char *fmt, ...) {
}

Debug::Debug()
    : mFailing(false), mThrowOnFail(false), mEnabled(true), mLog(NULL), mModalCallback(NULL) {
}

Debug::~Debug() {
    StopLog();
}

void Debug::Print(const char *str) {
    if (!mEnabled) {
        return;
    }
    if (mLog == NULL) {
        printf("%s", str);
        OutputDebugStringA(str);
        return;
    }
    const char lineEnd[] = { '\r', '\n' };
    for (; *str != '\0'; ++str) {
        if (*str == '\n') {
            mLog->Write(lineEnd, sizeof(lineEnd));
        } else {
            mLog->Write(str, 1);
        }
    }
}

void Debug::Fail(const char *fmt, ...) {
    if (mFailing) {
        return;
    }
    mFailing = true;
    va_list args;
    va_start(args, fmt);
    char *message = FormatStringV(fmt, args);
    va_end(args);
    if (mThrowOnFail) {
        throw message;
    }
    Modal(kModalFail, message);
}

void Debug::Notify(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *message = FormatStringV(fmt, args);
    va_end(args);
    Modal(kModalNotify, message);
}

void Debug::Modal(int type, const char *message) {
    strncpy(gModalMessage, message, sizeof(gModalMessage));
    gModalMessage[sizeof(gModalMessage) - 1] = '\0';
    if (type == kModalCancelable) {
        strcat(gModalMessage, "\n\nClick OK to continue, Cancel to break");
    }
    if (mModalCallback != NULL) {
        mModalCallback(type, gModalMessage);
    } else if (type == kModalFail) {
        MessageBoxA(NULL, gModalMessage, "Fail", MB_ICONERROR);
    } else if (type == kModalCancelable) {
        if (MessageBoxA(NULL, gModalMessage, "Notify", MB_OKCANCEL | MB_ICONEXCLAMATION) ==
            IDCANCEL) {
            type = kModalFail;
        }
    } else {
        MessageBoxA(NULL, gModalMessage, "Notify", MB_ICONEXCLAMATION);
    }
    if (type == kModalFail) {
        exit(0);
    }
}

void Debug::StartLog(const char *file) {
    StopLog();
    mLog = new FileStream(file, FileStream::kWrite, true, 0);
    if (mLog->Fail()) {
        TheDebug.Notify("Couldn't open log %s", file); // Yes, the global rather than this stream.
        delete mLog;
        mLog = NULL;
    }
}

void Debug::StopLog() {
    delete mLog;
    mLog = NULL;
}
