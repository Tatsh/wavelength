#pragma once

#include <cstddef>

#include "math/color.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndtransformable.h"

/**
 * Light, placed by its transform.
 *
 * The RTTI includes the class name and records RndTransformable as the one base.
 */
class RndLight : public RndTransformable {
public:
    /** The kinds of light. */
    enum Type {
        kTypePoint = 0,       /*!< Light from a point in every direction. */
        kTypeDirectional = 1, /*!< Light in one direction from infinitely far. */
        kTypeSpot = 2,        /*!< Light from a point in a cone. */
    };

    /**
     * Construct a white directional light with a range of 1000, cone angles of a quarter turn,
     * and no attenuation.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x002295d0
     * @ghidraAddress PAL: 0x00232340
     */
    explicit RndLight(const char *pszName);

    /**
     * Destroy the light.
     *
     * @ghidraAddress NTSC-U/C: 0x00382d58
     * @ghidraAddress PAL: 0x003f1460
     */
    ~RndLight() override;

    /**
     * Allocate a light, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Light", 0);
    }

    /**
     * Release a light.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Set the colour.
     *
     * @param color The colour.
     * @ghidraAddress NTSC-U/C: 0x00382cf0
     * @ghidraAddress PAL: 0x003f13f8
     */
    virtual void SetColor(const Color &color) {
        mColor = color;
    }

    /**
     * Set the kind of light.
     *
     * @param nType The Type.
     * @ghidraAddress NTSC-U/C: 0x00382d00
     * @ghidraAddress PAL: 0x003f1408
     */
    virtual void SetType(int nType) {
        mType = nType;
    }

    /**
     * Set the range.
     *
     * @param fRange The range.
     * @ghidraAddress NTSC-U/C: 0x00382d08
     * @ghidraAddress PAL: 0x003f1410
     */
    virtual void SetRange(float fRange) {
        mRange = fRange;
    }

    /**
     * Set the angles of the cone of a spot light.
     *
     * The name is inferred.
     *
     * @param fInnerAng The angle of full light.
     * @param fOuterAng The angle light ends at.
     * @ghidraAddress NTSC-U/C: 0x00382d10
     * @ghidraAddress PAL: 0x003f1418
     */
    virtual void SetAngles(float fInnerAng, float fOuterAng) {
        mOuterAng = fOuterAng;
        mInnerAng = fInnerAng;
    }

    /**
     * Set the attenuation.
     *
     * The name is inferred.
     *
     * @param fConstant The constant attenuation.
     * @param fLinear The attenuation in proportion to the distance.
     * @param fQuadratic The attenuation in proportion to the square of the distance.
     * @ghidraAddress NTSC-U/C: 0x00382d20
     * @ghidraAddress PAL: 0x003f1428
     */
    virtual void SetAttenuation(float fConstant, float fLinear, float fQuadratic) {
        mQuadraticAtten = fQuadratic;
        mConstantAtten = fConstant;
        mLinearAtten = fLinear;
    }

    /**
     * Pass the settings to the back end.
     *
     * The base body does nothing. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00382f88
     * @ghidraAddress PAL: 0x003f1690
     */
    virtual void Sync() {
    }

    /**
     * Write a description of the light and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00229928
     * @ghidraAddress PAL: 0x00232698
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the light and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00229aa0
     * @ghidraAddress PAL: 0x00232810
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another in the base.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00229eb8
     * @ghidraAddress PAL: 0x00232c28
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00382d48
     * @ghidraAddress PAL: 0x003f1450
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another light and its base, then pass the settings to the back end.
     *
     * @param pSource The light to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00229dd8
     * @ghidraAddress PAL: 0x00232b48
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, then pass the settings to the back end.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00229c08
     * @ghidraAddress PAL: 0x00232978
     */
    void Load(BinStream &stream) override;

    /**
     * Create the default light, add it to the default environment, and place it under the default
     * camera.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00229520
     * @ghidraAddress PAL: 0x00232290
     */
    static void CreateDefault();

    /**
     * Create a light.
     *
     * @param pszName The registry key.
     * @return The light.
     * @ghidraAddress NTSC-U/C: 0x00382f98
     * @ghidraAddress PAL: 0x003f16a0
     */
    static RndObject *New(const char *pszName) {
        return new RndLight(pszName);
    }

    /**
     * The light CreateDefault() made.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09c8
     */
    static RndLight *sDefault;

    /**
     * The class name a `.rnd` file writes, `Light`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09cc
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09d0
     */
    static int sRev;

    Color mColor;          /*!< The colour. */
    float mInnerAng;       /*!< The angle of full light of a spot light. */
    float mOuterAng;       /*!< The angle a spot light ends at. */
    float mRange;          /*!< The range. */
    float mConstantAtten;  /*!< The constant attenuation. */
    float mLinearAtten;    /*!< The attenuation in proportion to the distance. */
    float mQuadraticAtten; /*!< The attenuation in proportion to the square of the distance. */
    int mType;             /*!< The Type. */
};

/**
 * Write the name of a kind of light. An unknown kind writes nothing.
 *
 * @param stream The stream to write to.
 * @param eType The kind.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00229ed8
 * @ghidraAddress PAL: 0x00232c48
 */
PrnStream &operator<<(PrnStream &stream, RndLight::Type eType);
