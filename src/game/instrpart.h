#pragma once

#include "game/avatarpart.h"

/**
 * The model of the instrument an avatar plays.
 *
 * The RTTI includes the name and records AvatarPart as the base. The object is 0x80 bytes. Only
 * the constructor Avatar calls is declared.
 */
class InstrPart : public AvatarPart {
public:
    /**
     * Construct the model of the instrument.
     *
     * @param nParent The part the model hangs from.
     * @param pAvatar The avatar the part belongs to.
     * @ghidraAddress NTSC-U/C: 0x00274498
     * @ghidraAddress PAL: 0x0027dfb8
     */
    InstrPart(int nParent, Avatar *pAvatar);
};
