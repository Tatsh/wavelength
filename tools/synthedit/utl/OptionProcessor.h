#pragma once

#include <list>

#include "utl/Option.h"
#include "utl/Str.h"

/**
 * Set of command-line options applied to an argument vector.
 *
 * The object is 0x10 bytes. The name comes from the message Process() prints for an unknown
 * option.
 */
class OptionProcessor {
public:
    /**
     * Delete every option.
     *
     * @ghidraAddress 0x100131d0
     */
    ~OptionProcessor();

    /**
     * Add an option that stores a fixed value in a flag.
     *
     * @param name The option's name.
     * @param value The flag.
     * @param setValue The value stored when the option appears.
     * @ghidraAddress 0x10012eb0
     */
    void AddBoolOption(const char *name, bool *value, bool setValue);

    /**
     * Add an option that stores the argument after it.
     *
     * @param name The option's name.
     * @param value The string.
     * @ghidraAddress 0x10012f50
     */
    void AddStringOption(const char *name, String *value);

    /**
     * Apply the options to the arguments that start with a dash, stopping at the first that does
     * not.
     *
     * An unknown option prints a message and is skipped. The `--options` argument prints every
     * option's usage and exits.
     *
     * @param argc The number of arguments, the program name included.
     * @param argv The arguments.
     * @ghidraAddress 0x10012fe0
     */
    void Process(int argc, char **argv);

    /**
     * Split a command line into arguments at spaces outside double quotes.
     *
     * The vector and the copy of the command line share one allocation. Twenty arguments at most
     * are kept.
     *
     * @param commandLine The command line.
     * @param argc Receives the number of arguments.
     * @return The arguments.
     * @ghidraAddress 0x100130d0
     */
    static char **ParseCommandLine(const char *commandLine, int *argc);

private:
    std::list<Option *> mOptions; /*!< The options in the order they were added. */
    int mCurArg;                  /*!< The index of the argument being processed. */
    char **mArgv;                 /*!< The arguments being processed. */
    int mArgc;                    /*!< The number of arguments being processed. */
};
