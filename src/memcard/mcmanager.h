#pragma once

#include "game/playerprofile.h"
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
     * Read the memory card configuration and build the tasks.
     *
     * Metagame::Init() calls it.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c400
     * @ghidraAddress PAL: 0x0015dbf0
     */
    void Init();

    /**
     * Destroy the tasks Init() built.
     *
     * @ghidraAddress NTSC-U/C: 0x0015c550
     * @ghidraAddress PAL: 0x0015dd40
     */
    void Terminate();

    /**
     * Advance the running task by one frame.
     *
     * Metagame::Update() calls it every frame.
     *
     * @ghidraAddress NTSC-U/C: 0x0015ccd8
     * @ghidraAddress PAL: 0x0015e4c8
     */
    void Poll();

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

    /**
     * Start saving the Freq of a profile to a memory card.
     *
     * The task reports to the user when it ends.
     *
     * @param pUser The user to report to.
     * @param nSlot The memory card slot.
     * @param pProfile The profile.
     * @param pszName The name the Freq is saved under.
     * @param nOverwrite Non-zero when a saved Freq of the same name may be replaced.
     * @ghidraAddress NTSC-U/C: 0x0015c7e8
     * @ghidraAddress PAL: 0x0015dfd8
     */
    void SaveFreq(MemcardUser *pUser,
                  int nSlot,
                  PlayerProfile *pProfile,
                  const char *pszName,
                  int nOverwrite);

    /**
     * Report the localised name of a memory card slot.
     *
     * @param nSlot The memory card slot.
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0015d050
     * @ghidraAddress PAL: 0x0015e840
     */
    const char *GetSlotName(int nSlot);
};

/**
 * The memory card front end.
 *
 * @ghidraAddress NTSC-U/C: 0x004362b0
 */
extern MCManager TheMCManager;
