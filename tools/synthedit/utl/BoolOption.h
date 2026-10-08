#pragma once

#include "utl/Option.h"

/**
 * Option that stores a fixed value in a flag when it appears.
 *
 * The RTTI includes the class name and records Option as the base. The object is 0x10 bytes.
 */
class BoolOption : public Option {
public:
    /**
     * Create the option.
     *
     * @param name The option's name.
     * @param value The flag.
     * @param setValue The value stored when the option appears.
     * @ghidraAddress 0x100132a0
     */
    BoolOption(const char *name, bool *value, bool setValue);

    /**
     * Store the value. The option takes no argument.
     *
     * @param curArg The index of the next argument, unchanged.
     * @ghidraAddress 0x100133c0
     */
    virtual void Set(int &curArg);

    /**
     * Print `-name` to the diagnostic stream.
     *
     * @ghidraAddress 0x100132d0
     */
    virtual void PrintUsage();

private:
    bool *mValue;   /*!< The flag. */
    bool mSetValue; /*!< The value stored when the option appears. */
};
