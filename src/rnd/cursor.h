#pragma once

#include "rnd/animatable.h"
#include "rnd/drawable.h"

namespace Rnd {

/**
 * Animated highlight that moves over a view.
 *
 * The RTTI records the class as deriving from Drawable and from Animatable at `+0x20`. Only the
 * members its callers here use are declared.
 */
class Cursor : public Drawable, public Animatable {
public:
    /**
     * Release the cursor.
     *
     * @ghidraAddress NTSC-U/C: 0x00223930
     * @ghidraAddress PAL: 0x0022c700
     */
    ~Cursor() override;

    /**
     * Write the cursor as text.
     *
     * @param sink The destination.
     * @ghidraAddress NTSC-U/C: 0x002228a0
     * @ghidraAddress PAL: 0x0022b670
     */
    void DumpText(Dbg &sink) override;

    /**
     * Write the cursor to a stream.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00222a30
     * @ghidraAddress PAL: 0x0022b800
     */
    void Save(Stream &stream) override;

    /**
     * Replace a referenced object.
     *
     * @param pFrom The object to replace.
     * @param pTo The replacement.
     * @ghidraAddress NTSC-U/C: 0x002233b8
     * @ghidraAddress PAL: 0x0022c188
     */
    void Replace(Object *pFrom, Object *pTo) override;

    /**
     * Copy another cursor.
     *
     * @param pSource The cursor to copy.
     * @param nFlags The copy flags.
     * @ghidraAddress NTSC-U/C: 0x002227d0
     * @ghidraAddress PAL: 0x0022b5a0
     */
    void Copy(const Object *pSource, unsigned nFlags) override;

    /**
     * Read the cursor from a stream.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00222b30
     * @ghidraAddress PAL: 0x0022b900
     */
    void Load(Stream &stream) override;
};

} // namespace Rnd
