#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "mid/tickobj.h"

// Searches and sorted insertion over a vector of TickObj ordered by song position.
//
// Each template below was emitted once per element type and per translation unit that used it, and
// one declaration stands for several addresses. The instantiations share their instruction shapes
// exactly and differ only in the comparator and helper addresses they call. The element type of
// each instantiation is not recorded where the call sites have not settled it.
//
// The comparators are called through a function pointer rather than inlined, as std::upper_bound
// and std::lower_bound do with a comparator passed as a plain function. Each search inlines the
// standard algorithm and calls the comparator out of line.

/**
 * Order two entries by song position.
 *
 * Instantiated for `TickObj<float>` at `0x001b7068` in the Phrase unit, and three times in the
 * TrackData unit.
 *
 * @param first The first entry.
 * @param second The second entry.
 * @return Whether first comes strictly before second.
 * @ghidraAddress NTSC-U/C: 0x001b7068
 * @ghidraAddress PAL: 0x001bce40
 * @ghidraAddress NTSC-U/C: 0x001d7f70
 * @ghidraAddress PAL: 0x001dde50
 * @ghidraAddress NTSC-U/C: 0x001d7f80
 * @ghidraAddress PAL: 0x001dde60
 * @ghidraAddress NTSC-U/C: 0x001d7f90
 * @ghidraAddress PAL: 0x001dde70
 * @ghidraAddress NTSC-U/C: 0x001a9860
 * @ghidraAddress PAL: 0x001af5c8
 */
template <typename T>
bool TickObjLess(TickObj<T> first, TickObj<T> second) {
    return first.mPosition.mTick < second.mPosition.mTick;
}

/**
 * Report whether a song position comes strictly before an entry.
 *
 * @param nTick The song position, in MIDI ticks.
 * @param entry The entry.
 * @return Whether nTick comes strictly before entry.
 * @ghidraAddress NTSC-U/C: 0x001b70e0
 * @ghidraAddress PAL: 0x001bceb8
 * @ghidraAddress NTSC-U/C: 0x001d8028
 * @ghidraAddress PAL: 0x001ddf08
 * @ghidraAddress NTSC-U/C: 0x001d8038
 * @ghidraAddress PAL: 0x001ddf18
 * @ghidraAddress NTSC-U/C: 0x001d8048
 * @ghidraAddress PAL: 0x001ddf28
 */
template <typename T>
bool TickObjBefore(int nTick, TickObj<T> entry) {
    return nTick < entry.mPosition.mTick;
}

/**
 * Report whether an entry comes strictly before a song position.
 *
 * @param entry The entry.
 * @param nTick The song position, in MIDI ticks.
 * @return Whether entry comes strictly before nTick.
 * @ghidraAddress NTSC-U/C: 0x001d80a0
 * @ghidraAddress PAL: 0x001ddf80
 * @ghidraAddress NTSC-U/C: 0x001a9a10
 * @ghidraAddress PAL: 0x001af778
 */
template <typename T>
bool TickObjAfter(TickObj<T> entry, int nTick) {
    return entry.mPosition.mTick < nTick;
}

/**
 * Find the first entry a value strictly precedes.
 *
 * @param values The entries, sorted by position.
 * @param value The value to place.
 * @return The upper bound of value.
 * @ghidraAddress NTSC-U/C: 0x001b5b40
 * @ghidraAddress PAL: 0x001bb918
 * @ghidraAddress NTSC-U/C: 0x001d6898
 * @ghidraAddress PAL: 0x001dc778
 * @ghidraAddress NTSC-U/C: 0x001d6a48
 * @ghidraAddress PAL: 0x001dc928
 */
template <typename T>
typename std::vector<TickObj<T> >::iterator UpperBoundByEntry(std::vector<TickObj<T> > &values,
                                                              const TickObj<T> &value) {
    return std::upper_bound(values.begin(), values.end(), value, TickObjLess<T>);
}

/**
 * Find the first entry a value does not follow.
 *
 * @param values The entries, sorted by position.
 * @param value The value to place.
 * @return The lower bound of value.
 * @ghidraAddress NTSC-U/C: 0x001a91a8
 * @ghidraAddress PAL: 0x001aef10
 * @ghidraAddress NTSC-U/C: 0x001d5b98
 * @ghidraAddress PAL: 0x001dba78
 * @ghidraAddress NTSC-U/C: 0x001d61d8
 * @ghidraAddress PAL: 0x001dc0b8
 */
