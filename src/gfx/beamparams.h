#pragma once

#include "math/color.h"
#include "math/vector2.h"
#include "os/string.h"
#include "rnd/line.h"
#include "rnd/mat.h"
#include "script/dataarray.h"

/**
 * Look of a family of beams, read from the configuration entry of its name.
 *
 * The class emits no RTTI, and the name is inferred. The object is 0x80 bytes.
 */
class BeamParams {
public:
    /** The ends of a beam, the indices of mColors. */
    enum End {
        kEndStart = 0,  /*!< The end at the ship. */
        kEndFinish = 1, /*!< The end at the gem. */
    };

    /**
     * Construct the look of the beams of a configuration entry.
     *
     * @param pszName The entry.
     * @ghidraAddress NTSC-U/C: 0x001ecea8
     * @ghidraAddress PAL: 0x001f5c48
     */
    explicit BeamParams(const char *pszName);

    /**
     * Destroy the look.
     *
     * @ghidraAddress NTSC-U/C: 0x001ecf50
     * @ghidraAddress PAL: 0x001f5cf0
     */
    ~BeamParams();

    /**
     * Read the look from the `beams` entry of the configuration.
     *
     * @param pConfig The section of the kind of game.
     * @param pDefaults The `gfx` section.
     * @param bReload Whether the configuration was read before. The routine does not read it.
     * @param fScale The scale of the tunnel, which scales the width and the amplitudes.
     * @ghidraAddress NTSC-U/C: 0x001ecf98
     * @ghidraAddress PAL: 0x001f5d38
     */
    void Load(DataArray *pConfig, DataArray *pDefaults, bool bReload, float fScale);

    /**
     * Give the look to the line and the material of a beam, once the look is loaded.
     *
     * @param pLine The line of the beam.
     * @param pMat The material of the beam, which copies mMat.
     * @ghidraAddress NTSC-U/C: 0x001ed210
     * @ghidraAddress PAL: 0x001f5fb0
     */
    void Apply(Rnd::Line *pLine, Rnd::Mat *pMat) const;

    /**
     * Report the colour of an end of a beam.
     *
     * @param nEnd The end, one of End.
     * @param pDefault The colour a colour of no alpha gives way to.
     * @return The colour of the end, or pDefault.
     * @ghidraAddress NTSC-U/C: 0x001ed2a0
     * @ghidraAddress PAL: 0x001f6040
     */
    const Color *GetColor(int nEnd, const Color *pDefault) const;

    String mName;             /*!< The name of the configuration entry. */
    int mPoints;              /*!< The points of the line, or -1 before Load(). */
    float mWidth;             /*!< The width of the line. */
    float mFoldAngle;         /*!< The fold of the line, in degrees. */
    Rnd::Mat *mMat;           /*!< The material each beam copies. */
    Color mColors[2];         /*!< The colour of each end, one of End. */
    float mAmplitudes[2];     /*!< The size of the waves at each end. */
    Vector2 mMinFrequency;    /*!< The fewest waves along a beam, of the two waves. */
    Vector2 mMaxFrequency;    /*!< The most waves along a beam, of the two waves. */
    Vector2 mMinTravelPeriod; /*!< The shortest period of the travel of the two waves. */
    Vector2 mMaxTravelPeriod; /*!< The longest period of the travel of the two waves. */
};
