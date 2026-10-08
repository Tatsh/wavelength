#pragma once

#include "script/dataarray.h"

/**
 * Load the IOP module a configuration array describes.
 *
 * Node 0 is the module's file, with each `%` replaced by FileSourceLetter(). The other nodes are
 * the module's arguments, each a symbol or a `(concat ...)` or `(file ...)` directive. The module
 * is read from the device FormatDevicePath() selects, from the disc unless `host_config` is set.
 *
 * @param pModule The array.
 * @return The sceSifLoadModule() result.
 * @ghidraAddress NTSC-U/C: 0x0028a298
 * @ghidraAddress PAL: 0x00293a90
 */
int IrxLoadModule(const DataArray *pModule);

/**
 * Load the IOP module of every child array after node 0.
 *
 * @param pModules The array of module arrays.
 * @ghidraAddress NTSC-U/C: 0x0028a190
 * @ghidraAddress PAL: 0x00293988
 */
void IrxLoadAll(const DataArray *pModules);

/**
 * Load the IOP module of the first child array whose file includes a string.
 *
 * @param pModules The array of module arrays.
 * @param pszName The string.
 * @return The IrxLoadModule() result, or -1 when no file includes the string.
 * @ghidraAddress NTSC-U/C: 0x0028a1f8
 * @ghidraAddress PAL: 0x002939f0
 */
int IrxLoadMatching(const DataArray *pModules, const char *pszName);
