#pragma once

#include "rnd/stream.h"

/**
 * Reference-counted array of script data nodes.
 *
 * The configuration file and every script command arrive as one of these. The class is not
 * polymorphic. The name is inferred from the allocation tag "DataArray".
 */
class DataArray {
public:
    /**
     * Drop one reference and destroy the array when none remains.
     *
     * @ghidraAddress NTSC-U/C: 0x002960b0
     * @ghidraAddress PAL: 0x0029fcc8
     */
    void Release();

    /**
     * Parse script text into an array.
     *
     * @param pszPath The file to read when pStream is null, and the name errors give.
     * @param pStream The stream to read, or null to read the file.
     * @return The array, with one reference.
     * @ghidraAddress NTSC-U/C: 0x00297ee0
     * @ghidraAddress PAL: 0x002a1ae0
     */
    static DataArray *Read(const char *pszPath, Rnd::Stream *pStream);

    /**
     * Find the child array whose first node is a tag.
     *
     * @param pszTag The tag.
     * @param bFail Treat a missing child as an error.
     * @return The child, or null when none has the tag.
     * @ghidraAddress NTSC-U/C: 0x00296280
     * @ghidraAddress PAL: 0x0029fe98
     */
    DataArray *FindArray(const char *pszTag, bool bFail) const;

    /**
     * Find the symbol that follows a tag.
     *
     * @param pszName The symbol that starts the child array.
     * @param ppszValue Receives the symbol at index 1 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x002962c8
     * @ghidraAddress PAL: 0x0029fee0
     */
    bool FindSymbol(const char *pszName, const char **ppszValue, bool bFail);

    /**
     * Find the integer that follows a tag.
     *
     * @param pszName The symbol that starts the child array.
     * @param pnValue Receives the integer at index 1 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x00296360
     * @ghidraAddress PAL: 0x0029ff78
     */
    bool FindInt(const char *pszName, int *pnValue, bool bFail);

    /**
     * Find the floating-point number that follows a tag.
     *
     * @param pszName The symbol that starts the child array.
     * @param pfValue Receives the number at index 1 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x002963a8
     * @ghidraAddress PAL: 0x0029ffc0
     */
    bool FindFloat(const char *pszName, float *pfValue, bool bFail);

    /**
     * Report a node as a symbol.
     *
     * @param nIndex The node.
     * @return The interned symbol text.
     * @ghidraAddress NTSC-U/C: 0x00296720
     * @ghidraAddress PAL: 0x002a0338
     */
    const char *Sym(int nIndex) const;

    /**
     * Report a node as an integer.
     *
     * @param nIndex The node.
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x00296750
     * @ghidraAddress PAL: 0x002a0368
     */
    int Int(int nIndex) const;

    /**
     * Report a node as a float.
     *
     * @param nIndex The node.
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x00296768
     * @ghidraAddress PAL: 0x002a0380
     */
    float Float(int nIndex) const;

    /**
     * Report a node as an array.
     *
     * @param nIndex The node.
     * @return The array.
     * @ghidraAddress NTSC-U/C: 0x002967c0
     * @ghidraAddress PAL: 0x002a03d8
     */
    DataArray *Array(int nIndex) const;

    int mReserved00;      // +0x00, the node storage. The node type is not yet recovered.
    int mReserved04;      // +0x04, not yet identified.
    short mSize;          /*!< Number of nodes. */
    unsigned short mRefs; /*!< Reference count Release() decrements. */
};
