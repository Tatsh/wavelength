#pragma once

/**
 * Pitch offset tables a voice's vibrato reads, one entry per tick over a 200-tick cycle.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. The fast
 * table holds five sine periods per cycle, the slow one three, and the rectified one four positive
 * humps. The rectified table is never read.
 */
class Vibrato {
public:
    static constexpr int kCycleTicks = 200; /*!< Ticks per cycle. */

    /**
     * Read the fast table.
     *
     * @param tick Current tick of the cycle.
     * @param phase Offset of the voice into the cycle.
     * @return The pitch register offset.
     * @ghidraAddress NTSC-U/C: 0x00006060
     * @ghidraAddress PAL: 0x00006060
     */
    static int GetFast(int tick, int phase);

    /**
     * Read the slow table.
     *
     * @param tick Current tick of the cycle.
     * @param phase Offset of the voice into the cycle.
     * @return The pitch register offset.
     * @ghidraAddress NTSC-U/C: 0x0000608c
     * @ghidraAddress PAL: 0x0000608c
     */
    static int GetSlow(int tick, int phase);

    /**
     * Read the rectified table.
     *
     * @param tick Current tick of the cycle.
     * @param phase Offset into the cycle.
     * @return The pitch register offset.
     * @ghidraAddress NTSC-U/C: 0x000060b8
     * @ghidraAddress PAL: 0x000060b8
     */
    static int GetRectified(int tick, int phase);
};
