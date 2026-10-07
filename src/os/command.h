#pragma once

#include "app/attachment.h"
#include "os/binstream.h"

/**
 * Unit of work a Scheduler runs at a set time.
 *
 * The RTTI includes the class name and records Attachment as the base. The class adds no member.
 * A recorded song saves the commands that report themselves serializable, and its playback
 * rebuilds them by their identifier.
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

    /**
     * Report whether a recording saves the command.
     *
     * The name is inferred.
     *
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00334730
     * @ghidraAddress PAL: 0x003a1ce0
     */
    virtual bool IsSerializable() {
        return false;
    }

    /**
     * Report the identifier a recording rebuilds the command by.
     *
     * The name is inferred.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00334738
     * @ghidraAddress PAL: 0x003a1ce8
     */
    virtual int GetSerialId() {
        return 0;
    }

    /**
     * Write the command to a recording. The default writes nothing.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00334740
     * @ghidraAddress PAL: 0x003a1cf0
     */
    virtual void Save([[maybe_unused]] BinStream &stream) const {
    }

    /**
     * Read the command Save() writes. The default reads nothing.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00334748
     * @ghidraAddress PAL: 0x003a1cf8
     */
    virtual void Load([[maybe_unused]] BinStream &stream) {
    }
};
