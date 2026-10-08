#pragma once

#include "rnd/rnddrawable.h"
#include "rnd/rndtransformable.h"

/**
 * Camera.
 *
 * The RTTI includes the class name and records RndDrawable and RndTransformable as bases. Only the
 * members ported so far are declared.
 */
class RndCam : public RndDrawable, public RndTransformable {
public:
    /**
     * Release the camera.
     *
     * @ghidraAddress NTSC-U/C: 0x00221498
     * @ghidraAddress PAL: 0x0022a268
     */
    ~RndCam() override;

    /**
     * Write a description of the camera.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00220738
     * @ghidraAddress PAL: 0x00229508
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the camera.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00220960
     * @ghidraAddress PAL: 0x00229730
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00220e48
     * @ghidraAddress PAL: 0x00229c18
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0037f268
     * @ghidraAddress PAL: 0x003ed980
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another camera.
     *
     * @param pSource The camera to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00220d38
     * @ghidraAddress PAL: 0x00229b08
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00220ae8
     * @ghidraAddress PAL: 0x002298b8
     */
    void Load(BinStream &stream) override;

    /**
     * The camera drawing is set up for.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0988
     */
    static RndCam *sCurrent;

    /**
     * The class name a `.rnd` file writes, `Cam`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0990
     */
    static const char *sClassName;
};
