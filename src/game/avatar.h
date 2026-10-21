#pragma once

#include <memory>
#include <vector>

#include "game/avatarpart.h"
#include "math/rect.h"
#include "rnd/rndcam.h"
#include "rnd/rndloader.h"
#include "rnd/rndtex.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

class AvatarPartSet;

/**
 * One of the avatars the game can draw at a time, lent to the AvatarPartSet that shows.
 *
 * The RTTI includes the name. The object is 0x84 bytes. The `num_avatars` of the `avatar` entry
 * of the "db" section sets the number of avatars, and every avatar shares one camera. An avatar
 * plays a body animation, blended from the one it replaces, and a pose that also selects the model
 * of its instrument.
 */
class Avatar {
public:
    /** The time a blend that never started or already ended records. */
    static constexpr float kNever = 1.0e7f;

    /**
     * The position in one of the `anims` of the `avatar` entry of the "db" section.
     *
     * The class has no RTTI, and the name is inferred.
     */
    class Anim {
    public:
        /**
         * Start an animation.
         *
         * The `anims` entry of the animation lists frame ranges by an integer key. The range of
         * nKey, or the first range when the entry does not list nKey, gives the type, the first
         * frame, and the length.
         *
         * @param pszAnim The animation, a symbol, or null to set only mAnim.
         * @param nKey The key of the frame range.
         * @ghidraAddress NTSC-U/C: 0x00270be8
         * @ghidraAddress PAL: 0x0027a788
         */
        void Set(const char *pszAnim, int nKey);

        /**
         * Report the frame at Avatar::sTime.
         *
         * A looping animation wraps into its range, and another one stops at either end.
         *
         * @return The frame.
         * @ghidraAddress NTSC-U/C: 0x00270b00
         * @ghidraAddress PAL: 0x0027a6a0
         */
        float Frame() const;

        /**
         * Report whether an animation that does not loop has passed its last frame.
         *
         * @return Whether the animation is done.
         * @ghidraAddress NTSC-U/C: 0x00270bb0
         * @ghidraAddress PAL: 0x0027a750
         */
        bool IsDone() const;

        /** @return The animation, a symbol, or null. */
        const char *GetAnim() const {
            return mAnim;
        }

        /**
         * Set only the animation, without a frame range.
         *
         * @param pszAnim The animation, a symbol, or null.
         */
        void SetAnim(const char *pszAnim) {
            mAnim = pszAnim;
        }

        /** @return Non-zero when the range has type 3 (the looping type). */
        int GetLoop() const {
            return mLoop;
        }

    private:
        const char *mAnim; /*!< The animation, a symbol, or null. */
        int mLoop;         /*!< Non-zero when the range has type 3, which loops. */
        float mStart;      /*!< The first frame. */
        float mEnd;        /*!< The frame after the last one. */
        float mOffset;     /*!< Added to Avatar::sTime to give the frame. */
    };

    /**
     * Build an avatar and its part models.
     *
     * @param nIndex The index of the avatar, which selects its files.
     * @ghidraAddress NTSC-U/C: 0x0026f9b8
     * @ghidraAddress PAL: 0x00279558
     */
    explicit Avatar(int nIndex);

    /**
     * Delete the part models.
     *
     * @ghidraAddress NTSC-U/C: 0x0026fbc8
     * @ghidraAddress PAL: 0x00279768
     */
    ~Avatar();

    /**
     * Read the `avatar` entry of the "db" section, load the base model, and build the avatars.
     *
     * @ghidraAddress NTSC-U/C: 0x0026f5c0
     * @ghidraAddress PAL: 0x00279160
     */
    static void Init();

    /**
     * Delete the avatars and the loader of the base model.
     *
     * sAvatars retains its size, with every avatar released.
     *
     * @ghidraAddress NTSC-U/C: 0x0026f8c8
     * @ghidraAddress PAL: 0x00279468
     */
    static void Terminate();

