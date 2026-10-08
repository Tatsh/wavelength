#pragma once

/**
 * Prepare the registry of named objects.
 *
 * The body does nothing. The registry, a vector of entries that pair a String with an object, is
 * built on first use, and only NamedObjectRegistryTerminate() uses it. The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029e748
 * @ghidraAddress PAL: 0x002a8410
 */
void NamedObjectRegistryInit();

/**
 * Delete every entry of the registry of named objects.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x0029e750
 * @ghidraAddress PAL: 0x002a8418
 */
void NamedObjectRegistryTerminate();
