#pragma once

#include "game/triggeraction.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/mat.h"
#include "rnd/particlesys.h"
#include "script/dataarray.h"

/**
 * The `set_particles` action, which moves the ranges of a particle system to new values.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0xb0 bytes. Node 1 names
 * the system and node 2 is the time the move takes. The optional values are `max_particles`,
 * `life`, `speed`, `size`, `emit_rate`, `pos` (the low and the high corner), and `mat`. The pool
 * size and the material change at once, and the ranges move in a straight line.
 */
class SetParticlesAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00201828
     * @ghidraAddress PAL: 0x0020a5c8
     */
    explicit SetParticlesAction(DataArray *pAction);

    /**
     * Record the current ranges, apply the pool size and the material, and start the move.
     *
     * @ghidraAddress NTSC-U/C: 0x00202a98
     * @ghidraAddress PAL: 0x0020b838
     */
    void Exec() override;

    /**
     * Move the ranges for the time on the clock.
     *
     * @return Whether the move is over.
     * @ghidraAddress NTSC-U/C: 0x00202ed0
     * @ghidraAddress PAL: 0x0020bc38
     */
    bool Poll() override;

private:
    // The values the node set.
    enum Flag {
        kFlagMaxParticles = 0x1,
        kFlagLife = 0x2,
        kFlagSpeed = 0x4,
        kFlagSize = 0x8,
        kFlagMat = 0x10,
        kFlagEmitRate = 0x20,
        kFlagPos = 0x40,
    };

    // The corners of the emission box.
    enum Corner {
        kCornerLow = 0,
        kCornerHigh = 1,
        kNumCorners = 2,
    };

    int mFlags;
    Rnd::ParticleSys *mParticleSys;
    int mMaxParticles;
    float mDuration;
    float mStartTime;
    Vector2 mLife;
    Vector2 mStartLife;
    Vector2 mSpeed;
    Vector2 mStartSpeed;
    Vector2 mSize;
    Vector2 mStartSize;
    Vector2 mEmitRate;
    Vector2 mStartEmitRate;
    alignas(16) Vector3 mPos[kNumCorners];
    alignas(16) Vector3 mStartPos[kNumCorners];
    Rnd::Mat *mMat;
};