template <typename T>
typename std::vector<TickObj<T> >::iterator LowerBoundByEntry(std::vector<TickObj<T> > &values,
                                                              const TickObj<T> &value) {
    return std::lower_bound(values.begin(), values.end(), value, TickObjLess<T>);
}

/**
 * Find the first entry a song position strictly precedes.
 *
 * @param values The entries, sorted by position.
 * @param nTick The song position, in MIDI ticks.
 * @return The upper bound of nTick.
 * @ghidraAddress NTSC-U/C: 0x001b6278
 * @ghidraAddress PAL: 0x001bc050
 * @ghidraAddress NTSC-U/C: 0x001d7050
 * @ghidraAddress PAL: 0x001dcf30
 * @ghidraAddress NTSC-U/C: 0x001d7108
 * @ghidraAddress PAL: 0x001dcfe8
 * @ghidraAddress NTSC-U/C: 0x001d71c0
 * @ghidraAddress PAL: 0x001dd0a0
 */
template <typename T>
typename std::vector<TickObj<T> >::const_iterator
UpperBoundByTick(const std::vector<TickObj<T> > &values, int nTick) {
    return std::upper_bound(values.begin(), values.end(), nTick, TickObjBefore<T>);
}

/**
 * Find the first entry at or after a song position.
 *
 * @param values The entries, sorted by position.
 * @param nTick The song position, in MIDI ticks.
 * @return The lower bound of nTick.
 * @ghidraAddress NTSC-U/C: 0x001d7278
 * @ghidraAddress PAL: 0x001dd158
 */
template <typename T>
typename std::vector<TickObj<T> >::const_iterator
LowerBoundByTick(const std::vector<TickObj<T> > &values, int nTick) {
    return std::lower_bound(values.begin(), values.end(), nTick, TickObjAfter<T>);
}

/**
 * Find the last entry at or before a song position.
 *
 * @param values The entries, sorted by position.
 * @param nTick The song position, in MIDI ticks.
 * @return The entry, or the end when every entry follows nTick.
 * @ghidraAddress NTSC-U/C: 0x001b7290
 * @ghidraAddress PAL: 0x001bd068
 * @ghidraAddress NTSC-U/C: 0x001d80e8
 * @ghidraAddress PAL: 0x001ddfc8
 * @ghidraAddress NTSC-U/C: 0x001d8130
 * @ghidraAddress PAL: 0x001de010
 * @ghidraAddress NTSC-U/C: 0x001d8058
 * @ghidraAddress PAL: 0x001ddf38
 */
template <typename T>
typename std::vector<TickObj<T> >::const_iterator
FindAtOrBefore(const std::vector<TickObj<T> > &values, int nTick) {
    if (values.begin() == values.end()) {
        return values.end();
    }
    const auto upper = UpperBoundByTick(values, nTick);
    if (upper == values.begin()) {
        return values.end();
    }
    return upper - 1;
}

/**
 * Find the first entry at or after a song position.
 *
 * @param values The entries, sorted by position.
 * @param nTick The song position, in MIDI ticks.
 * @return The entry, or the end when every entry precedes nTick.
 * @ghidraAddress NTSC-U/C: 0x001d80b0
 * @ghidraAddress PAL: 0x001ddf90
 */
template <typename T>
typename std::vector<TickObj<T> >::const_iterator
FindAtOrAfter(const std::vector<TickObj<T> > &values, int nTick) {
    if (values.begin() == values.end()) {
        return values.end();
    }
    return LowerBoundByTick(values, nTick);
}

/**
 * Insert an entry at its sorted place, after every entry at the same position.
 *
 * An entry that does not precede the last one is appended without a search.
 *
 * @param values The entries, sorted by position.
 * @param value The entry to insert.
 * @ghidraAddress NTSC-U/C: 0x001b5e80
 * @ghidraAddress PAL: 0x001bbc58
 * @ghidraAddress NTSC-U/C: 0x001a90c0
 * @ghidraAddress PAL: 0x001aee28
 * @ghidraAddress NTSC-U/C: 0x001d6960
 * @ghidraAddress PAL: 0x001dc840
 * @ghidraAddress NTSC-U/C: 0x001d6b10
 * @ghidraAddress PAL: 0x001dc9f0
 */
