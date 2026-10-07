#pragma once

#include "script/dataarray.h"

/**
 * Registry of the C++ routines a script command can call by name.
 *
 * Each entry is 0x1c bytes, a 0x14-byte string with the command name followed by the handler and
 * the user value. The entries live in one vector at `0x005152e8`. The class name is inferred.
 */
class ScriptFunction {
public:
    /**
     * Routine a script command calls.
     *
     * The dispatcher ignores the return value.
     *
     * @param pCommand The command, with the command name as its first node.
     * @param pUserData The value given to Register().
     */
    typedef void (*Handler)(DataArray *pCommand, void *pUserData);

    /**
     * Add a command to the registry.
     *
     * @param pfnHandler The routine the command calls.
     * @param pszName The command name.
     * @param pUserData A value passed back to the handler unchanged.
     * @ghidraAddress NTSC-U/C: 0x002982c8
     * @ghidraAddress PAL: 0x002a1ed8
     */
    static void Register(Handler pfnHandler, const char *pszName, void *pUserData);

    /**
     * Remove the command that calls a routine.
     *
     * @param pfnHandler The routine Register() was given.
     * @ghidraAddress NTSC-U/C: 0x00298520
     * @ghidraAddress PAL: 0x002a2130
     */
    static void Unregister(Handler pfnHandler);
};
