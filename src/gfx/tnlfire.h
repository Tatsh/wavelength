#pragma once

#include <list>
#include <memory>
#include <vector>

#include "gfx/gfxtunnel.h"
#include "math/color.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndmultimesh.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndtransanim.h"
#include "rnd/rndview.h"

/**
 * The fire of a captured track, flames that burn along the track and chunks of its panels that fly
 * off.
 *
 * The class is not polymorphic. RTTI identifies it and its nested Chunk and ChunkModel. The object
 * is 0xe0 bytes. A capture starts the fire at a tick, and the head of the fire then moves along
 * the track until the end of the capture or of the built bars. The view of the fire follows the
 * head, and a chunk of a randomly chosen model flies off the track at the gems the head passes.
 */
class TnlFire {
public:
    /**
     * The flames of one particle system, scaled, coloured, and turned on and off by the fire.
     *
     * The class is not polymorphic, and the name is inferred. The object is 0x20 bytes. It saves
     * the emission rates of the system to stop and restart the emission, and it saves the force to
     * turn it with the track.
     */
    class Emitter {
    public:
        /**
         * Make an emitter of no system.
         *
         * @ghidraAddress NTSC-U/C: 0x001d42d0
         * @ghidraAddress PAL: 0x001dd070
         */
        Emitter();

        /**
         * Give the system back its saved emission rates and force.
         *
         * @ghidraAddress NTSC-U/C: 0x001d42e0
         * @ghidraAddress PAL: 0x001dd080
         */
        ~Emitter();

        /**
         * Take a system, save its emission rates and its scaled force, scale its emission box,
         * speed, and size to the panels, show it, and stop its emission.
         *
         * @param pSys The system.
         * @param pTunnel The tunnel. The emitter does not read it.
         * @ghidraAddress NTSC-U/C: 0x001d4330
         * @ghidraAddress PAL: 0x001dd0d0
         */
        void Init(RndParticleSys *pSys, GfxTunnel *pTunnel);

        /**
         * Colour the flames.
         *
         * The particles start white and end in the colour, opaque at the start and clear at the
         * end.
         *
         * @param pColor The colour.
         * @ghidraAddress NTSC-U/C: 0x001d4468
         * @ghidraAddress PAL: 0x001dd208
         */
        void SetColor(const Color *pColor);

        /**
         * Turn the saved force by a basis and give it to the system.
         *
         * @param pBasis The three rows of the basis.
         * @ghidraAddress NTSC-U/C: 0x001d44d0
         * @ghidraAddress PAL: 0x001dd270
         */
        void Rotate(const Vector3 *pBasis);

        /**
         * Free the particles of the system and restart its emission at the saved rates.
         *
         * @ghidraAddress NTSC-U/C: 0x001d4518
         * @ghidraAddress PAL: 0x001dd2b8
         */
        void Start();

        /**
         * Stop the emission of the system.
         *
         * @ghidraAddress NTSC-U/C: 0x001d4560
         * @ghidraAddress PAL: 0x001dd300
         */
        void Stop();

        RndParticleSys *mSys;       /*!< The system, or null. */
        float mEmitRateLow;         /*!< The low end of the saved emission rate. */
        float mEmitRateHigh;        /*!< The high end of the saved emission rate. */
        alignas(16) Vector3 mForce; /*!< The saved force, scaled to the panels. */
    };

    /**
     * A kind of chunk, a mesh and the animation it tumbles by, drawn as many instances.
     *
     * The class is not polymorphic. RTTI identifies it. The object is 0x10 bytes. The entries of
     * `capture_chunks` give the weight of the model and the names of its mesh and animation.
     */
    class ChunkModel {
    public:
        /**
         * Make a model with no mesh. The binary expands it inline.
         */
        ChunkModel() : mWeight(0.0f), mMultiMesh(nullptr), mTransAnim(nullptr), mMesh(nullptr) {
        }

        /** Move a model. The vector of models moves its entries as the vector grows. */
        ChunkModel(ChunkModel &&) noexcept = default;

        /**
         * Delete the instanced mesh.
         *
         * @ghidraAddress NTSC-U/C: 0x001d4618
         * @ghidraAddress PAL: 0x001dd3b8
         */
        ~ChunkModel();

        /**
         * Create the instanced mesh that draws the chunks of the model, named for the address of
         * the model.
         *
         * @ghidraAddress NTSC-U/C: 0x001d4580
         * @ghidraAddress PAL: 0x001dd320
         */
        void CreateMultiMesh();

        float mWeight; /*!< The chance of the model against the other models. */
        std::unique_ptr<RndMultiMesh> mMultiMesh; /*!< The instances of the mesh. */
        RndTransAnim *mTransAnim; /*!< The tumble of a chunk over its life. Borrowed. */
        RndMesh *mMesh;           /*!< The mesh. Borrowed. */
    };

    /**
     * A chunk flying off the track, an instance of the mesh of a model.
     *
     * The class is not polymorphic. RTTI identifies it. The object is 0x70 bytes. An idle chunk
     * waits in the pool of the fire, and its instance transform waits in the pool of transforms.
     */
    class Chunk {
    public:
        /**
         * Make an idle chunk. The binary expands it inline.
         */
        Chunk();

