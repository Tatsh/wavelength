#pragma once

#include <algorithm>
#include <list>

/**
 * Report whether a list holds a value.
 *
 * The name is inferred. The addresses are of the instantiations for the child lists of
 * RndAnimatable, RndCollideable, RndDrawable, and RndTransformable, and for the lights of
 * RndEnviron, in that order.
 *
 * @tparam T The type of the elements.
 * @param values The list.
 * @param value The value.
 * @return Whether an element equals the value.
 * @ghidraAddress NTSC-U/C: 0x00371098
 * @ghidraAddress PAL: 0x003df7c8
 * @ghidraAddress NTSC-U/C: 0x0037f468
 * @ghidraAddress PAL: 0x003edb80
 * @ghidraAddress NTSC-U/C: 0x00380230
 * @ghidraAddress PAL: 0x003ee948
 * @ghidraAddress NTSC-U/C: 0x00391758
 * @ghidraAddress PAL: 0x003ffe60
 * @ghidraAddress NTSC-U/C: 0x003811d0
 * @ghidraAddress PAL: 0x003ef8e8
 */
template <typename T>
bool ListContains(const std::list<T> &values, const T &value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}
