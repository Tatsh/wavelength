#pragma once

#include <list>
#include <vector>

#include "math/color.h"
#include "math/key.h"
#include "math/vector3.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndmat.h"
#include "rnd/rndtex.h"

/**
 * Animation of a material's colours, alpha, and texture stages by keys.
 *
 * The RTTI includes the class name and records RndAnimatable as the one base. The keys may be
 * shared, in which case mKeysOwner identifies the animation that has them.
 */
class RndMatAnim : public RndAnimatable {
public:
    /**
     * The keys of one texture stage.
     *
     * The RTTI includes the nested class name.
     */
    class Stage {
    public:
        /**
         * Construct a stage without keys.
         *
         * The binary leaves mMatAnim unset. RndMatAnim sets it before use.
         *
         * @ghidraAddress NTSC-U/C: 0x00388600
         * @ghidraAddress PAL: 0x003f6d08
         */
        Stage() : mMatAnim(nullptr) {
        }

        /**
         * Write a description of the keys.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x002302b8
         * @ghidraAddress PAL: 0x00238e80
         */
        void Print(PrnStream &stream) const;

        /**
         * Write the keys.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x00230380
         * @ghidraAddress PAL: 0x00238f48
         */
        void Save(BinStream &stream) const;

        /**
         * Read what Save() wrote, or an earlier version of it.
         *
         * The version is the one RndMatAnim::Load() read last. An early version stores a texture
         * list in place of the texture keys, which becomes one key per texture at frames 0, 1,
         * 2, and so on.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x002303d0
         * @ghidraAddress PAL: 0x00238f98
         */
        void Load(BinStream &stream);

        std::vector<Key<Vector3>> mTransKeys; /*!< The texture translation keys. */
        std::vector<Key<Vector3>> mScaleKeys; /*!< The texture scale keys. */
        std::vector<Key<Vector3>> mRotKeys;   /*!< The texture rotation keys, as Euler angles. */
        std::vector<Key<RndTex *>> mTexKeys;  /*!< The texture keys. */
        RndMatAnim *mMatAnim;                 /*!< The animation that has the stage. */
    };

    /**
     * Construct an animation of nothing without keys.
     *
     * The animation is its own keys owner. The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit RndMatAnim(const char *pszName);

    /**
     * Drop the references on the material, the keys owner, and the textures.
     *
     * @ghidraAddress NTSC-U/C: 0x003886b0
     * @ghidraAddress PAL: 0x003f6d48
     */
    ~RndMatAnim() override;

    /**
     * Report the last frame of any key.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x00230a68
     * @ghidraAddress PAL: 0x00239580
     */
    float EndFrame() override;

    /**
     * Add the material to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002309b8
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Apply a frame to the material.
     *
     * Each channel with keys sets its part of the material. A stage scale multiplies the basis of
     * the stage transform, which the rotation keys alone reset.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x00230cd8
     * @ghidraAddress PAL: 0x002398a0
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Write a description of the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022f900
     * @ghidraAddress PAL: 0x002384c8
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0022fa78
     * @ghidraAddress PAL: 0x00238640
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * A keys owner replaced by null becomes the animation itself. A texture key whose texture
     * becomes null, or was null, is erased.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0022f668
     * @ghidraAddress PAL: 0x002381d8
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00388ae8
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another animation and its base.
     *
     * The keys are copied when the source has its own and the flags do not request a shallow
     * copy. Otherwise the source's keys owner becomes this one's.
     *
     * @param pSource The animation to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00230010
     * @ghidraAddress PAL: 0x00238bd8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * An early version stores ambient colour keys, which become the base colour keys when there
     * are none.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0022fd20
     * @ghidraAddress PAL: 0x002388e8
     */
    void Load(BinStream &stream) override;

    /**
     * Set the material the animation drives.
     *
     * @param pMat The material, or null.
     * @ghidraAddress NTSC-U/C: 0x0022f610
     */
    void SetMat(RndMat *pMat);

    /**
     * Resize the stages of the keys owner.
     *
     * Removed stages drop their texture references. Every remaining stage of the keys owner then
     * records this animation as its own. The name is inferred.
     *
     * @param nStages The stage count.
     * @ghidraAddress NTSC-U/C: 0x00230110
     * @ghidraAddress PAL: 0x00238cd8
     */
    void ResizeStages(int nStages);

    /**
     * Create an animation.
     *
     * @param pszName The registry key.
     * @return The animation.
     * @ghidraAddress NTSC-U/C: 0x00388b00
     */
    static RndObject *New(const char *pszName) {
        return new RndMatAnim(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `MatAnim`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09e4
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09e8
     */
    static int sRev;

    /**
     * The version Load() read last, which Stage::Load() reads by.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c68c
     */
    static int sLoadRev;

    RndMat *mMat;                       /*!< The material the animation drives. */
    std::vector<Stage> mStages;         /*!< The keys of each texture stage. */
    RndMatAnim *mKeysOwner;             /*!< The animation that has the keys. */
    std::vector<Key<Color>> mLightKeys; /*!< The light colour keys. */
    std::vector<Key<Color>> mBaseKeys;  /*!< The base colour keys. */
    std::vector<Key<Color>> mEdgeKeys;  /*!< The edge colour keys. */
    std::vector<Key<float>> mAlphaKeys; /*!< The alpha keys. */

protected:
    /**
     * Drop the references on the material, the keys owner, and the textures of the stages.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022fb78
     * @ghidraAddress PAL: 0x00238740
     */
    void ReleaseRefs();

    /**
     * Take references on the material, the keys owner, and the textures of the stages.
     *
     * Each stage records this animation as its own first. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0022fc48
     * @ghidraAddress PAL: 0x00238810
     */
    void AcquireRefs();
};

/**
 * Write a texture key as the texture's name, empty for none, followed by its frame.
 *
 * @param stream The stream to write to.
 * @param key The key.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387640
 * @ghidraAddress PAL: 0x003f5d48
 */
BinStream &operator<<(BinStream &stream, const Key<RndTex *> &key);

/**
 * Read a texture key the texture key writer wrote, resolving the name in TheManager.
 *
 * An empty name, an unknown name, or an object of another class reads as null.
 *
 * @param stream The stream to read from.
 * @param key Receives the key.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003882c8
 * @ghidraAddress PAL: 0x003f69d0
 */
BinStream &operator>>(BinStream &stream, Key<RndTex *> &key);

/**
 * Write the stages of an animation as their count followed by each stage on its own line.
 *
 * @param stream The stream to write to.
 * @param stages The stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00385048
 * @ghidraAddress PAL: 0x003f3750
 */
PrnStream &operator<<(PrnStream &stream, const std::vector<RndMatAnim::Stage> &stages);

/**
 * Write the stages of an animation as their count followed by each stage.
 *
 * @param stream The stream to write to.
 * @param stages The stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003853e0
 * @ghidraAddress PAL: 0x003f3ae8
 */
BinStream &operator<<(BinStream &stream, const std::vector<RndMatAnim::Stage> &stages);

/**
 * Read the stages of an animation the stage writer wrote.
 *
 * @param stream The stream to read from.
 * @param stages Receives the stages.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00385690
 * @ghidraAddress PAL: 0x003f3d98
 */
BinStream &operator>>(BinStream &stream, std::vector<RndMatAnim::Stage> &stages);
