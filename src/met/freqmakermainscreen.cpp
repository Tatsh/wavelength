#include "met/freqmakermainscreen.h"

#include <string.h>

#include "game/avatarcam.h"
#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "math/color.h"
#include "math/rand.h"
#include "memcard/mcmanager.h"
#include "met/avatarpanel.h"
#include "met/freqmakercustomscreen.h"
#include "met/freqmakererrorscreen.h"
#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/metagame.h"
#include "met/metagameutil.h"
#include "met/saveeditedfreqscreen.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "synth/fxmidi.h"
#include "ui/uibutton.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

// The longest name the keyboard accepts, and the widest entry it allows.
constexpr int kMaxNameChars = 15;
constexpr int kMaxNameWidth = 115;

// The range of each random colour component.
constexpr float kRandomComponentMin = 0.2f;
constexpr float kRandomComponentMax = 1.0f;
constexpr float kOpaque = 1.0f;

// The first node of a prefab is its name.
constexpr int kPrefabName = 0;

// Each of `prefabs` and `locked_prefabs` starts with its tag.
constexpr int kFirstPrefab = 1;
constexpr int kPrefabTags = 2;

// The first entry of the choices of a part is no choice at all.
constexpr int kFirstRealChoice = 1;

// The name the save button treats as no name.
const char *const kDefaultName = "Player 1";

Rnd::Mesh *FindMesh(const char *pszName) {
    return dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(pszName));
}

Rnd::Mat *FindMat(const char *pszName) {
    return dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pszName));
}

AvatarPanel *FindAvatarPanel() {
    return dynamic_cast<AvatarPanel *>(TheUI.FindPanel("f_maker_p", false));
}

// Write the name of the Freq on the panel that shows it.
void ShowName(const char *pszName) {
    dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("f_maker_p_01.txt"))->SetText(pszName);
}

// Focus a component of a panel.
void FocusComponent(UIPanel *pPanel, const char *pszComponent) {
    pPanel->SetFocus(pPanel->FindComponent(pszComponent, false), kPadNone);
}

} // namespace

FreqMakerMainScreen::FreqMakerMainScreen(DataArray *pData)
    : FreqMakerUndoScreen(pData), mEditing(0), mNameTyped(0), mChanged(0) {
}

FreqMakerMainScreen::~FreqMakerMainScreen() {
}

void FreqMakerMainScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    if (strcmp(pPrevScreen->mName, "pre_f_maker_save") == 0 ||
        strcmp(pPrevScreen->mName, "pre_f_maker_save_net") == 0) {
        if (mProfile.IsModified() != 0 && TheGameDb->GetProfile(0)->IsModified() == 0) {
            mProfile.SetModified(0);
        }
    }

    SetAvatarCam("f_maker");
    FreqScreen::Enter(pPrevScreen, fTime);
    FindAvatarPanel()->SetAvatar(TheGameDb->GetAvatar(0));
    FindMesh("f_maker_panel.mesh")->SetMat(FindMat("panel_sub_hi.mat"));

    UIPanel *pPanel = TheUI.FindPanel("f_maker", false);
    UIButton *pName = dynamic_cast<UIButton *>(pPanel->FindComponent("name", false));
    if (TheGameDb->GetProfile(0)->mNameLocked != 0) {
        pName->SetStyle(TheUI.FindStyle("button_style_grey", false));
    }

    DataArray *pMetagame = SystemConfig()->FindArray("metagame", true);
    mPrefabsData = pMetagame->FindArray("prefabs", true);
    mLockedPrefabsData = pMetagame->FindArray("locked_prefabs", true);
    mPrefabs.clear();
    mPrefabs.reserve(mPrefabsData->Size() + mLockedPrefabsData->Size() - kPrefabTags);
    for (int i = kFirstPrefab; i < mPrefabsData->Size(); ++i) {
        mPrefabs.push_back(mPrefabsData->Array(i)->Sym(kPrefabName));
    }
    for (int i = kFirstPrefab; i < mLockedPrefabsData->Size(); ++i) {
        const char *pszPrefab = mLockedPrefabsData->Array(i)->Sym(kPrefabName);
        if (TheGameDb->GetProfile(0)->IsUnlocked(pszPrefab, GameDb::kSkillAny)) {
            mPrefabs.push_back(pszPrefab);
        }
    }
    mPrefabIndex = 0;

    UIScreen *pKeyboard = TheUI.FindScreen("kb_screen", false);
    if (pPrevScreen == pKeyboard) {
        const bool bEmpty = mFreqName.mLength == 0;
        if (bEmpty || mNameTrimmed != 0) {
            if (mNameTyped != 0) {
                UIScreen *pError = TheUI.FindScreen(
                    bEmpty ? "no_empty_filename_screen" : "no_lead_trail_spaces_screen", false);
                pError->ClearTransitions();
                pError->AddTransition("ok", kPadNone, "f_maker");
                TheUI.GotoScreen(pError);
                mSaveOnName = 0;
                return;
            }
        } else if (mNameTyped != 0) {
            FocusComponent(pPanel, "save");
        }
        ShowName(mFreqName.c_str());
        mSaveOnName = 0;
        return;
    }

    if (pPrevScreen != TheUI.FindScreen("f_maker_custom", false) &&
        pPrevScreen != TheUI.FindScreen("f_maker_name_locked_screen", false) &&
        pPrevScreen != TheUI.FindScreen("f_maker_lose_custom_changes", false) &&
        pPrevScreen != TheUI.FindScreen("f_maker_lose_main_changes", false) &&
        pPrevScreen != TheUI.FindScreen("no_lead_trail_spaces_screen", false)) {
        mNameTyped = 0;
        if (mEditing == 0 && mSaveStarted == 0) {
            PlayerProfile profile;
            mFreqName = "";
            profile.mName = mFreqName.c_str();
            AvatarPanel *pAvatarPanel = FindAvatarPanel();
            pAvatarPanel->SetAvatar(nullptr);
            *TheGameDb->GetProfile(0) = profile;
            pAvatarPanel->SetAvatar(TheGameDb->GetAvatar(0));
            DataArray *pPrefab = FindPrefab(mPrefabs[mPrefabIndex]);
            TheGameDb->GetAvatar(0)->Load(pPrefab);
            FocusComponent(pPanel, "prefabs");
        }
    }

    if (strcmp(mFreqName.c_str(), "") == 0) {
        mFreqName = TheGameDb->GetPlayerName(0);
    }
    ShowName(mFreqName.c_str());
    mSaveOnName = 0;
}

void FreqMakerMainScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
    FindMesh("f_maker_panel.mesh")->SetMat(FindMat("panel_sub.mat"));
    if (pNextScreen == nullptr || dynamic_cast<FreqMakerCustomScreen *>(pNextScreen) == nullptr) {
        SetAvatarCam("bust");
    }
}

DataArray *FreqMakerMainScreen::FindPrefab(const char *pszPrefab) {
    for (int i = kFirstPrefab; i < mPrefabsData->Size(); ++i) {
        if (pszPrefab == mPrefabsData->Array(i)->Sym(kPrefabName)) {
            return mPrefabsData->Array(i);
        }
    }
    for (int i = kFirstPrefab; i < mLockedPrefabsData->Size(); ++i) {
        if (pszPrefab == mLockedPrefabsData->Array(i)->Sym(kPrefabName)) {
            return mLockedPrefabsData->Array(i);
        }
    }
    return nullptr;
}

int FreqMakerMainScreen::ReceiveKeyboardText(const char *pszText) {
    mChanged = 1;
    mNameTyped = 1;
    mFreqName = pszText;
    mNameTrimmed = TrimSpaces(&mFreqName) ? 1 : 0;
    if (mFreqName.mLength != 0 && mSaveOnName != 0) {
        GotoSave();
        return 0;
    }
    return 1;
}

void FreqMakerMainScreen::OpenKeyboard() {
    KeyboardPanel *pKeyboard = dynamic_cast<KeyboardPanel *>(TheUI.FindPanel("keyboard", false));
    const char *pszName =
        mFreqName.mLength != 0 ? mFreqName.c_str() : TheLocale.Localize("player_1", true);
    KeyboardRequest request(this, this, pszName, 0, kMaxNameChars, kMaxNameWidth, 0, 1, 0, 0);
    pKeyboard->SetRequest(request);
    TheUI.GotoScreen("kb_screen");
}

void FreqMakerMainScreen::Undo() {
    FindAvatarPanel()->SetAvatar(nullptr);
    TheGameDb->ClearPlayers();
    TheGameDb->AddPlayer(&mProfile);
    mEditing = 0;
    mChanged = 0;
    mFreqName = "";
}

void FreqMakerMainScreen::ShowSaveHelp(UIComponent *pComponent) {
    if (strcmp(pComponent->mName, "save") != 0) {
        return;
    }
    String help(
        FormatString(TheLocale.Localize("f_maker_save_HELP", true), TheMCManager.GetSlotName(0)));
    TheMetagame.SetHelpText(help.c_str());
}

void FreqMakerMainScreen::GotoSave() {
    const char *pszScreen = TheGameDb->mCommunity == GameDb::kCommunitySolo ?
                                "fmaker_save_freq" :
                                "fmaker_save_freq_net";
    TheMetagame.mGizmo->SetShowAvatar(true);
    SaveEditedFreqScreen *pSave =
        dynamic_cast<SaveEditedFreqScreen *>(TheUI.FindScreen(pszScreen, false));
    pSave->mOverwriteStatus = mEditing;
    if (mEditing != 0) {
        pSave->mFreqName = TheGameDb->GetPlayerName(0);
    } else {
        pSave->mFreqName = mFreqName.c_str();
    }
    if (strcmp(mFreqName.c_str(), TheGameDb->GetPlayerName(0)) != 0) {
        pSave->mOverwriteStatus = 0;
    }
    TheGameDb->GetProfile(0)->mName = mFreqName.c_str();
    TheGameDb->GetProfile(0)->mCustom = 1;
    TheGameDb->GetProfile(0)->SetModified(1);
    TheUI.GotoScreen(pSave);
    mSaveStarted = 1;
}

