#include "rt/rtmutex.h"

#include <stddef.h>

#include <eekernel.h>

enum {
    kMutexResultKernelError = 1,
    kMutexResultInvalid = 2,
    kMutexResultNotOwner = 4,
    kMutexResultNotLocked = 5,
    kMutexSemaphoreCount = 1,
};

int RtCreateMutex(RtMutex *pMutex) {
    int nResult;

    if (pMutex == NULL) {
        return kMutexResultInvalid;
    }
    pMutex->bCreated = 0;
    pMutex->nLockCount = 0;
    pMutex->nOwnerThread = 0;
    nResult = RtCreateMutexSemaphore(pMutex);
    if (nResult != 0) {
        return nResult;
    }
    pMutex->bCreated = 1;
    return 0;
}

int RtDeleteMutex(RtMutex *pMutex) {
    int nResult;

    if (pMutex == NULL || pMutex->bCreated != 1) {
        return kMutexResultInvalid;
    }
    nResult = RtDeleteMutexSemaphore(pMutex);
    if (nResult != 0) {
        return nResult;
    }
    pMutex->bCreated = 0;
    return 0;
}

int RtLockMutex(RtMutex *pMutex, int *pnLockCount) {
    int nThread;
    int nResult;

    if (pMutex == NULL || pMutex->bCreated != 1) {
        return kMutexResultInvalid;
    }
    nResult = RtGetCurrentThread(&nThread);
    if (nResult != 0) {
        return nResult;
    }
    if (pMutex->nLockCount == 0 || nThread != pMutex->nOwnerThread) {
        nResult = RtWaitMutexSemaphore(pMutex);
        if (nResult != 0) {
            return nResult;
        }
        nResult = RtGetCurrentThread(&pMutex->nOwnerThread);
        if (nResult != 0) {
            return nResult;
        }
    }
    ++pMutex->nLockCount;
    if (pnLockCount != NULL) {
        *pnLockCount = pMutex->nLockCount;
    }
    return 0;
}

int RtUnlockMutex(RtMutex *pMutex, int *pnLockCount) {
    int nThread;
    int nResult;

    if (pMutex == NULL || pMutex->bCreated != 1) {
        return kMutexResultInvalid;
    }
    nResult = RtGetCurrentThread(&nThread);
    if (nResult != 0) {
        return nResult;
    }
    if (nThread != pMutex->nOwnerThread) {
        return kMutexResultNotOwner;
    }
    if (pMutex->nLockCount == 0) {
        return kMutexResultNotLocked;
    }
    --pMutex->nLockCount;
    if (pnLockCount != NULL) {
        *pnLockCount = pMutex->nLockCount;
    }
    if (pMutex->nLockCount != 0) {
        return 0;
    }
    pMutex->nOwnerThread = 0;
    nResult = RtSignalMutexSemaphore(pMutex);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtCreateMutexSemaphore(RtMutex *pMutex) {
    struct SemaParam param;
    int nSemaphore;

    if (pMutex == NULL) {
        return kMutexResultInvalid;
    }
    pMutex->nReserved = 0;
    // Yes, the binary leaves the other fields of the parameters uninitialised.
    param.maxCount = kMutexSemaphoreCount;
    param.initCount = kMutexSemaphoreCount;
    nSemaphore = CreateSema(&param);
    if (nSemaphore == -1) {
        return kMutexResultKernelError;
    }
    pMutex->nSemaphore = nSemaphore;
    return 0;
}

int RtDeleteMutexSemaphore(RtMutex *pMutex) {
    if (pMutex == NULL) {
        return kMutexResultInvalid;
    }
    return DeleteSema(pMutex->nSemaphore) == -1;
}

int RtWaitMutexSemaphore(RtMutex *pMutex) {
    if (pMutex == NULL) {
        return kMutexResultInvalid;
    }
    return WaitSema(pMutex->nSemaphore) == -1;
}

int RtSignalMutexSemaphore(RtMutex *pMutex) {
    if (pMutex == NULL) {
        return kMutexResultInvalid;
    }
    return SignalSema(pMutex->nSemaphore) == -1;
}

int RtGetCurrentThread(int *pnThread) {
    if (pnThread == NULL) {
        return kMutexResultInvalid;
    }
    *pnThread = GetThreadId();
    return 0;
}
