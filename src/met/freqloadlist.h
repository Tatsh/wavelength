#pragma once

#include <vector>

#include "game/playerprofile.h"
#include "met/freqlist.h"
#include "script/dataarray.h"

/**
 * The list of the Freqs on a memory card, one row for each, whose selection shows in the
 * `f_load_p` panel.
 *
 * The RTTI records the class as deriving from FreqList. The object is 0xd0 bytes and its vtable is
 * at `0x003cfad8`. The destructor at `0x0035fb30` is compiler-generated and has no declaration
 * here.
 */
class FreqLoadList : public FreqList {
public:
    /**
     * Construct the list from its script description, listing no Freq.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x0019a240
     * @ghidraAddress PAL: 0x001a1780
     */
    FreqLoadList(DataArray *pData, const char *pszPanel);

    /**
     * Fill a row with the name of a Freq.
     *
     * @param nRow The row.
     * @param nItem The Freq.
     * @ghidraAddress NTSC-U/C: 0x0019a340
     * @ghidraAddress PAL: 0x001a1880
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor, and show the creation date, the account state, the Freq, and the rank of
     * the selected Freq.
     *
     * @ghidraAddress NTSC-U/C: 0x0019a370
     * @ghidraAddress PAL: 0x001a18b0
     */
    void UpdateCursor() override;

    /**
     * Replace the listed Freqs and show them.
     *
     * @param profiles The Freqs.
     * @ghidraAddress NTSC-U/C: 0x0019a280
     * @ghidraAddress PAL: 0x001a17c0
     */
    void SetProfiles(const std::vector<PlayerProfile> &profiles);

    std::vector<PlayerProfile> mProfiles; /*!< The listed Freqs. +0xc0 */
};
