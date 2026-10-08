#pragma once

/**
 * The arguments OptionProcessor::Process() is working through.
 *
 * @ghidraAddress 0x100c38e4
 */
extern char **gOptionArgv;

/**
 * Command-line option that OptionProcessor matches by name.
 *
 * The RTTI includes the class name. The object is 8 bytes.
 */
class Option {
public:
    /**
     * Create an option.
     *
     * @param name The option's name, matched after the leading dash of an argument.
     * @ghidraAddress 0x10013260
     */
    explicit Option(const char *name);

    /**
     * Release the option.
     *
     * @ghidraAddress 0x100132f0
     */
    virtual ~Option();

    /**
     * Apply the option, consuming any value arguments after it.
     *
     * @param curArg The index of the next argument.
     */
    virtual void Set(int &curArg) = 0;

    /** Print the option's usage line. */
    virtual void PrintUsage() = 0;

    /**
     * Consume the current argument when it names this option.
     *
     * @param curArg The index of the current argument, advanced past it on a match.
     * @return Whether the argument names this option.
     * @ghidraAddress 0x10013360
     */
    bool Matches(int &curArg);

protected:
    const char *mName; /*!< The option's name. */
};
