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

    /**
     * Look up the text of a token in the language Init() loaded.
     *
     * @param pszToken The token.
     * @param bFail Treat a missing token as an error.
     * @return The text.
     * @ghidraAddress NTSC-U/C: 0x00299d30
     * @ghidraAddress PAL: 0x002a3930
     */
    const char *Localize(const char *pszToken, bool bFail);

    DataArray *mStrings; /*!< The text table Init() built, or null. */
    int mReserved04;     // +0x04, cleared by the constructor and not yet identified.
};

/**
 * The user interface text.
 *
 * @ghidraAddress NTSC-U/C: 0x00515330
 */
extern Locale TheLocale;