template <typename T>
void InsertSorted(std::vector<TickObj<T> > &values, const TickObj<T> &value) {
    if (values.size() != 0 && !(value.mPosition.mTick < values.back().mPosition.mTick)) {
        values.push_back(value);
        return;
    }
    values.insert(UpperBoundByEntry(values, value), value);
}

/**
 * Insert a value at its lower bound under a comparator, before every equal value.
 *
 * The addresses are of the instantiations for the entries of PitchTrackRiffData::RiffSet,
 * ScratchTrackData::ScratcherSet, AxeHarmony, and AxeTrackData::ContourSet, in that order.
 *
 * @param value The value to insert.
 * @param values The values, sorted under compare.
 * @param compare The ordering.
 * @ghidraAddress NTSC-U/C: 0x0033d520
 * @ghidraAddress PAL: 0x003aaa58
 * @ghidraAddress NTSC-U/C: 0x00340148
 * @ghidraAddress PAL: 0x003ad680
 * @ghidraAddress NTSC-U/C: 0x00348b80
 * @ghidraAddress PAL: 0x003b5fb0
 * @ghidraAddress NTSC-U/C: 0x003490a8
 * @ghidraAddress PAL: 0x003b64d8
 */
template <typename T, typename Compare>
void InsertOrdered(const T &value, std::vector<T> &values, Compare compare) {
    values.insert(std::lower_bound(values.begin(), values.end(), value, compare), value);
}

/**
 * Insert a value at its lower bound, before every equal value.
 *
 * The addresses are of the instantiations for the entries of PitchTrackRiffData::RiffSet,
 * ScratchTrackData::ScratcherSet, AxeHarmony, and AxeTrackData::ContourSet, in that order.
 *
 * @param value The value to insert.
 * @param values The values, sorted.
 * @ghidraAddress NTSC-U/C: 0x0033d920
 * @ghidraAddress PAL: 0x003aae58
 * @ghidraAddress NTSC-U/C: 0x00340478
 * @ghidraAddress PAL: 0x003ad9b0
 * @ghidraAddress NTSC-U/C: 0x00348e20
 * @ghidraAddress PAL: 0x003b6250
 * @ghidraAddress NTSC-U/C: 0x003493d8
 * @ghidraAddress PAL: 0x003b6808
 */
template <typename T>
void InsertOrdered(const T &value, std::vector<T> &values) {
    InsertOrdered(value, values, std::less<T>());
}

/**
 * Append a value that does not sort before the last one, and otherwise insert it at its lower
 * bound.
 *
 * The comparator decides only whether to append. The insertion orders the values by their own
 * less-than. The addresses are of the instantiations for the entries of
 * ScratchTrackData::ScratcherSet, AxeHarmony, and AxeTrackData::ContourSet, in that order.
 *
 * @param value The value to add.
 * @param values The values, sorted under compare.
 * @param compare The ordering.
 * @ghidraAddress NTSC-U/C: 0x00340498
 * @ghidraAddress PAL: 0x003ad9d0
 * @ghidraAddress NTSC-U/C: 0x00348e40
 * @ghidraAddress PAL: 0x003b6270
 * @ghidraAddress NTSC-U/C: 0x003493f8
 * @ghidraAddress PAL: 0x003b6828
 */
template <typename T, typename Compare>
void AddOrdered(const T &value, std::vector<T> &values, Compare compare) {
    if (!values.empty() && compare(value, values.back())) {
        InsertOrdered(value, values);
        return;
    }
    values.push_back(value);
}

/**
 * Append a value that does not sort before the last one, and otherwise insert it at its lower
 * bound.
 *
 * The addresses are of the instantiations for the entries of ScratchTrackData::ScratcherSet,
 * AxeHarmony, and AxeTrackData::ContourSet, in that order.
 *
 * @param value The value to add.
 * @param values The values, sorted.
 * @ghidraAddress NTSC-U/C: 0x003406d0
 * @ghidraAddress PAL: 0x003adc08
 * @ghidraAddress NTSC-U/C: 0x00349038
 * @ghidraAddress PAL: 0x003b6468
 * @ghidraAddress NTSC-U/C: 0x00349630
 * @ghidraAddress PAL: 0x003b6a60
 */
