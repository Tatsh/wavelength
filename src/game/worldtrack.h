#pragma once

#include <vector>

/**
 * The events of the track of a song named "WORLD", sorted by tick.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the track name. The
 * object is 0x10 bytes. The constructor makes seven empty event lists.
 */
class WorldTrack {
public:
    /**
     * Construct the track with seven empty event lists.
     *
     * @ghidraAddress NTSC-U/C: 0x00281030
     * @ghidraAddress PAL: 0x0028a930
     */
    WorldTrack();

    /**
     * Release the event lists.
     *
     * @ghidraAddress NTSC-U/C: 0x00281190
     * @ghidraAddress PAL: 0x0028aa90
     */
    ~WorldTrack();

    std::vector<std::vector<int>> mEvents; /*!< The event lists. */
};
