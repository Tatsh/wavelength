#pragma once

#include "math/color.h"
#include "rnd/rndmesh.h"

class Avatar;

/**
 * The model of one part of an avatar that an Avatar draws.
 *
 * The RTTI includes the name. The object is 0x70 bytes, and the vptr follows the data members at
 * `+0x64`. TorsoPart, InstrPart, and ArmPart derive from it. Only the members Avatar and the Freq
 * maker reach are declared.
 */
class AvatarPart {
public:
    /**
     * Construct the model of a part from its entry of the `parts` array of the `avatar` entry of
     * the "db" section.
     *
     * @param nPart The part, one of AvatarPartSet::Part.
     * @param nParent The part the model hangs from, or AvatarPartSet::kNumParts for none.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x00272cf8
     * @ghidraAddress PAL: 0x0027c818
     */
    AvatarPart(int nPart, int nParent, Avatar *pAvatar);

    /**
     * Release the model.
     *
     * @ghidraAddress NTSC-U/C: 0x00273108
     * @ghidraAddress PAL: 0x0027cc28
     */
    virtual ~AvatarPart();

    /**
     * Pose the model at one frame of its animations.
     *
     * The name is inferred.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x00273350
     * @ghidraAddress PAL: 0x0027ce70
     */
    virtual void SetFrame(float fFrame);

    /**
     * Pose the model between two frames of its animations.
     *
     * The name is inferred.
     *
     * @param fFrom The frame blended from.
     * @param fTo The frame blended to.
     * @param fBlend The weight of fTo, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x002733d0
     * @ghidraAddress PAL: 0x0027cef0
     */
    virtual void SetFrames(float fFrom, float fTo, float fBlend);

    /**
     * Bring the transforms of the model up to date.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00274358
     * @ghidraAddress PAL: 0x0027de78
     */
    virtual void Update();

    /**
     * Start loading the files of a choice of the part.
     *
     * The name is inferred.
     *
     * @param pszType The choice, a symbol.
     * @ghidraAddress NTSC-U/C: 0x00272b48
     * @ghidraAddress PAL: 0x0027c668
     */
    virtual void SetType(const char *pszType);

    /**
     * Build the meshes of the model from the files SetType() loaded.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00273e70
     * @ghidraAddress PAL: 0x0027d990
     */
    virtual void Rebuild();

    /**
     * Release the files SetType() loaded.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00272ca0
     * @ghidraAddress PAL: 0x0027c7c0
     */
    virtual void ReleaseType();

    /**
     * Draw the model, when it has a mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002732f0
     * @ghidraAddress PAL: 0x0027ce10
     */
    void Draw();

    /**
     * Set the frame of the blend animation of the model, when it has one.
     *
     * The name is inferred.
     *
     * @param fBlend The frame, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00273318
     * @ghidraAddress PAL: 0x0027ce38
     */
    void SetBlend(float fBlend);

    /**
     * Colour the model.
     *
     * @param pColor The colour.
     * @ghidraAddress NTSC-U/C: 0x00274448
     * @ghidraAddress PAL: 0x0027df68
     */
    void SetColor(const Color *pColor);

    const char *mType;        /*!< The choice the meshes show, or null before the first. */
    const char *mPendingType; /*!< The choice SetType() last requested, or null. */
    RndMesh *mTransRoot;      /*!< The `trans_root` mesh the model hangs from, or null. */
};
