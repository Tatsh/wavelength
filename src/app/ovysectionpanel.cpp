#include "app/ovysectionpanel.h"

#include "os/locale.h"
#include "rnd/manager.h"

namespace {

// The shared-screen layout, which shows only the label of the current section.
constexpr char kSharedHud = 'm';

// The number of the first entry mesh. Number 01 is the title.
constexpr int kFirstEntry = 2;

Rnd::Mesh *FindEntryMesh(char chHud, int nEntry) {
    return dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("HUD%cr_sect_%02d.mesh", chHud, nEntry)));
}

} // namespace

OvySectionPanel::OvySectionPanel(char chHud, const String &prefix, Rnd::Transformable *pParent)
    : HideablePanel(FormatString("%s_sect.tnm", prefix.c_str()), nullptr, false) {
    mHud = chHud;
    mCursorOffMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("HUD%cr cursor_no.mat", chHud)));
    mCursorOnMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("HUD%cr cursor_hi.mat", chHud)));
    mBackground = dynamic_cast<Rnd::Drawable *>(
        Rnd::TheManager.Find(FormatString("%s_sect_bg.mesh", prefix.c_str())));
    mPanel = dynamic_cast<Rnd::Transformable *>(
        Rnd::TheManager.Find(FormatString("%s_sect_panel.mesh", prefix.c_str())));
    auto *pTitle =
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(FormatString("HUD%cr_sect_01.txt", mHud)));
    pTitle->SetText(TheLocale.Localize("REMIX_SECTIONS_TITLE", true));
    for (int i = kFirstEntry;; ++i) {
        Rnd::Mesh *pMesh = FindEntryMesh(mHud, i);
        if (pMesh == nullptr) {
            break;
        }
        pMesh->SetShowing(false);
    }
    pParent->AddTrans(mPanel);
}

OvySectionPanel::~OvySectionPanel() = default;

void OvySectionPanel::Poll() {
    HideablePanel::Poll();
}

void OvySectionPanel::Draw() {
    if (mSlide.mValue != 0.0f) {
        mBackground->Draw();
    }
}

void OvySectionPanel::SetSectionCount(int nCount) {
    mSections.clear();
    mSections.resize(nCount, Section{String(), nullptr, nullptr});
    int nEntry = kFirstEntry;
    if (nCount != 0) {
        if (mHud == kSharedHud) {
            Section &section = mSections[0];
            section.mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUDmr_sect_02.txt"));
            mSections[0].mMesh =
                dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find("HUDmr_sect_02.mesh"));
            ++nEntry;
            mSections[0].mMesh->SetShowing(true);
            mSections[0].mMesh->SetMat(mCursorOffMat);
            mSections[0].mText->SetText("");
        } else {
            for (int i = 0; i < nCount; ++i) {
                mSections[i].mText = dynamic_cast<Rnd::Text *>(
                    Rnd::TheManager.Find(FormatString("HUD%cr_sect_%02d.txt", mHud, nEntry)));
                mSections[i].mMesh = FindEntryMesh(mHud, nEntry);
                ++nEntry;
                mSections[i].mMesh->SetShowing(true);
                mSections[i].mMesh->SetMat(mCursorOffMat);
                mSections[i].mText->SetText("");
            }
        }
    }
    for (;; ++nEntry) {
        Rnd::Mesh *pMesh = FindEntryMesh(mHud, nEntry);
        if (pMesh == nullptr) {
            break;
        }
        pMesh->SetShowing(false);
    }
}

void OvySectionPanel::SetSectionLabel(int nIndex, const char *pszLabel) {
    Section &section = mSections[nIndex];
    if (mHud == kSharedHud) {
        section.mLabel = pszLabel;
    } else {
        section.mText->SetText(pszLabel);
    }
}

void OvySectionPanel::HighlightSection(int nIndex) {
    if (mHud == kSharedHud) {
        mSections[0].mText->SetText(mSections[nIndex].mLabel.c_str());
        return;
    }
    for (int i = static_cast<int>(mSections.size()) - 1; i >= 0; --i) {
        mSections[i].mMesh->SetMat(i == nIndex ? mCursorOnMat : mCursorOffMat);
    }
}