        /**
         * Launch the chunk from a cell of a track with a random velocity and life.
         *
         * @param pModel The model.
         * @param bAuto Whether the capture is automatic. It picks the range of the velocity.
         * @param nTrack The track. It picks the scale of the chunk.
         * @param pCellXfm The transform of the cell.
         * @param pBlendXfm The transform of the middle of the tunnel. The chunk does not read it.
         * @param pFreeXfms The pool of transforms the instance of the chunk comes from.
         * @param flTick The tick of the launch.
         * @ghidraAddress NTSC-U/C: 0x001d4680
         * @ghidraAddress PAL: 0x001dd420
         */
        void Spawn(ChunkModel *pModel,
                   int bAuto,
                   int nTrack,
                   const Transform *pCellXfm,
                   const Transform *pBlendXfm,
                   std::list<Transform> *pFreeXfms,
                   float flTick);

        /**
         * Give the instance back to the pool of transforms and make the chunk idle.
         *
         * @param pFreeXfms The pool of transforms.
         * @ghidraAddress NTSC-U/C: 0x001d48e0
         * @ghidraAddress PAL: 0x001dd680
         */
        void Reset(std::list<Transform> *pFreeXfms);

        /**
         * Move the chunk under the world force and place its instance with its animation.
         *
         * @param flTicks The ticks since the last update.
         * @return Whether the life of the chunk is over, in which case nothing moves.
         * @ghidraAddress NTSC-U/C: 0x001d4948
         * @ghidraAddress PAL: 0x001dd6e8
         */
        bool Update(float flTicks);

        std::list<Transform>::iterator mInstance; /*!< The instance in the mesh of the model. */
        ChunkModel *mModel;                       /*!< The model, or null while idle. */
        alignas(16) Vector3 mVelocity;            /*!< The distance per tick. */
        alignas(16) Transform mXfm;               /*!< The scaled basis and the position. */
        float mStartTick;                         /*!< The tick of the launch. */
        float mEndTick;                           /*!< The tick the life ends at. */
    };

    /** The ends of the ranges of sCaptureVelocity and sAutoCaptureVelocity. */
    enum Range {
        kRangeMin = 0, /*!< The low end. */
        kRangeMax = 1, /*!< The high end. */
        kRangeCount,   /*!< The number of ends. */
    };

    /** The value of mPlayer of the fire that throws chunks. */
    static constexpr int kChunkPlayer = -1;

    /**
     * Find the view and particle systems of a fire by name and, when asked, load the models and
     * fill the pools of chunks.
     *
     * @param pszName The name of the fire. The objects are `<name>.view`, `<name>.ps`, and
     * `<name>a.ps`.
     * @param nPlayer The player of the fire. Only a fire of kChunkPlayer has outer flames and
     * throws chunks.
     * @param pTunnel The tunnel.
     * @param bChunks Whether to load the models in `capture_chunks` and fill the pools.
     * @ghidraAddress NTSC-U/C: 0x001d4a88
     * @ghidraAddress PAL: 0x001dd828
     */
    TnlFire(const char *pszName, int nPlayer, GfxTunnel *pTunnel, bool bChunks);

    /**
     * Take the view of an unused fire out of a view and empty its particle system.
     *
     * @param pszName The name of the fire.
     * @param pParent The view that draws the view of the fire.
     * @ghidraAddress NTSC-U/C: 0x001d5190
     * @ghidraAddress PAL: 0x001ddf30
     */
    static void Disable(const char *pszName, RndView *pParent);

    /**
     * Start the fire on a track, unless it burns or another fire's chunks still fly.
     *
     * A capture that is not automatic also colours the flames, turns their force upright, and
     * starts them.
     *
     * @param nPlayer The player, who has to be the player of the fire.
     * @param nTrack The track.
     * @param bAuto Whether the capture is automatic.
     * @param pInnerColor The colour of the inner flames.
     * @param pOuterColor The colour of the outer flames.
     * @param pTrackColor The colour of the track.
     * @param pGemTicks The ticks of the gems the fire burns, or null to collect them.
     * @param flTick The tick of the capture.
     * @param flStartTick The first tick of the fire.
     * @param flEndTick The last tick of the fire.
     * @return Whether the fire started.
     * @ghidraAddress NTSC-U/C: 0x001d52d0
     * @ghidraAddress PAL: 0x001de070
     */
    bool Start(int nPlayer,
               int nTrack,
               int bAuto,
               const Color *pInnerColor,
               const Color *pOuterColor,
               const Color *pTrackColor,
               const std::vector<float> *pGemTicks,
               float flTick,
               float flStartTick,
               float flEndTick);

    /**
     * Stop the flames and the fire.
     *
     * @param bClearChunks Whether to give every flying chunk back to the pool.
     * @ghidraAddress NTSC-U/C: 0x001d5520
     * @ghidraAddress PAL: 0x001de2c0
     */
    void Reset(bool bClearChunks);

