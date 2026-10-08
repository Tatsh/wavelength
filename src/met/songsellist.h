#pragma once

#include <vector>

#include "met/freqlist.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The list of the songs to choose from online, whose selection shows in the panel the
 * description's `pic_panel` names.
 *
 * The first items are choices other than a song (`host_random`, `host_custom`, or `search_all`),
 * and the rest are songs. The RTTI records the class as deriving from FreqList. Its vtable is at
 * `0x003cf8e0`. The destructor is compiler-generated and has no declaration here.
 */
class SongSelList : public FreqList {
public:
    /**
     * Construct the list from its script description, listing nothing.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019b178
     * @ghidraAddress PAL: 0x001a2eb0
     */
    SongSelList(DataArray *pData, const char *pszPanel);

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `song_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x003600e8
     * @ghidraAddress PAL: 0x003ce600
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new SongSelList(pData, pszPanel);
    }

    /**
     * Fill a row with a choice, or with the short artist of a song.
     *
     * @param nRow The row.
     * @param nItem The item.
     * @ghidraAddress NTSC-U/C: 0x0019b250
     * @ghidraAddress PAL: 0x001a2f88
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor, and show the genre and the band picture of the selected song, or the label
     * and the arena picture of the selected choice.
     *
     * @ghidraAddress NTSC-U/C: 0x0019b2f8
     * @ghidraAddress PAL: 0x001a3030
     */
    void UpdateCursor() override;

    /**
     * Replace the listed items and show them.
     *
     * @param songs The localised choices, then the songs.
     * @param nChoiceCount The number of choices ahead of the songs.
     * @ghidraAddress NTSC-U/C: 0x0019b1e0
     * @ghidraAddress PAL: 0x001a2f18
     */
    void SetSongs(const std::vector<String> &songs, int nChoiceCount);

    std::vector<String> mSongs; /*!< The localised choices, then the songs. +0xc0 */
    const char *mPicPanel;      /*!< `pic_panel`, the panel of the selection. +0xd0 */
    int mChoiceCount;           /*!< The number of choices ahead of the songs. */
};
