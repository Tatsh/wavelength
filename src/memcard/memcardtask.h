#pragma once

#include "memcard/memcardcbhandler.h"
#include "memcard/memcarddirentry.h"
#include "os/task.h"

/**
 * Task that issues one memory card command and ends when the command reports its result.
 *
 * The RTTI records the class as deriving from Task and from MemcardCBHandler at `+0x08`. The
 * object is 0x10 bytes. A subclass issues its command from OnStart(), and the handler method for
 * the command translates the library's result into sStatus and finishes the task. OnPoll() polls
 * the command in flight.
 */
class MemcardTask : public Task, public MemcardCBHandler {
public:
    /** Values of sStatus, the outcome every memory card task and user reports. */
    enum Status {
        kStatusOk = 0,           /*!< The work succeeded. */
        kStatusNoCard = 1,       /*!< No usable card is in the slot. */
        kStatusFull = 2,         /*!< The card lacks the space the work needs. */
        kStatusUnformatted = 3,  /*!< The card is not formatted. */
        kStatusChangedCard = 4,  /*!< Another card is in the slot than before. */
        kStatusExists = 5,       /*!< The file exists and may not be replaced. */
        kStatusLimit = 6,        /*!< The card includes as many saves of the kind as allowed. */
        kStatusFormatted = 7,    /*!< The card to format is formatted already. */
        kStatusNotFound = 8,     /*!< The file or the network configurations do not exist. */
        kStatusConfigsBad = 9,   /*!< The network configurations could not be read. */
        kStatusConfigsError = 10 /*!< Listing the network configurations failed otherwise. */
    };

    /** Construct an idle task for the first slot. */
    MemcardTask() : mPort(0) {
    }

    /**
     * Select the slot the command addresses.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x0015d848
     * @ghidraAddress PAL: 0x0015f060
     */
    void SetPort(int nPort);

    /**
     * Record a library result in sResult, and translate the result into sStatus.
     *
     * A result the translation does not cover is reported as `Unhandled memcard error code: %d`
     * and does not change sStatus.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x0015d788
     * @ghidraAddress PAL: 0x0015efa0
     */
    static void SetStatus(int nResult);

    /**
     * The file descriptor the last open reported.
     *
     * @ghidraAddress NTSC-U/C: 0x003af844
     */
    static int sFd;

    /**
     * The library result SetStatus() last translated.
     *
     * @ghidraAddress NTSC-U/C: 0x003af848
     */
    static int sResult;

    /**
     * The outcome of the last command, one of Status.
     *
     * @ghidraAddress NTSC-U/C: 0x003af84c
     */
    static int sStatus;

    /**
     * The entries the last directory listing reported.
     *
     * @ghidraAddress NTSC-U/C: 0x003af850
     */
    static MemcardDirEntry *sDirEntries;

    /**
     * The number of entries the last directory listing reported.
     *
     * @ghidraAddress NTSC-U/C: 0x003af854
     */
    static int sDirCount;

    int mPort; /*!< The memory card slot. */

protected:
    /**
     * Advance the memory card command in flight.
     *
     * @ghidraAddress NTSC-U/C: 0x0015d880
     * @ghidraAddress PAL: 0x0015f098
     */
    void OnPoll() override;
};
