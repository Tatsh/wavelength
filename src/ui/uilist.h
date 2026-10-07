#pragma once

#include <list>
#include <vector>

#include "msg/joypadinputmsg.h"
#include "os/prnstream.h"
#include "rnd/font.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uistyle.h"

/**
 * Component that shows a scrolling window of rows over a longer list of items.
 *
 * The RTTI records the class as deriving from UIComponent. The object is 0xc0 bytes, and the
 * vtable is at `0x003d2268`. Index 2 of the script description names the style, index 3 the number
 * of rows, index 4 the number of columns, and index 5 the distance between rows. The panel's file
 * provides the template row `<panel>_<name>_row.view`, the cursor `<panel>_<name>_cursor.mesh`, and
 * the arrows `<panel>_<name>_up.mesh` and `<panel>_<name>_down.mesh`.
 *
 * The constructor clones the template once for each row, the clone of row N being named
 * `v_NN_<template>`, and spaces the clones down by the row distance. Each text or mesh the template
 * draws becomes one cell of the row. A derived list fills the cells of a row in UpdateRow().
 */
class UIList : public UIComponent {
public:
    /** The value of Refresh()'s selection that keeps the selected item. */
    static constexpr int kKeepSelection = -99999;

    /** One cell of a row. Exactly one of the two pointers is set. */
    struct Cell {
        Rnd::Text *mText; /*!< The text of the cell, or null. */
        Rnd::Mesh *mMesh; /*!< The mesh of the cell, or null. */
    };

