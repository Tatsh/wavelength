#include "met/freqmakerpartconfigscreen.h"

#include <string.h>

#include "game/avatarcam.h"
#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "met/metagame.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

// A colour as a hue in degrees, a saturation, a value, and an unused fourth word.
struct Hsv {
    float h;
    float s;
    float v;
    float a;
};

// Each colour setting has this many steps.
constexpr int kNumSteps = 51;

// The degrees of hue in one step, and the share of saturation or brightness in one step.
constexpr float kHueStepDegrees = 360.0f / kNumSteps;
constexpr float kStepsPerUnit = 50.0f;

// The saturation a hue starts with when the colour had none.
constexpr int kDefaultSaturation = 25;

// Steps round to the nearest one.
constexpr float kRound = 0.5f;

constexpr float kOpaque = 1.0f;

// The hue wraps at a whole turn, and each sixth of it is one segment of the conversion.
constexpr float kFullTurn = 360.0f;
constexpr float kSegmentDegrees = 60.0f;
constexpr int kNumSegments = 6;

// The offsets of the green and blue segments of the hue, in segments.
constexpr float kGreenSegment = 2.0f;
constexpr float kBlueSegment = 4.0f;

// A colour this dark or darker has no saturation.
constexpr double kMinValue = 0.0001;

// Clamp a step to a range whose lowest step is nMin and whose highest is nLimit - 1. A step at
// either end counts as clamped.
//
// NTSC-U/C: 0x0019eb10, PAL: 0x001a6828
bool ClampStep(int nValue, int nMin, int nLimit, int *pValue) {
    const int nMax = nLimit - 1;
    if (nValue <= nMin) {
        *pValue = nMin;
        return true;
    }
    if (nValue >= nMax) {
        *pValue = nMax;
        return true;
    }
    return false;
}

// Convert a colour to a hue, saturation, and value. The hue is unchanged when no component is the
// largest, as for a component that is not a number.
//
// NTSC-U/C: 0x001a25d0, PAL: 0x001aa2b0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
void RgbToHsv(const Color *pRgb, Hsv *pHsv) {
    const float r = pRgb->r;
    const float g = pRgb->g;
    const float b = pRgb->b;
    const float fMaxGb = g < b ? b : g;
    const float fMax = r < fMaxGb ? fMaxGb : r;
    const float fMinGb = b < g ? b : g;
    const float fMin = fMinGb < r ? fMinGb : r;

    pHsv->v = fMax;
    if (static_cast<double>(fMax) - kMinValue > 0.0) {
        pHsv->s = (fMax - fMin) / fMax;
    } else {
        pHsv->s = 0.0f;
    }
    if (pHsv->s == 0.0f) {
        pHsv->h = 0.0f;
        return;
    }

    const float fDelta = fMax - fMin;
    if (r == fMax) {
        pHsv->h = (g - b) / fDelta;
    } else if (g == fMax) {
        pHsv->h = (b - r) / fDelta + kGreenSegment;
    } else if (b == fMax) {
        pHsv->h = (r - g) / fDelta + kBlueSegment;
    }
    pHsv->h = pHsv->h * kSegmentDegrees;
    if (pHsv->h < 0.0f) {
        pHsv->h = pHsv->h + kFullTurn;
    }
}
#pragma GCC diagnostic pop

