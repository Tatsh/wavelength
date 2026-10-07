#pragma once

#include "os/binstream.h"
#include "os/datetime.h"

/**
 * Settings of the game that are saved apart from the players, to the `settings` file of the Freq
 * directory.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the memory card tasks
 * that save and load it. The object is 0x1c bytes. Members its callers here do not use are
 * reserved.
 */
class GlobalSettings {
public:
    /**
     * Construct the default settings.
     *
     * @ghidraAddress NTSC-U/C: 0x0027c9b8
     * @ghidraAddress PAL: 0x00286328
     */
    GlobalSettings();

    int mReserved00[4]; // +0x00, not yet recovered.
    DateTime mDate;     /*!< The clock reading the settings record. +0x10 */
    int mModified;      /*!< Non-zero when the settings changed since they were saved. +0x18 */
};

/**
 * Write settings to a stream, after a version byte.
 *
 * @param stream The stream.
 * @param settings The settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027cde0
 * @ghidraAddress PAL: 0x002866f8
 */
BinStream &operator<<(BinStream &stream, const GlobalSettings &settings);

/**
 * Read settings from a stream.
 *
 * @param stream The stream.
 * @param settings Receives the settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027ce10
 * @ghidraAddress PAL: 0x00286728
 */
BinStream &operator>>(BinStream &stream, GlobalSettings &settings);
