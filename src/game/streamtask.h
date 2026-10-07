#pragma once

#include "os/task.h"

/**
 * Task that plays one narration stream of a StreamQueue.
 *
 * The RTTI includes the class name and records Task as the base.
 */
class StreamTask : public Task {};
