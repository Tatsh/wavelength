#pragma once

#include <list>

/**
 * Report the element of a list at an index, counting from the first.
 *
 * The walk starts at the end of the list and always takes one step more than the index. The name
 * is inferred. The addresses are of the instantiations for the lists of NetChooseIConfigScreen,
 * RemixDownloadList, the player lists, NetMainLobbiesList, and the launchpad lists, in that order.
 *
 * @tparam T The type of the elements.
 * @param values The list.
 * @param nIndex The index.
 * @return The element.
 * @ghidraAddress NTSC-U/C: 0x00358a20
 * @ghidraAddress PAL: 0x003c5dd0
 * @ghidraAddress NTSC-U/C: 0x0035f3a0
 * @ghidraAddress PAL: 0x003c7a08
 * @ghidraAddress NTSC-U/C: 0x0035f3e0
 * @ghidraAddress PAL: 0x003cd668
 * @ghidraAddress NTSC-U/C: 0x0035f810
 * @ghidraAddress PAL: 0x003cda98
 * @ghidraAddress NTSC-U/C: 0x0035fa58
 * @ghidraAddress PAL: 0x003cdce0
 */
template <typename T>
typename std::list<T>::iterator ListAt(std::list<T> &values, int nIndex) {
    typename std::list<T>::iterator it = values.end();
    do {
        ++it;
    } while (--nIndex >= 0);
    return it;
}
