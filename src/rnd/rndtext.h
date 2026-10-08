#pragma once

#include <cstddef>
#include <list>

#include "math/color.h"
#include "math/plane.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "os/string.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndfont.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtransformable.h"

/**
 * Text drawn with a bitmap font as a mesh of one quad per character.
 *
 * The RTTI includes the class name and records RndDrawable, RndCollideable, and RndTransformable
 * as bases. The mesh is rebuilt from mPreWrapText whenever the text, the font, or the layout
 * changes, and only while the text is showing.
 */
class RndText : public RndDrawable, public RndCollideable, public RndTransformable {
public:
    /** The bits of mAlign, one vertical and one horizontal. */
    enum Align {
        kAlignLeft = 1,      /*!< Lines start at the origin. */
        kAlignCenter = 2,    /*!< Lines centre on the origin. */
        kAlignRight = 4,     /*!< Lines end at the origin. */
        kAlignTop = 0x10,    /*!< The first line is at the origin. */
        kAlignMiddle = 0x20, /*!< The lines centre on the origin. */
        kAlignBottom = 0x40, /*!< The last line is at the origin. */
    };

    /**
     * Construct white, empty text aligned to the top left, without a font, wrapping at 100.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x002424e0
     * @ghidraAddress PAL: 0x0024afb8
     */
    explicit RndText(const char *pszName);

    /**
     * Drop the reference on the font and delete the mesh.
     *
     * @ghidraAddress NTSC-U/C: 0x00391268
     * @ghidraAddress PAL: 0x003ff8d0
     */
    ~RndText() override;

    /**
     * Allocate text, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Text", 0);
    }

    /**
     * Release text.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Add the material of the mesh to a list.
     *
     * Without a mesh nothing is added. The children are not added.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002439d8
     * @ghidraAddress PAL: 0x0024c4b0
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the text unless its mesh is outside the view of the current camera, then the children.
     *
     * The bounding sphere spans the first and the last vertex. Text without a mesh is not added.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00243a78
     * @ghidraAddress PAL: 0x0024c550
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Show or hide the text, building the mesh when it starts showing and deleting it when it
     * stops.
     *
     * The children are not changed.
     *
     * @param nShowing Non-zero to show.
     * @ghidraAddress NTSC-U/C: 0x00243820
     * @ghidraAddress PAL: 0x0024c2f8
     */
    void SetShowing(int nShowing) override;

    /**
     * Set the highlight of the text and of its mesh.
     *
     * @param nHighlight Non-zero to highlight.
     * @ghidraAddress NTSC-U/C: 0x002437d0
     * @ghidraAddress PAL: 0x0024c2a8
     */
    void SetHighlight(int nHighlight) override;

    /**
     * Draw the mesh, or the text with the renderer's line font at the local position when there
     * is no mesh.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x00243738
     * @ghidraAddress PAL: 0x0024c210
     */
    int DrawShowing() override;

    /**
     * Set the colour of every character.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00242ae8
     * @ghidraAddress PAL: 0x0024b5c0
     */
    virtual void SetColor(const Color &color);

    /**
     * Set the alignment.
     *
     * @param nAlign The Align bits.
     * @ghidraAddress NTSC-U/C: 0x00242b10
     * @ghidraAddress PAL: 0x0024b5e8
     */
    virtual void SetAlign(int nAlign);

    /**
     * Set the text.
     *
     * @param pszText The text. A newline starts a line.
     * @ghidraAddress NTSC-U/C: 0x00242d38
     * @ghidraAddress PAL: 0x0024b810
     */
    virtual void SetText(const char *pszText);

    /**
     * Set the font.
     *
     * @param pFont The font, or null.
     * @ghidraAddress NTSC-U/C: 0x00242a90
     * @ghidraAddress PAL: 0x0024b568
     */
    virtual void SetFont(RndFont *pFont);

    using RndCollideable::Collide;

    /**
     * Add each face of the mesh the segment strikes, as strikes of the text, then what the
     * children strike, to a list.
     *
     * Hidden text adds nothing.
     *
     * @param segment The segment, in world space.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002418a8
     * @ghidraAddress PAL: 0x0024a3b0
     */
    void Collide(const Segment &segment, std::list<Collision> &collisions) override;

    /**
     * Set the billboard mode of the text and of its mesh.
     *
     * @param nBillboard The Billboard mode.
     * @ghidraAddress NTSC-U/C: 0x00242dd0
     * @ghidraAddress PAL: 0x0024b8a8
     */
    void SetBillboard(int nBillboard) override;

    /**
     * Update the world transform of the text, then of its mesh from the text's.
     *
     * @param pParent The parent, or null.
     * @param bForce Non-zero to update even when the transform is not out of date.
     * @return What RndTransformable::UpdateWorldXfm() reports.
     * @ghidraAddress NTSC-U/C: 0x00243888
     * @ghidraAddress PAL: 0x0024c360
     */
    int UpdateWorldXfm(RndTransformable *pParent, int bForce) override;

    /**
     * Write a description of the text and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00241ac8
     * @ghidraAddress PAL: 0x0024a5d0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the text and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00241ca0
     * @ghidraAddress PAL: 0x0024a7a8
     */
    void Save(BinStream &stream) override;

    /**
     * Replace the font when it is the object being replaced.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x002419f8
     * @ghidraAddress PAL: 0x0024a500
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x003916a8
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another text and its bases, then build the mesh again.
     *
     * @param pSource The text to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x002423e8
     * @ghidraAddress PAL: 0x0024aec0
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, then build the mesh.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00241ed8
     * @ghidraAddress PAL: 0x0024a9c8
     */
    void Load(BinStream &stream) override;

