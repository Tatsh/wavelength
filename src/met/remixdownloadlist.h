#pragma once

#include <list>

#include "game/songentry.h"
#include "met/freqlist.h"
#include "netflow/netreporemix.h"
#include "script/dataarray.h"

/**
 * The list of the remixes the online repository offers, whose selection shows in the panel the
 * description's `pic_panel` names, with the note of the remix or the band picture.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x003cf988`. The
 * destructor is compiler-generated and has no declaration here.
 */
class RemixDownloadList : public FreqList {
public:
    /**
     * Construct the list from its script description, listing no remix and showing the note.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019adc0
     * @ghidraAddress PAL: 0x001a2318
     */
    RemixDownloadList(DataArray *pData, const char *pszPanel);

    /**
     * Fill a row with the month and the day of a remix and its name.
     *
     * @param nRow The row.
     * @param nItem The remix.
     * @ghidraAddress NTSC-U/C: 0x0019afa8
     * @ghidraAddress PAL: 0x001a2500
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor and show the selected remix and its note in mPicPanel.
     *
     * @ghidraAddress NTSC-U/C: 0x0019b058
     * @ghidraAddress PAL: 0x001a25b0
     */
    void UpdateCursor() override;

    /**
     * Show either the note of the selection or the band picture of its song.
     *
     * @param nShowing Non-zero to show the note, zero to show the band picture.
     * @ghidraAddress NTSC-U/C: 0x0019ae70
     * @ghidraAddress PAL: 0x001a23c8
     */
    void SetNoteShowing(int nShowing);

    /**
     * Replace the listed remixes and show them.
     *
     * @param remixes The remixes.
     * @ghidraAddress NTSC-U/C: 0x0019af30
     * @ghidraAddress PAL: 0x001a2488
     */
    void SetRemixes(const std::list<NetRepoRemix> &remixes);

    std::list<NetRepoRemix> mRemixes; /*!< The listed remixes. +0xc0 */
    const char *mPicPanel;            /*!< `pic_panel`, the panel of the selection. +0xc8 */
    int mNoteShowing;                 /*!< Non-zero while the note shows. */
    SongEntry mSong;                  /*!< The song of the selection. +0xd0 */
};
