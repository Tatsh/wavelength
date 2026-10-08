#pragma once

#include <vector>

#include "met/freqlist.h"
#include "met/playlistsongdata.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * List of the jukebox playlist, with a light for each entry that plays and the picture of the
 * selected entry on the panel the description's `pic_panel` gives.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x003d0180`. The entries
 * before mNumGroups stand for groups of songs and show their name. The other entries show the
 * short artist of their song.
 */
class PlayList : public FreqList {
public:
    /**
     * Construct a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x001a82a0
     * @ghidraAddress PAL: 0x001aff90
     */
    PlayList(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x003621b8
     * @ghidraAddress PAL: 0x003d06d0
     */
    ~PlayList() override {
    }

    /**
     * Create a list from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x003622b8
     * @ghidraAddress PAL: 0x003d07d0
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new PlayList(pData, pszPanel);
    }

    /**
     * Show the light and the name or the artist of an entry in a row.
     *
     * @param nRow The row.
     * @param nItem The entry.
     * @ghidraAddress NTSC-U/C: 0x001a83a0
     * @ghidraAddress PAL: 0x001b0090
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the picture of the selected entry.
     *
     * @ghidraAddress NTSC-U/C: 0x001a85b8
     * @ghidraAddress PAL: 0x001b02a8
     */
    void UpdateCursor() override;

    /**
     * Replace the entries and keep the selection.
     *
     * The name is inferred.
     *
     * @param songs The entries to copy.
     * @param nNumGroups The number of entries at the start that stand for groups of songs.
     * @ghidraAddress NTSC-U/C: 0x001a8338
     * @ghidraAddress PAL: 0x001b0028
     */
    void SetSongs(const std::vector<PlaylistSongData> &songs, int nNumGroups);

    /**
     * Turn the selected entry on or off. The first entry turns every entry with it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001a86b8
     * @ghidraAddress PAL: 0x001b03a8
     */
    void ToggleSelected();

    std::vector<PlaylistSongData> mSongs; /*!< The entries of the list. */

private:
    String mPicPanel; // The description's `pic_panel`, the panel of the picture.
    int mNumGroups;   // The number of entries at the start that stand for groups of songs.
};
