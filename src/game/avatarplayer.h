#pragma once

#include "game/avatarpart.h"

/**
 * One of the avatars the game can draw at a time, lent to the AvatarPartSet that shows.
 *
 * The name is inferred. The `num_avatars` of the `avatar` entry of the "db" section sets the
 * number of players. Only the members the Freq maker reaches are declared.
 */
class AvatarPlayer {
public:
    /**
     * Read the `avatar` entry of the "db" section, load the base model, and build the players.
     *
     * @ghidraAddress NTSC-U/C: 0x0026f5c0
     * @ghidraAddress PAL: 0x00279160
     */
    static void Init();

    /**
     * Report the model of a part.
     *
     * @param nPart One of AvatarPartSet::Part.
     * @return The model.
     * @ghidraAddress NTSC-U/C: 0x002701d8
     * @ghidraAddress PAL: 0x00279d78
     */
    AvatarPart *GetPart(int nPart);
};
