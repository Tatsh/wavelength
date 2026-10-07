#pragma once

#include "script/dataarray.h"

/**
 * Text of the user interface in the console language.
 *
 * The RTTI includes the class name. Two data words precede the vptr at `+0x08`. The one instance is
 * TheLocale.
 */
class Locale {
public:
    /**
     * Construct a locale with no text loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00299c80
     * @ghidraAddress PAL: 0x002a3880
     */
    Locale();

    /**
     * Release the text through Terminate().
     *
     * @ghidraAddress NTSC-U/C: 0x00299cd8
     * @ghidraAddress PAL: 0x002a38d8
     */
    virtual ~Locale();

    /**
     * Build the text table of one language from the configuration.
     *
     * @param pszLanguage The language key GetSystemLanguage() reports.
     * @ghidraAddress NTSC-U/C: 0x00299ac8
     * @ghidraAddress PAL: 0x002a36c8
     */
    void Init(const char *pszLanguage);

    /**
     * Release the text table, if one is loaded.
     *
     * @ghidraAddress NTSC-U/C: 0x00299ca0
     * @ghidraAddress PAL: 0x002a38a0
     */
    void Terminate();

    DataArray *mStrings; /*!< The text table Init() built, or null. */
    int mReserved04;     // +0x04, cleared by the constructor and not yet identified.
};

/**
 * The user interface text.
 *
 * @ghidraAddress NTSC-U/C: 0x00515330
 */
extern Locale TheLocale;
