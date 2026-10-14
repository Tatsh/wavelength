#pragma once

#include "game/avatarpart.h"

/**
 * The model of an arm of an avatar.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. Only
 * the constructor Avatar calls is declared.
 */
class ArmPart : public AvatarPart {
public:
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
};
