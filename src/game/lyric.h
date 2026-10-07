#pragma once

/**
 * The lyrics of a song, shown at their ticks.
 *
 * The class is not polymorphic and has no RTTI. Only the members GameLogic uses are declared.
 */
class Lyric {
public:
    /**
     * Schedule the first lyric.
     *
     * @ghidraAddress NTSC-U/C: 0x00122c68
     * @ghidraAddress PAL: 0x001243e8
     */
    void Start();

    /**
     * Withdraw the scheduled lyric.
     *
     * @ghidraAddress NTSC-U/C: 0x00122ce8
     * @ghidraAddress PAL: 0x00124468
     */
    void Stop();
};
