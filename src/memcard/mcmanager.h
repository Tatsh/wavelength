#pragma once

#include "memcard/memcarduser.h"

/**
 * Front end of the memory card tasks, which runs one MCTask subclass at a time on behalf of a
 * MemcardUser.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the MCTask classes it
 * runs. Only the members its callers here use are declared.
 */
class MCManager {
public:
    /**
     * Start saving a block of memory to a file of a memory card.
     *
     * The task reports to the user when it ends.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pszName The name of the file.
     * @param pData The data.
     * @param nSize The number of bytes.
     * @param bReserved Passed by every caller. The body does not read it.
     * @ghidraAddress NTSC-U/C: 0x0015cb70
     * @ghidraAddress PAL: 0x0015e360
     */
    void SaveFile(MemcardUser *pUser,
                  int nSlot,
                  const char *pszName,
                  const void *pData,
                  int nSize,
                  bool bReserved);
};

/**
 * The memory card front end.
 *
 * @ghidraAddress NTSC-U/C: 0x004362b0
 */
extern MCManager TheMCManager;
