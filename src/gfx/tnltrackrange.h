#pragma once

/**
 * A run of tracks over a span of ticks, which the tunnel reports when the region changes.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0xc bytes.
 */
class TnlTrackRange {
public:
    /**
     * Construct an empty range.
     *
     * @ghidraAddress NTSC-U/C: 0x001e4480
     * @ghidraAddress PAL: 0x001ed220
     */
    TnlTrackRange();

    /**
     * Destroy the range.
     *
     * @ghidraAddress NTSC-U/C: 0x001e44a8
     * @ghidraAddress PAL: 0x001ed248
     */
    ~TnlTrackRange();

    /**
     * Empty the range.
     *
     * @ghidraAddress NTSC-U/C: 0x001e44d0
     * @ghidraAddress PAL: 0x001ed270
     */
    void Clear();

    /**
     * Grow the range to cover a run of tracks over a span of ticks.
     *
     * An empty range becomes the run and the span.
     *
     * @param nFirstTrack The first track of the run.
     * @param nEndTrack The track after the last of the run.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001e45c0
     * @ghidraAddress PAL: 0x001ed360
     */
    void Expand(char nFirstTrack, char nEndTrack, float flStartTick, float flEndTick);

    /**
     * Report whether a span of ticks on a track overlaps the range.
     *
     * @param nTrack The track.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @return Whether the track is in the range and the spans overlap.
     * @ghidraAddress NTSC-U/C: 0x001e44f0
     * @ghidraAddress PAL: 0x001ed290
     */
    bool Overlaps(char nTrack, float flStartTick, float flEndTick) const;

    /**
     * Report whether a tick on a track lies in the range.
     *
     * @param nTrack The track.
     * @param flTick The tick.
     * @return Whether the track and the tick are in the range.
     * @ghidraAddress NTSC-U/C: 0x001e4568
     * @ghidraAddress PAL: 0x001ed308
     */
    bool Contains(char nTrack, float flTick) const;

    char mFirstTrack; /*!< The first track of the range, or -1. */
    char mEndTrack;   /*!< The track after the last of the range, or -1. */
    float mStartTick; /*!< The start of the range. */
    float mEndTick;   /*!< The end of the range. */
};
