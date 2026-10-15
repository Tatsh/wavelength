#pragma once

#include <cstddef>

#include "math/color.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "os/mem.h"
#include "rnd/rndmat.h"

/**
 * Material, PlayStation 2.
 *
 * The RTTI includes the class name and records RndMat as the one base. The class adds no data
 * member and overrides only the destructor. It selects a material for drawing: the GS blend,
 * alpha test, and clamp registers, the stage texture, and the globals the mesh and particle paths
 * read. The material last selected is cached in sCurrent.
 */
class PsMat : public RndMat {
public:
    /**
     * Construct a material.
     *
     * The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit PsMat(const char *pszName) : RndMat(pszName) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x0037cb18
     * @ghidraAddress PAL: 0x003eb248
     */
    ~PsMat() override {
    }

    /**
     * Allocate a material from its pool.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(PsMat), "PsMat", 0);
    }

    /**
     * Return a material to its pool.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolFree(sizeof(PsMat), pBlock);
    }

    /**
     * Select one pass of the material for drawing.
     *
     * A material that is already selected and has fewer than two stages changes nothing, beyond
     * selecting its blend again after SelectAlphaBlend(). Otherwise the blend, the stage texture,
     * and the texture coordinate transform of the pass are selected.
     *
     * @param nPass The pass. The pass is also the stage.
     * @return The next pass, or 0 after the last.
     * @ghidraAddress NTSC-U/C: 0x00213698
     * @ghidraAddress PAL: 0x0021c4b0
     */
    int Select(int nPass);

    /**
     * Program the GS blend, dither, alpha test, and alpha correction for a pass.
     *
     * The first pass takes the surface blend and the later passes the blend of their stage. The
     * name is inferred.
     *
     * @param nPass The pass.
     * @ghidraAddress NTSC-U/C: 0x00213788
     * @ghidraAddress PAL: 0x0021c5a0
     */
    void SelectBlendMode(int nPass);

    /**
     * Clamp the texture coordinates of a stage, or let them repeat.
     *
     * The name is inferred.
     *
     * @param stage The stage.
     * @ghidraAddress NTSC-U/C: 0x00213a88
     * @ghidraAddress PAL: 0x0021c8a0
     */
    void SelectStageClamp(const Stage &stage);

    /**
     * Report the GS texture function a blend draws its texture with.
     *
     * The name is inferred.
     *
     * @param nBlend The Blend.
     * @return The TFX field of TEX0.
     * @ghidraAddress NTSC-U/C: 0x00213ad8
     * @ghidraAddress PAL: 0x0021c8f0
     */
    int TexFunction(int nBlend);

    /**
     * Bind the texture of a pass's stage and set the colour scale for its texture function.
     *
     * A stage without a texture, or with RndMat::kBlendDest, binds nothing. The name is inferred.
     *
     * @param nPass The pass.
     * @ghidraAddress NTSC-U/C: 0x00213af0
     * @ghidraAddress PAL: 0x0021c908
     */
    void BindStageTexture(int nPass);

    /**
     * Build the texture coordinate transform of a pass's stage into g_uvXfm and g_stageXfm.
     *
     * The name is inferred.
     *
     * @param nPass The pass.
     * @ghidraAddress NTSC-U/C: 0x00213c70
     * @ghidraAddress PAL: 0x0021ca88
     */
    void SetupUvXfm(int nPass);

    /**
     * Forget the selected material and return the GS to opaque drawing with the default dither.
     *
     * @ghidraAddress NTSC-U/C: 0x00213570
     * @ghidraAddress PAL: 0x0021c388
     */
    static void SelectDefault();

    /**
     * Blend by the source alpha, and make the next Select() of the selected material select its
     * blend again.
     *
     * @ghidraAddress NTSC-U/C: 0x00213660
     * @ghidraAddress PAL: 0x0021c478
     */
    static void SelectAlphaBlend();

    /**
     * Install the material factory for the class `Mat` of a `.rnd` file.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00213e38
     * @ghidraAddress PAL: 0x0021cc50
     */
    static void Init();

    /**
     * Create a material.
     *
     * @param pszName The registry key.
     * @return The material.
     * @ghidraAddress NTSC-U/C: 0x0037ca68
     * @ghidraAddress PAL: 0x003eb198
     */
    static RndObject *New(const char *pszName) {
        return new PsMat(pszName);
    }

    /**
     * The material Select() last selected, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0768
     * @ghidraAddress PAL: 0x0042f600
     */
    static PsMat *sCurrent;
};

/**
 * Texture coordinate generation mode of the stage PsMat::SetupUvXfm() last processed.
 *
 * @ghidraAddress NTSC-U/C: 0x003b076c
 * @ghidraAddress PAL: 0x0042f604
 */
extern int g_nSelectedGenMode;

/**
 * Flat shading flag of the selected material.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0770
 * @ghidraAddress PAL: 0x0042f608
 */
extern int g_nSelectedFlat;

/**
 * Non-zero while a stage texture is bound.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0774
 * @ghidraAddress PAL: 0x0042f60c
 */
extern int g_nTextured;

/**
 * Non-zero once ALPHA_1 is a blending equation rather than an opaque copy.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0778
 * @ghidraAddress PAL: 0x0042f610
 */
extern int g_nAlphaBlendEnabled;

/**
 * Lighting flag of the selected material, cleared when the stage texture is applied as a decal.
 *
 * @ghidraAddress NTSC-U/C: 0x003b077c
 * @ghidraAddress PAL: 0x0042f614
 */
extern int g_nLightingEnabled;

/**
 * Non-zero after PsMat::SelectAlphaBlend(), until the next PsMat::Select() of the selected
 * material selects its blend again.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0780
 * @ghidraAddress PAL: 0x0042f618
 */
extern int g_nReselectBlend;

/**
 * Scale from a vertex colour component to the GS range, 128 for a modulated texture and 255
 * otherwise, with alpha always 128.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c340
 */
extern Color g_colorScale;

/**
 * Transform of the texture coordinates of the selected stage.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c350
 */
extern Transform g_uvXfm;

/**
 * Basis rows of the stage transform a sphere-mapped stage applies before its sphere map.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c390
 */
extern Vector3 g_stageXfm[3];
