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

    int mReserved00;      // +0x00, the node storage. The node type is not yet recovered.
    int mReserved04;      // +0x04, not yet identified.
    short mSize;          /*!< Number of nodes. */
    unsigned short mRefs; /*!< Reference count Release() decrements. */
};
