#pragma once

#include <vector>

#include "os/string.h"
#include "script/dataarray.h"

/**
 * Registry of the C++ routines a script command can call by name.
 *
 * The class name is inferred.
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

    /**
     * Run a script command through the routine registered for its name.
     *
     * An unregistered name is passed to DebugPrint().
     *
     * @param pCommand The command, with the command name as its first node.
     * @return Whether a routine was registered for the name.
     * @ghidraAddress NTSC-U/C: 0x00298600
     * @ghidraAddress PAL: 0x002a2210
     */
    static bool Dispatch(DataArray *pCommand);

private:
    // One registered command.
    struct Entry {
        String mName;     // The command name.
        Handler mHandler; // The routine the command calls.
        void *mUserData;  // The value passed back to the handler.
    };

    // The registered commands, in the order Register() added them.
    static std::vector<Entry> sFunctions;
};
