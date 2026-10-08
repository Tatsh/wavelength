#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndtext.h"

/**
 * Text cursor that highlights a text object.
 *
 * The RTTI includes the class name and records RndAnimatable and RndDrawable as bases. Only the
 * members ported so far are declared.
 */
class RndCursor : public RndAnimatable, public RndDrawable {
public:
    /**
     * Write a description of the cursor and its bases.
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
     * Replace a referenced object with another.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x002233b8
     * @ghidraAddress PAL: 0x0022c188
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

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
     * Set the text the cursor highlights, then rebuild the highlight mask.
     *
     * @param pText The text, or null.
     * @ghidraAddress NTSC-U/C: 0x00224080
     * @ghidraAddress PAL: 0x0022ce50
     */
    void SetText(RndText *pText);

    RndText *mText; /*!< The text the cursor highlights. */
};
