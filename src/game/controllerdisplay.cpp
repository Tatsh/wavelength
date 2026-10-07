#include "game/controllerdisplay.h"

#include "game/gamedb.h"
#include "game/playerprofile.h"
#include "gfx/gfxmanager.h"
#include "math/vector2.h"
#include "os/debug.h"

namespace {

constexpr int kNumLanes = 3;

// The action of the first lane. The other lanes follow it.
constexpr int kFirstLaneAction = 3;

// The player whose bindings the labels show.
constexpr int kFirstPlayer = 0;

// The controller buttons, and the glyph of the icon of each.
enum Button {
    kButtonFirst = 0,
    kButtonSecond = 1,
    kButtonThird = 2,
    kButtonFourth = 3,
};
constexpr char kGlyphThird[] = "e";
constexpr char kGlyphFirst[] = "f";
constexpr char kGlyphFourth[] = "g";
constexpr char kGlyphSecond[] = "h";
constexpr char kIllegalButton[] = "illegal button";

// The icon of the first lane, and the space between the icons of neighbouring lanes.
constexpr int kFirstIconX = -65;
constexpr int kIconSpacingX = 67;
constexpr float kIconY = -95.0f;

} // namespace

ControllerDisplay *TheControllerDisplay = ControllerDisplay::shared();

ControllerDisplay::ControllerDisplay() : mShowCount(0), mOption(0), mHighlight(0) {
}

ControllerDisplay *ControllerDisplay::shared() {
    static ControllerDisplay sDisplay;
    return &sDisplay;
}

const char *ControllerDisplay::GetButtonGlyph(InputMap *pMap, int nAction) {
    if (pMap->GetButtonAction(kButtonThird) == nAction) {
        return kGlyphThird;
    }
    if (pMap->GetButtonAction(kButtonFirst) == nAction) {
        return kGlyphFirst;
    }
    if (pMap->GetButtonAction(kButtonFourth) == nAction) {
        return kGlyphFourth;
    }
    if (pMap->GetButtonAction(kButtonSecond) == nAction) {
        return kGlyphSecond;
    }
    DebugWarn(kIllegalButton);
    return nullptr;
}

void ControllerDisplay::Init() {
    Refresh();
    RefreshLabels();
    SetHighlight(mHighlight != 0);
}

void ControllerDisplay::Show() {
    ++mShowCount;
    Refresh();
}

void ControllerDisplay::Hide() {
    --mShowCount;
    Refresh();
}

void ControllerDisplay::SetOption(bool bOption) {
    mOption = bOption;
    Refresh();
}

void ControllerDisplay::SetHighlight(bool bHighlight) {
    mHighlight = bHighlight;
    for (int nLane = 0; nLane < kNumLanes; ++nLane) {
        TheGfxManager.SetButtonIconHighlight(nLane, mHighlight);
    }
}

void ControllerDisplay::Clear() {
    mHighlight = 0;
    mShowCount = 0;
    mOption = 0;
}

void ControllerDisplay::Refresh() {
    int nX = kFirstIconX;
    for (int nLane = 0; nLane < kNumLanes; ++nLane) {
        if (TheGameDb->IsWinSequence() == 0 && mOption == 0 && mShowCount > 0) {
            const Vector2 position{static_cast<float>(nX), kIconY};
            TheGfxManager.ShowButtonIcon(nLane, &position, &position);
        } else {
            TheGfxManager.HideButtonIcon(nLane);
        }
        nX += kIconSpacingX;
    }
}

void ControllerDisplay::RefreshLabels() {
    if (TheGameDb->mTutorial != 0) {
        return;
    }
    InputMap *pMap = TheGameDb->GetProfile(kFirstPlayer)->GetInputMap();
    for (int nLane = 0; nLane < kNumLanes; ++nLane) {
        TheGfxManager.SetButtonIconGlyph(nLane, GetButtonGlyph(pMap, nLane + kFirstLaneAction));
    }
}
