#pragma once

#include "game/avatarpartset.h"
#include "os/string.h"

/**
 * The description of one player of an online session that the lobby lists.
 *
 * The record is 0x94 bytes. The network layer copies the records member by member. The purposes of
 * most members are not yet recovered.
 */
struct NetLaunchpadPlayer {
    int mReserved00;           // +0x00, not yet recovered.
    String mName;              /*!< The name of the player. +0x04 */
    int mReserved18[7];        // +0x18, not yet recovered.
    unsigned char mBytes34[6]; // +0x34, not yet recovered.
    AvatarPartSet mAvatar;     /*!< The Freq of the player. +0x3c */
};
