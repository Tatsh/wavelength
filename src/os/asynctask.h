#pragma once

/**
 * Routine the worker thread runs.
 *
 * @return The result passed to the routine's completion.
 */
typedef int (*AsyncTaskFunc)();

/**
 * Routine the main thread runs once the worker thread has run the task.
 *
 * @param nResult The result of the task.
 */
typedef void (*AsyncTaskDoneFunc)(int nResult);

/**
 * Start the worker thread that runs queued tasks one at a time, at priority 1. A failure to create
 * the semaphore or the thread is reported and leaves no worker.
 *
 * The name is inferred. SystemInit() calls it.
 *
 * @ghidraAddress NTSC-U/C: 0x0028cd60
 * @ghidraAddress PAL: 0x00296740
 */
void InitAsyncTasks();

/**
 * Delete the worker thread. SystemTerminate() calls it.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0028ce70
 * @ghidraAddress PAL: 0x00296850
 */
void TerminateAsyncTasks();

/**
 * Queue a task for the worker thread. The queue holds five tasks and wraps without checking for
 * room.
 *
 * The name is inferred.
 *
 * @param pfnTask The task.
 * @param pfnDone The completion, which PollAsyncTasks() runs on the main thread.
 * @ghidraAddress NTSC-U/C: 0x0028cf18
 * @ghidraAddress PAL: 0x002968f8
 */
void EnqueueAsyncTask(AsyncTaskFunc pfnTask, AsyncTaskDoneFunc pfnDone);

/**
 * Run the completion of a finished task, and wake the worker for the next queued task. SystemPoll()
 * calls it once per frame.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0028cf60
 * @ghidraAddress PAL: 0x00296940
 */
void PollAsyncTasks();
