#pragma once

#include "memcard/memcarduser.h"
#include "met/overwritesavescreen.h"
#include "os/string.h"

/**
 * Memory card screen that saves a remix, such as `save_remix` of the front-end description.
 *
 * The RTTI records the class as deriving from OverwriteSaveScreen and from MemcardUser at `+0xb0`.
 * Only the members the metagame uses are declared, and the routines of the class are not
 * reconstructed.
 */
class SaveRemixScreen : public OverwriteSaveScreen, public MemcardUser {
public:
    int mReservedB4[3]; // +0xb4, not yet recovered.
    String mRemixName;  /*!< The name the remix is saved under. +0xc0 */
};