    /**
     * Report the number of avatars.
     *
     * @return The size of sAvatars.
     * @ghidraAddress NTSC-U/C: 0x0026f980
     * @ghidraAddress PAL: 0x00279520
     */
    static int NumAvatars();

    /**
     * Report an avatar.
     *
     * @param nIndex The index.
     * @return The avatar.
     * @ghidraAddress NTSC-U/C: 0x0026f9a0
     * @ghidraAddress PAL: 0x00279540
     */
    static Avatar *GetAvatar(int nIndex);

    /**
     * Advance every avatar to a time.
     *
     * The routine builds the instrument model of at most one avatar a call, and moves the shared
     * camera while it blends between two of the `cams`. The name is inferred.
     *
     * @param fTime The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00270fe8
     * @ghidraAddress PAL: 0x0027ab88
     */
    static void Poll(float fTime);

    /**
     * Draw the shared camera alone, and restore the current camera and environment.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002707a8
     * @ghidraAddress PAL: 0x0027a348
     */
    static void DrawCamera();

    /**
     * Report the depth range the shared camera draws into.
     *
     * The name is inferred.
     *
     * @param pfNear Receives the near end of the range.
     * @param pfFar Receives the far end of the range.
     * @ghidraAddress NTSC-U/C: 0x00270810
     * @ghidraAddress PAL: 0x0027a3b0
     */
    static void GetDepthRange(float *pfNear, float *pfFar);

    /**
     * Set the depth range the shared camera draws into.
     *
     * The name is inferred.
     *
     * @param fNear The near end of the range.
     * @param fFar The far end of the range.
     * @ghidraAddress NTSC-U/C: 0x00270830
     * @ghidraAddress PAL: 0x0027a3d0
     */
    static void SetDepthRange(float fNear, float fFar);

    /**
     * Turn the avatars in front of the shared camera.
     *
     * The name is inferred.
     *
     * @param fTilt The angle about the horizontal axis, in radians.
     * @param fTurn The angle about the vertical axis, in radians.
     * @ghidraAddress NTSC-U/C: 0x00270860
     * @ghidraAddress PAL: 0x0027a400
     */
    static void SetViewAngles(float fTilt, float fTurn);

    /**
     * Choose the camera for a size of the avatar display.
     *
     * The small display takes `game_far`, and the large and hidden ones take `game_close`. The
     * name is inferred.
     *
     * @param nFreqSize The size, one of GameOptions::FreqSize.
     * @param bSnap Whether to move the camera at once rather than blend.
     * @ghidraAddress NTSC-U/C: 0x00270a40
     * @ghidraAddress PAL: 0x0027a5e0
     */
    static void SetFreqSize(int nFreqSize, bool bSnap);

    /**
     * Report whether the avatar display is shown at all.
     *
     * The name is inferred.
     *
     * @return Whether sFreqSize is not GameOptions::kFreqSizeHidden.
     * @ghidraAddress NTSC-U/C: 0x00270ae8
     * @ghidraAddress PAL: 0x0027a688
     */
    static bool IsShown();

    /**
     * Blend the shared camera to one of the `cams`, unless it already heads there.
     *
     * The name is inferred. When the camera already heads there, the PAL build also makes it the
     * camera the blend starts from.
     *
     * @param pszCam The camera, such as `f_maker` or `bust`.
     * @ghidraAddress NTSC-U/C: 0x00271440
     * @ghidraAddress PAL: 0x0027afe0
     */
    static void SetCamera(const char *pszCam);

    /**
     * Move the shared camera to one of the `cams` at once, with no blend.
     *
     * The name is inferred.
     *
     * @param pszCam The camera, such as `bust`.
     * @ghidraAddress NTSC-U/C: 0x00271488
     * @ghidraAddress PAL: 0x0027b038
     */
    static void SnapCamera(const char *pszCam);

