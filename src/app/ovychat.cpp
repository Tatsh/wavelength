#include "app/ovychat.h"

#include <iterator>

#include "app/overlay.h"
#include "game/gamedb.h"
#include "gfx/gfxconfig.h"
#include "os/keyboard.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/manager.h"

namespace {

// The time of a chat that has not polled, and of a panel with no activity.
constexpr float kNever = -1.0e9f;

// The most characters a player types in a line.
constexpr int kMaxLineLength = 37;

// The frame of the panel slide when hidden and when shown.
constexpr float kHiddenPosition = 0.0f;
constexpr float kShownPosition = 240.0f;

// The speed of the panel slide before SetSpeed(), and after it.
constexpr float kInitialSlideSpeed = 0.1f;
constexpr float kSlideSpeed = 1.0f;

// The channel of the chat of a game.
constexpr int kGameChatChannel = 3;

// The recipient of a line for every console, and the echo flag of a line the player sends.
constexpr int kEveryConsole = 0;
constexpr int kNoEcho = 0;

// The sender of a line that has no player.
constexpr int kNoSender = -2;

// The number of the first line text.
constexpr int kFirstLine = 1;

// The rows a line starts at when Layout() leaves the bottom row to the typed line.
constexpr int kFirstRow = 1;
constexpr int kFirstRowWhileEditing = 2;

// The player slots and the colours of their lines.
constexpr int kSlotPurple = 1;
constexpr int kSlotRed = 2;
constexpr int kSlotYellow = 3;
const Color kWhiteLine{1.0f, 1.0f, 1.0f, 1.0f};
const Color kGreenLine{0.45f, 1.0f, 0.2f, 1.0f};
const Color kPurpleLine{1.0f, 0.5f, 1.0f, 1.0f};
const Color kRedLine{1.0f, 0.5f, 0.0f, 1.0f};
const Color kYellowLine{1.0f, 0.75f, 0.0f, 1.0f};

// The colour SetColor() gives a line with no player.
constexpr int kNoPlayer = -1;

// The translation row of a transform, and the columns of a row.
constexpr int kRowTranslation = 3;
enum XfmColumn {
    kColumnX = 0,
    kColumnY = 1,
    kColumnZ = 2,
    kColumnW = 3,
};

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

} // namespace

float OvyChat::Line::sSlideTime = kNever;

#pragma mark - Line

OvyChat::Line::Line(DataArray *pConfig, DataArray *pDefaults, Rnd::Text *pText) {
    mText = pText;
    DataArray *pSlide;
    FindConfigArray(pConfig, pDefaults, "chat_line_slide", &pSlide, true);
    mSlide = Interpolator::Create(pSlide->Array(1));
    if (sSlideTime == kNever) {
        sSlideTime = mSlide->mX1;
    }
}

OvyChat::Line::~Line() {
    sSlideTime = kNever;
    delete mSlide;
}

void OvyChat::Line::SetPosition(bool bJump, float fTime, float fRow) {
    if (bJump) {
        mSlide->Reset(fRow, fRow, fTime - 1.0f, fTime);
        return;
    }
    mSlide->Reset(mSlide->Eval(fTime), fRow, fTime, fTime + sSlideTime);
}

void OvyChat::Line::SetColor(int nPlayer) {
    Color color = kWhiteLine;
    if (nPlayer >= 0) {
        switch (TheGameDb->GetPlayerSlot(nPlayer)) {
        case kSlotPurple:
            color = kPurpleLine;
            break;
        case kSlotRed:
            color = kRedLine;
            break;
        case kSlotYellow:
            color = kYellowLine;
            break;
        default:
            color = kGreenLine;
            break;
        }
    }
    mText->SetColor(color);
}

bool OvyChat::Line::Update(float fTime) {
    const float fRow = mSlide->Eval(fTime);
    Rnd::Transformable *pTrans = mText;
    float (&translation)[Rnd::kXfmRowFloatCount] = pTrans->mLocalXfm[kRowTranslation];
    if (fRow == translation[kColumnZ]) {
        return false;
    }
    translation[kColumnZ] = fRow;
    pTrans->mDirty = 1;
    return true;
}

#pragma mark - OvyChat