bool FreqMakerMainScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent != nullptr) {
        ShowSaveHelp(pMsg->mComponent);
    }
    return false;
}

bool FreqMakerMainScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (pMsg->mScreen == this && mFocusPanel != nullptr && mFocusPanel->mFocus != nullptr) {
        ShowSaveHelp(mFocusPanel->mFocus);
    }
    return false;
}

bool FreqMakerMainScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross && strcmp(pMsg->mComponent->mName, "save") == 0) {
        GotoSave();
    }
    return UIScreen::HandleSelect(pMsg);
}

bool FreqMakerMainScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszButton = pMsg->mComponent->mName;
    if (pMsg->mButton != kPadCross) {
        if (strcmp(pszButton, "prefabs") != 0) {
            return FreqScreen::HandleSelectStart(pMsg);
        }
        mChanged = 1;
        // Yes, the binary discards this lookup.
        (void)SystemConfig()->FindArray("metagame", true)->FindArray("prefabs", true);
        const int nPrefabs = static_cast<int>(mPrefabs.size());
        if (pMsg->mButton == kPadDLeft) {
            mPrefabIndex = mPrefabIndex - 1 > -1 ? mPrefabIndex - 1 : nPrefabs - 1;
            FxMidi::PlayMenuDown();
        } else if (pMsg->mButton == kPadDRight) {
            mPrefabIndex = mPrefabIndex + 1 < nPrefabs ? mPrefabIndex + 1 : 0;
            FxMidi::PlayMenuUp();
        }
        AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
        pAvatar->Load(FindPrefab(mPrefabs[mPrefabIndex]));
        return true;
    }

    if (strcmp(pszButton, "random") == 0) {
        AvatarPartSet *pAvatar = TheGameDb->GetAvatar(0);
        std::vector<const char *> types;
        for (int nPart = 0; nPart < AvatarPartSet::kNumParts; ++nPart) {
            if (nPart == AvatarPartSet::kPartInstrument || nPart == AvatarPartSet::kPartRightArm) {
                continue;
            }
            TheGameDb->GetProfile(0)->GetUnlockedParts(nPart, &types);
            const int nFirst =
                nPart == AvatarPartSet::kPartHeadGear || nPart == AvatarPartSet::kPartFaceGear ?
                    0 :
                    kFirstRealChoice;
            const unsigned char nType =
                static_cast<unsigned char>(RandomInt(nFirst, static_cast<int>(types.size())));
            Color color;
            color.r = RandomFloat(kRandomComponentMin, kRandomComponentMax);
            color.g = RandomFloat(kRandomComponentMin, kRandomComponentMax);
            color.b = RandomFloat(kRandomComponentMin, kRandomComponentMax);
            color.a = kOpaque;
            pAvatar->SetPart(nPart, types[nType]);
            pAvatar->SetColor(nPart, &color);
        }
        TheGameDb->GetProfile(0)->GetUnlockedEmblems(&types);
        const unsigned char nEmblem =
            static_cast<unsigned char>(RandomInt(0, static_cast<int>(types.size())));
        pAvatar->SetEmblem(types[nEmblem]);
        mChanged = 1;
    } else if (strcmp(pszButton, "save") == 0) {
        if (mFreqName.mLength == 0 || (mNameTyped == 0 && mFreqName == kDefaultName)) {
            mSaveOnName = 1;
            OpenKeyboard();
            return true;
        }
    } else if (strcmp(pszButton, "name") == 0) {
        if (TheGameDb->GetProfile(0)->mNameLocked != 0) {
            TheUI.GotoScreen("f_maker_name_locked_screen");
        } else {
            OpenKeyboard();
        }
    } else if (strcmp(pszButton, "custom") == 0) {
        mChanged = 1;
    } else if (strcmp(pszButton, "prefabs") == 0) {
        FocusComponent(TheUI.FindPanel("f_maker", false), "save");
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool FreqMakerMainScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && pMsg->mPressed != 0) {
        const bool bOnline = TheGameDb->mCommunity == GameDb::kCommunityOnline;
        if (mChanged != 0) {
            FreqMakerErrorScreen *pError = dynamic_cast<FreqMakerErrorScreen *>(
                TheUI.FindScreen("f_maker_lose_main_changes", false));
            pError->SetScreens(bOnline ? "f_net_confirm" : "f_confirm", "f_maker");
            TheUI.GotoScreen(pError);
        } else {
            Undo();
            TheUI.GotoScreen(bOnline ? "f_net_confirm" : "f_confirm");
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool FreqMakerMainScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
