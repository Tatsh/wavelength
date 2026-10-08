#pragma once

#include "utl/Data.h"

/**
 * Size the string table and empty its hash table.
 *
 * @ghidraAddress 0x10016100
 */
void DataStringInit();

/**
 * Release every macro and forget them.
 *
 * @ghidraAddress 0x100162a0
 */
void DataStringTerminate();

/**
 * Find the interned copy of a string.
 *
 * A pointer into the string table is returned unchanged.
 *
 * @param str The string.
 * @return The interned copy, or null.
 * @ghidraAddress 0x10016360
 */
const char *DataFindString(const char *str);

/**
 * Intern a string that is not interned yet.
 *
 * @param str The string.
 * @return The interned copy.
 * @ghidraAddress 0x10016400
 */
const char *DataAddString(const char *str);

/**
 * Find the interned copy of a string, interning it first when it is not interned yet. The control
 * compiles this inline.
 *
 * @param str The string.
 * @return The interned copy.
 */
inline const char *DataInternString(const char *str) {
    const char *interned = DataFindString(str);
    if (interned == NULL) {
        interned = DataAddString(str);
    }
    return interned;
}

/**
 * Define or replace a macro.
 *
 * @param name The macro's name.
 * @param macro The macro's value. It gains a reference.
 * @ghidraAddress 0x10016500
 */
void DataSetMacro(const char *name, DataArray *macro);

/**
 * Look up a macro.
 *
 * @param name The macro's name.
 * @return The macro's value, or null.
 * @ghidraAddress 0x100165a0
 */
DataArray *DataGetMacro(const char *name);
