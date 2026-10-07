#pragma once

#include "os/refcounted.h"

/**
 * Deferred action that a scheduler or a script runs later.
 *
 * The RTTI includes the class name. The object begins with the RefCounted words, and a Ptr is
 * the usual owner.
 */
class Command : public RefCounted {
public:
    /** Run the action. */
    virtual void Execute() = 0;
};
