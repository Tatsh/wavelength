#pragma once

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
     * Find the integer after a tag in the child array with the tag.
     *
     * @param pszTag The tag.
     * @param pValue Receives the value when the child exists.
     * @param bFail Treat a missing child as an error.
     * @return Whether the child exists.
     * @ghidraAddress NTSC-U/C: 0x00296360
     * @ghidraAddress PAL: 0x0029ff78
     */
    bool FindData(const char *pszTag, int *pValue, bool bFail) const;

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

    int mReserved00;      // +0x00, the node storage. The node type is not yet recovered.
    int mReserved04;      // +0x04, not yet identified.
    short mSize;          /*!< Number of nodes. */
    unsigned short mRefs; /*!< Reference count Release() decrements. */
};