    /**
     * Construct a list from its script description, cloning the template row once for each row.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @ghidraAddress NTSC-U/C: 0x00205830
     * @ghidraAddress PAL: 0x0020e5e8
     */
    UIList(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the row clones and every object cloned with them.
     *
     * @ghidraAddress NTSC-U/C: 0x00205d30
     * @ghidraAddress PAL: 0x0020eae8
     */
    ~UIList() override;

    /**
     * Create a list from its script description.
     *
     * UIManager::Init() registers the routine for the entry type `list_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the list belongs to.
     * @return The new list.
     * @ghidraAddress NTSC-U/C: 0x00377608
     * @ghidraAddress PAL: 0x003e5d38
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new UIList(pData, pszPanel);
    }

    /**
     * Pass a controller button to HandleJoypad().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00207258
     * @ghidraAddress PAL: 0x00210010
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Write a description of the list. The routine writes nothing.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00206840
     * @ghidraAddress PAL: 0x0020f5f8
     */
    void Print(PrnStream &stream) override;

    /**
     * Fill the cells of a row with an item.
     *
     * The base routine does nothing.
     *
     * @param nRow The row.
     * @param nItem The item.
     * @ghidraAddress NTSC-U/C: 0x00377658
     * @ghidraAddress PAL: 0x003e5d88
     */
    virtual void UpdateRow([[maybe_unused]] int nRow, [[maybe_unused]] int nItem) {
    }

    /**
     * Move the cursor to the row with the selection, or dim it when the list is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00207138
     * @ghidraAddress PAL: 0x0020fef0
     */
    virtual void UpdateCursor();

    /**
     * Give the cursor the material of the selected or of the normal state.
     *
     * @param bSelected Whether the cursor shows the selected state.
     * @ghidraAddress NTSC-U/C: 0x002071f8
     * @ghidraAddress PAL: 0x0020ffb0
     */
    virtual void SetCursorSelected(bool bSelected);

    /**
     * Scroll for up and down, and pass the cross button to the panel's `cursor` component.
     *
     * @param pMsg The message of the button.
     * @return Whether the cursor component handled the cross button.
     * @ghidraAddress NTSC-U/C: 0x002072d0
     * @ghidraAddress PAL: 0x00210088
     */
    virtual bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Select the previous item.
     *
     * @ghidraAddress NTSC-U/C: 0x00206d70
     * @ghidraAddress PAL: 0x0020fb28
     */
    virtual void ScrollUp();

    /**
     * Select the next item.
     *
     * @ghidraAddress NTSC-U/C: 0x00206df0
     * @ghidraAddress PAL: 0x0020fba8
     */
    virtual void ScrollDown();

    /**
     * Clone the template row for one row.
     *
     * @param nRow The row.
     * @param pTemplate The template row.
     * @return The clone, or null.
     * @ghidraAddress NTSC-U/C: 0x00205fd0
     * @ghidraAddress PAL: 0x0020ed88
     */
    Rnd::View *CloneRow(int nRow, Rnd::View *pTemplate);

    /**
     * Record the cells of a row, emptying each text and hiding each mesh.
     *
     * @param pRow The row.
     * @ghidraAddress NTSC-U/C: 0x002060e0
     * @ghidraAddress PAL: 0x0020ee98
     */
    void BuildRowCells(Rnd::View *pRow);

    /**
     * Show the mesh of a cell with a material.
     *
     * @param nRow The row.
     * @param nColumn The column.
     * @param pMat The material.
     * @ghidraAddress NTSC-U/C: 0x00206848
     * @ghidraAddress PAL: 0x0020f600
     */
    void SetCellMat(int nRow, int nColumn, Rnd::Mat *pMat);

    /**
     * Set the text of a cell.
     *
     * @param nRow The row.
     * @param nColumn The column.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00206a18
     * @ghidraAddress PAL: 0x0020f7d0
     */
    void SetCellText(int nRow, int nColumn, const char *pszText);

    /**
     * Set the font of a cell's text.
     *
     * @param nRow The row.
     * @param nColumn The column.
     * @param pFont The font.
     * @ghidraAddress NTSC-U/C: 0x00206be0
     * @ghidraAddress PAL: 0x0020f998
     */
    void SetCellFont(int nRow, int nColumn, Rnd::Font *pFont);

    /**
     * Set the font of every text of a row.
     *
     * @param nRow The row.
     * @param pFont The font.
     * @ghidraAddress NTSC-U/C: 0x00206c88
     * @ghidraAddress PAL: 0x0020fa40
     */
    void SetRowFont(int nRow, Rnd::Font *pFont);

    /**
     * Select an item, keeping the number of items.
     *
     * @param nSelected The item.
     * @ghidraAddress NTSC-U/C: 0x00206e80
     * @ghidraAddress PAL: 0x0020fc38
     */
    void SetSelected(int nSelected);

    /**
     * Set the number of items and the selection, and fill the rows.
     *
     * The selection stays within the items, and the cursor stays within the rows.
     *
     * @param nItemCount The number of items.
     * @param nSelected The item to select, or kKeepSelection.
     * @ghidraAddress NTSC-U/C: 0x00206ea0
     * @ghidraAddress PAL: 0x0020fc58
     */
    void Refresh(int nItemCount, int nSelected);

    UIStyle *mStyle;                         /*!< The style of the cursor. */
    int mItemCount;                          /*!< The number of items. */
    int mRowCount;                           /*!< The number of rows. */
    int mColumnCount;                        /*!< The number of cells of each row. */
    int mCursorRow;                          /*!< The row of the selected item. */
    int mSelected;                           /*!< The selected item. */
    int mRowPitch;                           /*!< The distance between rows. */
    std::list<std::vector<Cell> > mRowCells; /*!< The cells of each row. */
    std::vector<Rnd::View *> mRows;          /*!< The row clones. */
    std::vector<Rnd::Text *> mTextCells;     /*!< The text cells of every row. */
    const char *mPanelName;                  /*!< The name of the panel the list belongs to. */
    Rnd::Mesh *mCursor;                      /*!< The cursor, or null. */
    Rnd::Mesh *mUpArrow;                     /*!< The arrow above the rows, or null. */
    Rnd::Mesh *mDownArrow;                   /*!< The arrow below the rows, or null. */
    alignas(
        16) float mCursorXfm[Rnd::kXfmRowCount][Rnd::kXfmRowFloatCount]; /*!< The cursor's transform
                                                                            on the first row. */
    bool mShowUpArrow;   /*!< Whether the up arrow shows even when no item lies above. */
    bool mShowDownArrow; /*!< Whether the down arrow shows even when no item lies below. */
};
