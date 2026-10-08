#include "met/chatpanel.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/metagameutil.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "rnd/manager.h"
#include "rnd/transformable.h"
#include "ui/uimanager.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kChatTypeTag[] = "chat_type";
constexpr char kNumChatLinesTag[] = "num_chat_lines";
constexpr char kSpacingTag[] = "spacing";
constexpr char kLobbyType[] = "lobby";
constexpr char kLaunchpadType[] = "launchpad";
constexpr char kSenderTextFormat[] = "%s_username01.txt";
constexpr char kMessageTextFormat[] = "%s_message01.txt";
constexpr char kClonePrefixFormat[] = "t_%02d_";
constexpr char kSenderFormat[] = "%s: ";
constexpr char kEntryComponent[] = "msg";
constexpr char kKeyboardPanel[] = "keyboard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kLobbyScreen[] = "fn_main_join";
constexpr char kHostPlayScreen[] = "fn_h_lpad_play";
constexpr char kHostScreen[] = "fn_h_lpad";
constexpr char kGuestPlayScreen[] = "fn_g_lpad_play";
constexpr char kGuestScreen[] = "fn_g_lpad";
constexpr char kNoText[] = "";

constexpr unsigned kCloneFlags = 0x200;
constexpr int kTranslationRow = Rnd::kXfmRowCount - 1;

// The player whose name is shown in the entry.
constexpr int kFirstPlayer = 0;

// The player ShowLine() receives for a line with no sender.
constexpr int kNoPlayer = -1;

// The colour of a sender who is not a player of the launchpad.
constexpr int kNoColor = -1;

// The arguments of a line this console sends.
constexpr int kEcho = 1;

// The limits of the keyboard request of the chat.
constexpr int kKeyboardPad = 0;
constexpr int kKeyboardMaxChars = 256;
constexpr int kKeyboardMaxWidth = 512;

Vector3 Translation(const Rnd::Text *pText) {
    const float *pRow = pText->mLocalXfm[kTranslationRow];
    Vector3 position;
    position.x = pRow[0];
    position.y = pRow[1];
    position.z = pRow[2];
    position.w = pRow[3];
    return position;
}

void MoveText(Rnd::Text *pText, const Vector3 &position) {
    pText->mDirty = 1;
    float *pRow = pText->mLocalXfm[kTranslationRow];
    pRow[0] = position.x;
    pRow[1] = position.y;
    pRow[2] = position.z;
    pRow[3] = position.w;
}

Rnd::Text *FindText(const char *pszFormat, const char *pszPanel) {
    Rnd::Object *pObject = Rnd::TheManager.Find(FormatString(pszFormat, pszPanel));
    return pObject != nullptr ? dynamic_cast<Rnd::Text *>(pObject) : nullptr;
}

} // namespace

ChatPanel::ChatPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    const char *pszType;
    pData->FindSymbol(kChatTypeTag, &pszType, true);
    pData->FindInt(kNumChatLinesTag, &mNumLines, true);
    pData->FindInt(kSpacingTag, &mSpacing, true);
    if (std::strcmp(pszType, kLobbyType) == 0) {
        mChatType = kChatLobby;
    } else if (std::strcmp(pszType, kLaunchpadType) == 0) {
        mChatType = kChatLaunchpad;
    }
    ChatMsg::AddHandler(mChatType, this);
    mKeyboardText = kNoText;
}

void ChatPanel::Unload() {
    FreqPanel::Unload();
    if (mLoadRefs != 0) {
        return;
    }
    for (unsigned int i = 1; i < mRows.size(); ++i) {
        if (mRows[i].first != nullptr) {
            delete static_cast<Rnd::Object *>(mRows[i].first);
        }
        mRows[i].first = nullptr;
        if (mRows[i].second != nullptr) {
            delete static_cast<Rnd::Object *>(mRows[i].second);
        }
        mRows[i].second = nullptr;
    }
    mRows.clear();
    mRowPositions.clear();
}