    /**
     * Report the first tick of the fire of a player.
     *
     * @param nPlayer The player.
     * @return The tick, or 1e9 when the fire is of another player.
     * @ghidraAddress NTSC-U/C: 0x001d55f0
     * @ghidraAddress PAL: 0x001de390
     */
    float StartTick(int nPlayer) const;

    /**
     * Move the chunks, move the head of the fire, throw the chunks of the gems it passes, and
     * stop the fire at its end.
     *
     * @param flTicks The ticks since the last poll.
     * @param flTime The time since the last poll. The fire does not read it.
     * @ghidraAddress NTSC-U/C: 0x001d5610
     * @ghidraAddress PAL: 0x001de3b0
     */
    void Update(float flTicks, float flTime);

    /**
     * Draw the flying chunks in the colour of the track. A fire with no flying chunk draws
     * nothing.
     *
     * @ghidraAddress NTSC-U/C: 0x001d5a18
     * @ghidraAddress PAL: 0x001de7b8
     */
    void Draw();

    /**
     * The ticks the head moves for each tick of a capture, `track_capture_speed`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa58
     * @ghidraAddress PAL: 0x0042e8f0
     */
    static float sCaptureSpeed;

    /**
     * The ticks the head moves for each tick of an automatic capture, `track_autocapture_speed`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa5c
     * @ghidraAddress PAL: 0x0042e8f4
     */
    static float sAutoCaptureSpeed;

    /**
     * The size of a panel, the scale of the flames. The tunnel sets it when it builds its panels.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa74
     * @ghidraAddress PAL: 0x0042e90c
     */
    static float sPanelSize;

    /**
     * The chunks of a fire, `capture_max_chunks`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa7c
     * @ghidraAddress PAL: 0x0042e914
     */
    static int sMaxChunks;

    /**
     * The height above the cell a chunk starts at, `capture_chunk_y_displace`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa80
     * @ghidraAddress PAL: 0x0042e918
     */
    static float sChunkYDisplace;

    /**
     * Whether the chunks are lit, `capture_chunks_lit`. GfxTunnel::LoadConfig() writes it and the
     * flag is never read.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa84
     * @ghidraAddress PAL: 0x0042e91c
     */
    static int sChunksLit;

    /**
     * The scale of the basis of a chunk on each track. The tunnel sets it when it builds its
     * panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b3a8
     */
    static std::vector<Vector3> sChunkScales;

    /**
     * The shortest and the longest ticks between chunks of a capture, `capture_chunk_interval`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b410
     */
    static Vector2 sCaptureInterval;

    /**
     * The shortest and the longest ticks between chunks of an automatic capture,
     * `autocapture_chunk_interval`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b418
     */
    static Vector2 sAutoCaptureInterval;

    /**
     * The change of velocity of a chunk per tick, `capture_chunk_world_force`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b420
     */
    static Vector3 sChunkForce;

    /**
     * The range of the velocity of a chunk of a capture, in the basis of the cell,
     * `capture_chunk_initial_velocity_min` and `capture_chunk_initial_velocity_max`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b430
     */
    static Vector3 sCaptureVelocity[kRangeCount];

    /**
     * The range of the velocity of a chunk of an automatic capture, in the basis of the cell,
     * `autocapture_chunk_initial_velocity_min` and `autocapture_chunk_initial_velocity_max`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b450
     */
    static Vector3 sAutoCaptureVelocity[kRangeCount];

    /**
     * The shortest and the longest ticks a chunk flies, `capture_chunk_lifespan`.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b470
     */
    static Vector2 sChunkLifespan;

private:
    std::list<Chunk> mFreeChunks;    /*!< The idle chunks. */
    std::list<Chunk> mChunks;        /*!< The flying chunks. */
    std::vector<ChunkModel> mModels; /*!< The models of `capture_chunks`. */
    float mTotalWeight;              /*!< The sum of the weights of the models. */
    std::list<Transform> mFreeXfms;  /*!< The instance transforms of the idle chunks. */
    RndMat *mMat;                    /*!< The material of the chunks. */
    GfxTunnel *mTunnel;              /*!< The tunnel. */
    RndView *mView;                  /*!< The view of the fire, placed at the head. */
    alignas(16) Emitter mInner;      /*!< The inner flames, `<name>.ps`. */
    Emitter mOuter;                  /*!< The outer flames, `<name>a.ps`. */
    float mStartTick;                /*!< The tick the head starts at. */
    float mEndTick;                  /*!< The tick the head stops at. */
    float mSpeed;                    /*!< The ticks the head moves for each tick. */
    Vector2 mInterval;               /*!< The shortest and the longest ticks between chunks. */
    int mTrack;                      /*!< The track, or -1. */
    alignas(16) Color mTrackColor;   /*!< The colour of the track. */
    std::vector<float> mGemTicks;    /*!< The ticks of the gems the fire burns, in order. */
    float mTick;                     /*!< The tick of the capture. */
    float mNextChunkTick;            /*!< The tick of the head the next chunk is due at. */
    int mActive;                     /*!< Whether the fire burns. */
    int mAuto;                       /*!< Whether the capture is automatic. */
    int mPlayer;                     /*!< The player of the fire. */
};
