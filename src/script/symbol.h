#pragma once

/**
 * Find the interned copy of a symbol's text.
 *
 * Text that already lies in the symbol pool is its own interned copy. Other text is looked up in
 * the symbol table, which the routine never adds to.
 *
 * @param pszText The text.
 * @return The interned copy, or null when the symbol table has no such text.
 * @ghidraAddress NTSC-U/C: 0x002971b8
 * @ghidraAddress PAL: 0x002a0dc8
 */
const char *LookupSymbol(const char *pszText);
