#pragma once

#include <list>

#include "gfx/gfxarena.h"
#include "rnd/particlesys.h"
#include "rnd/transformable.h"

/**
 * Arena of the Constructo levels, whose building parts collide with planes that move with the
 * song.
 *
 * The RTTI identifies the class. The object is 0xac bytes.
 */
class Constructo : public GfxArena {
public:
    /** The level of the arena, from its name. */
    enum Level {
        kLevelP1 = 1,   /*!< `ConstructoP1`. */
        kLevelP2 = 2,   /*!< `ConstructoP2`. */
        kLevelP3 = 3,   /*!< `ConstructoP3`. */
        kLevelP4 = 4,   /*!< `ConstructoP4`. */
        kLevelBoss = 5, /*!< `Constructo_Boss`, which has no moving planes. */
    };

    /**
     * A particle system that collides with a plane of a moving transformable, 0xc bytes.
     *
     * The name is inferred.
     */
    class MovingPSPlane {
    public:
        /**
         * Set the collision plane from the transformable and advance the particles.
         *
         * @param fFrame The frame of the particles.
         * @ghidraAddress NTSC-U/C: 0x001edf98
         * @ghidraAddress PAL: 0x001f6d38
         */
        void Update(float fFrame);

        Rnd::ParticleSys *mParticles; /*!< The particles. */
        Rnd::Transformable *mPlane;   /*!< The transformable the plane moves with. */
        char mAxis; /*!< The world axis of mPlane that is the normal, `x`, `y`, or `z`. */
    };

    /**
     * Construct the arena of a name.
     *
     * @param pszName The arena.
     * @ghidraAddress NTSC-U/C: 0x001eda30
     * @ghidraAddress PAL: 0x001f67d0
     */
    explicit Constructo(const char *pszName);

    /**
     * Build the arena of a name that starts with `Constructo`.
     *
     * @param pszName The arena.
     * @return The arena, or null for another name.
     * @ghidraAddress NTSC-U/C: 0x001ee040
     * @ghidraAddress PAL: 0x001f6de0
     */
    static GfxArena *Create(const char *pszName);

    /**
     * Destroy the arena.
     *
     * @ghidraAddress NTSC-U/C: 0x001edb68
     * @ghidraAddress PAL: 0x001f6908
     */
    ~Constructo() override;

    /**
     * Fit the camera path to the song, and take the building parts out of the animation of their
     * view to collide them with their planes.
     *
     * @param fSongTicks The length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x001edbd0
     * @ghidraAddress PAL: 0x001f6970
     */
    void SetSongTicks(float fSongTicks) override;

    /**
     * Poll the arena and advance the building parts.
     *
     * @param fTick The tick of the song.
     * @param fTime The time of the display.
     * @ghidraAddress NTSC-U/C: 0x001edf10
     * @ghidraAddress PAL: 0x001f6cb0
     */
    void Poll(float fTick, float fTime) override;

    Level mLevel;                     /*!< The level, unset for a name of no level. */
    std::list<MovingPSPlane> mPlanes; /*!< The building parts and their planes. */
};
