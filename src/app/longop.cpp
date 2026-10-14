#include "app/longop.h"

LongOperationProc g_pfnLongOperationPollProc;
LongOperationProc g_pfnLongOperationDrawProc;

void SetLongOperationPollProc(LongOperationProc pfnPoll) {
    g_pfnLongOperationPollProc = pfnPoll;
}

void RunLongOperationPollProc() {
    if (g_pfnLongOperationPollProc != nullptr) {
        g_pfnLongOperationPollProc();
    }
}

void SetLongOperationDrawProc(LongOperationProc pfnDraw) {
    g_pfnLongOperationDrawProc = pfnDraw;
}

void RunLongOperationDrawProc() {
    if (g_pfnLongOperationDrawProc != nullptr) {
        g_pfnLongOperationDrawProc();
    }
}
