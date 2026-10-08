#include "app/ovyremixgenpanel.h"

#include <cmath>

#include "game/gamedb.h"
#include "math/color.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The head-up display layout whose panels have no player background material.
constexpr char kNoBackgroundHud = 's';

// The copy flags of a clone.
constexpr unsigned kCloneFlags = 0x108;

// The options space is reserved for before the first is added.
constexpr int kReservedOptions = 6;

// The ticks a blink runs for, and the period and lit part of one blink.
constexpr float kBlinkTicks = 179.0f;
constexpr float kBlinkPeriod = 90.0f;
constexpr float kBlinkOffTicks = 40.0f;

// The time a new panel's blink started at. Poll() turns it into kNoFlash.
constexpr float kInitialFlashTime = 1e9f;

const Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};

} // namespace

OvyRemixGenPanel::OvyRemixGenPanel() {
    mBackground = nullptr;
    mPanel = nullptr;
    mFlashTime = kInitialFlashTime;
    mUpArrow = nullptr;
    mDownArrow = nullptr;
    mRule = nullptr;
    mSelected = nullptr;
    mCursorOffMat = nullptr;
    mCursorOnMat = nullptr;
    mFonts[kFontNormal] = nullptr;
    mFonts[kFontSelected] = nullptr;
}

OvyRemixGenPanel::~OvyRemixGenPanel() {
    for (auto it = mClones.begin(); it != mClones.end();) {
        delete *it;
        it = mClones.erase(it);
    }
}

void OvyRemixGenPanel::Load(char chHud,
                            const char *pszPrefix,
                            int nPlayer,
                            Rnd::Transformable *pParent) {
    mCursorOffMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("HUD%cr cursor_no.mat", chHud)));
    mFonts[kFontNormal] = nullptr;
    mCursorOnMat =
        dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(FormatString("HUD%cr cursor_hi.mat", chHud)));
    mFonts[kFontSelected] = dynamic_cast<Rnd::Font *>(
        Rnd::TheManager.Find(FormatString("HUD%cr selected.font", chHud)));
    mBackground = dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_bg.mesh", pszPrefix), nPlayer));
    if (chHud != kNoBackgroundHud) {
        mBackground->SetMat(dynamic_cast<Rnd::Mat *>(
            Rnd::TheManager.Find(FormatString("HUD%cr freq%d bg.mat", chHud, nPlayer))));
    }
    mPanel = dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_panel.mesh", pszPrefix), nPlayer));

    mOptions.reserve(kReservedOptions);
    for (int i = 1;; ++i) {
        auto *pMesh =
            dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_%02d.mesh", pszPrefix, i), nPlayer));
        if (pMesh == nullptr) {
            break;
        }
        auto *pText =
            dynamic_cast<Rnd::Text *>(Clone(FormatString("%s_%02d.txt", pszPrefix, i), nPlayer));
        mOptions.push_back(Option{pMesh, pText});
        const Option &option = mOptions.back();
        option.mMesh->AddDraw(option.mText, nullptr);
        option.mMesh->AddTrans(option.mText);
        mBackground->AddDraw(option.mMesh, nullptr);
        mBackground->AddTrans(option.mMesh);
        option.mMesh->SetMat(mCursorOffMat);
        option.mText->SetText("");
        option.mText->SetColor(kWhite);
        if (mFonts[kFontNormal] == nullptr && option.mText->mFont != mFonts[kFontSelected]) {
            mFonts[kFontNormal] = option.mText->mFont;
        }
    }
    for (const Option &option : mOptions) {
        option.mText->SetFont(mFonts[kFontNormal]);
    }

    mUpArrow = dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_up.mesh", pszPrefix), nPlayer));
    if (mUpArrow != nullptr) {
        mBackground->AddDraw(mUpArrow, nullptr);
        mBackground->AddTrans(mUpArrow);
    }
    mDownArrow = dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_down.mesh", pszPrefix), nPlayer));
    if (mDownArrow != nullptr) {
        mBackground->AddDraw(mDownArrow, nullptr);
        mBackground->AddTrans(mDownArrow);
    }
    mRule = dynamic_cast<Rnd::Mesh *>(Clone(FormatString("%s_hr.mesh", pszPrefix), nPlayer));
    if (mRule != nullptr) {
        mBackground->AddDraw(mRule, nullptr);
        mBackground->AddTrans(mRule);
    }

    if (mFonts[kFontNormal] == nullptr) {
        mFonts[kFontNormal] = mFonts[kFontSelected];
    }
    mPanel->AddTrans(mBackground);
    mBackground->AddDraw(mPanel, nullptr);
    if (pParent != nullptr) {
        pParent->AddTrans(mPanel);
    }
}

Rnd::Object *OvyRemixGenPanel::Clone(const char *pszName, int nPlayer) {
    Rnd::Object *pSource = Rnd::TheManager.Find(pszName);
    if (pSource == nullptr) {
        return nullptr;
    }
    Rnd::TheManager.Clone(pSource, "Copy foo ", &mClones, kCloneFlags, 0, 0);
    Rnd::Object *pClone = mClones.back();
    pClone->SetName(FormatString("%s pl %d", pSource->mName.mStr, nPlayer));
    return pClone;
}

void OvyRemixGenPanel::Poll([[maybe_unused]] float fDelta) {
    if (mFlashTime == kNoFlash) {
        return;
    }
    const float fElapsed = TheGameDb->mSongTime - mFlashTime;
    if (mSelected == nullptr || kBlinkTicks <= fElapsed) {
        mFlashTime = kNoFlash;
        if (mSelected != nullptr) {
            mSelected->mMesh->SetMat(mCursorOnMat);
        }
        return;
    }
    float fPhase = std::fmod(fElapsed, kBlinkPeriod);
    if (fPhase < 0.0f) {
        fPhase += kBlinkPeriod;
    }
    mSelected->mMesh->SetMat(kBlinkOffTicks < fPhase ? mCursorOnMat : mCursorOffMat);
}

void OvyRemixGenPanel::Draw() {
    mBackground->Draw();
}

void OvyRemixGenPanel::SetText(int nIndex, const char *pszText) {
    mOptions[nIndex].mText->SetText(pszText);
}

void OvyRemixGenPanel::Flash() {
    if (mSelected != nullptr) {
        mFlashTime = TheGameDb->mSongTime;
    }
}

void OvyRemixGenPanel::Select(int nIndex) {
    if (mSelected != nullptr) {
        mSelected->mMesh->SetMat(mCursorOffMat);
    }
    if (nIndex >= 0) {
        mSelected = &mOptions[nIndex];
        mSelected->mMesh->SetMat(mCursorOnMat);
    } else {
        mSelected = nullptr;
    }
}

void OvyRemixGenPanel::SetLit(int nIndex, bool bLit) {
    mOptions[nIndex].mText->SetFont(mFonts[bLit ? kFontSelected : kFontNormal]);
}
