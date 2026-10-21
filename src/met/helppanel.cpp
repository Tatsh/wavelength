#include "met/helppanel.h"

#include <cstring>
#include <memory>

#include "math/vector3.h"
#include "os/locale.h"
#include "os/system.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "ui/uimanager.h"

namespace {

constexpr char kEmptyText[] = "";
constexpr char kDefaultFont[] = "F1";
constexpr char kFontMarkStart[] = "<F";
constexpr char kFontMarkEnd[] = ">";
constexpr char kMetagameKey[] = "metagame";
constexpr char kHelpOffsetKey[] = "help_offset";
constexpr char kHelpFormat[] = "%s_HELP";
constexpr char kComponentHelpFormat[] = "%s_%s_HELP";
constexpr char kActionFormat[] = "%s_ACTION";
constexpr char kDefaultAction[] = "default_ACTION";

constexpr int kAxisX = 0;
constexpr int kAxisY = 1;
constexpr int kAxisZ = 2;

RndText *FindText(const char *pszName) {
    return dynamic_cast<RndText *>(TheManager.Find(pszName));
}

// Read the `help_offset` entry. An absent entry does not change the offset.
inline void ReadHelpOffset(int &nOffset) {
    SystemConfig()->FindArray(kMetagameKey, false)->FindInt(kHelpOffsetKey, &nOffset, false);
}

// Set the font and the text of a text object and place it at a position.
inline void PlaceRun(RndText *pText, const HelpPanel::HelpText &run, const float *pPosition) {
    const char *pszText = run.mText.c_str();
    pText->mDirty = 1;
    Vector3 &translation = pText->mLocalXfm.mTranslation;
    translation.x = pPosition[kAxisX];
    translation.y = pPosition[kAxisY];
    translation.z = pPosition[kAxisZ];
    translation.w = pPosition[kVec3PaddingFloat];
    pText->SetFont(dynamic_cast<RndFont *>(TheManager.Find(run.mFont.c_str())));
    pText->SetText(pszText);
}

} // namespace

std::unique_ptr<RndLoader> HelpPanel::sLoader;

HelpPanel::HelpPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    mActionText = kEmptyText;
    mView = nullptr;
    mHelpText = kEmptyText;
}

HelpPanel::~HelpPanel() {
}

void HelpPanel::Load() {
    if (sLoader == nullptr) {
        sLoader = TheManager.AddLoader(
            mFile.c_str(), RndLoader::kAsync | RndLoader::kDeleteObjects, nullptr, nullptr);
    }
    mLoader = sLoader.get();
    UIPanel::Load();
}

void HelpPanel::Unload() {
    if (mLoadRefs == 1) {
        mLoader = nullptr;
    }
    UIPanel::Unload(); // Yes, the binary skips FreqPanel::Unload().
}

void HelpPanel::FinishLoad() {
    if (mLoaded) {
        return;
    }
    FreqPanel::FinishLoad();
    mFonts = SystemConfig()->FindArray(kMetagameKey, false)->FindArray("help_fonts", false);
    for (int i = 1; i <= kNumTexts; ++i) {
        RndText *pText = FindText(FormatString("help_help%d.txt", i));
        pText->SetText(kEmptyText);
        mHelpTexts.push_back(pText);
        pText = FindText(FormatString("help_action%d.txt", i));
        pText->SetText(kEmptyText);
        mActionTexts.push_back(pText);
    }
    mView = dynamic_cast<RndView *>(TheManager.Find("help.view"));
    std::memcpy(mHelpOrigin, &mHelpTexts[0]->mLocalXfm.mTranslation, sizeof(mHelpOrigin));
    std::memcpy(mActionOrigin, &mActionTexts[0]->mLocalXfm.mTranslation, sizeof(mActionOrigin));
}

const char *HelpPanel::FontOf(const char *pszName) {
    const char *pszFont = nullptr;
    mFonts->FindSymbol(pszName, &pszFont, true);
    return pszFont;
}

void HelpPanel::ParseText(const String &text) {
    String font(kDefaultFont);
    String rest(text);
    mRuns.clear();
    while (rest.mLength != 0) {
        const int nOpen = rest.Find(kFontMarkStart);
        if (nOpen == String::npos) {
            break;
        }
        const int nClose = rest.Find(kFontMarkEnd);
        if (nClose == String::npos) {
            continue; // Yes, the binary loops forever on a mark that is not closed.
        }
        if (nOpen > 0) {
            HelpText run;
            run.mFont = FontOf(font.c_str());
            run.mText = rest.Substring(0, nOpen).c_str();
            mRuns.push_back(run);
        }
        font = rest.Substring(nOpen + 1, nClose - 1 - nOpen);
        rest = rest.Substring(nClose + 1);
    }
    if (rest.mLength != 0) {
        HelpText run;
        run.mFont = FontOf(font.c_str());
        run.mText = rest.c_str();
        mRuns.push_back(run);
    }
}

