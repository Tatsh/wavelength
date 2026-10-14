#include "rt/rtringbuffer.h"

#include <stddef.h>
#include <string.h>

#include "rt/rtmemory.h"

enum {
    kRingResultShort = 1,
    kRingResultInvalid = 2,
    kRingResultNoMemory = 3,
    kRingResultNoScratch = 4,
};

int RtInitRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize, unsigned char *pbScratch) {
    if (pBuffer == NULL || uSize == 0) {
        return kRingResultInvalid;
    }
    memset(pBuffer, 0, sizeof(RtRingBuffer));
    if (RtAllocZeroedBlock((void **)&pBuffer->pbData, uSize) != 0) {
        return kRingResultNoMemory;
    }
    if (pBuffer->pbData == NULL) {
        return kRingResultNoMemory;
    }
    pBuffer->pbScratch = pbScratch;
    pBuffer->uSize = uSize;
    pBuffer->uRead = 0;
    pBuffer->uWrite = 0;
    pBuffer->uUsed = 0;
    return 0;
}

int RtDestroyRingBuffer(RtRingBuffer *pBuffer) {
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    if (pBuffer->pbData != NULL) {
        RtFreeBlock((void **)&pBuffer->pbData);
    }
    pBuffer->uUsed = 0;
    pBuffer->uRead = 0;
    pBuffer->uWrite = 0;
    pBuffer->uSize = 0;
    return 0;
}

int RtResetRingBuffer(RtRingBuffer *pBuffer) {
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    pBuffer->uUsed = 0;
    pBuffer->uRead = 0;
    pBuffer->uWrite = 0;
    return 0;
}

int RtConsumeRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize) {
    unsigned int uUsed;
    unsigned int uToEnd;
    int nResult;

    uUsed = 0;
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    if (uSize == 0) {
        return 0;
    }
    nResult = RtGetRingBufferUsed(pBuffer, &uUsed);
    if (nResult != 0) {
        return nResult;
    }
    if (uUsed < uSize) {
        return kRingResultShort;
    }
    uToEnd = pBuffer->uSize - pBuffer->uRead;
    if (uSize < uToEnd) {
        pBuffer->uRead += uSize;
    } else {
        pBuffer->uRead = uSize - uToEnd;
    }
    pBuffer->uUsed -= uSize;
    nResult = RtCompactRingBuffer(pBuffer, 0);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtCommitRingBuffer(RtRingBuffer *pBuffer, unsigned int uSize) {
    unsigned int uFree;
    unsigned int uToEnd;
    int nResult;

    uFree = 0;
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    if (uSize == 0) {
        return 0;
    }
    nResult = RtGetRingBufferFree(pBuffer, &uFree);
    if (nResult != 0) {
        return nResult;
    }
    if (uFree < uSize) {
        return kRingResultShort;
    }
    uToEnd = pBuffer->uSize - pBuffer->uWrite;
    if (uSize < uToEnd) {
        pBuffer->uWrite += uSize;
    } else {
        pBuffer->uWrite = uSize - uToEnd;
    }
    pBuffer->uUsed += uSize;
    nResult = RtCompactRingBuffer(pBuffer, 0);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtWriteRingBuffer(RtRingBuffer *pBuffer, const void *pData, unsigned int uSize) {
    unsigned int uFree;
    unsigned int uToEnd;
    int nResult;

    uFree = 0;
    if (pBuffer == NULL || pData == NULL || uSize == 0) {
        return kRingResultInvalid;
    }
    nResult = RtGetRingBufferFree(pBuffer, &uFree);
    if (nResult != 0) {
        return nResult;
    }
    if (uFree < uSize) {
        return kRingResultShort;
    }
    uToEnd = pBuffer->uSize - pBuffer->uWrite;
    if (uToEnd >= uSize) {
        memcpy(&pBuffer->pbData[pBuffer->uWrite], pData, uSize);
        pBuffer->uUsed += uSize;
        pBuffer->uWrite += uSize;
        if (pBuffer->uWrite == pBuffer->uSize) {
            pBuffer->uWrite = 0;
        }
        return 0;
    }
    uSize -= uToEnd;
    memcpy(&pBuffer->pbData[pBuffer->uWrite], pData, uToEnd);
    pBuffer->uUsed += uToEnd;
    // Yes, the binary copies the wrapped part from the start of the data again.
    memcpy(pBuffer->pbData, pData, uSize);
    pBuffer->uWrite = uSize;
    pBuffer->uUsed += uSize;
    return 0;
}

int RtPeekRingBuffer(void *pDest, const RtRingBuffer *pBuffer, unsigned int uSize) {
    unsigned char *pbDest;
    unsigned int uUsed;
    unsigned int uReadable;
    int nResult;

    pbDest = pDest;
    uUsed = 0;
    uReadable = 0;
    if (pbDest == NULL || pBuffer == NULL || uSize == 0) {
        return kRingResultInvalid;
    }
    nResult = RtGetRingBufferUsed(pBuffer, &uUsed);
    if (nResult != 0) {
        return nResult;
    }
    if (uUsed < uSize) {
        return kRingResultShort;
    }
    nResult = RtGetRingBufferReadable(pBuffer, &uReadable);
    if (nResult != 0) {
        return nResult;
    }
    if (uReadable >= uSize) {
        memcpy(pbDest, &pBuffer->pbData[pBuffer->uRead], uSize);
        return 0;
    }
    memcpy(pbDest, &pBuffer->pbData[pBuffer->uRead], uReadable);
    memcpy(&pbDest[uReadable], pBuffer->pbData, uSize - uReadable);
    return 0;
}

int RtCompactRingBuffer(RtRingBuffer *pBuffer, unsigned char bMove) {
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    if (bMove && pBuffer->pbScratch == NULL) {
        return kRingResultNoScratch;
    }
    return RtCompactRingBufferWith(pBuffer, bMove, pBuffer->pbScratch);
}

int RtCompactRingBufferWith(RtRingBuffer *pBuffer, unsigned char bMove, unsigned char *pbScratch) {
    unsigned int uUsed;
    int nResult;

    uUsed = 0;
    if (pBuffer == NULL) {
        return kRingResultInvalid;
    }
    if (bMove && pbScratch == NULL) {
        return kRingResultInvalid;
    }
    if (pBuffer->uRead == 0) {
        return 0;
    }
    nResult = RtGetRingBufferUsed(pBuffer, &uUsed);
    if (nResult != 0) {
        return nResult;
    }
    if (uUsed == 0) {
        pBuffer->uWrite = 0;
        pBuffer->uRead = 0;
        return 0;
    }
    if (bMove != 1) {
        return 0;
    }
    nResult = RtPeekRingBuffer(pbScratch, pBuffer, uUsed);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtResetRingBuffer(pBuffer);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtWriteRingBuffer(pBuffer, pbScratch, uUsed);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtGetRingBufferUsed(const RtRingBuffer *pBuffer, unsigned int *puSize) {
    if (pBuffer == NULL || puSize == NULL) {
        return kRingResultInvalid;
    }
    *puSize = pBuffer->uUsed;
    return 0;
}

int RtGetRingBufferReadable(const RtRingBuffer *pBuffer, unsigned int *puSize) {
    if (pBuffer == NULL || puSize == NULL) {
        return kRingResultInvalid;
    }
    if (pBuffer->uUsed == 0) {
        *puSize = 0;
    } else if (pBuffer->uRead < pBuffer->uWrite) {
        *puSize = pBuffer->uWrite - pBuffer->uRead;
    } else {
        *puSize = pBuffer->uSize - pBuffer->uRead;
    }
    return 0;
}

int RtGetRingBufferFree(const RtRingBuffer *pBuffer, unsigned int *puSize) {
    if (pBuffer == NULL || puSize == NULL) {
        return kRingResultInvalid;
    }
    *puSize = pBuffer->uSize - pBuffer->uUsed;
    return 0;
}

int RtGetRingBufferWritable(const RtRingBuffer *pBuffer, unsigned int *puSize) {
    if (pBuffer == NULL || puSize == NULL) {
        return kRingResultInvalid;
    }
    if (pBuffer->uUsed == pBuffer->uSize) {
        *puSize = 0;
    } else if (pBuffer->uWrite < pBuffer->uRead) {
        *puSize = pBuffer->uRead - pBuffer->uWrite;
    } else {
        *puSize = pBuffer->uSize - pBuffer->uWrite;
    }
    return 0;
}

int RtGetRingBufferReadPointer(const RtRingBuffer *pBuffer, unsigned char **ppbRead) {
    unsigned int uUsed;
    int nResult;

    uUsed = 0;
    if (pBuffer == NULL || ppbRead == NULL) {
        return kRingResultInvalid;
    }
    nResult = RtGetRingBufferUsed(pBuffer, &uUsed);
    if (nResult != 0) {
        return nResult;
    }
    if (uUsed == 0) {
        return kRingResultShort;
    }
    *ppbRead = &pBuffer->pbData[pBuffer->uRead];
    return 0;
}

int RtGetRingBufferWritePointer(const RtRingBuffer *pBuffer, unsigned char **ppbWrite) {
    unsigned int uFree;
    int nResult;

    uFree = 0;
    if (pBuffer == NULL || ppbWrite == NULL) {
        return kRingResultInvalid;
    }
    nResult = RtGetRingBufferFree(pBuffer, &uFree);
    if (nResult != 0) {
        return nResult;
    }
    if (uFree == 0) {
        return kRingResultShort;
    }
    *ppbWrite = &pBuffer->pbData[pBuffer->uWrite];
    return 0;
}

int RtIsRingBufferWrapped(const RtRingBuffer *pBuffer, unsigned char *pbWrapped) {
    if (pBuffer == NULL || pbWrapped == NULL) {
        return kRingResultInvalid;
    }
    if (pBuffer->uRead < pBuffer->uWrite || pBuffer->uUsed == 0) {
        *pbWrapped = 0;
    } else if (pBuffer->uRead != 0) {
        *pbWrapped = 1;
    } else {
        *pbWrapped = 0;
    }
    return 0;
}
