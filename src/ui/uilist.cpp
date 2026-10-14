#include "ui/uilist.h"

#include "os/joypad.h"
#include "os/string.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmanager.h"
#include "rnd/rndobject.h"
#include "rnd/rndtransformable.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

// Indices of the script description.
constexpr int kStyleIndex = 2;
constexpr int kRowCountIndex = 3;
constexpr int kColumnCountIndex = 4;
constexpr int kRowPitchIndex = 5;

// The copy flags the rows are cloned with.
constexpr unsigned kRowCloneFlags = 0x200;

// The transform row that holds the translation, and the axis the rows are spaced along.

// Find the cells of a row.
std::list<std::vector<UIList::Cell> >::iterator
FindRowCells(std::list<std::vector<UIList::Cell> > &rowCells, int nRow) {
    std::list<std::vector<UIList::Cell> >::iterator it = rowCells.begin();
    for (int i = 0; i != nRow && it != rowCells.end(); ++i) {
        ++it;
    }
    return it;
}

} // namespace

UIList::UIList(DataArray *pData, const char *pszPanel)
    : UIComponent(pData), mItemCount(0), mRowCount(0), mColumnCount(0), mCursorRow(0), mSelected(0),
      mRowPitch(0), mPanelName(pszPanel), mShowUpArrow(0), mShowDownArrow(0) {
    mStyle = TheUI.FindStyle(pData->Sym(kStyleIndex), false);

    RndObject *pObject = TheManager.Find(FormatString("%s_%s_row.view", pszPanel, pData->Sym(1)));
    RndView *pTemplate = pObject != nullptr ? dynamic_cast<RndView *>(pObject) : nullptr;
    mRowCount = pData->Int(kRowCountIndex);
    mColumnCount = pData->Int(kColumnCountIndex);
    (void)pTemplate->mDraws.size(); // Yes, the binary counts the template's draws and discards
                                    // the count.
    mRowPitch = pData->Int(kRowPitchIndex);

    pObject = TheManager.Find(FormatString("%s_%s_cursor.mesh", mPanelName, pData->Sym(1)));
    mCursor = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    // Yes, the binary copies the cursor's transform without testing the cursor.
    mCursorXfm = mCursor->mLocalXfm;

    std::list<RndDrawable *> &templateDraws = pTemplate->mDraws;
    for (std::list<RndDrawable *>::iterator it = templateDraws.begin(); it != templateDraws.end();
         ++it) {
        (*it)->SetShowing(1);
    }

    mRows.resize(mRowCount, nullptr);
    for (int i = 0; i < mRowCount; ++i) {
        RndView *pRow = CloneRow(i, pTemplate);
        Vector3 translation = pTemplate->mLocalXfm.mTranslation;
        translation.y -= mRowPitch * i;
        pRow->mDirty = 1;
        pRow->mLocalXfm.mTranslation = translation;
        BuildRowCells(pRow);
        mRows[i] = pRow;
    }

    pTemplate->SetShowing(0);

    pObject = TheManager.Find(FormatString("%s_%s_up.mesh", mPanelName, pData->Sym(1)));
    mUpArrow = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    if (mUpArrow != nullptr) {
        mUpArrow->SetShowing(0);
    }
    pObject = TheManager.Find(FormatString("%s_%s_down.mesh", mPanelName, pData->Sym(1)));
    mDownArrow = pObject != nullptr ? dynamic_cast<RndMesh *>(pObject) : nullptr;
    if (mDownArrow != nullptr) {
        mDownArrow->SetShowing(0);
    }
}

UIList::~UIList() {
    for (int i = 0; i < mRowCount; ++i) {
        RndObject *pObject =
            TheManager.Find(FormatString("v_%02d_%s_%s_row.view", i, mPanelName, mName));
        RndView *pRow = pObject != nullptr ? dynamic_cast<RndView *>(pObject) : nullptr;

        std::list<RndObject *> objects;
        RndAnimatable::CollectChildren(objects, pRow);
        RndCollideable::CollectChildren(objects, pRow);
        RndDrawable::CollectChildren(objects, pRow);
        RndTransformable::CollectChildren(objects, pRow);
        objects.sort();
        objects.unique();
        for (std::list<RndObject *>::iterator it = objects.begin(); it != objects.end(); ++it) {
            delete *it;
        }
        delete static_cast<RndObject *>(pRow);
    }
}

RndView *UIList::CloneRow(int nRow, RndView *pTemplate) {
    const char *pszPrefix = FormatString("v_%02d_", nRow);
    std::list<RndObject *> clones;
    TheManager.Clone(pTemplate, pszPrefix, &clones, kRowCloneFlags, 1, 1);
    RndObject *pClone = clones.front();
    return pClone != nullptr ? dynamic_cast<RndView *>(pClone) : nullptr;
}

void UIList::BuildRowCells(RndView *pRow) {
    std::vector<Cell> cells;
    std::list<RndDrawable *> &draws = pRow->mDraws;
    for (std::list<RndDrawable *>::iterator it = draws.begin(); it != draws.end(); ++it) {
        RndText *pText = *it != nullptr ? dynamic_cast<RndText *>(*it) : nullptr;
        RndMesh *pMesh = *it != nullptr ? dynamic_cast<RndMesh *>(*it) : nullptr;
        if (pText != nullptr) {
            cells.push_back(Cell{pText, nullptr});
            pText->SetText("");
            mTextCells.push_back(pText);
        } else if (pMesh != nullptr) {
            cells.push_back(Cell{nullptr, pMesh});
            pMesh->SetShowing(0);
        }
    }
    mRowCells.push_back(cells);
}