// Convert a hue, saturation, and value to a colour. The alpha is not written.
//
// NTSC-U/C: 0x001a27a0, PAL: 0x001aa480
void HsvToRgb(const Hsv *pHsv, Color *pRgb) {
    const float s = pHsv->s;
    const float v = pHsv->v;
    float h = pHsv->h;
    if (s == 0.0f) {
        pRgb->r = v;
        pRgb->b = v;
        pRgb->g = v;
        return;
    }
    if (h == kFullTurn) {
        h = 0.0f;
    }
    h = h / kSegmentDegrees;
    const float p = v * (1.0f - s);
    const int nSegment = static_cast<int>(h);
    const float f = h - static_cast<float>(nSegment);
    const float q = v * (1.0f - s * f);
    const float t = v * (1.0f - s * (1.0f - f));
    if (static_cast<unsigned>(nSegment) >= static_cast<unsigned>(kNumSegments)) {
        return;
    }
    switch (nSegment) {
    case 0:
        pRgb->b = p;
        pRgb->r = v;
        pRgb->g = t;
        break;
    case 1:
        pRgb->b = p;
        pRgb->r = q;
        pRgb->g = v;
        break;
    case 2:
        pRgb->b = t;
        pRgb->r = p;
        pRgb->g = v;
        break;
    case 3:
        pRgb->b = v;
        pRgb->r = p;
        pRgb->g = q;
        break;
    case 4:
        pRgb->b = v;
        pRgb->r = t;
        pRgb->g = p;
        break;
    default:
        pRgb->b = q;
        pRgb->r = v;
        pRgb->g = p;
        break;
    }
}

Rnd::Mesh *FindMesh(const char *pszName) {
    return dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(pszName));
}

Rnd::Mat *FindMat(const char *pszName) {
    return dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszName));
}

LRButton *FindSetting(const char *pszButton) {
    return dynamic_cast<LRButton *>(TheUI.FindComponent("f_maker_s", pszButton, false));
}

// Write a step on the button of its setting.
void ShowStep(LRButton *pButton, const char *pszToken, int nStep) {
    pButton->SetText(FormatString(TheLocale.Localize(pszToken, true), nStep));
}

} // namespace

FreqMakerPartConfigScreen::FreqMakerPartConfigScreen(DataArray *pData) : FreqScreen(pData) {
    mPart = AvatarPartSet::kNumParts;
    mOriginalIndex = 0;
    mPartLabel = nullptr;
    mIndex = 0;
    mHue = 0;
    mSaturation = 0;
    mBrightness = 0;
    mPartButton = nullptr;
    mHueButton = nullptr;
    mBrightnessButton = nullptr;
    mSaturationButton = nullptr;
}

FreqMakerPartConfigScreen::~FreqMakerPartConfigScreen() {
}

void FreqMakerPartConfigScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mChoices.clear();

    UIComponent *pFocus = TheUI.FindPanel("f_maker_c", false)->mFocus;
    const char *pszButton = pFocus->mName;
    if (strcmp(pszButton, "heads") == 0) {
        mPart = AvatarPartSet::kPartHead;
    } else if (strcmp(pszButton, "torsos") == 0) {
        mPart = AvatarPartSet::kPartTorso;
    } else if (strcmp(pszButton, "arms") == 0) {
        mPart = AvatarPartSet::kPartLeftArm;
    } else if (strcmp(pszButton, "legs") == 0) {
        mPart = AvatarPartSet::kPartLowerBody;
    } else if (strcmp(pszButton, "head_gear") == 0) {
        mPart = AvatarPartSet::kPartHeadGear;
    } else if (strcmp(pszButton, "face_gear") == 0) {
        mPart = AvatarPartSet::kPartFaceGear;
    } else {
        DebugWarn(" No such avatar part: %s\n", pszButton);
    }

    TheGameDb->GetProfile(0)->GetUnlockedParts(mPart, &mChoices);
    AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
    mPartLabel = pFocus->Text();
    ShowPartHelp();

    const char *pszCurrent = pAvatar->PartName(mPart);
    mOriginalIndex = -1;
    mIndex = -1;
    unsigned i = 0;
    for (; i < mChoices.size(); ++i) {
        if (strcmp(mChoices[i], pszCurrent) == 0) {
            mIndex = static_cast<int>(i);
            mOriginalIndex = static_cast<int>(i);
            break;
        }
    }
    if (i == mChoices.size()) {
        mChoices.push_back(pszCurrent);
        mOriginalIndex = static_cast<int>(mChoices.size()) - 1;
        mIndex = static_cast<int>(mChoices.size()) - 1;
    }

    SetAvatarCam(pszButton);
    mPartButton = dynamic_cast<UIButton *>(TheUI.FindComponent("f_maker_s", "part", false));
    mPartButton->SetText(TheLocale.Localize(mChoices[mIndex], true));
    TheUI.FindPanel("f_maker_s", false)->SetFocus(mPartButton, kPadNone);

    const Color color = pAvatar->PartColor(mPart);
    Hsv hsv; // Yes, the binary leaves it uninitialised, and RgbToHsv() may read its hue.
    RgbToHsv(&color, &hsv);
    mOriginalColor = color;

    mHueButton = FindSetting("colorize");
    mHue = static_cast<int>(hsv.h / kHueStepDegrees + kRound);
    ShowStep(mHueButton, "f_maker_s_12", mHue);
    mSaturationButton = FindSetting("saturation");
    mSaturation = static_cast<int>(hsv.s * kStepsPerUnit + kRound);
    ShowStep(mSaturationButton, "f_maker_s_13", mSaturation);
    mBrightnessButton = FindSetting("brightness");
    mBrightness = static_cast<int>(hsv.v * kStepsPerUnit + kRound);
    ShowStep(mBrightnessButton, "f_maker_s_14", mBrightness);

    FindMesh("f_maker_s_panel.mesh")->SetMat(FindMat("panel_sub_hi.mat"));
    mErrorPlayed = 0;
    mReservedA8 = 1;
    mReservedAC = 0;
}

void FreqMakerPartConfigScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    FindMesh("f_maker_s_panel.mesh")->SetMat(FindMat("panel_sub.mat"));
    SetAvatarCam("f_maker");
}

void FreqMakerPartConfigScreen::ShowPartHelp() {
    if (mPartLabel == nullptr) {
        return;
    }
    String help(FormatString(TheLocale.Localize("f_maker_s_part_HELP", true), mPartLabel));
    TheMetagame.SetHelpText(help.c_str());
}

const char *FreqMakerPartConfigScreen::Title() {
    return TheLocale.Localize("f_maker_c_TITLE", true);
}

void FreqMakerPartConfigScreen::ApplyColor() {
    Hsv hsv;
    hsv.a = kOpaque;
    hsv.h = static_cast<float>(mHue) * kHueStepDegrees;
    hsv.s = static_cast<float>(mSaturation) / kStepsPerUnit;
    hsv.v = static_cast<float>(mBrightness) / kStepsPerUnit;
    Color color;
    HsvToRgb(&hsv, &color);
    TheGameDb->GetAvatar(0)->SetColor(mPart, &color);
}

void FreqMakerPartConfigScreen::StepDown(
    int nValue, int nMin, int nLimit, int *pValue, LRButton *pButton) {
    int nStep = nValue;
    if (nValue == nMin) {
        pButton->SetArrowShowing(LRButton::kArrowLeft, false);
        if (mErrorPlayed == 0) {
            FxMidi::PlayWrong();
            mErrorPlayed = 1;
        }
        return;
    }
    nStep = nValue - 1;
    if (ClampStep(nStep, nMin, nLimit, &nStep)) {
        pButton->SetArrowShowing(LRButton::kArrowLeft, false);
    } else {
        pButton->SetArrowShowing(LRButton::kArrowRight, true);
    }
    FxMidi::PlayMenuDown();
    *pValue = nStep;
}

void FreqMakerPartConfigScreen::StepUp(
    int nValue, int nMin, int nLimit, int *pValue, LRButton *pButton) {
    int nStep = nValue;
    if (nValue == nLimit - 1) {
        pButton->SetArrowShowing(LRButton::kArrowRight, false);
        if (mErrorPlayed == 0) {
            FxMidi::PlayWrong();
            mErrorPlayed = 1;
        }
        return;
    }
    nStep = nValue + 1;
    if (ClampStep(nStep, nMin, nLimit, &nStep)) {
        pButton->SetArrowShowing(LRButton::kArrowRight, false);
    } else {
        pButton->SetArrowShowing(LRButton::kArrowLeft, true);
    }
    FxMidi::PlayMenuUp();
    *pValue = nStep;
}