template <typename T>
void AddOrdered(const T &value, std::vector<T> &values) {
    AddOrdered(value, values, std::less<T>());
}

/**
 * Find the last value that does not sort after a key.
 *
 * The addresses are of the instantiations for the entries of ScratchTrackData::ScratcherSet,
 * AxeTrackData::ContourSet, and AxeHarmony, in that order.
 *
 * @param key The key.
 * @param values The values, sorted under compare.
 * @param compare The ordering.
 * @return The value, or the end when every value sorts after key.
 * @ghidraAddress NTSC-U/C: 0x00340088
 * @ghidraAddress PAL: 0x003ad5c0
 * @ghidraAddress NTSC-U/C: 0x003496a0
 * @ghidraAddress PAL: 0x003b6ad0
 * @ghidraAddress NTSC-U/C: 0x00349760
 * @ghidraAddress PAL: 0x003b6b90
 */
template <typename T, typename Compare>
typename std::vector<T>::const_iterator
FindOrdered(const T &key, const std::vector<T> &values, Compare compare) {
    const auto upper = std::upper_bound(values.begin(), values.end(), key, compare);
    if (upper == values.begin()) {
        return values.end();
    }
    return upper - 1;
}

/**
 * Find the last value that does not sort after a key.
 *
 * The addresses are of the instantiations for the entries of ScratchTrackData::ScratcherSet,
 * AxeTrackData::ContourSet, and AxeHarmony, in that order.
 *
 * @param key The key.
 * @param values The values, sorted.
 * @return The value, or the end when every value sorts after key.
 * @ghidraAddress NTSC-U/C: 0x003400d8
 * @ghidraAddress PAL: 0x003ad610
 * @ghidraAddress NTSC-U/C: 0x003496f0
 * @ghidraAddress PAL: 0x003b6b20
 * @ghidraAddress NTSC-U/C: 0x003497b0
 * @ghidraAddress PAL: 0x003b6be0
 */
template <typename T>
typename std::vector<T>::const_iterator FindOrdered(const T &key, const std::vector<T> &values) {
    return FindOrdered(key, values, std::less<T>());
}

/**
 * Store a value at a song position, overwriting the first entry already at that position.
 *
 * @param values The entries, sorted by position.
 * @param value The value.
 * @param nTick The song position, in MIDI ticks.
 * @ghidraAddress NTSC-U/C: 0x001d5f00
 * @ghidraAddress PAL: 0x001dbde0
 * @ghidraAddress NTSC-U/C: 0x001d6540
 * @ghidraAddress PAL: 0x001dc420
 */
template <typename T>
void SetAtTick(std::vector<TickObj<T> > &values, const T &value, int nTick) {
    TickObj<T> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue = value;
    typename std::vector<TickObj<T> >::iterator it = LowerBoundByEntry(values, entry);
    if (it != values.end() && it->mPosition.mTick == nTick) {
        *it = entry;
        return;
    }
    values.insert(it, entry);
}

/**
 * Insert an entry before every entry at the same position, always searching first.
 *
 * MultiMuse::Add() takes this path when it is told not to try appending.
 *
 * @param values The entries, sorted by position.
 * @param value The entry to insert.
 * @ghidraAddress NTSC-U/C: 0x001a9a20
 * @ghidraAddress PAL: 0x001af788
 */
template <typename T>
void InsertAtLowerBound(std::vector<TickObj<T> > &values, const TickObj<T> &value) {
    values.insert(LowerBoundByEntry(values, value), value);
}

/**
 * Insert a value at a song position, after every entry at the same position.
 *
 * @param values The entries, sorted by position.
 * @param value The value.
 * @param nTick The song position, in MIDI ticks.
 * @ghidraAddress NTSC-U/C: 0x001d7fa0
 * @ghidraAddress PAL: 0x001dde80
 * @ghidraAddress NTSC-U/C: 0x001d7fc8
 * @ghidraAddress PAL: 0x001ddea8
 * @ghidraAddress NTSC-U/C: 0x001d8000
 * @ghidraAddress PAL: 0x001ddee0
 */
template <typename T>
void InsertAtTick(std::vector<TickObj<T> > &values, const T &value, int nTick) {
    TickObj<T> entry;
    entry.mPosition.mTick = nTick;
    entry.mValue = value;
    InsertSorted(values, entry);
}
