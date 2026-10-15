#include <stddef.h>
#include <string.h>

#include "rt/rtdme.h"

// The field list the next registration takes.
// NTSC-U/C: 0x003b6190, PAL: 0x00435050
static int g_nDmeFieldCount;
// NTSC-U/C: 0x00519be0
static RtDmeField g_aDmeFields[RT_DME_FIELD_COUNT];

int RtDmeBeginFields(void) {
    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    g_nDmeFieldCount = 0;
    return 0;
}

int RtDmeAddField(const void *pRecord, const void *pField, int nSize, int nCount) {
    RtDmeField *pEntry;

    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pRecord == NULL || pField == NULL || nSize <= 0 || nCount <= 0) {
        return RT_DME_RESULT_INVALID;
    }
    // Yes, the binary does not check the list for room.
    pEntry = &g_aDmeFields[g_nDmeFieldCount++];
    pEntry->uCount = nCount;
    // Yes, the binary stores the record minus the field, the negated offset.
    pEntry->nOffset = (const unsigned char *)pRecord - (const unsigned char *)pField;
    pEntry->uSize = nSize;
    return 0;
}

int RtDmeRegisterMessageType(int *pnType, unsigned int nClass, rtDmeMessageHandler pfnHandler) {
    RtDmeMessageType *pType;

    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pnType == NULL || nClass >= RT_DME_MESSAGE_CLASS_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    *pnType = g_auDmeMessageTypeCount[nClass];
    // Yes, the binary does not check the allocation.
    pType = RtDmeAlloc(sizeof(RtDmeMessageType));
    pType->nFieldCount = g_nDmeFieldCount;
    g_apDmeMessageTypes[nClass][*pnType] = pType;
    memcpy(pType->aFields, g_aDmeFields, g_nDmeFieldCount * sizeof(RtDmeField));
    pType->pfnHandler = pfnHandler;
    ++g_auDmeMessageTypeCount[nClass];
    return 0;
}

int RtDmeRegisterApplicationMessage(int *pnType, rtDmeMessageHandler pfnHandler) {
    return RtDmeRegisterMessageType(pnType, RT_DME_CLASS_APPLICATION, pfnHandler);
}
