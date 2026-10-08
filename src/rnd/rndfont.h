#pragma once

#include <cstddef>
#include <map>

#include "math/vector2.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "os/string.h"
#include "rnd/rndmat.h"
#include "rnd/rndobject.h"

/**
 * Bitmap font, a grid of character cells in the texture of a material's first stage.
 *
 * The RTTI includes the class name and records RndObject as the one base. The characters fill
 * the cells row by row in the order mChars lists them.
 */
class RndFont : public RndObject {
public:
    /**
     * The placement of one character.
     *
     * The RTTI includes the nested class name.
     */
    class CharInfo {
    public:
        float mWidth;     /*!< The advance, in the units of mSize. */
        Vector2 mUvStart; /*!< The texture coordinates of the top left corner. */
        Vector2 mUvEnd;   /*!< The texture coordinates of the bottom right corner. */
    };

    /**
     * Construct a font of one cell without a material or characters.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00226740
     * @ghidraAddress PAL: 0x0022f4f0
     */
    explicit RndFont(const char *pszName);

    /**
     * Drop the reference on the material.
     *
     * @ghidraAddress NTSC-U/C: 0x003822f0
     * @ghidraAddress PAL: 0x003f0970
     */
    ~RndFont() override;

    /**
     * Allocate a font, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Font", 0);
    }

    /**
     * Release a font.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Write a description of the font.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00225e38
     * @ghidraAddress PAL: 0x0022ebf0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the font.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00225f70
     * @ghidraAddress PAL: 0x0022ed28
     */
    void Save(BinStream &stream) override;

    /**
     * Replace the material when it is the object being replaced.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00225d90
     * @ghidraAddress PAL: 0x0022eb48
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x003823a8
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another font, then place the characters again.
     *
     * @param pSource The font to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00226690
     * @ghidraAddress PAL: 0x0022f440
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, then place the characters.
     *
     * The characters of an early version are the printable ASCII range.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00226358
     * @ghidraAddress PAL: 0x0022f110
     */
    void Load(BinStream &stream) override;

    /**
     * Set the material, the characters, and the grid, then place the characters.
     *
     * A row or column count below 1 becomes 1. The name is inferred.
     *
     * @param pMat The material.
     * @param pszChars The characters, in cell order.
     * @param fRows The number of rows of cells.
     * @param fCols The number of columns of cells.
     * @param fSize The size of the characters.
     * @param fSpace The space between characters.
     * @ghidraAddress NTSC-U/C: 0x00225cc0
     * @ghidraAddress PAL: 0x0022ea78
     */
    void
    Set(RndMat *pMat, const char *pszChars, float fRows, float fCols, float fSize, float fSpace);

    /**
     * Measure the cell of each character in the texture and record its placement.
     *
     * A character past the last cell is reported and ends the placement. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00226080
     * @ghidraAddress PAL: 0x0022ee38
     */
    void BuildCharMap();

    /**
     * Measure the opaque columns of one cell of the texture.
     *
     * A cell without an opaque pixel is a quarter of the cell wide. A font without a texture
     * reports zeros. The name is inferred.
     *
     * @param nRow The row of the cell.
     * @param nCol The column of the cell.
     * @return The placement of the cell's character.
     * @ghidraAddress NTSC-U/C: 0x00225918
     * @ghidraAddress PAL: 0x0022e6d0
     */
    CharInfo CellInfo(int nRow, int nCol) const;

    /**
     * Report the texture coordinates of a character.
     *
     * A character the font does not have reports zeros. The name is inferred.
     *
     * @param ch The character.
     * @param uvStart Receives the top left corner.
     * @param uvEnd Receives the bottom right corner.
     * @ghidraAddress NTSC-U/C: 0x00226840
     * @ghidraAddress PAL: 0x0022f5f0
     */
    void CharUv(char ch, Vector2 &uvStart, Vector2 &uvEnd) const;

    /**
     * Report the advance of a character.
     *
     * The name is inferred.
     *
     * @param ch The character.
     * @return The advance, or 0 for a character the font does not have.
     * @ghidraAddress NTSC-U/C: 0x002268f8
     * @ghidraAddress PAL: 0x0022f6a8
     */
    float CharWidth(char ch) const;

    /**
     * Create a font.
     *
     * @param pszName The registry key.
     * @return The font.
     * @ghidraAddress NTSC-U/C: 0x003823b8
     * @ghidraAddress PAL: 0x003f0ac0
     */
    static RndObject *New(const char *pszName) {
        return new RndFont(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Font`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09b8
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09bc
     */
    static int sRev;

    std::map<char, CharInfo> mCharMap; /*!< The placement of each character. */
    RndMat *mMat;                      /*!< The material whose first stage has the texture. */
    float mRows;                       /*!< The number of rows of cells. */
    float mCols;                       /*!< The number of columns of cells. */
    float mSize;                       /*!< The size of the characters. */
    float mSpace;                      /*!< The space between characters. */
    String mChars;                     /*!< The characters, in cell order. */

protected:
    /**
     * Drop the reference on the material.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00226050
     * @ghidraAddress PAL: 0x0022ee08
     */
    void ReleaseRefs();
};
