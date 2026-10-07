#pragma once

#include "memcard/memcarddirentry.h"

/**
 * Receiver of the result of one memory card command.
 *
 * The RTTI includes the class name and records no base. The vptr is the only member. MemcardPoll()
 * reports a finished command through the method for that command, to the handler the command was
 * issued for. Every method does nothing here, and each MemcardTask overrides the one for the
 * command it issues.
 */
class MemcardCBHandler {
public:
    /**
     * Release the handler.
     *
     * @ghidraAddress NTSC-U/C: 0x00351320
     * @ghidraAddress PAL: 0x003be598
     */
    virtual ~MemcardCBHandler() {
    }

    /**
     * Receive the result of MemcardGetInfo().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351350
     */
    virtual void OnGetInfo([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the free entries of a directory. The game issues no such command.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351358
     */
    virtual void OnGetEntSpace([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardOpen().
     *
     * @param nResult The file descriptor, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x00351360
     */
    virtual void OnOpen([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardClose().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351368
     */
    virtual void OnClose([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardRead().
     *
     * @param nResult The number of bytes read, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x00351370
     */
    virtual void OnRead([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardWrite().
     *
     * @param nResult The number of bytes written, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x00351378
     */
    virtual void OnWrite([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardSeek().
     *
     * @param nResult The new position, or a negative library error.
     * @ghidraAddress NTSC-U/C: 0x00351380
     */
    virtual void OnSeek([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardMkdir().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351388
     */
    virtual void OnMkdir([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of a rename. The game issues no such command.
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351390
     */
    virtual void OnRename([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardDelete().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x00351398
     */
    virtual void OnDelete([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardFormat().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x003513a0
     */
    virtual void OnFormat([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardUnformat().
     *
     * @param nResult The library's result.
     * @ghidraAddress NTSC-U/C: 0x003513a8
     */
    virtual void OnUnformat([[maybe_unused]] int nResult) {
    }

    /**
     * Receive the result of MemcardGetDir().
     *
     * @param nResult The number of entries, or a negative library error.
     * @param pEntries The entries. The memory card layer retains them.
     * @ghidraAddress NTSC-U/C: 0x003513b0
     */
    virtual void OnGetDir([[maybe_unused]] int nResult,
                          [[maybe_unused]] MemcardDirEntry *pEntries) {
    }
};
