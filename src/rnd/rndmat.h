#pragma once

#include <cstddef>
#include <vector>

#include "math/color.h"
#include "math/transform.h"
#include "math/triangle.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndobject.h"
#include "rnd/rndtex.h"

/**
 * Surface description: blending, lighting colours, culling, and a list of texture stages.
 *
 * The RTTI includes the class name and records RndObject as the one base. PsMat derives from it.
 */
class RndMat : public RndObject {
public:
    /** How a surface or a texture stage combines with what is beneath it. */
    enum Blend {
        kBlendDest = 0,            /*!< Keep the destination. */
        kBlendSrc = 1,             /*!< Replace the destination. */
        kBlendAdd = 2,             /*!< Add the source to the destination. */
        kBlendMultiply = 3,        /*!< Multiply the destination by the source. */
        kBlendMultiply2 = 4,       /*!< Multiply the destination by twice the source. */
        kBlendSrcAlpha = 5,        /*!< Blend by the source alpha. */
        kBlendSrcAlphaAdd = 6,     /*!< Add the source scaled by its alpha. */
        kBlendSrcAdd = 7,          /*!< Add the source. */
        kBlendInvSrcAlpha = 8,     /*!< Blend by the inverse of the source alpha. */
        kBlendDestAlpha = 9,       /*!< Blend by the destination alpha. */
        kBlendInvDestAlpha = 10,   /*!< Blend by the inverse of the destination alpha. */
        kBlendSrcAlphaOpaque = 11, /*!< Blend by the source alpha and write opaque alpha. */
        kBlendSrcAlphaCutout = 12, /*!< Discard the source below an alpha threshold. */
        kBlendSubtract = 13,       /*!< Subtract the source from the destination. */
    };

    /** How a texture stage produces its texture coordinates. */
    enum TexGen {
        kTexGenFixed = 0,     /*!< Use the vertex coordinates. */
        kTexGenSphere = 1,    /*!< Sphere map. */
        kTexGenPlanar = 2,    /*!< Planar projection. */
        kTexGenOrthoCube = 3, /*!< Cube map in object space. */
        kTexGenLocalCube = 4, /*!< Cube map in local space. */
    };

    /** How texture coordinates outside the unit range wrap. */
    enum TexWrap {
        kTexWrapClamp = 0,  /*!< Clamp to the edge. */
        kTexWrapRepeat = 1, /*!< Repeat. */
        kTexWrapMirror = 2, /*!< Repeat, mirroring every other copy. */
    };

    /**
     * One texture stage.
     *
     * The RTTI includes the nested class name.
     */
    class Stage {
    public:
        /**
         * Construct a multiplying stage with an identity transform and no texture.
         *
         * @ghidraAddress NTSC-U/C: 0x0022ebb8
         * @ghidraAddress PAL: 0x002377a0
         */
        Stage();

        /**
         * Set the texture, moving the material's reference to it.
         *
         * @param pTex The texture, or null.
         * @ghidraAddress NTSC-U/C: 0x0022ec28
         * @ghidraAddress PAL: 0x00237810
         */
        void SetTex(RndTex *pTex);

        /**
         * Write a description of the stage.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0022ec78
         * @ghidraAddress PAL: 0x00237860
         */
        void Print(PrnStream &stream) const;

        /**
         * Write the stage.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x0022ed98
         * @ghidraAddress PAL: 0x00237980
         */
        void Save(BinStream &stream) const;

        /**
         * Read what Save() wrote, or an earlier version of it.
         *
         * The version is the one RndMat::Load() read last.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x0022efc8
         * @ghidraAddress PAL: 0x00237ba0
         */
        void Load(BinStream &stream);

        int mBlend;      /*!< The Blend of the stage. */
        int mCoordIndex; /*!< Which vertex texture coordinates the stage reads. */
        int mGenMode;    /*!< The TexGen of the stage. */
        Transform mXfm;  /*!< The texture coordinate transform. */
        int mUseXfm;     /*!< Non-zero to apply mXfm. */
        int mWrap;       /*!< The TexWrap of the stage. */
        RndTex *mTex;    /*!< The texture, or null. */
        RndMat *mMat;    /*!< The material that has the stage. */
    };

    /**
     * Construct an enabled material that blends by source alpha, with a black base colour, a white
     * light colour, and a black edge colour.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0022dd00
     * @ghidraAddress PAL: 0x002369b8
     */
    explicit RndMat(const char *pszName);

    /**
     * Drop the references on the stage textures.
     *
     * @ghidraAddress NTSC-U/C: 0x00384ea8
     * @ghidraAddress PAL: 0x003f35b0
     */
    ~RndMat() override;

    /**
     * Allocate a material, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Mat", 0);
    }

    /**
     * Release a material.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Write a description of the material.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022ddb8
     * @ghidraAddress PAL: 0x00236a70
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the material.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022dfe0
     * @ghidraAddress PAL: 0x00236c98
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a stage texture with another object.
     *
     * A replacement that is not a texture, or null, clears the stage texture.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0022dc00
     * @ghidraAddress PAL: 0x002368b8
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00384f68
     * @ghidraAddress PAL: 0x003f3670
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another material.
     *
     * The flat flag is not copied.
     *
     * @param pSource The material to copy from.
     * @param nFlags The set of fields to copy. The routine ignores it.
     * @ghidraAddress NTSC-U/C: 0x0022e9e0
     * @ghidraAddress PAL: 0x002375c8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0022e3b8
     * @ghidraAddress PAL: 0x00237018
     */
    void Load(BinStream &stream) override;

    /**
     * Bring the platform state of one stage up to date.
     *
     * The base body is empty. The name is inferred.
     *
     * @param nStage The stage.
     * @ghidraAddress NTSC-U/C: 0x00384e40
     * @ghidraAddress PAL: 0x003f3548
     */
    virtual void SyncStage([[maybe_unused]] int nStage) {
    }

