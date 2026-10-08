#pragma once

/**
 * Command-line option an OptionProcessor matches.
 *
 * The RTTI includes the class name and records no base. BoolOption and StringOption derive from
 * it. The name is at `+0x00` and the vptr at `+0x04`.
 */
class Option {
public:
    /**
     * Construct an option of a name.
     *
     * @param pszName The name, without the leading `-`.
     * @ghidraAddress NTSC-U/C: 0x0029d918
     * @ghidraAddress PAL: 0x002a75e0
     */
    explicit Option(const char *pszName);

    /** Release the option. */
    virtual ~Option() {
    }

    /**
     * Read the value that follows the option on the command line.
     *
     * @param pnIndex The index of the next argument, advanced past what the option reads.
     */
    virtual void Parse(int *pnIndex) = 0;

    /** Print the option's usage line. */
    virtual void PrintUsage() const = 0;

    /**
     * Report whether the argument at an index names this option, and step past it when it does.
     *
     * @param pnIndex The index of the argument.
     * @return Whether the argument names the option.
     * @ghidraAddress NTSC-U/C: 0x0029d9e0
     * @ghidraAddress PAL: 0x002a76a8
     */
    bool Match(int *pnIndex) const;

    const char *mName; /*!< The name, without the leading `-`. */
};
