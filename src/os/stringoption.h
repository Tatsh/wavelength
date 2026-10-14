#pragma once

#include "os/debug.h"
#include "os/option.h"
#include "os/string.h"

/**
 * Option that reads the argument after it into a string.
 *
 * The RTTI includes the class name and records Option as the base.
 */
class StringOption : public Option {
public:
    /**
     * Construct an option of a name.
     *
     * @param pszName The name, without the leading `-`.
     * @param pTarget Receives the argument.
     * @ghidraAddress NTSC-U/C: 0x0029d998
     * @ghidraAddress PAL: 0x002a7660
     */
    StringOption(const char *pszName, String *pTarget);

    /**
     * Copy the next argument into the string.
     *
     * @param pnIndex The index of the argument, advanced past it.
     * @ghidraAddress NTSC-U/C: 0x0029da60
     */
    void Parse(int *pnIndex) override;

    /**
     * Print the name of the option and its argument.
     *
     * @ghidraAddress NTSC-U/C: 0x003ab6d8
     * @ghidraAddress PAL: 0x0041a438
     */
    void PrintUsage() const override {
        DebugPrint("\t-%s <string>\n", mName);
    }

    String *mTarget; /*!< Receives the argument. */
};