Rnd::Text *ChatPanel::CloneText(int nRow, Rnd::Text *pTemplate) {
    const char *pszPrefix = FormatString(kClonePrefixFormat, nRow);
    std::list<Rnd::Object *> clones;
    Rnd::TheManager.Clone(pTemplate, pszPrefix, &clones, kCloneFlags, 1, 1);
    Rnd::Object *pClone = clones.front();
    return pClone != nullptr ? dynamic_cast<Rnd::Text *>(pClone) : nullptr;
}

void ChatPanel::FinishLoad() {
    FreqPanel::FinishLoad();
    Rnd::Text *pFirstSender = FindText(kSenderTextFormat, mName);
    mRowPositions.push_back(Translation(pFirstSender));
    mTextStart = Translation(pFirstSender);
    mRowStart = mTextStart;
    Rnd::Text *pFirstText = FindText(kMessageTextFormat, mName);
    mRows.push_back(std::make_pair(pFirstSender, pFirstText));

    for (int i = 1; i < mNumLines; ++i) {
        const float fDrop = static_cast<float>(mSpacing * i);

        Rnd::Text *pSender = CloneText(i, pFirstSender);
        pSender->SetText(kNoText);
        mTexts.push_back(pSender);
        Vector3 position = Translation(pFirstSender);
        position.y -= fDrop;
        MoveText(pSender, position);
        mRowPositions.push_back(position);

        Rnd::Text *pText = CloneText(i, pFirstText);
        pText->SetText(kNoText);
        mTexts.push_back(pText);
        position = Translation(pFirstText);
        position.y -= fDrop;
        MoveText(pText, position);
        mRows.push_back(std::make_pair(pSender, pText));
    }
}

void ChatPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    mReserved110 = 0;
    const String name(TheGameDb->GetPlayerName(kFirstPlayer));
    const String sender(FormatString(kSenderFormat, name.c_str()));
    Rnd::Text *pSender = mRows[0].first;
    pSender->SetText(sender.c_str());
    const Vector3 end = pSender->CharPosition(sender.mLength);
    mTextStart.x = mRowStart.x + end.x;
    mTextStart.y = mRowStart.y + end.y;
    mTextStart.z = mRowStart.z + end.z;
    const float fWidth = mRows[0].first->MeasureWidth(sender.c_str(), sender.mLength);
    auto *pEntry = static_cast<UITextEntry *>(FindComponent(kEntryComponent, false));
    pEntry->mMaxEntryWidth = mRows[0].first->mWrapWidth - fWidth;
    pEntry->SetEditing(true);
    Refresh();
    if (std::strcmp(kNoText, mKeyboardText.c_str()) != 0) {
        if (!ChatMsg::Send(mChatType, mChatType == kChatLobby, mKeyboardText.c_str(), kEcho)) {
            pEntry->SetText(mKeyboardText.c_str());
        }
        mKeyboardText = kNoText;
    }
}

void ChatPanel::Exit(bool bForce, float fTime) {
    for (unsigned int i = 0; i < mRows.size(); ++i) {
        mRows[i].first->SetText(kNoText);
        mRows[i].second->SetText(kNoText);
    }
    FreqPanel::Exit(bForce, fTime);
    static_cast<UITextEntry *>(FindComponent(kEntryComponent, false))->SetEditing(false);
}

int ChatPanel::ReceiveKeyboardText(const char *pszText) {
    mKeyboardText = pszText;
    return 1;
}

