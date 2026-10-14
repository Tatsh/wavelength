#pragma once

#include <vector>

/**
 * Remove the first element of a vector equal to a value.
 *
 * The online session instantiates the template for `int` and for HostpadRT::ClientData, in that
 * order.
 *
 * @param vector The vector.
 * @param value The value.
 * @ghidraAddress NTSC-U/C: 0x0025aec0
 * @ghidraAddress PAL: 0x00263f10
 * @ghidraAddress NTSC-U/C: 0x0025af40
 * @ghidraAddress PAL: 0x00263f90
 */
template <class T>
void VectorRemove(std::vector<T> &vector, const T &value) {
    for (auto it = vector.begin(); it != vector.end(); ++it) {
        if (*it == value) {
            vector.erase(it);
            return;
        }
    }
}

/**
 * Find the first element of a vector equal to a value.
 *
 * The online session instantiates the template for HostpadRT::ClientData.
 *
 * @param vector The vector.
 * @param value The value.
 * @return The element, or null.
 * @ghidraAddress NTSC-U/C: 0x0025afe0
 * @ghidraAddress PAL: 0x00264030
 */
template <class T>
T *VectorFind(std::vector<T> &vector, const T &value) {
    for (auto &element : vector) {
        if (element == value) {
            return &element;
        }
    }
    return nullptr;
}
