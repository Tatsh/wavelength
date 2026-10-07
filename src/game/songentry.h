#pragma once

#include "script/dataarray.h"

/**
 * View of the entry of one song in the "songs" section of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is the one
 * pointer to the entry. Only the members GameLogic uses are declared.
 */
class SongEntry {
public:
    /**
     * Report the `type` of the song.
     *
     * @return The type, or 0 when the entry does not specify one.
     * @ghidraAddress NTSC-U/C: 0x0027d320
     * @ghidraAddress PAL: 0x00286c38
     */
    int GetType() const;

    DataArray *mData; /*!< The entry. */
};
