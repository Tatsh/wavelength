#pragma once

#include "utl/Option.h"
#include "utl/Str.h"

/**
 * Option that stores the argument after it in a string.
 *
 * The RTTI includes the class name and records Option as the base. The object is 0xc bytes.
 */
class StringOption : public Option {
public:
    /**
     * Create the option.
     *
     * @param name The option's name.
     * @param value The string.
     * @ghidraAddress 0x10013300
     */
    StringOption(const char *name, String *value);

    /**
     * Store the next argument and consume it.
     *
     * @param curArg The index of the next argument, advanced past the value.
     * @ghidraAddress 0x100133d0
     */
    virtual void Set(int &curArg);

    /**
     * Print `-name <string>` to the diagnostic stream.
     *
     * @ghidraAddress 0x10013320
     */
    virtual void PrintUsage();

private:
    String *mValue; /*!< The string. */
};
