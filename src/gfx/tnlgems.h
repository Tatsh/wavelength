#pragma once

#include <list>
#include <map>
#include <vector>

#include "gfx/gfxtunnel.h"
#include "gfx/meshgroup.h"
#include "gfx/spritegroup.h"
#include "gfx/tnlgem.h"
#include "gfx/tnltrackrange.h"
#include "math/transform.h"
#include "rnd/animatable.h"
#include "rnd/mesh.h"
#include "rnd/object.h"
#include "script/dataarray.h"

/**
 * The gems of the tunnel, with the groups that draw them.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x4c bytes. The mesh
 * groups are indexed by gem type, and the sprite groups by gem type less TnlGem::kTypeSprite.
 */
class TnlGems {
public:
    /**
     * Construct an empty set of gems with a pool of instance transforms.
     *
     * @param pTunnel The tunnel.
     * @param nMaxGems The most gems placed at once.
     * @param nNumTracks The number of tracks. The constructor does not read it.
     * @ghidraAddress NTSC-U/C: 0x001f4528
     * @ghidraAddress PAL: 0x001fd2c8
     */
    TnlGems(GfxTunnel *pTunnel, int nMaxGems, int nNumTracks);

    /**
     * Delete the groups and the animations the gems created.
     *
     * @ghidraAddress NTSC-U/C: 0x001f4748
     * @ghidraAddress PAL: 0x001fd4e8
     */
    ~TnlGems();

    /**
     * Load the shared settings of the gems.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. The routine does not
     *                read it.
     * @ghidraAddress NTSC-U/C: 0x001f4930
     * @ghidraAddress PAL: 0x001fd6d0
     */
    static void
    LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Add one mesh group for each multimesh `<name><n>.mm`, chained in that order.
     *
     * @param pszName The prefix of the multimeshes.
     * @param pszDrawParent The drawable the multimeshes draw under, or null.
     * @param flLookahead The ticks beyond the minimum screen size of a mesh the group draws to.
     * @return The gem type of the first group.
     * @ghidraAddress NTSC-U/C: 0x001f4ae8
     * @ghidraAddress PAL: 0x001fd888
     */
    char AddMeshGroups(const char *pszName, const char *pszDrawParent, float flLookahead);

    /**
     * Add one composite mesh group for each multimesh `<name>_<n>.mm`, chained in that order.
     *
     * The meshes of group n are `<name><letter>_<n>.mesh` from `a` on, and the glow particles are
     * `<name>.ps`.
     *
     * @param pszName The prefix of the multimeshes.
     * @param pszDrawParent The drawable the multimeshes draw under, or null.
     * @param flLookahead The ticks beyond the minimum screen size of a mesh the group draws to.
     * @return The gem type of the first group.
     * @ghidraAddress NTSC-U/C: 0x001f4dc8
     * @ghidraAddress PAL: 0x001fdb68
     */
    char AddCompositeMeshGroups(const char *pszName, const char *pszDrawParent, float flLookahead);

    /**
     * Add a sprite group for the particle system `<name>.ps`.
     *
     * @param pszName The name of the particle system, without its extension.
     * @return The gem type of the group.
     * @ghidraAddress NTSC-U/C: 0x001f5278
     * @ghidraAddress PAL: 0x001fe018
     */
    char AddSpriteGroup(const char *pszName);

    /**
     * Add an animation that turns each mesh of the groups of a gem type.
     *
     * A mesh that already has its animation `<mesh> rot.msnm` is passed over.
     *
     * @param nType The gem type.
     * @param nKeys The number of keys of each animation.
     * @param bLoop Whether the animations loop between their first and last frames.
     * @param pParent The animatable the animations run under.
     * @param flStartFrame The frame of the first key.
     * @param flStartAngle The angle of the first key, in radians.
     * @param flEndFrame The frame of the last key.
     * @param flEndAngle The angle of the last key, in radians.
     * @ghidraAddress NTSC-U/C: 0x001f5438
     * @ghidraAddress PAL: 0x001fe1d8
     */
    void AddRotations(char nType,
                      int nKeys,
                      int bLoop,
                      Rnd::Animatable *pParent,
                      float flStartFrame,
                      float flStartAngle,
                      float flEndFrame,
                      float flEndAngle);

