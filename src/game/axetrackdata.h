#pragma once

#include <vector>

#include "gs/axecontour.h"
#include "gs/axeharmony.h"
#include "mid/tickobj.h"

/**
 * The notes and chords of a guitar track, one set of each for each span of the song.
 *
 * The class is not polymorphic. The RTTI of the nested ContourSet records the name. The object is
 * 0x28 bytes. A set starts at its tick and lasts until the tick of the next set. The data manages
 * the notes and chords it is given.
 */
class AxeTrackData {
public:
    /** The number of gem buttons, each with the notes of one AxeContour. */
    static constexpr int kSetSize = 3;

    /** The notes of the gem buttons from one tick on. */
    struct ContourSet {
        AxeContour *mContours[kSetSize]; /*!< The notes of each gem button. */
    };

    /**
     * Construct empty data.
     *
     * @param nEndTick The tick at and after which AddContours() and AddHarmony() discard what
     * they are given.
     * @param nReserved The value recorded second, which each Axer receives.
     * @ghidraAddress NTSC-U/C: 0x00149170
     * @ghidraAddress PAL: 0x0014ab30
     */
    AxeTrackData(int nEndTick, int nReserved);

    /**
     * Release every set of notes and every chord.
     *
     * @ghidraAddress NTSC-U/C: 0x001491a0
     * @ghidraAddress PAL: 0x0014ab60
     */
    ~AxeTrackData();

    /**
     * Add the chord that plays from a tick on.
     *
     * A chord at or after the end tick is deleted.
     *
     * @param nTick The tick.
     * @param pHarmony The chord.
     * @ghidraAddress NTSC-U/C: 0x00149330
     * @ghidraAddress PAL: 0x0014acf0
     */
    void AddHarmony(int nTick, const AxeHarmony *pHarmony);

    /**
     * Add the notes of the gem buttons that play from a tick on.
     *
     * Notes at or after the end tick are deleted.
     *
     * @param nTick The tick.
     * @param pFirst The notes of the first gem button.
     * @param pSecond The notes of the second gem button.
     * @param pThird The notes of the third gem button.
     * @ghidraAddress NTSC-U/C: 0x00149388
     * @ghidraAddress PAL: 0x0014ad48
     */
    void AddContours(int nTick, AxeContour *pFirst, AxeContour *pSecond, AxeContour *pThird);

    /**
     * Report the notes of the set that plays at a tick.
     *
     * The set is the last that starts at or before the tick. The data must have one.
     *
     * @param nTick The tick.
     * @param ppFirst Receives the notes of the first gem button.
     * @param ppSecond Receives the notes of the second gem button.
     * @param ppThird Receives the notes of the third gem button.
     * @return The tick of the next set, or -1 when the set is the last.
     * @ghidraAddress NTSC-U/C: 0x00149440
     * @ghidraAddress PAL: 0x0014ae00
     */
    int
    GetContours(int nTick, AxeContour **ppFirst, AxeContour **ppSecond, AxeContour **ppThird) const;

    /**
     * Report the chord that plays at a tick.
     *
     * The chord is the last that starts at or before the tick. The data must have one.
     *
     * @param nTick The tick.
     * @param ppHarmony Receives the chord.
     * @return The tick of the next chord, or -1 when the chord is the last.
     * @ghidraAddress NTSC-U/C: 0x001494e8
     * @ghidraAddress PAL: 0x0014aea8
     */
    int GetHarmony(int nTick, const AxeHarmony **ppHarmony) const;

    int mEndTick;  /*!< The end tick AddContours() and AddHarmony() test. */
    int mReserved; /*!< The value each Axer receives. Its use is not yet identified. */
    std::vector<TickObj<ContourSet> > mContours;          /*!< The sets of notes by tick. */
    std::vector<TickObj<const AxeHarmony *> > mHarmonies; /*!< The chords by tick. */
};
