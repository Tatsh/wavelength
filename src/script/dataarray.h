#pragma once

#include "math/color.h"
#include "math/vector3.h"
#include "os/string.h"
#include "rnd/stream.h"

/**
 * Reference-counted array of script data nodes.
 *
 * The configuration file and every script command arrive as one of these. The class is not
 * polymorphic. The name is inferred from the allocation tag "DataArray".
 */
class DataArray {
public:
    /** Kinds of node Type() reports. The other values are not yet identified. */
    enum NodeType {
        kNodeInt = 0,    /*!< An integer. */
        kNodeSymbol = 1, /*!< A symbol. */
    };

    /**
     * Take one more reference.
     *
     * @ghidraAddress NTSC-U/C: 0x002960a0
     * @ghidraAddress PAL: 0x0029fcb8
     */
    void AddRef();

    /**
     * Report the number of nodes.
     *
     * @return mSize.
     */
    int Size() const {
        return mSize;
    }

    /**
     * Report the kind of a node.
     *
     * @param nIndex The node.
     * @return One of NodeType.
     * @ghidraAddress NTSC-U/C: 0x002966f0
     * @ghidraAddress PAL: 0x002a0308
     */
    int Type(int nIndex) const;

    /**
     * Find the integer that follows a tag and report it as a flag.
     *
     * @param pszName The symbol that starts the child array.
     * @param pbValue Receives whether the integer at index 1 of the child array is non-zero, when
     *                the child array exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x002963f0
     * @ghidraAddress PAL: 0x002a0008
     */
    bool FindBool(const char *pszName, bool *pbValue, bool bFail) const;

    /**
     * Find the three numbers that follow a tag.
     *
     * The fourth word of the vector is left as it was.
     *
     * @param pszName The symbol that starts the child array.
     * @param pValue Receives the numbers at indices 1 to 3 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x002964a8
     * @ghidraAddress PAL: 0x002a00c0
     */
    bool FindVector(const char *pszName, Vector3 *pValue, bool bFail) const;

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
     * Find the child array whose first node is an integer tag.
     *
     * An array marked as sorted is searched by halves.
     *
     * @param nTag The tag.
     * @return The child, or null when none has the tag.
     * @ghidraAddress NTSC-U/C: 0x002960e8
     * @ghidraAddress PAL: 0x0029fd00
     */
    DataArray *FindArray(int nTag) const;

    /**
     * Find the symbol that follows a tag and copy it into a string.
     *
     * @param pszName The symbol that starts the child array.
     * @param pValue Receives the symbol at index 1 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x00296310
     * @ghidraAddress PAL: 0x0029ff28
     */
    bool FindString(const char *pszName, String *pValue, bool bFail) const;

    /**
     * Find the colour that follows a tag.
     *
     * The alpha is read only when the child array has a fourth component.
     *
     * @param pszName The symbol that starts the child array.
     * @param pValue Receives the colour at indices 1 to 4 of the child array when it exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x00296520
     * @ghidraAddress PAL: 0x002a0138
     */
    bool FindColor(const char *pszName, Color *pValue, bool bFail) const;

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
    bool FindSymbol(const char *pszName, const char **ppszValue, bool bFail) const;

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
    bool FindInt(const char *pszName, int *pnValue, bool bFail) const;

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
    bool FindFloat(const char *pszName, float *pfValue, bool bFail) const;

    /**
     * Find the truth value that follows a tag.
     *
     * @param pszName The symbol that starts the child array.
     * @param pnValue Receives 1 when the integer at index 1 of the child array is not zero and 0
     * otherwise, when the child array exists.
     * @param bFail Passed to FindArray().
     * @return Whether the child array exists.
     * @ghidraAddress NTSC-U/C: 0x002963f0
     * @ghidraAddress PAL: 0x002a0008
     */
    bool FindBool(const char *pszName, int *pnValue, bool bFail) const;

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
    const char *mFile;    /*!< The file the array was read from. */
    short mSize;          /*!< Number of nodes. */
    unsigned short mRefs; /*!< Reference count Release() decrements. */
};