    /**
     * Set the red, green, and blue of the base colour.
     *
     * @param color The colour. Its alpha is ignored.
     * @ghidraAddress NTSC-U/C: 0x00384e48
     * @ghidraAddress PAL: 0x003f3550
     */
    virtual void SetBaseColor(const Color &color) {
        mBaseColor.r = color.r;
        mBaseColor.b = color.b;
        mBaseColor.g = color.g;
    }

    /**
     * Set the light colour.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00384e68
     * @ghidraAddress PAL: 0x003f3570
     */
    virtual void SetLightColor(const Color &color) {
        mLightColor = color;
    }

    /**
     * Set the edge colour.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00384e78
     * @ghidraAddress PAL: 0x003f3580
     */
    virtual void SetEdgeColor(const Color &color) {
        mEdgeColor = color;
    }

    /**
     * Set the alpha of the base colour.
     *
     * @param fAlpha The alpha.
     * @ghidraAddress NTSC-U/C: 0x00384e88
     * @ghidraAddress PAL: 0x003f3590
     */
    virtual void SetAlpha(float fAlpha) {
        mBaseColor.a = fAlpha;
    }

    /**
     * Set the lighting switches.
     *
     * @param bEnable Non-zero to light the surface.
     * @param bVertBase Non-zero to take the base colour from the vertices.
     * @param bVertLight Non-zero to take the light colour from the vertices.
     * @param bVertEdge Non-zero to take the edge colour from the vertices.
     * @param bNormalize Non-zero to normalise the normals.
     * @param bBaseAmbient Non-zero to light the base colour ambiently.
     * @ghidraAddress NTSC-U/C: 0x0022dbe0
     * @ghidraAddress PAL: 0x00236898
     */
    virtual void SetLighting(int bEnable,
                             int bVertBase,
                             int bVertLight,
                             int bVertEdge,
                             int bNormalize,
                             int bBaseAmbient);

    /**
     * Take a reference on every stage texture and bring every stage up to date.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022e328
     * @ghidraAddress PAL: 0x00236f88
     */
    virtual void Refresh();

    /**
     * Append a default stage.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022ead0
     * @ghidraAddress PAL: 0x002376b8
     */
    void AddStage();

    /**
     * Create a material.
     *
     * @param pszName The registry key.
     * @return The material.
     * @ghidraAddress NTSC-U/C: 0x00384f78
     * @ghidraAddress PAL: 0x003f3680
     */
    static RndObject *New(const char *pszName) {
        return new RndMat(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Mat`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09dc
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09e0
     */
    static int sRev;

    /**
     * The version Load() read last, which Stage::Load() consults.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c688
     */
    static int sLoadRev;

    std::vector<Stage> mStages; /*!< The texture stages. */
    int mBlend;                 /*!< The Blend of the surface. */
    Color mBaseColor;           /*!< The base colour. */
    Color mLightColor;          /*!< The light colour. */
    Color mEdgeColor;           /*!< The edge colour. */
    int mEnable;                /*!< Non-zero to light the surface. */
    int mVertBase;              /*!< Non-zero to take the base colour from the vertices. */
    int mVertLight;             /*!< Non-zero to take the light colour from the vertices. */
    int mVertEdge;              /*!< Non-zero to take the edge colour from the vertices. */
    int mNormalize;             /*!< Non-zero to normalise the normals. */
    int mBaseAmbient;           /*!< Non-zero to light the base colour ambiently. */
    int mCull;                  /*!< The CullMode of the surface. */
    int mFlat;                  /*!< Non-zero for flat shading. */
    int mMultiPass;             /*!< The number of extra passes. */

protected:
    /**
     * Drop the reference on every stage texture.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022e2c0
     * @ghidraAddress PAL: 0x00236f20
     */
    void ReleaseStageTex();
};

/**
 * Write the name of a blend mode.
 *
 * @param stream The stream to write to.
 * @param eBlend The mode. An unknown mode writes nothing.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0022f330
 * @ghidraAddress PAL: 0x00237ef8
 */
PrnStream &operator<<(PrnStream &stream, RndMat::Blend eBlend);

/**
 * Write the name of a texture coordinate generator.
 *
 * @param stream The stream to write to.
 * @param eTexGen The generator. An unknown generator writes nothing.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0022f4c0
 * @ghidraAddress PAL: 0x00238088
 */
PrnStream &operator<<(PrnStream &stream, RndMat::TexGen eTexGen);

/**
 * Write the name of a wrap mode.
 *
 * @param stream The stream to write to.
 * @param eWrap The mode. An unknown mode writes nothing.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0022f578
 * @ghidraAddress PAL: 0x00238140
 */
PrnStream &operator<<(PrnStream &stream, RndMat::TexWrap eWrap);

/**
 * Write the stages of a material as their count followed by each stage on its own line.
 *
 * @param stream The stream to write to.
 * @param stages The stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00384360
 * @ghidraAddress PAL: 0x003f2a68
 */
PrnStream &operator<<(PrnStream &stream, const std::vector<RndMat::Stage> &stages);

/**
 * Write the stages of a material as their count followed by each stage.
 *
 * @param stream The stream to write to.
 * @param stages The stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00384458
 * @ghidraAddress PAL: 0x003f2b60
 */
BinStream &operator<<(BinStream &stream, const std::vector<RndMat::Stage> &stages);

/**
 * Read the stages of a material the stage writer wrote.
 *
 * @param stream The stream to read from.
 * @param stages Receives the stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003844f0
 * @ghidraAddress PAL: 0x003f2bf8
 */
BinStream &operator>>(BinStream &stream, std::vector<RndMat::Stage> &stages);
