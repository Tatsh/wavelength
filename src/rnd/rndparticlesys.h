#pragma once

#include <cstddef>
#include <list>

#include "math/color.h"
#include "math/plane.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmat.h"
#include "rnd/rndtransformable.h"

/**
 * One particle of a RndParticleSys pool.
 *
 * The structure is not polymorphic and emits no RTTI descriptor. The name is inferred from the tag
 * the pool is allocated under. A live particle is linked into the live list of a system through
 * mPrev and mNext, and a free particle into the free list of the pool owner through mNext.
 */
struct Particle {
    /**
     * Allocate a pool of particles, billed to the structure name.
     *
     * @param nSize The pool size the compiler supplies.
     * @return The block.
     */
    void *operator new[](size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Particle", 0);
    }

    /**
     * Release a pool of particles.
     *
     * @param pBlock The block.
     */
    void operator delete[](void *pBlock) {
        PoolMemFree(pBlock);
    }

    Color mCol;             /*!< The colour. */
    Color mColVel;          /*!< The change of colour per frame. */
    Vector3 mPos;           /*!< The position, in world space. */
    Vector3 mVel;           /*!< The change of position per frame. */
    Vector3 mBubbleSize;    /*!< The displacement of the bubble motion at its peak. */
    float mBubbleFrequency; /*!< The angular rate of the bubble motion per frame. */
    float mBubblePhase;     /*!< The phase of the bubble motion at frame 0. */
    float mSize;            /*!< The size of the drawn primitive. */
    float mDeathFrame;      /*!< The frame the particle is freed at. */
    float mBirthFrame;      /*!< The frame the particle was allocated at. */
    Particle *mPrev; /*!< The previous live particle, the particle itself at the head, or null. */
    Particle *mNext; /*!< The next particle of the live or free list. */
};

/**
 * Emitter that spawns, moves, and fades a population of particles.
 *
 * The RTTI includes the class name and records RndAnimatable, RndTransformable, and RndDrawable
 * as bases. The pool belongs to mParticlesOwner, which may be another system. Each system has its
 * own live list, drawn from the free list of the owner. The platform subclass draws the particles.
 */
class RndParticleSys : public RndAnimatable, public RndTransformable, public RndDrawable {
public:
    /** The primitive each particle is drawn as. */
    enum Mode {
        kModePoint = 0,  /*!< A point. */
        kModeLine = 1,   /*!< A line. */
        kModeSprite = 2, /*!< A sprite. */
    };

    /** The bit of the Copy() flags that shares the source's pool rather than allocating one. */
    static constexpr int kCopyShareParticles = 0x400;

    /**
     * Construct a system that owns a pool of 10 particles.
     *
     * The particles are white, live 100 frames, start at the origin, and move at speed 1.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0023efb8
     * @ghidraAddress PAL: 0x00247ae8
     */
    explicit RndParticleSys(const char *pszName);

    /**
     * Free the particles and the pool.
     *
     * @ghidraAddress NTSC-U/C: 0x0023eb48
     * @ghidraAddress PAL: 0x00247678
     */
    ~RndParticleSys() override;

    /**
     * Allocate a system, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ParticleSys", 0);
    }

    /**
     * Release a system.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Move the particles on to a frame, then emit for the elapsed frames unless SetEmitFrame()
     * clocks the emission.
     *
     * Nothing moves on the first frame.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x0023e268
     * @ghidraAddress PAL: 0x00246d98
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Add the system, the material, and the textures of the material's stages, last first, to a
     * list.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0023f968
     * @ghidraAddress PAL: 0x00248498
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the system, when a live particle is inside the view volume of the current camera, then
     * the children, to a list.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0023fb50
     * @ghidraAddress PAL: 0x00248680
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Write a description of the system and its bases.
     *
     * The pool is listed at a dump level of 2 and above.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023cb08
     * @ghidraAddress PAL: 0x00245688
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the system and its bases.
     *
     * The collision plane is written as a point on it followed by its normal.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023cf88
     * @ghidraAddress PAL: 0x00245b08
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another in the bases, the material, and the pool owner.
     *
     * A pool owner replaced by null becomes the system itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0023dfd8
     * @ghidraAddress PAL: 0x00246b08
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00390708
     * @ghidraAddress PAL: 0x003fee10
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another system and its bases.
     *
     * The line length is not copied. The pool is shared when the flags include
     * kCopyShareParticles or the source does not own its pool.
     *
     * @param pSource The system to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0023c938
     * @ghidraAddress PAL: 0x002454b8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0023d648
     * @ghidraAddress PAL: 0x002461a8
     */
    void Load(BinStream &stream) override;

