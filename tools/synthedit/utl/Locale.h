#pragma once

#include "utl/Data.h"

/**
 * The localized text of the control.
 *
 * The RTTI records the class. The object is 0xc bytes. The control never loads a table, so only
 * Terminate() is recovered, and the word at `+0x08` has no known use.
 */
class Locale {
public:
    /**
     * Create a locale with no table.
     *
     * @ghidraAddress 0x10013510
     */
    Locale() : mTable(NULL), mReserved08(0) {
    }

    /**
     * Release the table.
     *
     * @ghidraAddress 0x10013570
     */
    virtual ~Locale();

    /**
     * Release the table.
     *
     * @ghidraAddress 0x10013550
     */
    void Terminate();

private:
    DataArray *mTable; /*!< The localized text, or null. */
    int mReserved08;   // +0x08, cleared by the constructor and not read.
};

/**
 * The locale of the control.
 *
 * @ghidraAddress 0x100c38e8
 */
extern Locale TheLocale;
