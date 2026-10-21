#pragma once

#include "game/avatarpart.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/rndmesh.h"

class Avatar;

/**
 * The model of an arm of an avatar.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. The
 * arm reaches for the instrument. Its first two animations place the elbow and the hand on the
 * instrument, and the upper arm and the forearm are posed between the shoulder and the hand by two
 * bone inverse kinematics. The right arm takes its choice and its material from the left arm.
 */
class ArmPart : public AvatarPart {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x003a3588
     * @ghidraAddress PAL: 0x00412268
     */
    ~ArmPart() override = default;

    /**
     * Construct the model of an arm.
     *
     * @param nPart The arm, AvatarPartSet::kPartLeftArm or AvatarPartSet::kPartRightArm.
     * @param nParent The part the model hangs from.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x00274c30
     * @ghidraAddress PAL: 0x0027e750
     */
    ArmPart(int nPart, int nParent, Avatar *pAvatar);

    /**
     * Pose the arm at one frame of its animations.
     *
     * The name follows the base.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x00274c70
     * @ghidraAddress PAL: 0x0027e790
     */
    void SetFrame(float fFrame) override;

    /**
     * Pose the arm between two frames of its animations.
     *
     * The name follows the base.
     *
     * @param fFrom The frame blended from.
     * @param fTo The frame blended to.
     * @param fBlend The weight of fTo, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x00274fa8
     * @ghidraAddress PAL: 0x0027eac8
     */
    void SetFrames(float fFrom, float fTo, float fBlend) override;

    /**
     * Hang the upper arm, the forearm, and the hand from the joint of the torso.
     *
     * The name follows the base.
     *
     * @ghidraAddress NTSC-U/C: 0x00276868
     * @ghidraAddress PAL: 0x00280388
     */
    void Update() override;

    /**
     * Start loading the files of a choice of the left arm. The right arm ignores the call.
     *
     * The name follows the base.
     *
     * @param pszType The choice, a symbol.
     * @ghidraAddress NTSC-U/C: 0x00276558
     * @ghidraAddress PAL: 0x00280078
     */
    void SetType(const char *pszType) override;

    /**
     * Build the meshes of the arm and measure its bones.
     *
     * The forearm and the hand are detached from the upper arm. The left arm then rebuilds the
     * right arm with the same choice and material. The name follows the base.
     *
     * @return False while a file is still loading, otherwise true.
     * @ghidraAddress NTSC-U/C: 0x002765a0
     * @ghidraAddress PAL: 0x002800c0
     */
    bool Rebuild() override;

private:
    float mUpperLength; /*!< The distance from the shoulder to the elbow. */
    RndMesh *mForearm;  /*!< The mesh of the first name of the `anims` array, or null. Borrowed. */
    float mForearmLength; /*!< The distance from the elbow to the wrist. */
    RndMesh *mHand; /*!< The mesh of the second name of the `anims` array, or null. Borrowed. */

    /**
     * Pose the upper arm, the forearm, and the hand to put the hand at a transform.
     *
     * The elbow bends in the plane of the shoulder, the wrist, and the pole, at the point the two
     * bone lengths allow. A wrist beyond the reach of the forearm folds the elbow onto the line
     * from the wrist to the shoulder. The name is inferred.
     *
     * @param hand The transform of the hand in the space of the joint.
     * @param pole The point the elbow bends towards, in the space of the joint.
     * @ghidraAddress NTSC-U/C: 0x00276158
     * @ghidraAddress PAL: 0x0027fc78
     */
    void SolveIK(const Transform &hand, const Vector3 &pole);
};