OvyChat::OvyChat(DataArray *pConfig, DataArray *pDefaults, Rnd::View *pHudView)
    : mSlide(kHiddenPosition, nullptr, nullptr, kInitialSlideSpeed), mLastActivity(kNever),
      mEditLine(nullptr), mEdit(kMaxLineLength) {
    mLastTime = kNever;
    mHiddenPosition = kHiddenPosition;
    mShownPosition = kShownPosition;
    const char *pszChat = "HUDnr_chat";
    mDrawable = FindObject<Rnd::Drawable>(FormatString("%s_mask.mesh", pszChat));
    Rnd::Transformable *pPanel =
        FindObject<Rnd::Transformable>(FormatString("%s_panel.mesh", pszChat));
    FindObject<Rnd::Transformable>(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix))
        ->AddTrans(pPanel);
    mTypeBox = FindObject<Rnd::Drawable>("HUDnr_chat_04.mesh");
    mRule = FindObject<Rnd::Drawable>("HUDnr_chat_hr.mesh");
    mCursor = FindObject<Rnd::Mesh>(FormatString("%s_cursor.mesh", pszChat));
    mSlide.SetAnim(FindObject<Rnd::Animatable>(FormatString("%s.tnm", pszChat)));
    mSlide.SetSpeed(kSlideSpeed);
    Rnd::Animatable *pCursorAnim = FindObject<Rnd::Animatable>("HUD cursor.mnm");
    if (pCursorAnim != nullptr) {
        static_cast<Rnd::Animatable *>(pHudView)->AddAnim(pCursorAnim);
    }
    for (int i = kFirstLine;; ++i) {
        Rnd::Text *pText = FindObject<Rnd::Text>(FormatString("%s_%02d.txt", pszChat, i));
        if (pText == nullptr) {
            break;
        }
        mLines.push_back(new Line(pConfig, pDefaults, pText));
        static_cast<Rnd::Transformable *>(pText)->RemoveTrans(mCursor);
    }
    mRows.resize(mLines.size() + 1, 0.0f);
    std::vector<float>::iterator pRow = mRows.begin();
    for (Line *pLine : mLines) {
        *pRow++ =
            static_cast<Rnd::Transformable *>(pLine->mText)->mLocalXfm[kRowTranslation][kColumnZ];
    }
    const int nLast = static_cast<int>(mRows.size()) - 1;
    mRows[nLast] = mRows[nLast - 1] + mRows[nLast - 1] - mRows[nLast - 2];
    int nPlayer = 0;
    while (nPlayer < TheGameDb->GetNumPlayers() && !TheGameDb->IsLocalPlayer(nPlayer)) {
        ++nPlayer;
    }
    mLocalPlayer = nPlayer;
    (void)TheGameDb->GetNumPlayers(); // Yes, the binary discards this call's result.
    FindConfigFloat(pConfig, pDefaults, "chat_timeout", &mTimeout, true);
    Reset();
    KeyboardAddSink(this);
    ChatMsg::AddHandler(kGameChatChannel, this);
}

OvyChat::~OvyChat() {
    ChatMsg::RemoveSink(kGameChatChannel);
    KeyboardRemoveSink(this);
    for (Line *pLine : mLines) {
        delete pLine;
    }
}

void OvyChat::Reset() {
    mSlide.Jump(kHiddenPosition, kHiddenPosition);
    mPending.clear();
    std::vector<float>::iterator pRow = mRows.begin();
    for (Line *pLine : mLines) {
        pLine->mText->SetText("");
        pLine->SetPosition(true, mLastTime, *pRow++);
    }
    mSlide.Jump(mHiddenPosition, mHiddenPosition);
    mDrawable->SetShowing(false);
    mTypeBox->SetShowing(false);
    mRule->SetShowing(false);
    mCursor->SetShowing(false);
    mLastActivity = kNever;
    mEdit.Clear();
    if (mEditLine != nullptr) {
        static_cast<Rnd::Transformable *>(mEditLine->mText)->RemoveTrans(mCursor);
    }
    mEditLine = nullptr;
}

void OvyChat::Poll() {
    const float fNow = SystemMs();
    if (mLastTime == kNever) {
        mLastTime = fNow;
    }
    const float fDelta = fNow - mLastTime;
    mLastTime = fNow;
    const bool bSliding = mSlide.Update(fDelta, false);
    mDrawable->SetShowing(mSlide.mValue != mHiddenPosition);
    if (mDrawable->mShowing == 0) {
        return;
    }
    bool bMoved = false;
    for (Line *pLine : mLines) {
        if (pLine->Update(fNow)) {
            bMoved = true;
        }
    }
    if (bMoved) {
        mLastActivity = fNow;
    }
    if (!mPending.empty()) {
        mLines.front()->mText->SetText(mPending.front().mText.c_str());
        mLines.front()->SetColor(mPending.front().mPlayer);
        mPending.pop_front();
        Layout(mEditLine != nullptr);
    }
    if (!bSliding && mTimeout < fNow - mLastActivity) {
        mSlide.SetTarget(mHiddenPosition);
    }
}

