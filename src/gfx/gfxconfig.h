#pragma once

#include "math/color.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "script/dataarray.h"

// Each routine looks for a tagged entry in a configuration section and then in a section of
// defaults. The names are inferred.

/**
 * Find a child array in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param ppValue Receives the child array, or null.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2ca8
 * @ghidraAddress PAL: 0x001eba48
 */
bool FindConfigArray(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, DataArray **ppValue, bool bFail);

/**
 * Find an integer in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pnValue Receives the integer when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2d48
 * @ghidraAddress PAL: 0x001ebae8
 */
bool FindConfigInt(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, int *pnValue, bool bFail);

/**
 * Find a number in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pfValue Receives the number when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2de0
 * @ghidraAddress PAL: 0x001ebb80
 */
bool FindConfigFloat(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, float *pfValue, bool bFail);

/**
 * Find a flag in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pnValue Receives 1 for a non-zero flag and 0 otherwise, when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2e78
 * @ghidraAddress PAL: 0x001ebc18
 */
bool FindConfigBool(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, int *pnValue, bool bFail);

/**
 * Find a symbol in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param ppszValue Receives the symbol when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2f10
 * @ghidraAddress PAL: 0x001ebcb0
 */
bool FindConfigSymbol(DataArray *pConfig,
                      DataArray *pDefaults,
                      const char *pszName,
                      const char **ppszValue,
                      bool bFail);

/**
 * Find two numbers in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pValue Receives the numbers when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e2fa8
 * @ghidraAddress PAL: 0x001ebd48
 */
bool FindConfigVector2(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Vector2 *pValue, bool bFail);

/**
 * Find three numbers in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pValue Receives the numbers when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e3040
 * @ghidraAddress PAL: 0x001ebde0
 */
bool FindConfigVector3(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Vector3 *pValue, bool bFail);

/**
 * Find a colour in a configuration section or in its defaults.
 *
 * @param pConfig The section, or null.
 * @param pDefaults The section of defaults, or null.
 * @param pszName The tag.
 * @param pValue Receives the colour when the entry exists.
 * @param bFail Whether to report an entry missing from both sections.
 * @return Whether the entry exists.
 * @ghidraAddress NTSC-U/C: 0x001e30d8
 * @ghidraAddress PAL: 0x001ebe78
 */
bool FindConfigColor(
    DataArray *pConfig, DataArray *pDefaults, const char *pszName, Color *pValue, bool bFail);

/**
 * Find the `gfx` section of the configuration.
 *
 * @return The section.
 * @ghidraAddress NTSC-U/C: 0x001d8288
 * @ghidraAddress PAL: 0x001e1028
 */
DataArray *GetGfxConfig();

/**
 * Find the child of the `gfx` section for the kind of game, `solo`, `duel`, `net`, or `multi`.
 *
 * @return The section, or null.
 * @ghidraAddress NTSC-U/C: 0x001d8200
 * @ghidraAddress PAL: 0x001e0fa0
 */
DataArray *GetModeGfxConfig();
