#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqlist.h"
#include "met/uploadinfo.h"
#include "script/dataarray.h"

/**
 * The list of the files of the player's upload directory on the upload tool screen.
 *
 * The RTTI records the class as deriving from FreqList. Its vtable is at `0x00466250`. Each row
 * shows the date, the song, and the title of a file, and `yes` when the file is in the
 * repository.
 */
class UploadToolList : public FreqList {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x003ce0d8
     */
    ~UploadToolList() override = default;

    /**
     * Construct the list from its script description, listing no file.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress PAL: 0x001a26d8
     */
    UploadToolList(DataArray *pData, const char *pszPanel) : FreqList(pData, pszPanel) {
    }

    /**
     * Create a list from its script description.
     *
     * The metagame registers the routine for the component type `upload_tool_list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress PAL: 0x003ce1d0
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new UploadToolList(pData, pszPanel);
    }

    /**
     * Replace the listed files and show them.
     *
     * @param files The files.
     * @param nSelected The item to select, or kKeepSelection.
     * @ghidraAddress PAL: 0x001a2758
     */
    void SetFiles(const std::list<UploadInfo> &files, int nSelected);

    /**
     * Fill a row with the date, the song, and the title of a file, and with `yes` when the file is
     * in the repository.
     *
     * @param nRow The row.
     * @param nItem The file.
     * @ghidraAddress PAL: 0x001a2890
     */
    void UpdateRow(int nRow, int nItem) override;

    /**
     * Move the cursor.
     *
     * @ghidraAddress PAL: 0x001a2af0
     */
    void UpdateCursor() override;

    /**
     * Select the previous item, or from the first item move the focus to the `sort_date` button.
     *
     * @ghidraAddress PAL: 0x001a27d8
     */
    void ScrollUp() override;

    std::list<UploadInfo> mFiles; /*!< The listed files. */
};

#endif
