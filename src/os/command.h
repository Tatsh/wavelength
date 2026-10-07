#pragma once

#include "app/attachment.h"

/**
 * Unit of work a Scheduler runs at a set time.
 *
 * The RTTI includes the class name and records Attachment as the base. The class adds no member.
 * The vtable also lists four members with default bodies after Execute(), which this header does
 * not yet declare.
 */
class Command : public Attachment {
public:
    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x00334e70
     * @ghidraAddress PAL: 0x003a2420
     */
    ~Command() override;

    /** Do the work. */
    virtual void Execute() = 0;
};
