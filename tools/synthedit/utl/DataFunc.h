#pragma once

#include "utl/Data.h"
#include "utl/Str.h"

/**
 * Handler of a script command.
 *
 * @param args The command, its name first.
 * @param data The data the handler was registered with.
 */
typedef void (*DataFunc)(DataArray *args, void *data);

/**
 * Registered script command.
 *
 * The object is 0x1c bytes. The name comes from the RTTI of the vector of commands.
 */
struct FuncDesc {
    String mName;   /*!< The command's name. */
    DataFunc mFunc; /*!< The handler. */
    void *mData;    /*!< Passed to the handler. */
};

/**
 * Register a script command.
 *
 * @param func The handler.
 * @param name The command's name.
 * @param data Passed to the handler.
 * @ghidraAddress 0x10019cb0
 */
void DataRegisterFunc(DataFunc func, const char *name, void *data);

/**
 * Run the script command an array names in its first node.
 *
 * An unknown command prints a message.
 *
 * @param args The command.
 * @return Whether the command is registered.
 * @ghidraAddress 0x10019d90
 */
bool DataExecute(DataArray *args);
