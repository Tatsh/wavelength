#pragma once

#include "math/color.h"
#include "math/transform.h"
#include "rnd/tunnelseekstrip.h"

namespace Rnd {

class Mat;
class Mesh;
class Object;
class Stream;
class Transformable;
class Tunnel;

/**
 * Marker that travels around the tunnel rings towards a target ring.
 *
 * The record is not polymorphic and has no RTTI. The name is inferred from the "_seek" titles of
 * the meshes its strip generates and from UpdateLane(), which moves the lane a bounded step
 * towards mTargetRing on every frame.
 *
 * The record is 0x80 bytes, the element size of the seeker vector of Rnd::Tunnel. Its destructor is
 * the implicit one at 0x00477cc0, and the copy constructor at 0x004720c0 is compiler-generated. It
 * runs the implicit copy constructor of mStrip at 0x00471e90 and then copies the remaining words.
 * The tunnel holds every reference the record takes, so each reference is released on behalf of
 * mTunnel.
 *
 * Every frame Rnd::Tunnel::SetFrameSelf() evaluates the tunnel path at three offsets from the
 * current frame. The position and look offsets orient the transform handed to SetTransXfm(), and
 * the mesh offset places the transform handed to SetMeshXfm().
 */
class TunnelSeeker {
public:
    /**
     * Construct a seeker with no tunnel.
     *
     * mTransFrameOffset starts at -500. mSavedFlag is not set.
     */
    TunnelSeeker();

    /**
     * Release the transformable and mesh references and clear the strip.
     *
     * The two pointers are not cleared.
     */
    void ReleaseRefs();

    /**
     * Attach the seeker to a tunnel.
     *
     * Takes the tunnel's filtered frame as mFrame, takes references on mTrans and mMesh on behalf
     * of the tunnel, and builds the strip.
     *
     * @param pTunnel The tunnel.
     * @param nIndex The index of the seeker in the tunnel.
     */
    void SetTunnel(Tunnel *pTunnel, int nIndex);

    /**
     * Replace the transformable the seeker drives.
     *
     * @param pTrans The transformable, or null.
     */
    void SetTrans(Transformable *pTrans);

    /**
     * Set the ring the lane moves towards.
     *
     * @param nRing The ring.
     */
    void SetTargetRing(int nRing);

    /**
     * Replace the mesh the seeker draws.
     *
     * @param pMesh The mesh, or null.
     */
    void SetMesh(Mesh *pMesh);

    /**
     * Set the path frame offset used to place the mesh.
     *
     * @param flOffset The offset from the tunnel frame.
     */
    void SetMeshFrameOffset(float flOffset);

    /**
     * Set the path frame offset used to place the transformable.
     *
     * @param flOffset The offset from the tunnel frame.
     */
    void SetTransFrameOffset(float flOffset);

    /**
     * Set the path frame offset the transformable is oriented towards.
     *
     * @param flOffset The offset from the tunnel frame.
     */
    void SetLookFrameOffset(float flOffset);

    /**
     * Place the strip on a run of slices.
     *
     * @param nFirstSlice The first slice of the run.
     * @param nSliceCount The number of slices.
     * @param nRing The ring.
     */
    void SetRange(int nFirstSlice, int nSliceCount, int nRing);

    /**
     * Set the colour of the strip.
     *
     * @param color The colour.
     */
    void SetColor(const Color &color);

    /**
     * Set the material of the strip.
     *
     * @param pMat The material, or null.
     */
    void SetMat(Mat *pMat);

    // The five accessors below are inline. Each has an out-of-line copy without callers.

    /**
     * Report the first slice of the strip.
     *
     * @return The first slice.
     */
    int GetFirstSlice() const {
        return mStrip.mFirstSlice;
    }

    /**
     * Report the slice count of the strip.
     *
     * @return The number of slices.
     */
    int GetSliceCount() const {
        return mStrip.mSliceCount;
    }

    /**
     * Report the ring of the strip.
     *
     * @return The ring.
     */
    int GetRing() const {
        return mStrip.mRing;
    }

    /**
     * Report the colour of the strip.
     *
     * @return The colour, by value.
     */
    Color GetColor() const {
        return mStrip.mColor;
    }

    /**
     * Report the material of the strip.
     *
     * @return The material, or null.
     */
    Mat *GetMat() const {
        return mStrip.mMat;
    }

    /**
     * Draw the strip section placed on one slice.
     *
     * @param nSlice The slice being drawn.
     * @param flScreenSize The screen size used to pick the level of detail.
     */
    void DrawSection(int nSlice, float flScreenSize);

    /**
     * Draw the mesh, when there is one.
     */
    void DrawMesh();

    /**
     * Move the lane towards the target ring and return it.
     *
     * The ring distance is wrapped into half the tunnel ring count either way. The step is the
     * number of tunnel frames since the last call, divided by Rnd::Tunnel::mLaneChangeFrames and
     * scaled by the ring distance when the distance exceeds one. A lane within one step of the
     * target snaps onto it.
     *
     * @return The new lane.
     */
    float UpdateLane();

    /**
     * Install a transform as the local transform of mTrans and mark it dirty.
     *
     * @param xfm The transform.
     */
    void SetTransXfm(const Transform &xfm);

    /**
     * Install a transform as the local transform of mMesh and recompose its world transform.
     *
     * @param xfm The transform.
     */
    void SetMeshXfm(const Transform &xfm);

    /**
     * Replace the transformable, mesh, and material references.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement.
     * @param pReferrer The object the references are held on behalf of.
     */
    void Replace(Object *pFrom, Object *pTo, Object *pReferrer);

    /**
     * Write the seeker.
     *
     * The order is mTrans by name, mTargetRing, mMesh by name, mSavedFlag as one byte, mLane, the
     * three frame offsets, the strip run, the strip material by name, and the strip colour.
     *
     * @param stream The stream to write to.
     */
    void Save(Stream &stream) const;

    /**
     * Read the seeker in the order Save() writes it.
     *
     * The frame offsets are read only from a Rnd::Tunnel stream revision of 34 or later. No
     * reference is taken on the objects read by name.
     *
     * @param stream The stream to read from.
     */
    void Load(Stream &stream);

    TunnelSeekStrip mStrip; /*!< The highlighted run of slices. */
    Transformable *mTrans;  /*!< The transformable the seeker drives. */
    /*!< A flag Save() and Load() move as one byte and the copy assignment copies. No other
         routine reads it. +0x54 */
    int mSavedFlag;
    int mTargetRing;         /*!< The ring the lane moves towards. */
    float mMeshFrameOffset;  /*!< The path frame offset of the mesh. */
    float mTransFrameOffset; /*!< The path frame offset of the transformable. */
    float mLookFrameOffset;  /*!< The path frame offset the transformable faces. */
    Mesh *mMesh;             /*!< The mesh the seeker draws. */
    Tunnel *mTunnel;         /*!< The tunnel, which holds the references. */
    float mLane;             /*!< The current ring position, fractional while moving. */
    float mFrame;            /*!< The tunnel frame of the last UpdateLane(). */
};

} // namespace Rnd
