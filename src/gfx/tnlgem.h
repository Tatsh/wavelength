#pragma once

#include <list>

#include "gfx/gfxtunnel.h"
#include "gfx/meshgroup.h"
#include "gfx/spritegroup.h"
#include "gfx/tnltrackrange.h"
#include "math/color.h"
#include "math/transform.h"
#include "rnd/particle.h"

class TnlGems;

/**
 * One gem on a track of the tunnel, 0x24 bytes.
 *
 * The class is not polymorphic, and the name is inferred. TnlGems stores the gems in a list sorted
 * by mTick. A mesh gem is drawn as an instance of a MeshGroup, and a sprite gem as a particle of a
 * SpriteGroup.
 */
class TnlGem {
public:
    /** Flags of mType. */
    enum {
        kTypeSprite = 0x40, /*!< The gem is a sprite rather than a mesh. */
    };

    /** Bits of mFlags. */
    enum {
        kFlagColorMask = 0x03,   /*!< The player colour of a gem with kFlagPlayerColor. */
        kFlagPlayerColor = 0x08, /*!< The sprite takes a player colour. */
        kFlagNoFlash = 0x10,     /*!< The gem does not flash as it passes. */
        kFlagFlash = 0x20,       /*!< The gem flashes as it passes. */
        kFlagPassed = 0x40,      /*!< The gem has passed and fired its trigger events. */
    };

    /** A tick that marks a gem as never removed. */
    static constexpr float kNever = 1e9f;

    /** A gem in slot n sits n times kSlotWidth plus kSlotOffset across its track. */
    static constexpr float kSlotWidth = 0.295f;

    /** The position across the track of slot 0. */
    static constexpr float kSlotOffset = 0.205f;

    /**
     * Place the gem on its track, giving it a mesh or sprite first, and pick the mesh group of
     * its distance.
     *
     * A gem that has its mesh or sprite is placed again only inside a range of the tunnel that
     * changed.
     *
     * @param pGems The gems.
     * @param pTunnel The tunnel.
     * @param pRange The range that changed, or null.
     * @param flTick The tick of the song.
     * @ghidraAddress NTSC-U/C: 0x001f3ce0
     * @ghidraAddress PAL: 0x001fca80
     */
    void Update(TnlGems *pGems, GfxTunnel *pTunnel, const TnlTrackRange *pRange, float flTick);

    /**
     * Return the mesh instance or the particle of the gem.
     *
     * @param pTunnel The tunnel to tell of a removed mesh gem, or null.
     * @ghidraAddress NTSC-U/C: 0x001f4058
     * @ghidraAddress PAL: 0x001fcdf8
     */
    void Release(GfxTunnel *pTunnel);

    /**
     * Start a flash of the tunnel at the gem.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f40e8
     * @ghidraAddress PAL: 0x001fce88
     */
    void Flash(GfxTunnel *pTunnel);

    /**
     * Report the position of the gem across its track.
     *
     * Every caller expands it.
     *
     * @return The position, from 0 to 1.
     */
    float Lateral() const {
        return (static_cast<float>(mSlot) * kSlotWidth) + kSlotOffset;
    }

    /**
     * The height of the glow particle above a mesh gem, `gem_particle_height`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afba0
     */
    static float sParticleHeight;

    /**
     * The height of a sprite gem as a raise of PlaceCell(), one less `sprite_gem_height`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afba4
     */
    static float sSpriteHeight;

    /**
     * The size of the flash of a sprite gem as a multiple of its particle size.
     *
     * @ghidraAddress NTSC-U/C: 0x003afba8
     */
    static float sFlareFlashSize;

    /**
     * One over `gem_flare_flash_time`, a multiple of the particle size of a sprite gem.
     *
     * @ghidraAddress NTSC-U/C: 0x003afbac
     */
    static float sFlareFlashRate;

    /**
     * The size of the flash of a mesh gem, `gem_mesh_flash_size`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afbb0
     */
    static float sMeshFlashSize;

    /**
     * The world length of a tick divided by `gem_mesh_flash_time`, the growth of a mesh flash.
     *
     * @ghidraAddress NTSC-U/C: 0x003afbb4
     */
    static float sMeshFlashRate;

    char mType;                                /*!< The gem type, with the flags above. */
    unsigned char mFlags;                      /*!< The flags of the gem. */
    char mTrack;                               /*!< The track the gem sits on. */
    float mTick;                               /*!< The tick of the gem. */
    char mSlot;                                /*!< The lateral slot of the gem on its track. */
    float mShowTick;                           /*!< The tick the gem is placed from. */
    float mRemoveTick = kNever;                /*!< The tick the gem is removed at, or kNever. */
    MeshGroup *mMeshGroup = nullptr;           /*!< The group that draws a mesh gem, or null. */
    SpriteGroup *mSpriteGroup = nullptr;       /*!< The group that draws a sprite gem, or null. */
    std::list<Transform>::iterator mTransform; /*!< The instance of the gem in mMeshGroup. */
    Rnd::Particle *mParticle = nullptr;        /*!< The particle of the gem, or null. */

private:
    /**
     * Report the colour of a player slot.
     *
     * @param nColor The slot, 0 for green, 1 for purple, 2 for red, and 3 for yellow.
     * @return The colour, white for any other slot.
     * @ghidraAddress NTSC-U/C: 0x001f35a0
     * @ghidraAddress PAL: 0x001fc340
     */
    static Color PlayerColor(char nColor);
};