    /**
     * Set the pool size, then reallocate the pool of the owner.
     *
     * @param nCount The number of particles.
     * @ghidraAddress NTSC-U/C: 0x0023c8b8
     * @ghidraAddress PAL: 0x00245438
     */
    void SetNumParticles(int nCount);

    /**
     * Set the system whose pool this one draws from.
     *
     * @param pOwner The owner, or this system.
     * @ghidraAddress NTSC-U/C: 0x0023c8f8
     * @ghidraAddress PAL: 0x00245478
     */
    void SetParticlesOwner(RndParticleSys *pOwner);

    /**
     * Set the material the particles are drawn with.
     *
     * @param pMat The material, or null.
     * @ghidraAddress NTSC-U/C: 0x0023f798
     * @ghidraAddress PAL: 0x002482c8
     */
    void SetMat(RndMat *pMat);

    /**
     * Report the number of live particles.
     *
     * @return The length of the live list.
     * @ghidraAddress NTSC-U/C: 0x0023f930
     * @ghidraAddress PAL: 0x00248460
     */
    int NumLiveParticles() const;

    /**
     * Move a particle from the free list of the owner to the head of the live list.
     *
     * @return The particle, or null when the pool is exhausted.
     * @ghidraAddress NTSC-U/C: 0x0023e158
     * @ghidraAddress PAL: 0x00246c88
     */
    Particle *AllocParticle();

    /**
     * Move a live particle to the free list of the owner.
     *
     * A particle that is already free is reported, and nothing else is done.
     *
     * @param pParticle The particle, or null.
     * @return The particle after it in the live list, or null.
     * @ghidraAddress NTSC-U/C: 0x0023e1b0
     * @ghidraAddress PAL: 0x00246ce0
     */
    Particle *FreeParticle(Particle *pParticle);

    /**
     * Free every live particle.
     *
     * @ghidraAddress NTSC-U/C: 0x0023e7e0
     * @ghidraAddress PAL: 0x00247310
     */
    void FreeAllParticles();

    /**
     * Emit for the frames since the previous call, which then clocks the emission in place of
     * SetFrameSelf().
     *
     * The first call emits nothing. The name is inferred.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x0023e2f0
     * @ghidraAddress PAL: 0x00246e20
     */
    void SetEmitFrame(float fFrame);

    /**
     * Return the emission clock to SetFrameSelf().
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023e348
     * @ghidraAddress PAL: 0x00246e78
     */
    void ResetEmitFrame();

    /**
     * Emit the particles the emission rate owes for a number of frames.
     *
     * The fraction of a particle left over is retained for the next call, and dropped when the
     * pool is exhausted.
     *
     * @param fDelta The number of frames.
     * @ghidraAddress NTSC-U/C: 0x0023e360
     * @ghidraAddress PAL: 0x00246e90
     */
    void SpawnParticles(float fDelta);

    /**
     * Give a particle a colour and a size from the start ranges.
     *
     * @param pParticle The particle.
     * @ghidraAddress NTSC-U/C: 0x0023e760
     * @ghidraAddress PAL: 0x00247290
     */
    void RandomizeColorAndSize(Particle *pParticle);

    /**
     * Free the particles outside their lives, and move and fade the others over a number of
     * frames.
     *
     * A particle behind the collision plane that moves further behind it is reflected.
     *
     * @param fDelta The number of frames.
     * @ghidraAddress NTSC-U/C: 0x0023e830
     * @ghidraAddress PAL: 0x00247360
     */
    void UpdateParticles(float fDelta);