    /**
     * Change the pose, and with it the model of the instrument.
     *
     * While a one-shot body animation plays, the pose is only queued. The name is inferred.
     *
     * @param pszPose The pose, or `fm_none` to follow the body animation.
     * @param bImmediate Whether to change at once rather than blend.
     * @ghidraAddress NTSC-U/C: 0x0026fc70
     * @ghidraAddress PAL: 0x00279810
     */
    void SetPose(const char *pszPose, bool bImmediate);

    /**
     * Change the body animation.
     *
     * The name is inferred.
     *
     * @param pszAnim The animation.
     * @param bOneShot Whether the animation plays once and then gives way to the one it replaced.
     * @param bImmediate Whether to change at once rather than blend.
     * @ghidraAddress NTSC-U/C: 0x0026fdf8
     * @ghidraAddress PAL: 0x00279998
     */
    void SetBaseAnim(const char *pszAnim, bool bOneShot, bool bImmediate);

    /**
     * Return to the idle animation with no pose.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00270130
     * @ghidraAddress PAL: 0x00279cd0
     */
    void Reset();

    /**
     * Report the model of a part.
     *
     * @param nPart One of AvatarPartSet::Part.
     * @return The model.
     * @ghidraAddress NTSC-U/C: 0x002701d8
     * @ghidraAddress PAL: 0x00279d78
     */
    AvatarPart *GetPart(int nPart);

    /**
     * Pose the part models at sTime and draw them with the shared camera.
     *
     * The name is inferred.
     *
     * @param pScreenRect The part of the screen to draw, or null for the target texture of the
     * camera.
     * @ghidraAddress NTSC-U/C: 0x002701f0
     * @ghidraAddress PAL: 0x00279d90
     */
    void Draw(const Rect *pScreenRect);

    /**
     * React to a gem the player of the avatar caught.
     *
     * The name is inferred.
     *
     * @param nKey The key of the frame range of the pose.
     * @param bRestartPose Whether the pose restarts.
     * @ghidraAddress NTSC-U/C: 0x00271348
     * @ghidraAddress PAL: 0x0027aee8
     */
    void CatchGem(int nKey, int bRestartPose);

    /**
     * Set mUser.
     *
     * @param pUser The set that borrows the avatar, or null.
     * @ghidraAddress NTSC-U/C: 0x002715a0
     * @ghidraAddress PAL: 0x0027b150
     */
    void SetUser(AvatarPartSet *pUser);

    /** @return The index of the avatar. */
    int GetIndex() const {
        return mIndex;
    }

    /** @return The set that borrows the avatar, or null. Borrowed. */
    AvatarPartSet *GetUser() {
        return mUser;
    }

    /** @return The symbol `fm_none`. */
    static const char *GetNone() {
        return sNone;
    }

    /** @return The target texture of the shared camera as Init() found it. Borrowed. */
    static RndTex *GetTargetTex() {
        return sTargetTex;
    }

private:
    std::vector<std::unique_ptr<AvatarPart>> mParts; /*!< The models, one for each part. */
    int mIndex;                                      /*!< The index of the avatar. */
    AvatarPartSet *mUser;  /*!< The set that borrows the avatar, or null. Borrowed. */
    Anim mBody;            /*!< The body animation. */
    Anim mPrevBody;        /*!< The body animation mBody blends from. */
    int mOneShot;          /*!< Non-zero while a one-shot body animation plays. */
    float mBodyBlendStart; /*!< When the body blend started, or kNever. */
    Anim mPoseAnim;        /*!< The animation of the pose. */
    Anim mPrevPoseAnim;    /*!< The animation mPoseAnim blends from. */
    float mPoseBlendStart; /*!< When the pose blend started, or kNever. */
    const char *mPose;     /*!< The pose, a symbol. */
    const char *mPrevPose; /*!< The pose mPose replaced. */
    float mPoseChangeTime; /*!< When mPose changed, or kNever once the swap is done. */
    float mActiveTime;     /*!< The last time the avatar acted, the start of the wait for idle. */

