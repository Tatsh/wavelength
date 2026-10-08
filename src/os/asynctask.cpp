#include "os/asynctask.h"

#include <string.h>

#include <eekernel.h>

#include "os/debug.h"

namespace {

// A queued task and its completion.
struct AsyncTask {
    AsyncTaskFunc mTask;     // The task, or null for a free entry.
    AsyncTaskDoneFunc mDone; // The completion.
    int mResult;             // The result of the task.
};

constexpr int kTaskCount = 5;
constexpr int kStackSize = 0x4000;
constexpr int kThreadPriority = 1;
constexpr int kSemaMaxCount = 1;
constexpr int kSemaInitCount = 0;

// NTSC-U/C: 0x004919c0
AsyncTask sTasks[kTaskCount];

// NTSC-U/C: 0x004819c0
alignas(16) unsigned char sStack[kStackSize];

// NTSC-U/C: 0x003b2240
int sSema;

// NTSC-U/C: 0x003b2244
int sThread;

// NTSC-U/C: 0x003b2248
volatile int sTaskDone;

// NTSC-U/C: 0x003b224c
int sTaskPending;

// NTSC-U/C: 0x003b2250
int sReadIndex;

// NTSC-U/C: 0x003b2254
int sWriteIndex;

// NTSC-U/C: 0x0028ce98, PAL: 0x00296878
void AsyncTaskThread([[maybe_unused]] void *pArg) {
    for (;;) {
        WaitSema(sSema);
        const int nResult = sTasks[sReadIndex].mTask();
        sTaskDone = 1;
        sTasks[sReadIndex].mResult = nResult;
    }
}

} // namespace

void InitAsyncTasks() {
    memset(sTasks, 0, sizeof(sTasks));
    sReadIndex = 0;
    sWriteIndex = 0;

    SemaParam sema;
    sema.maxCount = kSemaMaxCount;
    sema.initCount = kSemaInitCount;
    sSema = CreateSema(&sema);
    if (sSema <= 0) {
        DebugPrint("CreateSema() failed.(%d)\n", sSema);
        sSema = 0;
        return;
    }

    ThreadParam thread;
    thread.entry = AsyncTaskThread;
    thread.stack = sStack;
    thread.stackSize = kStackSize;
    thread.gpReg = _gp;
    thread.initPriority = kThreadPriority;
    sThread = CreateThread(&thread);
    if (sThread <= 0) {
        DebugPrint("CreateThread() failed.(%d)\n", sThread);
        return;
    }
    const int nStarted = StartThread(sThread, nullptr);
    if (nStarted < 0) {
        DebugPrint("StartThread() failed.(%d)\n", nStarted);
        DeleteThread(sThread);
        sThread = 0;
    }
}

void TerminateAsyncTasks() {
    if (sThread != 0) {
        DeleteThread(sThread);
    }
}

void EnqueueAsyncTask(AsyncTaskFunc pfnTask, AsyncTaskDoneFunc pfnDone) {
    AsyncTask &task = sTasks[sWriteIndex];
    ++sWriteIndex;
    task.mTask = pfnTask;
    task.mDone = pfnDone;
    if (sWriteIndex >= kTaskCount) {
        sWriteIndex = 0;
    }
}

void PollAsyncTasks() {
    if (sTaskDone != 0) {
        AsyncTask &task = sTasks[sReadIndex];
        if (task.mTask != nullptr) {
            sTaskPending = 0;
            sTaskDone = 0;
            ++sReadIndex;
            if (sReadIndex >= kTaskCount) {
                sReadIndex = 0;
            }
            task.mTask = nullptr;
            task.mDone(task.mResult);
        }
    }
    if (sTaskPending == 0 && sTasks[sReadIndex].mTask != nullptr) {
        sTaskPending = 1;
        SignalSema(sSema);
    }
}
