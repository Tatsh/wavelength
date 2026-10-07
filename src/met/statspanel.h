#pragma once

/**
 * Mix-in of a panel that shows the statistics of the song just played.
 *
 * The RTTI includes the class name and records no base. The subobject is the vptr alone, and its
 * one virtual routine refreshes the statistics.
 */
class StatsPanel {
public:
    /** Show the statistics of the song just played. */
    virtual void Refresh() = 0;
};
