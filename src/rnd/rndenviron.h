#pragma once

#include <cstddef>
#include <list>

#include "math/color.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndlight.h"

/**
 * Lighting and fog environment, made current for the drawables after it when it is drawn.
 *
 * The RTTI includes the class name and records RndDrawable as the one base.
 */
class RndEnviron : public RndDrawable {
public:
    /** The fog modes. */
    enum FogMode {
        kFogNone = 0,        /*!< No fog. */
        kFogVertExp = 1,     /*!< Exponential fog per vertex. */
        kFogVertExp2 = 2,    /*!< Squared exponential fog per vertex. */
        kFogVertLinear = 3,  /*!< Linear fog per vertex. */
        kFogPixelExp = 4,    /*!< Exponential fog per pixel. */
        kFogPixelExp2 = 5,   /*!< Squared exponential fog per pixel. */
        kFogPixelLinear = 6, /*!< Linear fog per pixel. */
    };

    /** The bit of the Copy() flags that keeps the lights rather than copying the source's. */
    static constexpr int kCopyKeepLights = 1;

    /**
     * Construct an environment without lights, with a black ambient colour and no white fog.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00224cf0
     * @ghidraAddress PAL: 0x0022daa8
     */
    explicit RndEnviron(const char *pszName);

    /**
     * Drop the references on the lights.
     *
     * @ghidraAddress NTSC-U/C: 0x003812c8
     * @ghidraAddress PAL: 0x003ef928
     */
    ~RndEnviron() override;

    /**
     * Allocate an environment, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Environ", 0);
    }

    /**
     * Release an environment.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Add the lights to a list.
     *
     * The children are not added.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00225818
     * @ghidraAddress PAL: 0x0022e5d0
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Make the environment current.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x003812a0
     */
    int DrawShowing() override {
        sCurrent = this;
        return 1;
    }

    /**
     * Write a description of the environment and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00224fc8
     * @ghidraAddress PAL: 0x0022dd80
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the environment and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00225128
     * @ghidraAddress PAL: 0x0022dee0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced light with another.
     *
     * A light that becomes null is removed. A replacement already among the lights is reported.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00224b30
     * @ghidraAddress PAL: 0x0022d8e8
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00381528
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another environment and its base.
     *
     * @param pSource The environment to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00225540
     * @ghidraAddress PAL: 0x0022e2f8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002253e0
     * @ghidraAddress PAL: 0x0022e198
     */
    void Load(BinStream &stream) override;

    /**
     * Add a light, or report one already among the lights.
     *
     * @param pLight The light.
     * @ghidraAddress NTSC-U/C: 0x00225720
     * @ghidraAddress PAL: 0x0022e4d8
     */
    void AddLight(RndLight *pLight);

    /**
     * Create the default environment and add it to the default camera.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00224a98
     * @ghidraAddress PAL: 0x0022d850
     */
    static void CreateDefault();

    /**
     * Create an environment.
     *
     * @param pszName The registry key.
     * @return The environment.
     * @ghidraAddress NTSC-U/C: 0x00381540
     * @ghidraAddress PAL: 0x003efc58
     */
    static RndObject *New(const char *pszName) {
        return new RndEnviron(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Environ`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09a8
     */
    static const char *sClassName;

    /**
     * The environment drawn last.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09ac
     */
    static RndEnviron *sCurrent;

    /**
     * The environment CreateDefault() made.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09b0
     */
    static RndEnviron *sDefault;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09b4
     */
    static int sRev;

    std::list<RndLight *> mLights; /*!< The lights. */
    Color mAmbientColor;           /*!< The ambient colour. */
    float mFogStart;               /*!< The distance fog starts at. */
    float mFogEnd;                 /*!< The distance fog is full at. */
    float mFogDensity;             /*!< The density of exponential fog. */
    Color mFogColor;               /*!< The fog colour. */
    int mFogMode;                  /*!< The FogMode. */

protected:
    /**
     * Drop the references on the lights.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002252d0
     * @ghidraAddress PAL: 0x0022e088
     */
    void ReleaseRefs();

    /**
     * Take references on the lights.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00225358
     * @ghidraAddress PAL: 0x0022e110
     */
    void AcquireRefs();
};

/**
 * Write the name of a fog mode. An unknown mode writes nothing.
 *
 * @param stream The stream to write to.
 * @param eMode The mode.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00225628
 * @ghidraAddress PAL: 0x0022e3e0
 */
PrnStream &operator<<(PrnStream &stream, RndEnviron::FogMode eMode);
