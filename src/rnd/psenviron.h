#pragma once

#include "math/color.h"
#include "math/sphere.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/rndenviron.h"

/**
 * Environment, PlayStation 2.
 *
 * The RTTI includes the class name and records RndEnviron as the one base. The class adds no data
 * member. DrawShowing() turns the fog and the lights into the globals below, and SelectLights()
 * sends them to VU1 ahead of each mesh. One point light, or else up to three directional lights,
 * light the meshes. Spot lights are ignored.
 */
class PsEnviron : public RndEnviron {
public:
    /** The VU1 lighting microprograms, by their addresses in VU1 code memory. */
    enum LightProgram {
        kLightProgramPoint = 0x6c,        /*!< One point light. */
        kLightProgramDirectional = 0x260, /*!< Up to three directional lights. */
        kLightProgramAmbient = 0x373,     /*!< The ambient colour only. */
        kLightProgramUnlit = 0x3d7,       /*!< No lighting. */
    };

    /**
     * Construct an environment.
     *
     * The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit PsEnviron(const char *pszName) : RndObject(pszName), RndEnviron(pszName) {
    }

    /**
     * @ghidraAddress NTSC-U/C: 0x0037d2e0
     * @ghidraAddress PAL: 0x003eba10
     */
    ~PsEnviron() override {
    }

    /**
     * Set the fog and the lights up for the drawables after the environment, and make it current.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x00219470
     * @ghidraAddress PAL: 0x00222288
     */
    int DrawShowing() override;

    /**
     * Add the lighting block of the VU1 mesh microprogram to the packet, eight quadwords.
     *
     * The block is a quadword of settings, the base colour, and the lights moved into the space of
     * the object. Without lighting the block is the settings alone and the cursor skips the rest.
     * A point light out of reach of the bounds of the object, and an object without a transform,
     * take the ambient program. The name is inferred.
     *
     * @param pXfm The transform of the object drawn, or null.
     * @param pSphere The bounding sphere of the object in its own space. A radius of 0 skips the
     * reach test. It is read only for a point light and a transform.
     * @ghidraAddress NTSC-U/C: 0x002197a0
     * @ghidraAddress PAL: 0x002225b8
     */
    static void SelectLights(const Transform *pXfm, const Sphere *pSphere);

    /**
     * Install the environment factory for the class `Environ` of a `.rnd` file, and create the
     * default environment and light.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00219268
     * @ghidraAddress PAL: 0x00222080
     */
    static void Init();

    /**
     * Restore the generic environment factory.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00219370
     * @ghidraAddress PAL: 0x00222188
     */
    static void Terminate();

    /**
     * Create an environment.
     *
     * @param pszName The registry key.
     * @return The environment.
     * @ghidraAddress NTSC-U/C: 0x0037d680
     * @ghidraAddress PAL: 0x003ebdb0
     */
    static RndObject *New(const char *pszName) {
        return new PsEnviron(pszName);
    }
};

/**
 * Scale from a depth to the GS fog range, 255 over the fog distance, or 0 without fog.
 *
 * @ghidraAddress NTSC-U/C: 0x003b07e0
 * @ghidraAddress PAL: 0x0042f678
 */
extern float g_fogScale;

/**
 * Offset of the GS fog range, minus the fog end times g_fogScale, or 255 without fog.
 *
 * @ghidraAddress NTSC-U/C: 0x003b07e4
 * @ghidraAddress PAL: 0x0042f67c
 */
extern float g_fogOffset;

/**
 * Non-zero while the current environment has fog.
 *
 * @ghidraAddress NTSC-U/C: 0x003b07e8
 * @ghidraAddress PAL: 0x0042f680
 */
extern int g_nFogEnabled;

/**
 * The lighting microprogram of the current environment.
 *
 * @ghidraAddress NTSC-U/C: 0x003b07ec
 * @ghidraAddress PAL: 0x0042f684
 */
extern PsEnviron::LightProgram g_lightProgram;

/**
 * Ambient colour of the current environment, with alpha 1.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c610
 */
extern Color g_ambientColor;

/**
 * The lights of the current environment as a matrix.
 *
 * For directional lights, column n of the first three components is the negated direction of
 * light n. For a point light, the first row is the position, and the fourth components are -1
 * over the range, the range squared, and the range.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c620
 */
extern Vector3 g_lightMatrix[3];

/**
 * Colour of each light of the current environment, with alpha 0. Unused rows are black.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c650
 */
extern Color g_lightColors[3];
