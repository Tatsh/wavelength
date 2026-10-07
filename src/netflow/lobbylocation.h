#pragma once

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