bool ChatPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nChatMsgType) {
        return HandleChat(static_cast<ChatMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool ChatPanel::HandleChat(ChatMsg *pMsg) {
    ChatMessage line;
    line.mText = pMsg->mText;
    line.mSender = pMsg->mSender;
    line.mPlayer = pMsg->mPlayer;
    mMessages.push_back(line);
    if (mMessages.size() > static_cast<unsigned int>(mNumLines)) {
        mMessages.pop_front();
    }
    if (IsLoaded()) {
        Refresh();
    }
    return false;
}

void ChatPanel::Refresh() {
    mRow = 0;
    std::list<ChatMessage> shown;
    int nLines = 0;
    for (auto it = mMessages.rbegin(); it != mMessages.rend(); ++it) {
        // Each line is laid out in the first row to count the rows it takes.
        mRow = 0;
        ShowLine(it->mSender.c_str(), it->mText.c_str(), it->mPlayer);
        const int nTotal = nLines + mRows[0].second->CountLines();
        if (mNumLines < nTotal) {
            break;
        }
        nLines = nTotal;
        shown.push_front(*it);
    }

    mRow = 0;
    for (const auto &line : shown) {
        ShowLine(line.mSender.c_str(), line.mText.c_str(), line.mPlayer);
    }
    if (static_cast<unsigned int>(nLines) < mRows.size()) {
        ShowLine(kNoText, kNoText, kNoPlayer);
    }
}

bool ChatPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mButton != kPadCircle || pMsg->mPressed == 0) {
        return false;
    }
    UIPanel *pPanel = TheUI.FindPanel(kKeyboardPanel, false);
    auto *pKeyboard = pPanel != nullptr ? dynamic_cast<KeyboardPanel *>(pPanel) : nullptr;

    UIScreen *pReturnScreen;
    if (mChatType == kChatLobby) {
        pReturnScreen = TheUI.FindScreen(kLobbyScreen, false);
    } else if (std::strcmp(kHostPlayScreen, TheUI.mCurrentScreen->mName) == 0) {
        pReturnScreen = TheUI.FindScreen(kHostScreen, false);
    } else if (std::strcmp(kGuestPlayScreen, TheUI.mCurrentScreen->mName) == 0) {
        pReturnScreen = TheUI.FindScreen(kGuestScreen, false);
    } else {
        pReturnScreen = TheUI.mCurrentScreen;
    }

    const KeyboardRequest request(this,
                                  pReturnScreen,
                                  kNoText,
                                  kKeyboardPad,
                                  kKeyboardMaxChars,
                                  kKeyboardMaxWidth,
                                  1,
                                  0,
                                  0,
                                  0);
    pKeyboard->SetRequest(request);
    TheUI.GotoScreen(kKeyboardScreen);
    return false;
}

bool ChatPanel::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    mText = pMsg->mText;
    if (mText.mLength != 0 &&
        ChatMsg::Send(mChatType, mChatType == kChatLobby, mText.c_str(), kEcho)) {
        FindComponent(kEntryComponent, false)->SetText(kNoText);
    }
    return false;
}

void ChatPanel::ShowLine(const char *pszSender, const char *pszText, int nPlayer) {
    String sender(kNoText);
    if (std::strcmp(pszSender, kNoText) != 0) {
        sender = FormatString(kSenderFormat, pszSender);
    }

    if (TheNetLaunchpad != nullptr && mChatType == kChatLaunchpad) {
        int nColor = kNoColor;
        for (const auto &player : *TheNetLaunchpad->GetPlayers()) {
            if (player.mId == nPlayer) {
                nColor = player.mDifficulty;
            }
        }
        mRows[mRow].first->SetFont(FindLucidaFont(GetPlayerColorName(nColor)));
    }

    Rnd::Text *pSender = mRows[mRow].first;
    pSender->SetText(sender.c_str());
    const Vector3 end = pSender->CharPosition(sender.mLength);
    const Vector3 &rowPosition = mRowPositions[mRow];
    Vector3 position;
    position.x = rowPosition.x + end.x;
    position.y = rowPosition.y + end.y;
    position.z = rowPosition.z + end.z;
    MoveText(mRows[mRow].second, position);

    // The binary measures the name and reads the wrap width in the first row for every row.
    const float fWidth = mRows[0].first->MeasureWidth(sender.c_str(), sender.mLength);
    mRows[mRow].second->SetWrapWidth(mRows[0].first->mWrapWidth - fWidth);
    mRows[mRow].second->SetText(pszText);

    if (mRow < mNumLines) {
        if (mRows[mRow].second->CountLines() >= 2) {
            // The binary clears the next row without checking that it exists.
            mRows[mRow + 1].first->SetText(kNoText);
            mRows[mRow + 1].second->SetText(kNoText);
        }
        mRow += mRows[mRow].second->CountLines();
    }
}