bool OvyChat::AddLine(ChatMsg *pMsg) {
    const int nSender = pMsg->mPlayer;
    if (nSender == kNoSender) {
        return false;
    }
    int nPlayer = TheGameDb->GetNumPlayers() - 1;
    while (nPlayer >= 0 && TheGameDb->GetPlayerNetOrder(nPlayer) != nSender) {
        --nPlayer;
    }
    if (nPlayer < 0) {
        return false;
    }
    mPending.push_back(PendingMsg());
    mPending.back().mText = pMsg->mText;
    mPending.back().mPlayer = nPlayer;
    mLastActivity = mLastTime;
    mSlide.SetTarget(mShownPosition);
    return true;
}

bool OvyChat::HandleKey(KeyboardKeyMsg *pMsg) {
    mLastActivity = mLastTime;
    mSlide.SetTarget(mShownPosition);
    bool bEditing = mEditLine != nullptr;
    const int nResult = mEdit.HandleKey(pMsg);
    int nAction = nResult;
    if (nResult == EditField::kKeySubmit) {
        const char *pszText = mEdit.GetText();
        while (*pszText == ' ') {
            ++pszText;
        }
        if (*pszText == '\0') {
            nAction = EditField::kKeyIgnored;
            bEditing = false;
        } else if (ChatMsg::Send(kGameChatChannel, kEveryConsole, mEdit.GetText(), kNoEcho)) {
            bEditing = false;
        }
    } else if (nResult == EditField::kKeyCancel) {
        mEdit.Clear();
        if (mEditLine == nullptr) {
            mLastActivity = kNever;
        }
        bEditing = false;
    } else if (nResult == EditField::kKeyChanged) {
        bEditing = true;
    }
    if (bEditing != (mEditLine != nullptr)) {
        mTypeBox->SetShowing(bEditing);
        mRule->SetShowing(bEditing);
        mCursor->SetShowing(bEditing);
        if (bEditing) {
            mEditLine = mLines.front();
            static_cast<Rnd::Transformable *>(mEditLine->mText)->AddTrans(mCursor);
            mLines.front()->mText->SetText("");
            mLines.front()->SetColor(kNoPlayer);
            Layout(false);
        } else {
            mEdit.Clear();
            if (nAction == EditField::kKeySubmit) {
                mEditLine->SetColor(mLocalPlayer);
            } else {
                Collapse();
            }
            static_cast<Rnd::Transformable *>(mEditLine->mText)->RemoveTrans(mCursor);
            mEditLine = nullptr;
        }
    }
    if (mEditLine != nullptr) {
        if (nAction == EditField::kKeyChanged) {
            mEditLine->mText->SetText(mEdit.GetText());
        }
        const Vector3 position = mEditLine->mText->CharPosition(mEdit.mCursor);
        Rnd::Transformable *pCursor = mCursor;
        float (&translation)[Rnd::kXfmRowFloatCount] = pCursor->mLocalXfm[kRowTranslation];
        translation[kColumnX] = position.x;
        translation[kColumnY] = position.y;
        translation[kColumnZ] = position.z;
        translation[kColumnW] = position.w;
        pCursor->mDirty = 1;
    }
    return false;
}

void OvyChat::Layout(bool bEditing) {
    std::vector<float>::iterator pRow =
        mRows.begin() + (bEditing ? kFirstRowWhileEditing : kFirstRow);
    bool bSwap = bEditing;
    bool bSlide = bEditing;
    std::list<Line *>::iterator it = mLines.begin();
    while (it != mLines.end()) {
        std::list<Line *>::iterator next = std::next(it);
        if (bSwap) {
            bSwap = false;
            if (next != mLines.end()) {
                next = std::next(next);
                mLines.splice(next, mLines, it);
            }
        }
        if (it == mLines.begin()) {
            (*it)->SetPosition(true, mLastTime, mRows[0]);
        }
        (*it)->SetPosition(bSlide, mLastTime, *pRow);
        if (next == mLines.end()) {
            mLines.splice(mLines.begin(), mLines, it);
        }
        ++pRow;
        bSlide = false;
        it = next;
    }
}

void OvyChat::Collapse() {
    if (mLines.empty()) {
        return;
    }
    mLines.front()->mText->SetText("");
    mLines.splice(mLines.end(), mLines, mLines.begin());
    std::vector<float>::iterator pRow = mRows.begin();
    std::list<Line *>::iterator it = mLines.begin();
    for (std::list<Line *>::iterator next = std::next(it); next != mLines.end(); ++next) {
        (*it)->SetPosition(false, mLastTime, *pRow++);
        it = next;
    }
    (*it)->SetPosition(true, mLastTime, *pRow);
}

bool OvyChat::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nChatMsgType) {
        return AddLine(static_cast<ChatMsg *>(pMsg));
    }
    if (nType == g_nKeyboardKeyMsgType) {
        return HandleKey(static_cast<KeyboardKeyMsg *>(pMsg));
    }
    return false;
}
