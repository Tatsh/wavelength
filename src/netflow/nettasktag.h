#pragma once

#include "os/string.h"

class NetTask;

/**
 * The text that identifies a NetTask to the callbacks of the online library.
 *
 * The record has no RTTI of its own. The name comes from the type information the standard library
 * list of the type records.
 */
struct NetTaskTag {
    NetTask *mTask; /*!< The task. */
    String mTag;    /*!< The text, a decimal number. */
};
