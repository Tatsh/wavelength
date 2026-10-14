#pragma once

#include <cstddef>
#include <list>

#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndblur.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"

/**
 * Cursor that sweeps along one line of text, hiding the characters it has not reached.
 *
 * The RTTI includes the class name and records RndAnimatable and RndDrawable as bases. The frame
 * counts characters. A mask mesh in front of the text hides the characters past the cursor, and an
 * internal copy of mBlur marks the cursor position. The cursor rests on each character for
 * mStutter of a frame before it moves on.
 */
class RndCursor : public RndAnimatable, public RndDrawable {
public:
    /**
     * Construct a cursor without a text or a blur, offset by -0.1 and resting 0.8 of a frame.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00223c80
     * @ghidraAddress PAL: 0x0022ca50
     */
    explicit RndCursor(const char *pszName);

    /**
     * Drop the references and delete the mask and the internal blur.
     *
     * @ghidraAddress NTSC-U/C: 0x00223930
     * @ghidraAddress PAL: 0x0022c700
     */
    ~RndCursor() override;

    /**
     * Allocate a cursor, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Cursor", 0);
    }

    /**
     * Release a cursor.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Report the last frame, the length of the text plus the rest at the end.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x00223600
     * @ghidraAddress PAL: 0x0022c3d0
     */
    float EndFrame() override;

    /**
     * Add the objects of the blur and of the text to a list.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00223870
     * @ghidraAddress PAL: 0x0022c640
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the drawables of the text, then the children, to a list.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002238d8
     * @ghidraAddress PAL: 0x0022c6a8
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Move the mask and the internal blur to the cursor position and draw them with the text.
     *
     * Nothing is drawn while the text is missing or hidden. The internal blur is collapsed before
     * the first character and after the last.
     *
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x00223668
     * @ghidraAddress PAL: 0x0022c438
     */
    int DrawShowing() override;

    /**
     * Write a description of the cursor and its bases.
     *
     * The mask and the internal blur are listed at a dump level of 2 and above.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002228a0
     * @ghidraAddress PAL: 0x0022b670
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the cursor and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00222a30
     * @ghidraAddress PAL: 0x0022b800
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced text or blur with another, and rebuild what depends on it.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x002233b8
     * @ghidraAddress PAL: 0x0022c188
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00380180
     * @ghidraAddress PAL: 0x003ee898
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another cursor and its bases.
     *
     * @param pSource The cursor to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x002227d0
     * @ghidraAddress PAL: 0x0022b5a0
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00222b30
     * @ghidraAddress PAL: 0x0022b900
     */
    void Load(BinStream &stream) override;

    /**
     * Set the text the cursor sweeps, then rebuild the mask.
     *
     * @param pText The text, or null.
     * @ghidraAddress NTSC-U/C: 0x00224080
     * @ghidraAddress PAL: 0x0022ce50
     */
    void SetText(RndText *pText);

    /**
     * Take the references on the text and the blur, then build the mask and the internal blur.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00222d30
     * @ghidraAddress PAL: 0x0022bb00
     */
    void AcquireRefs();

    /**
     * Drop the references on the text and the blur, and delete the mask and the internal blur.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00223318
     * @ghidraAddress PAL: 0x0022c0e8
     */
    void ReleaseRefs();

    /**
     * Replace the internal blur with a copy of mBlur, whose mesh is not drawn by itself.
     *
     * A blur without a mesh gives no internal blur. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00222d90
     * @ghidraAddress PAL: 0x0022bb60
     */
    void BuildCursor();

    /**
     * Replace the mask with a quad over the line of the text, and measure the line.
     *
     * Text without a font gives no mask, and text of more than one line is refused with a notice.
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00222eb0
     * @ghidraAddress PAL: 0x0022bc80
     */
    void BuildMask();

    /**
     * Report the character the cursor has reached, resting on each for mStutter of a frame.
     *
     * The name is inferred.
     *
     * @return The position, in characters.
     * @ghidraAddress NTSC-U/C: 0x00223598
     * @ghidraAddress PAL: 0x0022c368
     */
    float CursorPos();

    /**
     * Create a cursor.
     *
     * @param pszName The registry key.
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x00380190
     * @ghidraAddress PAL: 0x003ee8a8
     */
    static RndObject *New(const char *pszName) {
        return new RndCursor(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Cursor`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b099c
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09a0
     */
    static int sRev;

    /**
     * The version Load() read last.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c684
     */
    static int sLoadRev;

    RndText *mText;   /*!< The text the cursor sweeps, or null. */
    RndBlur *mBlur;   /*!< The blur the internal blur copies, or null. */
    RndBlur *mCursor; /*!< The internal blur that marks the cursor, or null. */
    RndMesh *mMask;   /*!< The mesh that hides the characters past the cursor, or null. */
    float mOffset;    /*!< The depth of the mask in front of the text. */
    float mStutter;   /*!< The part of each frame the cursor rests on its character. */
    float mWidth;     /*!< The width of the line, measured by BuildMask(). */
    float mStartX;    /*!< The left of the line, measured by BuildMask(). */
    float mCenterZ;   /*!< The vertical centre of the line, measured by BuildMask(). */
};
