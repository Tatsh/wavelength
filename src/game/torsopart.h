#pragma once

#include "game/avatarpart.h"

/**
 * The model of the torso of an avatar, with the emblem.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. Only
 * the constructor Avatar calls is declared.
 */
class TorsoPart : public AvatarPart {
public:
    /**
     * Construct the model of the torso.
     *
     * @param nParent The part the model hangs from.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x002747e8
     * @ghidraAddress PAL: 0x0027e308
     */
    TorsoPart(int nParent, Avatar *pAvatar);
};
