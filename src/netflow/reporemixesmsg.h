#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netreporemix.h"

/**
 * Message that reports the remixes NetLobby::RequestRemixes() found in the online repository.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6048`. Only the
 * members its callers here use are declared.
 */
class RepoRemixesMsg : public Message {
public:
    int mResult;                       /*!< The outcome, zero on success. +0x04 */
    std::list<NetRepoRemix> *mRemixes; /*!< The remixes found. +0x08 */
};
