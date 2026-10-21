#pragma once

#include "os/debug.h"
#include "os/option.h"

/**
 * Option that stores a value when it is present.
 *
 * The RTTI includes the class name and records Option as the base.
 */
class BoolOption : public Option {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003ab550
     * @ghidraAddress PAL: 0x0041a2b0
     */
    ~BoolOption() override = default;

    /**
     * Construct an option of a name.
     *
     * @param pszName The name, without the leading `-`.
     * @param pnTarget Receives nValue when the option is present.
     * @param nValue The value.
     * @ghidraAddress NTSC-U/C: 0x0029d930
     * @ghidraAddress PAL: 0x002a75f8
     */
    BoolOption(const char *pszName, int *pnTarget, int nValue);

    /**
     * Store the value. The option reads no argument.
     *
     * @param pnIndex Not used.
     * @ghidraAddress NTSC-U/C: 0x0029da40
     * @ghidraAddress PAL: 0x002a7708
     */
    void Parse(int *pnIndex) override;

    /**
     * Print the name of the option.
     *
     * @ghidraAddress NTSC-U/C: 0x003ab5f8
     * @ghidraAddress PAL: 0x0041a358
     */
    void PrintUsage() const override {
        DebugPrint("\t-%s\n", mName);
    }

private:
    int *mTarget; /*!< Receives mValue when the option is present. */
    int mValue;   /*!< The value. */
};