    /**
     * Give the text again to every cursor that refers to it.
     *
     * @ghidraAddress NTSC-U/C: 0x002417f0
     * @ghidraAddress PAL: 0x0024a2f8
     */
    void UpdateCursors();

    /**
     * Copy mPreWrapText to mText with each tab as three spaces, limit it to 800 characters, wrap
     * it when word wrap is on, and build the mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00242258
     * @ghidraAddress PAL: 0x0024ad30
     */
    void SyncText();

    /**
     * Turn word wrap on or off.
     *
     * @param nWordWrap Non-zero to wrap.
     * @ghidraAddress NTSC-U/C: 0x00242350
     * @ghidraAddress PAL: 0x0024ae28
     */
    void SetWordWrap(int nWordWrap);

    /**
     * Set the width lines wrap at.
     *
     * @param fWrapWidth The width.
     * @ghidraAddress NTSC-U/C: 0x00242370
     * @ghidraAddress PAL: 0x0024ae48
     */
    void SetWrapWidth(float fWrapWidth);

    /**
     * Give the mesh the depth test mDepthTest selects.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00242390
     * @ghidraAddress PAL: 0x0024ae68
     */
    void SyncDepthTest();

    /**
     * Turn the depth test on or off.
     *
     * The name is inferred.
     *
     * @param nDepthTest Non-zero to test depth.
     * @ghidraAddress NTSC-U/C: 0x002423c8
     * @ghidraAddress PAL: 0x0024aea0
     */
    void SetDepthTest(int nDepthTest);

    /**
     * Measure the advance of a run of characters, without the spacing.
     *
     * @param pszText The characters.
     * @param nCount The number of characters.
     * @return The advance, or 0 without a font.
     * @ghidraAddress NTSC-U/C: 0x00242b30
     * @ghidraAddress PAL: 0x0024b608
     */
    float MeasureWidth(const char *pszText, int nCount);

    /**
     * Break text into lines no wider than mWrapWidth, at the last space where there is one.
     *
     * The space stays at the end of its line. The name is inferred.
     *
     * @param pszText The text.
     * @param wrapped Receives the wrapped text.
     * @ghidraAddress NTSC-U/C: 0x00242ba8
     * @ghidraAddress PAL: 0x0024b680
     */
    void WrapText(const char *pszText, String &wrapped);

    /**
     * Count the lines of mText.
     *
     * @return The count.
     * @ghidraAddress NTSC-U/C: 0x00242d68
     * @ghidraAddress PAL: 0x0024b840
     */
    int NumLines();

    /**
     * Measure the line of mText that starts at a character, with the spacing.
     *
     * The name is inferred.
     *
     * @param pszLine The first character of the line.
     * @param fWidth Receives the width.
     * @return The character after the end of the line.
     * @ghidraAddress NTSC-U/C: 0x00242e28
     * @ghidraAddress PAL: 0x0024b900
     */
    const char *MeasureLine(const char *pszLine, float &fWidth);

    /**
     * Build a mesh of one quad per character of mText, in the font's material.
     *
     * Hidden text, text without a font, and empty text get no mesh. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00242ef8
     * @ghidraAddress PAL: 0x0024b9d0
     */
    void BuildMesh();

    /**
     * Fill the quads of one line of the mesh.
     *
     * The name is inferred.
     *
     * @param nFirstChar The index of the line's first quad.
     * @param pszStart The first character of the line.
     * @param pszEnd The character after the end of the line.
     * @param fLine The line's position, in lines from the origin.
     * @param fWidth The width of the line.
     * @ghidraAddress NTSC-U/C: 0x00243338
     * @ghidraAddress PAL: 0x0024be10
     */
    void
    BuildLine(int nFirstChar, const char *pszStart, const char *pszEnd, float fLine, float fWidth);

    /**
     * Report the position of a character of mPreWrapText in the text's space.
     *
     * The name is inferred.
     *
     * @param nIndex The index of the character.
     * @return The position, or the origin without a font.
     * @ghidraAddress NTSC-U/C: 0x00243520
     * @ghidraAddress PAL: 0x0024bff8
     */
    Vector3 CharPosition(int nIndex);

    /**
     * Create text.
     *
     * @param pszName The registry key.
     * @return The text.
     * @ghidraAddress NTSC-U/C: 0x003916b8
     * @ghidraAddress PAL: 0x003ffdc0
     */
    static RndObject *New(const char *pszName) {
        return new RndText(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Text`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a50
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a54
     */
    static int sRev;

    Color mColor;        /*!< The colour of every character. */
    int mAlign;          /*!< The Align bits. */
    RndFont *mFont;      /*!< The font, or null. */
    int mWordWrap;       /*!< Non-zero to wrap lines at mWrapWidth. */
    float mWrapWidth;    /*!< The width lines wrap at. */
    String mText;        /*!< The text the mesh shows, after the tabs and the wrapping. */
    String mPreWrapText; /*!< The text as set. */
    RndMesh *mMesh;      /*!< The mesh of the characters, or null. */
    int mDepthTest;      /*!< Non-zero for the mesh to test depth. */

protected:
    /**
     * Drop the reference on the font and delete the mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00241e38
     * @ghidraAddress PAL: 0x0024a928
     */
    void ReleaseRefs();

    /**
     * Take a reference on the font and build the text again.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00241e98
     * @ghidraAddress PAL: 0x0024a988
     */
    void AcquireRefs();
};

/**
 * Write the names of an alignment's vertical and horizontal parts.
 *
 * @param stream The stream to write to.
 * @param eAlign The Align bits.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002438f8
 * @ghidraAddress PAL: 0x0024c3d0
 */
PrnStream &operator<<(PrnStream &stream, RndText::Align eAlign);
