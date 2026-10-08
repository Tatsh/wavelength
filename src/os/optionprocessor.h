#pragma once

#include <list>

#include "os/option.h"
#include "os/string.h"

/**
 * Parser of the command line that main() receives.
 *
 * The class is not polymorphic. The name comes from its messages.
 */
class OptionProcessor {
public:
    /** Construct a processor without options. */
    OptionProcessor() = default;

    /**
     * Delete every option.
     *
     * @ghidraAddress NTSC-U/C: 0x0029d870
     * @ghidraAddress PAL: 0x002a7538
     */
    ~OptionProcessor();

    /**
     * Add an option that stores a value when it is present.
     *
     * @param pszName The option's name, without the leading `-`.
     * @param pnTarget Receives nValue when the option is present.
     * @param nValue The value.
     * @ghidraAddress NTSC-U/C: 0x0029d520
     * @ghidraAddress PAL: 0x002a71e8
     */
    void AddBool(const char *pszName, int *pnTarget, int nValue);

    /**
     * Add an option that reads the argument after it into a string.
     *
     * @param pszName The option's name, without the leading `-`.
     * @param pTarget Receives the argument.
     * @ghidraAddress NTSC-U/C: 0x0029d5f0
     * @ghidraAddress PAL: 0x002a72b8
     */
    void AddString(const char *pszName, String *pTarget);

    /**
     * Apply the options of a command line.
     *
     * Adds an `-options` option. When it is present, every option prints its usage line and the
     * program exits.
     *
     * @param argc The argument count.
     * @param argv The arguments.
     * @ghidraAddress NTSC-U/C: 0x0029d6b0
     * @ghidraAddress PAL: 0x002a7378
     */
    void Process(int argc, char **argv);

    std::list<Option *> mOptions; /*!< The options, in the order they were added. */
    int mIndex;                   /*!< The index of the next argument Process() reads. */
    char **mArgv;                 /*!< The arguments Process() received. */
    int mArgc;                    /*!< The argument count Process() received. */
};