    /**
     * Empty the instances of each multimesh `<name><n>.mm`.
     *
     * @param pszName The prefix of the multimeshes.
     * @ghidraAddress NTSC-U/C: 0x001f5588
     * @ghidraAddress PAL: 0x001fe328
     */
    void ClearInstances(const char *pszName);

    /**
     * Scale the meshes and particle systems of the gems by the world length of a tick.
     *
     * @ghidraAddress NTSC-U/C: 0x001f5648
     * @ghidraAddress PAL: 0x001fe3e8
     */
    static void ApplyRenderScales();

    /**
     * Report the first mesh group of a gem type.
     *
     * @param nType The gem type.
     * @return The group.
     * @ghidraAddress NTSC-U/C: 0x001f5670
     * @ghidraAddress PAL: 0x001fe410
     */
    MeshGroup *GetMeshGroup(char nType);

    /**
     * Report the sprite group of a gem type.
     *
     * @param nType The gem type, with TnlGem::kTypeSprite.
     * @return The group.
     * @ghidraAddress NTSC-U/C: 0x001f5688
     * @ghidraAddress PAL: 0x001fe428
     */
    SpriteGroup *GetSpriteGroup(char nType);

    /**
     * Add a gem, and set the removal of a gem of the same tick, slot, and track to its show tick.
     *
     * @param nTrack The track.
     * @param nType The gem type.
     * @param nSlot The lateral slot.
     * @param bFlash Whether the gem flashes as it passes.
     * @param nColor The player colour of a sprite gem, or a negative value for its own colour.
     * @param flTick The tick of the gem.
     * @param flShowTick The tick the gem is placed from.
     * @ghidraAddress NTSC-U/C: 0x001f56a8
     * @ghidraAddress PAL: 0x001fe448
     */
    void AddGem(char nTrack,
                char nType,
                char nSlot,
                int bFlash,
                char nColor,
                float flTick,
                float flShowTick);

    /**
     * Remove the gems of a track over a span of ticks.
     *
     * @param nTrack The track.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001f58b8
     * @ghidraAddress PAL: 0x001fe658
     */
    void RemoveGems(char nTrack, float flStartTick, float flEndTick);

    /**
     * Remove the gems of a tick, slot, and track.
     *
     * @param nTrack The track.
     * @param nSlot The lateral slot.
     * @param flTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001f59a0
     * @ghidraAddress PAL: 0x001fe740
     */
    void RemoveGem(char nTrack, char nSlot, float flTick);

    /**
     * Remove every gem.
     *
     * @ghidraAddress NTSC-U/C: 0x001f5aa0
     * @ghidraAddress PAL: 0x001fe840
     */
    void Clear();

    /**
     * Drop the gems behind the song and far ahead of it, fire the events of the gems that pass,
     * and place the gems that show.
     *
     * @param pRange The range of the tunnel that changed, or null.
     * @ghidraAddress NTSC-U/C: 0x001f5b40
     * @ghidraAddress PAL: 0x001fe8e0
     */
    void Poll(const TnlTrackRange *pRange);

    /**
     * Draw the composite mesh groups, each mesh through the multimesh in turn.
     *
     * The glow particles of a group draw after its first mesh.
     *
     * @ghidraAddress NTSC-U/C: 0x001f5dd0
     * @ghidraAddress PAL: 0x001feb70
     */
    void DrawComposites();

    /**
     * Find the first gem at or after a tick.
     *
     * @param flTick The tick.
     * @return The gem, or the end of mGems when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x001f5ed8
     * @ghidraAddress PAL: 0x001fec78
     */
    std::list<TnlGem>::iterator LowerBound(float flTick);