void HelpPanel::ClearHelp() {
    for (unsigned int i = 0; i < mHelpTexts.size(); ++i) {
        mHelpTexts[i]->SetText(kEmptyText);
    }
}

void HelpPanel::LayoutHelp() {
    ClearHelp();
    SetTextsShowing(true);
    const int nRuns = static_cast<int>(mRuns.size());
    if (nRuns != 0) {
        float position[sizeof(Vector3) / sizeof(float)];
        std::memcpy(position, mHelpOrigin, sizeof(position));
        int nOffset = 0;
        for (int i = 0; i < nRuns; ++i) {
            RndText *pText = mHelpTexts[i];
            const HelpText &run = mRuns[i];
            PlaceRun(pText, run, position);
            Vector3 end = pText->CharPosition(static_cast<int>(strlen(run.mText.c_str())));
            ReadHelpOffset(nOffset);
            end.x += static_cast<float>(nOffset);
            position[kAxisX] += end.x;
            position[kAxisY] += end.y;
            position[kAxisZ] += end.z;
        }

        constexpr float kHalf = 0.5f;
        const float fShift = (position[kAxisX] - mHelpOrigin[kAxisX]) * kHalf;
        for (int i = 0; i < nRuns; ++i) {
            RndText *pText = mHelpTexts[i];
            pText->mLocalXfm.mTranslation.x -= fShift;
            pText->mDirty = 1;
        }
    }
    (void)mView->UpdateWorldXfm(nullptr, 1); // Yes, the binary discards the result.
}

void HelpPanel::LayoutActions() {
    SetTextsShowing(true);
    for (unsigned int i = 0; i < mActionTexts.size(); ++i) {
        mActionTexts[i]->SetText(kEmptyText);
    }
    const int nRuns = static_cast<int>(mRuns.size());
    if (nRuns != 0) {
        float position[sizeof(Vector3) / sizeof(float)];
        std::memcpy(position, mActionOrigin, sizeof(position));
        int nOffset = 0;
        for (int i = nRuns - 1; i >= 0; --i) {
            RndText *pText = mActionTexts[i];
            PlaceRun(pText, mRuns[i], position);
            Vector3 start = pText->CharPosition(0);
            ReadHelpOffset(nOffset);
            start.x -= static_cast<float>(nOffset);
            position[kAxisX] += start.x;
            position[kAxisY] += start.y;
            position[kAxisZ] += start.z;
        }
    }
    (void)mView->UpdateWorldXfm(nullptr, 1); // Yes, the binary discards the result.
}

void HelpPanel::Refresh() {
    UIScreen *pScreen = TheUI.GetCurrentScreen();
    if (pScreen == nullptr || pScreen->GetFocusPanel() == nullptr) {
        ClearHelp();
        return;
    }
    ShowHelp(pScreen->GetFocusPanel(), pScreen->GetFocusPanel()->GetFocus());
}

void HelpPanel::ShowHelp(UIPanel *pPanel, UIComponent *pComponent) {
    const char *pszKey = FormatString(kHelpFormat, TheUI.GetCurrentScreen()->GetName());
    const char *pszText = TheLocale.Localize(pszKey, false);
    if (strcmp(pszText, pszKey) == 0) {
        pszKey = FormatString(kHelpFormat, pPanel->GetName());
        pszText = TheLocale.Localize(pszKey, false);
        // The locale returns the token itself when it lacks the token.
        if (pszText == pszKey && pComponent != nullptr) {
            pszText = TheLocale.Localize(
                FormatString(kComponentHelpFormat, pPanel->GetName(), pComponent->GetName()),
                false);
        }
    }
    if (pszText == nullptr || strcmp(mHelpText, pszText) == 0) {
        return;
    }
    mHelpText = pszText;
    ParseText(String(pszText));
    LayoutHelp();
}

void HelpPanel::ShowActions(UIScreen *pScreen) {
    const char *pszKey = kEmptyText;
    const char *pszText;
    if (pScreen == nullptr) {
        pszText = TheLocale.Localize(kDefaultAction, false);
    } else {
        UIPanel *pPanel = pScreen->GetFocusPanel();
        pszKey = FormatString(kActionFormat, pScreen->GetName());
        pszText = TheLocale.Localize(pszKey, false);
        if (strcmp(pszText, pszKey) == 0 && pPanel != nullptr) {
            pszKey = FormatString(kActionFormat, pPanel->GetName());
            pszText = TheLocale.Localize(pszKey, false);
        }
    }
    if (strcmp(pszText, pszKey) == 0) {
        pszText = TheLocale.Localize(kDefaultAction, false);
    }
    if (pszText == nullptr || strcmp(mActionText, pszText) == 0) {
        return;
    }
    mActionText = pszText;
    ParseText(String(pszText));
    LayoutActions();
}

void HelpPanel::SetHelpText(const char *pszText) {
    mHelpText = pszText;
    ParseText(String(pszText));
    LayoutHelp();
}

void HelpPanel::SetActionText(const char *pszText) {
    ParseText(String(pszText));
    mActionText = pszText;
    LayoutActions();
}