bool FreqMakerPartConfigScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadCross && strcmp(pMsg->mComponent->mName, "done") != 0) {
        UIPanel *pPanel = TheUI.FindPanel("f_maker_s", false);
        pPanel->SetFocus(pPanel->FindComponent("done", false), kPadNone);
    }

    const char *pszButton = pMsg->mComponent->mName;
    const int nButton = pMsg->mButton;
    if (strcmp(pszButton, "part") == 0) {
        const int nChoices = static_cast<int>(mChoices.size());
        if (nButton == kPadDLeft) {
            mIndex = mIndex - 1 > -1 ? mIndex - 1 : nChoices - 1;
            FxMidi::PlayMenuDown();
        } else if (nButton == kPadDRight) {
            mIndex = mIndex + 1 < nChoices ? mIndex + 1 : 0;
            FxMidi::PlayMenuUp();
        }
        const char *pszChoice = mChoices[mIndex];
        mPartButton->SetText(TheLocale.Localize(pszChoice, true));
        TheGameDb->GetAvatar(0)->SetPart(mPart, pszChoice);
        return true;
    }

    if (strcmp(pszButton, "colorize") == 0) {
        if (nButton == kPadDLeft) {
            StepDown(mHue, 0, kNumSteps, &mHue, mHueButton);
        } else if (nButton == kPadDRight) {
            StepUp(mHue, 0, kNumSteps, &mHue, mHueButton);
        }
        ShowStep(mHueButton, "f_maker_s_12", mHue);
        if (mSaturation == 0 && mHue != 0) {
            mSaturation = kDefaultSaturation;
            ShowStep(mSaturationButton, "f_maker_s_13", mSaturation);
            mSaturationButton->SetArrowShowing(LRButton::kArrowLeft, true);
            mSaturationButton->SetArrowShowing(LRButton::kArrowRight, true);
        }
    } else if (strcmp(pszButton, "brightness") == 0) {
        if (nButton == kPadDLeft) {
            StepDown(mBrightness, 0, kNumSteps, &mBrightness, mBrightnessButton);
        } else if (nButton == kPadDRight) {
            StepUp(mBrightness, 0, kNumSteps, &mBrightness, mBrightnessButton);
        }
        ShowStep(mBrightnessButton, "f_maker_s_14", mBrightness);
    } else if (strcmp(pszButton, "saturation") == 0) {
        if (nButton == kPadDLeft) {
            StepDown(mSaturation, 0, kNumSteps, &mSaturation, mSaturationButton);
        } else if (nButton == kPadDRight) {
            StepUp(mSaturation, 0, kNumSteps, &mSaturation, mSaturationButton);
        }
        ShowStep(mSaturationButton, "f_maker_s_13", mSaturation);
    } else {
        return FreqScreen::HandleSelectStart(pMsg);
    }
    ApplyColor();
    return true;
}

bool FreqMakerPartConfigScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    const int nButton = pMsg->mButton;
    if (nButton == kPadTriangle && pMsg->mPressed != 0) {
        const char *pszChoice = mChoices[mOriginalIndex];
        AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
        pAvatar->SetPart(mPart, pszChoice);
        pAvatar->SetColor(mPart, &mOriginalColor);
        TheUI.GotoScreen(TheUI.FindScreen("f_maker_custom", false));
    } else if (pMsg->mPressed == 0 && (nButton == kPadDLeft || nButton == kPadDRight)) {
        mErrorPlayed = 0;
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool FreqMakerPartConfigScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent != nullptr &&
        pMsg->mComponent == TheUI.FindComponent("f_maker_s", "part", false)) {
        ShowPartHelp();
    }
    return false;
}

bool FreqMakerPartConfigScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
