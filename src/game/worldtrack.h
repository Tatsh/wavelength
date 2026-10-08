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

    /**
     * Add the tick of an event to the list of its letter, keeping the list sorted.
     *
     * The name is inferred.
     *
     * @param nTick The tick.
     * @param cLetter The letter, from `A` to `G`.
     * @ghidraAddress NTSC-U/C: 0x00281238
     * @ghidraAddress PAL: 0x0028ab38
     */
    void Insert(int nTick, char cLetter);

    /**
     * Report the event list of a letter.
     *
     * The name is inferred.
     *
     * @param cLetter The letter, from `A` to `G`.
     * @return The ticks of the events, sorted.
     * @ghidraAddress NTSC-U/C: 0x00281548
     * @ghidraAddress PAL: 0x0028ae48
     */
    std::vector<int> *GetEvents(char cLetter);

    /** The letter of the first event list. */
    static constexpr char kFirstLetter = 'A';

    /** The number of event lists, one for each letter from `A` to `G`. */
    static constexpr int kListCount = 7;

    std::vector<std::vector<int>> mEvents; /*!< The event lists. */
};