    /**
     * The `anim_blend` of the `avatar` entry, the length of a blend in milliseconds.
     *
     * @ghidraAddress NTSC-U/C: 0x00435680
     */
    static float sAnimBlend;

    /**
     * The `time_to_idle` of the `avatar` entry, in milliseconds.
     *
     * @ghidraAddress NTSC-U/C: 0x00435684
     */
    static float sTimeToIdle;

    /**
     * The field of view of sCam as Init() found it.
     *
     * @ghidraAddress NTSC-U/C: 0x00435688
     */
    static float sFov;

    /**
     * The `cam_blend` of the `avatar` entry, the length of a camera blend in milliseconds.
     *
     * @ghidraAddress NTSC-U/C: 0x0043568c
     */
    static float sCamBlend;

    /**
     * The loader of the `base_rnd` of the `avatar` entry.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14c8
     * @ghidraAddress PAL: 0x00430398
     */
    static std::unique_ptr<RndLoader> sLoader;

    /**
     * The target texture of sCam as Init() found it.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14cc
     * @ghidraAddress PAL: 0x0043039c
     */
    static RndTex *sTargetTex;

    /**
     * The `base_freq.view` view the avatars turn with.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14d0
     * @ghidraAddress PAL: 0x004303a0
     */
    static RndView *sView;

    /**
     * The symbol `fm_none`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14d4
     * @ghidraAddress PAL: 0x004303a4
     */
    static const char *sNone;

    /**
     * The symbol `idle`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14d8
     * @ghidraAddress PAL: 0x004303a8
     */
    static const char *sIdle;

    /**
     * The `cams` of the `avatar` entry.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14dc
     * @ghidraAddress PAL: 0x004303ac
     */
    static DataArray *sCams;

    /**
     * The `base_freq.cam` camera the avatars share.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14e0
     * @ghidraAddress PAL: 0x004303b0
     */
    static RndCam *sCam;

    /**
     * The camera sCam blends to.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14e4
     * @ghidraAddress PAL: 0x004303b4
     */
    static const char *sCurrentCam;

    /**
     * The camera sCam blends from.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14e8
     * @ghidraAddress PAL: 0x004303b8
     */
    static const char *sPreviousCam;

    /**
     * When the camera blend started, or kNever.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14ec
     * @ghidraAddress PAL: 0x004303bc
     */
    static float sCamBlendStart;

    /**
     * The value of SetFreqSize().
     *
     * @ghidraAddress NTSC-U/C: 0x003b14f0
     * @ghidraAddress PAL: 0x004303c0
     */
    static int sFreqSize;

    /**
     * The time of the last Poll(), in milliseconds.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14f4
     * @ghidraAddress PAL: 0x004303c4
     */
    static float sTime;

    /**
     * The `anims` of the `avatar` entry.
     *
     * @ghidraAddress NTSC-U/C: 0x003b14f8
     * @ghidraAddress PAL: 0x004303c8
     */
    static DataArray *sAnims;

    /**
     * The avatars Init() built.
     *
     * @ghidraAddress NTSC-U/C: 0x00440d48
     */
    static std::vector<std::unique_ptr<Avatar>> sAvatars;
    /**
     * Swap the instrument model between mPrevPose and mPose as the pose blend passes its middle,
     * and fade it out and in.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00270cc8
     * @ghidraAddress PAL: 0x0027a868
     */
    void UpdateInstrument();

    /**
     * Pose a range of the part models from two animations.
     *
     * The name is inferred.
     *
     * @param pFrom The animation blended from.
     * @param pTo The animation blended to.
     * @param pfBlendStart When the blend started. Set to kNever once the blend is done.
     * @param nFirst The first part posed.
     * @param nEnd The part after the last one posed.
     * @ghidraAddress NTSC-U/C: 0x00270e58
     * @ghidraAddress PAL: 0x0027a9f8
     */
    void
    SetPartFrames(const Anim *pFrom, const Anim *pTo, float *pfBlendStart, int nFirst, int nEnd);
};
