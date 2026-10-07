#pragma once

#include "rnd/tex.h"

/**
 * Parts and colours of a player's avatar.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each PlayerProfile stores
 * one at `+0x74`. Only the members the front end uses are declared.
 */
class AvatarPartSet {
public:
    /**
     * Apply the parts to the avatar and render it into the avatar texture.
     *
     * @param bFlag Passed to the player's render routine. Its meaning is not yet recovered.
     * @ghidraAddress NTSC-U/C: 0x00272518
     * @ghidraAddress PAL: 0x0027c0c8
     */
    void Render(bool bFlag);
};

/**
 * The texture the avatar renders into.
 *
 * @ghidraAddress NTSC-U/C: 0x003b14cc
 */
extern Rnd::Tex *g_pAvatarTex;