    /**
     * Create a system.
     *
     * @param pszName The registry key.
     * @return The system.
     * @ghidraAddress NTSC-U/C: 0x00390790
     */
    static RndObject *New(const char *pszName) {
        return new RndParticleSys(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `ParticleSys`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a38
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a3c
     */
    static int sRev;

    /**
     * The version Load() read last.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c7d4
     */
    static int sLoadRev;

    RndParticleSys *mParticlesOwner;      /*!< The system that has the pool, possibly this one. */
    Particle *mParticles;                 /*!< The pool. */
    int mNumParticles;                    /*!< The size of the pool. */
    Particle *mLiveParticles;             /*!< The head of the live list. */
    Particle *mFreeParticles;             /*!< The head of the free list, used in the owner only. */
    float mLastFrame;                     /*!< The frame SetFrameSelf() last received. */
    float mLastEmitFrame;                 /*!< The frame SetEmitFrame() last received. */
    float mEmitAccumulator;               /*!< The fraction of a particle the emission rate owes. */
    std::list<RndParticleSys *> mSharers; /*!< The systems drawing from the pool of the owner. */
    Vector2 mBubblePeriod;                /*!< The range of the period of the bubble motion. */
    Vector2 mBubbleSize;                  /*!< The range of the size of the bubble motion. */
    Vector2 mLife;                        /*!< The range of the life, in frames. */
    Vector3 mPosLow;       /*!< The low corner of the emission box, in local space. */
    Vector3 mPosHigh;      /*!< The high corner of the emission box, in local space. */
    Vector2 mSpeed;        /*!< The range of the speed. */
    Vector2 mPitch;        /*!< The range of the pitch of the direction, in radians. */
    Vector2 mYaw;          /*!< The range of the yaw of the direction, in radians. */
    Vector2 mEmitRate;     /*!< The range of the particles emitted per frame. */
    Vector2 mSize;         /*!< The range of the size. */
    Color mStartColorLow;  /*!< The low end of the colour a particle starts with. */
    Color mStartColorHigh; /*!< The high end of the colour a particle starts with. */
    Color mEndColorLow;    /*!< The low end of the colour a particle fades to. */
    Color mEndColorHigh;   /*!< The high end of the colour a particle fades to. */
    int mCollide;          /*!< Non-zero to reflect the particles off mCollidePlane. */
    Plane mCollidePlane;   /*!< The collision plane, in world space. */
    Vector3 mForce;        /*!< The change of velocity per frame. */
    RndMat *mMat;          /*!< The material the particles are drawn with. */
    Mode mMode;            /*!< The primitive each particle is drawn as. */
    int mBubble;           /*!< Non-zero to add the bubble motion. */
    int mReadZ;            /*!< Non-zero to test the particles against the depth buffer. */
    float mLineLength;     /*!< The length of a line particle. */

protected:
    /**
     * Take the references on the material and the owner, and prepare the pool.
     *
     * An owner reallocates its pool and empties the live list of every sharer. A sharer joins the
     * sharers of its owner. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023dd38
     * @ghidraAddress PAL: 0x00246868
     */
    void AddObjectRefs();

    /**
     * Drop the references on the material and the owner, free the live particles, and leave the
     * sharers of the owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023df00
     * @ghidraAddress PAL: 0x00246a30
     */
    void RemoveObjectRefs();
};

/**
 * Write a particle's position, velocity, colours, size, and life.
 *
 * @param stream The stream to write to.
 * @param particle The particle.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0023f7e8
 * @ghidraAddress PAL: 0x00248318
 */
PrnStream &operator<<(PrnStream &stream, const Particle &particle);

/**
 * Write the name of a mode.
 *
 * kModeSprite writes nothing.
 *
 * @param stream The stream to write to.
 * @param eMode The mode.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0023f8d8
 * @ghidraAddress PAL: 0x00248408
 */
PrnStream &operator<<(PrnStream &stream, RndParticleSys::Mode eMode);

/**
 * Read a list of planes, each as a point on it followed by its normal.
 *
 * @param stream The stream to read from.
 * @param planes Receives the planes.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003904d8
 * @ghidraAddress PAL: 0x003febe0
 */
BinStream &operator>>(BinStream &stream, std::list<Plane> &planes);