    /**
     * Find the first gem at or after a tick.
     *
     * @param flTick The tick.
     * @return The gem, or the end of mGems when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x001f5ef8
     * @ghidraAddress PAL: 0x001fec98
     */
    std::list<TnlGem>::iterator Find(float flTick);

    GfxTunnel *mTunnel;                       /*!< The tunnel. */
    std::list<Transform> mTransforms;         /*!< The pool of free instance transforms. */
    std::vector<MeshGroup *> mMeshGroups;     /*!< The first mesh group of each type. */
    std::vector<SpriteGroup *> mSpriteGroups; /*!< The sprite group of each sprite type. */
    std::list<Rnd::Object *> mObjects;        /*!< The animations the gems created. */
    std::list<TnlGem> mGems;                  /*!< The gems, sorted by tick. */
    float mTrailTicks;                        /*!< The ticks a gem stays behind the song. */
    int mMaxGems;                             /*!< The most gems placed at once. */
    char mFocusTrack;                         /*!< The track whose passing gems fire hit events. */
    int mHasFocusTrack;                       /*!< Whether mFocusTrack applies. */

private:
    /**
     * Set the render scale of an object.
     *
     * @param pObject The object, a mesh or a particle system.
     * @param flScale The scale.
     * @ghidraAddress NTSC-U/C: 0x001f3688
     * @ghidraAddress PAL: 0x001fc428
     */
    static void SetRenderScale(Rnd::Object *pObject, float flScale);

    /**
     * Set the render scale of the geometry a mesh draws.
     *
     * @param pMesh The mesh.
     * @param flScale The scale.
     * @ghidraAddress NTSC-U/C: 0x001f3750
     * @ghidraAddress PAL: 0x001fc4f0
     */
    static void SetRenderScale(Rnd::Mesh *pMesh, float flScale);

    /**
     * Scale each object with a render scale, and forget the scales.
     *
     * A mesh scales its vertices across and along by the scale times flWorldScale, and up by
     * flWorldScale. A particle system scales its size range by the same product.
     *
     * @param flWorldScale The world length of a tick.
     * @ghidraAddress NTSC-U/C: 0x001f3828
     * @ghidraAddress PAL: 0x001fc5c8
     */
    static void ApplyRenderScales(float flWorldScale);

    /**
     * Add an animation that turns one mesh, unless it already has one.
     *
     * @param pMesh The mesh.
     * @param pObjects Receives the new animation.
     * @param nKeys The number of keys.
     * @param bLoop Whether the animation loops between its first and last frames.
     * @param pParent The animatable the animation runs under.
     * @param flStartFrame The frame of the first key.
     * @param flStartAngle The angle of the first key, in radians.
     * @param flEndFrame The frame of the last key.
     * @param flEndAngle The angle of the last key, in radians.
     * @ghidraAddress NTSC-U/C: 0x001f3a28
     * @ghidraAddress PAL: 0x001fc7c8
     */
    static void AddRotation(Rnd::Mesh *pMesh,
                            std::list<Rnd::Object *> *pObjects,
                            int nKeys,
                            int bLoop,
                            Rnd::Animatable *pParent,
                            float flStartFrame,
                            float flStartAngle,
                            float flEndFrame,
                            float flEndAngle);

    /**
     * Find the first gem at or after a tick, walking from whichever end of mGems is nearer.
     *
     * @param flTick The tick.
     * @return The gem, or the end of mGems when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x001f5f30
     * @ghidraAddress PAL: 0x001fecd0
     */
    std::list<TnlGem>::iterator FindNearest(float flTick);

    /**
     * Find the first gem at or after a tick, walking from whichever end of mGems is nearer.
     *
     * @param flTick The tick.
     * @return The gem, or the end of mGems when every gem is earlier.
     * @ghidraAddress NTSC-U/C: 0x001f6038
     * @ghidraAddress PAL: 0x001fedd8
     */
    std::list<TnlGem>::iterator FindLowerBound(float flTick);
};
