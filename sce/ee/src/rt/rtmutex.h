#ifndef RTMUTEX_H
#define RTMUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Recursive lock of the SCE-RT library over a kernel semaphore. The routines report 0 for success,
 * 1 when the kernel fails, and 2 for a null lock or one that does not exist.
 */

/** A recursive lock. */
typedef struct {
    unsigned char bCreated; /*!< Non-zero once the semaphore exists. */
    int nLockCount;         /*!< The number of times the owner holds the lock. */
    int nOwnerThread;       /*!< The thread that holds the lock, or 0. */
    int nSemaphore;         /*!< The kernel semaphore. */
    int nReserved;          /*!< Cleared when the semaphore is created, and not read. */
} RtMutex;

/**
 * Create the semaphore of a lock and mark the lock as existing.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aaa48
 * @ghidraAddress PAL: 0x002b49b8
 */
int RtCreateMutex(RtMutex *pMutex);

/**
 * Delete the semaphore of a lock.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aaa94
 * @ghidraAddress PAL: 0x002b4a04
 */
int RtDeleteMutex(RtMutex *pMutex);

/**
 * Take a lock, waiting for it when another thread holds it.
 *
 * @param pMutex The lock.
 * @param pnLockCount Receives the number of times the thread now holds the lock, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aaae4
 * @ghidraAddress PAL: 0x002b4a54
 */
int RtLockMutex(RtMutex *pMutex, int *pnLockCount);

/**
 * Release a lock once.
 *
 * @param pMutex The lock.
 * @param pnLockCount Receives the number of times the thread still holds the lock, or null.
 * @return 0, an error, 4 when another thread holds the lock, or 5 when it is not held.
 * @ghidraAddress NTSC-U/C: 0x002aab8c
 * @ghidraAddress PAL: 0x002b4afc
 */
int RtUnlockMutex(RtMutex *pMutex, int *pnLockCount);

/**
 * Create the kernel semaphore of a lock.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aac34
 * @ghidraAddress PAL: 0x002b4ba4
 */
int RtCreateMutexSemaphore(RtMutex *pMutex);

/**
 * Delete the kernel semaphore of a lock.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aac8c
 * @ghidraAddress PAL: 0x002b4bfc
 */
int RtDeleteMutexSemaphore(RtMutex *pMutex);

/**
 * Wait for the kernel semaphore of a lock.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aacb8
 * @ghidraAddress PAL: 0x002b4c28
 */
int RtWaitMutexSemaphore(RtMutex *pMutex);

/**
 * Signal the kernel semaphore of a lock.
 *
 * @param pMutex The lock.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002aace4
 * @ghidraAddress PAL: 0x002b4c54
 */
int RtSignalMutexSemaphore(RtMutex *pMutex);

/**
 * Report the running thread.
 *
 * @param pnThread Receives the thread.
 * @return 0, or 2 when pnThread is null.
 * @ghidraAddress NTSC-U/C: 0x002aad10
 * @ghidraAddress PAL: 0x002b4c80
 */
int RtGetCurrentThread(int *pnThread);

#ifdef __cplusplus
}
#endif

#endif