void UIList::Print([[maybe_unused]] PrnStream &stream) {
}

void UIList::SetCellMat(int nRow, int nColumn, RndMat *pMat) {
    std::vector<Cell> cells = *FindRowCells(mRowCells, nRow); // Yes, the binary copies the row.
    Cell cell = cells[nColumn];
    if (cell.mMesh != nullptr) {
        cell.mMesh->SetShowing(1);
        cell.mMesh->SetMat(pMat);
    }
}

void UIList::SetCellText(int nRow, int nColumn, const char *pszText) {
    std::vector<Cell> cells = *FindRowCells(mRowCells, nRow); // Yes, the binary copies the row.
    Cell cell = cells[nColumn];
    if (cell.mText != nullptr) {
        cell.mText->SetText(pszText);
    }
}

void UIList::SetCellFont(int nRow, int nColumn, RndFont *pFont) {
    Cell cell = (*FindRowCells(mRowCells, nRow))[nColumn];
    if (cell.mText != nullptr) {
        cell.mText->SetFont(pFont);
    }
}

void UIList::SetRowFont(int nRow, RndFont *pFont) {
    std::list<std::vector<Cell> >::iterator it = FindRowCells(mRowCells, nRow);
    for (int i = 0; i < mColumnCount; ++i) {
        Cell cell = (*it)[i];
        if (cell.mText != nullptr) {
            cell.mText->SetFont(pFont);
        }
    }
}

void UIList::ScrollUp() {
    SetCursorSelected(false);
    if (mItemCount == 0) {
        return;
    }
    if (mSelected > 0) {
        if (mCursorRow > 0) {
            --mCursorRow;
        }
        --mSelected;
    }
    Refresh(mItemCount, kKeepSelection);
}

void UIList::ScrollDown() {
    SetCursorSelected(false);
    if (mItemCount == 0) {
        return;
    }
    if (mSelected < mItemCount - 1) {
        if (mCursorRow < mRowCount - 1) {
            ++mCursorRow;
        }
        ++mSelected;
    }
    Refresh(mItemCount, kKeepSelection);
}

void UIList::SetSelected(int nSelected) {
    Refresh(mItemCount, nSelected);
}

void UIList::Refresh(int nItemCount, int nSelected) {
    mItemCount = nItemCount;
    if (nItemCount != 0 && mSelected >= nItemCount) {
        mSelected = nItemCount - 1;
        mCursorRow = mRowCount < nItemCount - 1 ? mRowCount - 1 : nItemCount - 1;
    }

    if (nSelected != kKeepSelection) {
        mSelected = nSelected;
        if (nSelected < 0) {
            mSelected = 0;
        } else if (mItemCount != 0 && nSelected >= mItemCount) {
            mSelected = mItemCount - 1;
        }
        mCursorRow = mSelected >= mRowCount ? mRowCount - 1 : mSelected;
    }

    int nFirst = mSelected - mCursorRow;
    if (nFirst < 0) {
        nFirst = 0;
    }
    int nItem = nFirst;
    int nRow = 0;
    for (std::list<std::vector<Cell> >::iterator it = mRowCells.begin(); it != mRowCells.end();
         ++it) {
        if (nRow < mItemCount && nItem < mItemCount) {
            mRows[nRow]->SetShowing(1);
            UpdateRow(nRow, nItem);
        } else {
            mRows[nRow]->SetShowing(0);
        }
        ++nRow;
        ++nItem;
    }

    if (mDownArrow != nullptr) {
        mDownArrow->SetShowing(mShowDownArrow || nItem < mItemCount ? 1 : 0);
    }
    if (mUpArrow != nullptr) {
        mUpArrow->SetShowing(mShowUpArrow || nFirst > 0 ? 1 : 0);
    }
    UpdateCursor();
}

void UIList::UpdateCursor() {
    if (mItemCount == 0) {
        SetCursorSelected(false);
        return;
    }
    Vector3 translation = mCursorXfm.mTranslation;
    translation.y -= mCursorRow * mRowPitch;
    mCursor->mDirty = 1;
    mCursor->mLocalXfm.mTranslation = translation;
    mCursor->SetMat(mStyle->GetMat(kStateSelected));
    SetCursorSelected(true);
}

void UIList::SetCursorSelected(bool bSelected) {
    mCursor->SetMat(mStyle->GetMat(bSelected ? kStateSelected : kStateNormal));
}

bool UIList::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIComponent::DispatchPriv(pMsg);
}

bool UIList::HandleJoypad(JoypadInputMsg *pMsg) {
    if ((pMsg->mButton == kPadDUp || pMsg->mButton == kPadDDown) && pMsg->mPressed != 0) {
        if (pMsg->mButton == kPadDUp) {
            ScrollUp();
        } else {
            ScrollDown();
        }
        return false;
    }
    if (pMsg->mButton != kPadCross || pMsg->mPressed == 0) {
        return false;
    }
    UIComponent *pCursor = TheUI.FindPanel(mPanelName, false)->FindComponent("cursor", false);
    if (pCursor == nullptr) {
        return false;
    }
    pCursor->SetState(kStateSelected, true);
    return pCursor->Dispatch(pMsg);
}
