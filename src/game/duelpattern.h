#pragma once

#include "script/dataarray.h"

/** Gem lanes of a pattern. */
constexpr int kDuelPatternLanes = 3;

/** Steps of a pattern, one bar. */
constexpr int kDuelPatternSteps = 32;

/**
 * Grid of the steps of a bar at which each lane of a duel track may have a gem.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the `duel_patterns`
 * configuration the grid is read from.
 */
class DuelPattern {
public:
    /**
     * Read a grid from three symbols, one per lane, where `x` at a step allows a gem.
     *
     * @param pLanes The lane symbols, or null for the grid that allows all but the last 3 steps.
     * @ghidraAddress NTSC-U/C: 0x00128798
     * @ghidraAddress PAL: 0x00129f90
     */
    explicit DuelPattern(const DataArray *pLanes);

    int mAllowed[kDuelPatternSteps][kDuelPatternLanes]; /*!< Whether each step allows a gem. */
};
