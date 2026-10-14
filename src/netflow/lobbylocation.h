#pragma once

#include "os/prnstream.h"
#include "os/string.h"

/**
 * A location of the lobby server the player can log in to.
 *
 * The name is inferred. The structure has no RTTI.
 */
struct LobbyLocation {
    String mName; /*!< The name the server selection shows. */
    int mId;      /*!< The identifier NetLobby::Login() takes. */
};

/**
 * Write a location as its name and identifier.
 *
 * @param stream The stream.
 * @param location The location.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00267d80
 * @ghidraAddress PAL: 0x00271a58
 */
PrnStream &operator<<(PrnStream &stream, const LobbyLocation &location);
