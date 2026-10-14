#include "ui/uitextentry.h"

#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "ui/uimanager.h"
#include "ui/uitextentrycompletemsg.h"
#include "ui/uitextentryinvalidmsg.h"

namespace {

// Keys ProcessKey() acts on.
constexpr int kKeyBackspace = 8;
constexpr int kKeyReturn = 10;
constexpr int kKeyDelete = 311;
constexpr int kKeyLeft = 320;
constexpr int kKeyRight = 321;

// The printable characters run from the space for this many codes.
constexpr int kFirstPrintable = ' ';
constexpr int kPrintableCount = 95;

// The font's glyph that stands in for a typed double quote.
constexpr int kQuoteGlyph = -80;

// The value of mMaxEntryWidth and mMaxNumChars that sets no limit.
constexpr float kNoWidthLimit = -1.0f;
constexpr int kNoCharLimit = -1;

constexpr char kMaskChar = '*';

RndText *FindText(const char *pszName) {
    return dynamic_cast<RndText *>(TheManager.Find(pszName));
}

// Set the three components of a caret position to an origin moved by an offset. The fourth
// component is left as it was.
void PlaceCaret(Vector3 &caret, const Vector3 &origin, const Vector3 &offset) {
    caret.x = origin.x + offset.x;
    caret.y = origin.y + offset.y;
    caret.z = origin.z + offset.z;
}

} // namespace

float UITextEntry::sBlinkMs = 450.0f;

UITextEntry::UITextEntry(DataArray *pData, const char *pszPanel)
    : UIComponent(pData), mMesh(nullptr), mHighlightStyle(nullptr), mScroll(0), mWordWrapLines(0),
      mPassword(0), mEditing(0), mNextBlink(0.0f), mInvalidChars(nullptr) {
    mCursor = 0;
    mMaxEntryWidth = kNoWidthLimit;
    mNumLines = 1;
    mMaxNumChars = kNoCharLimit;
    pData->FindBool("scroll", &mScroll, false);
    pData->FindInt("num_lines", &mNumLines, false);
    pData->FindInt("word_wrap_lines", &mWordWrapLines, false);
    pData->FindFloat("max_entry_width", &mMaxEntryWidth, false);
    pData->FindInt("max_num_chars", &mMaxNumChars, false);
    pData->FindBool("password", &mPassword, false);
    mInvalidChars = pData->FindArray("invalid_chars", false);

    mMesh = dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s_%s.mesh", pszPanel, mName)));
    if (mMesh != nullptr) {
        const char *pszStyle = "";
        pData->FindSymbol("hilight_style", &pszStyle, true);
        mHighlightStyle = TheUI.FindStyle(pszStyle, false);
    }

    mTextObj = FindText(FormatString("%s_%s.txt", pszPanel, mName));
    mTextObj->SetText("");
    mCaret = FindText(FormatString("%s_%s_cursor.txt", pszPanel, mName));
    mCaret->SetShowing(false);
    mCaretOrigin = mCaret->mLocalXfm.mTranslation;
    mLastCaret = FindText(FormatString("%s_%s_lastcursor.txt", pszPanel, mName));
    mLastCaret->SetText("");
    mLastCaretOrigin = mLastCaret->mLocalXfm.mTranslation;
    if (mWordWrapLines != 0) {
        mMaxEntryWidth = mTextObj->mWrapWidth;
    }
    if (mMaxEntryWidth == kNoWidthLimit && !mScroll) {
        mMaxEntryWidth = mLastCaretOrigin.x - mCaretOrigin.x;
    }
    mLastCaret->SetShowing(false);
    mVisibleText = "";
    mScrollStart = 0;
    mCaretPos = mCaretOrigin;
}

UITextEntry::~UITextEntry() {
}

String UITextEntry::MaskText(const char *pszText) const {
    String masked(pszText);
    for (int i = 0; i < masked.mLength; ++i) {
        masked.Replace(i, 1, String("*"));
    }
    return masked;
}

float UITextEntry::CharWidth(int nIndex) {
    RndFont *pFont = mTextObj->mFont;
    float fWidth = pFont->CharWidth(kMaskChar);
    const float fSpace = pFont->mSpace;
    if (!mPassword) {
        fWidth = pFont->CharWidth(mText[nIndex]);
    }
    return fSpace + fWidth;
}

void UITextEntry::SetText(const char *pszText) {
    mText = pszText;
    mCursor = mText.mLength;
    mScrollStart = 0;
    if (mPassword) {
        String masked = MaskText(mText.c_str());
        mTextObj->SetText(masked.c_str());
    } else {
        mTextObj->SetText(mText.c_str());
    }
    PlaceCaret(mCaretPos, mCaretOrigin, mTextObj->CharPosition(mCursor));
    Layout();
}

const char *UITextEntry::Text() const {
    return mText.c_str();
}

void UITextEntry::SetState(int nState, bool bForce) {
    if (!bForce && GetState() == nState) {
        return;
    }
    mState = nState;
    if (mHighlightStyle == nullptr) {
        return;
    }
    if (mMesh != nullptr) {
        mMesh->SetMat(mHighlightStyle->GetMat(nState));
    }
    if (mTextObj != nullptr) {
        mTextObj->SetFont(mHighlightStyle->GetFont(nState));
        UpdateCaret();
    }
}

void UITextEntry::SetEditing(bool bEditing) {
    mEditing = bEditing;
    if (bEditing) {
        mNextBlink = TheUI.mTime;
    } else {
        mCaret->SetShowing(false);
        mNextBlink = 0.0f;
    }
}

bool UITextEntry::HandleKey(int nKey) {
    return ProcessKey(nKey);
}

void UITextEntry::ToggleCaret() {
    if (mCaret->mShowing != 0) {
        mCaret->SetShowing(false);
    } else {
        mCaret->SetShowing(true);
    }
}

void UITextEntry::Poll(float fTime) {
    if (mEditing == 0) {
        mCaret->SetShowing(false);
        return;
    }
    if (mNextBlink < fTime) {
        ToggleCaret();
        mNextBlink = fTime + sBlinkMs;
    }
}

void UITextEntry::Layout() {
    if (mPassword) {
        String masked = MaskText(mText.c_str());
        mTextObj->SetText(masked.c_str());
    } else {
        mTextObj->SetText(mText.c_str());
    }

    if (mWordWrapLines != 0 || !mScroll) {
        mVisibleText = mText;
        PlaceCaret(mCaretPos, mCaretOrigin, mTextObj->CharPosition(mCursor));
        UpdateCaret();
        return;
    }

    RndFont *pFont = mTextObj->mFont;
    const float fSpace = pFont->mSpace;
    const float fMaskWidth = pFont->CharWidth(kMaskChar);

    if (0.0f < mCaretPos.x - mLastCaretOrigin.x &&
        static_cast<unsigned int>(mScrollStart) < static_cast<unsigned int>(mText.mLength)) {
        do {
            mCaretPos.x -= CharWidth(mScrollStart);
            ++mScrollStart;
        } while (0.0f < mCaretPos.x - mLastCaretOrigin.x &&
                 static_cast<unsigned int>(mScrollStart) <
                     static_cast<unsigned int>(mText.mLength));
    }
    if (0.0f < mCaretOrigin.x - mCaretPos.x && mScrollStart > 0) {
        do {
            // Yes, the binary measures the character at mScrollStart before stepping back to it.
            mCaretPos.x += CharWidth(mScrollStart);
            --mScrollStart;
        } while (0.0f < mCaretOrigin.x - mCaretPos.x && mScrollStart > 0);
    }

    mVisibleText = mText;
    mVisibleText.Erase(0, mScrollStart);
    float fWidth = 0.0f;
    int nCount = 0;
    if (0.0f < mLastCaretOrigin.x - mCaretOrigin.x && mVisibleText.mLength != 0) {
        do {
            fWidth += fSpace;
            if (mPassword) {
                fWidth += fMaskWidth;
            } else {
                fWidth += pFont->CharWidth(mVisibleText[nCount]);
            }
            ++nCount;
        } while (fWidth < mLastCaretOrigin.x - mCaretOrigin.x &&
                 static_cast<unsigned int>(nCount) <
                     static_cast<unsigned int>(mVisibleText.mLength));
    }
    mVisibleText.Truncate(nCount);
    mTextObj->SetText(mVisibleText.c_str()); // Yes, a scrolled password shows unmasked.
    UpdateCaret();
}

void UITextEntry::UpdateCaret() {
    mCaret->mDirty = 1;
    mCaret->mLocalXfm.mTranslation = mCaretPos;
}

bool UITextEntry::ProcessKey(int nKey) {
    if (nKey == kKeyReturn) {
        UITextEntryCompleteMsg msg(mText.c_str(), this);
        Dispatch(&msg);
        return true;
    }
    if (nKey == kKeyBackspace) {
        if (mCursor > 0) {
            const float fWidth = CharWidth(mCursor - 1);
            mText.Erase(mCursor - 1, 1);
            --mCursor;
            mCaretPos.x -= fWidth;
            Layout();
        }
        return true;
    }
    if (nKey == kKeyLeft) {
        if (mCursor > 0) {
            const float fWidth = CharWidth(mCursor - 1);
            --mCursor;
            mCaretPos.x -= fWidth;
            Layout();
        }
        return true;
    }
    if (nKey == kKeyRight) {
        if (static_cast<unsigned int>(mCursor) < static_cast<unsigned int>(mText.mLength)) {
            const float fWidth = CharWidth(mCursor);
            ++mCursor;
            mCaretPos.x += fWidth;
            Layout();
        }
        return true;
    }
    if (nKey == kKeyDelete) {
        if (static_cast<unsigned int>(mCursor) < static_cast<unsigned int>(mText.mLength)) {
            mText.Erase(mCursor, 1);
            Layout();
        }
        return true;
    }
    if (static_cast<unsigned int>(nKey - kFirstPrintable) >= kPrintableCount &&
        nKey != kQuoteGlyph) {
        return true;
    }
    if (nKey == '"') {
        nKey = kQuoteGlyph;
    }
    const char ch = static_cast<char>(nKey);

    if (mInvalidChars != nullptr) {
        for (int i = 1; i < mInvalidChars->Size(); ++i) {
            if (mInvalidChars->Sym(i)[0] == ch) {
                UITextEntryInvalidMsg msg(ch, false, this);
                Dispatch(&msg);
                return true;
            }
        }
    }

    bool bFits;
    if (mScroll || mWordWrapLines != 0) {
        mVisibleText = mTextObj->mPreWrapText;
        mTextObj->SetWordWrap(1);
        mTextObj->SetWrapWidth(mMaxEntryWidth);
        String text(mText);
        text.Insert(mCursor, 1, ch);
        mTextObj->SetText(text.c_str());
        const int nLines = mTextObj->NumLines();
        if (mScroll) {
            mTextObj->SetWordWrap(0);
            bFits = !(mNumLines < nLines);
        } else {
            bFits = !(mWordWrapLines < nLines); // Yes, the binary leaves the wrapping on here.
        }
        if (mPassword) {
            String masked = MaskText(mVisibleText.c_str());
            mTextObj->SetText(masked.c_str());
        } else {
            mTextObj->SetText(mVisibleText.c_str());
        }
    } else {
        if (mMaxEntryWidth == kNoWidthLimit) {
            bFits = true;
        } else {
            const Vector3 end = mTextObj->CharPosition(mText.mLength);
            char glyph = ch;
            const float fWidth = mTextObj->MeasureWidth(&glyph, 1);
            bFits = static_cast<float>(static_cast<int>(end.x + fWidth)) < mMaxEntryWidth;
        }
        if (bFits && mMaxNumChars != kNoCharLimit) {
            bFits =
                static_cast<unsigned int>(mText.mLength) < static_cast<unsigned int>(mMaxNumChars);
        }
    }

    if (!bFits) {
        UITextEntryInvalidMsg msg(ch, true, this);
        Dispatch(&msg);
        return true;
    }
    mText.Insert(mCursor++, 1, ch);
    mCaretPos.x += CharWidth(mCursor - 1);
    Layout();
    return true;
}

bool UITextEntry::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nKeyboardKeyMsgType) {
        return HandleKeyMsg(static_cast<KeyboardKeyMsg *>(pMsg));
    }
    return UIComponent::DispatchPriv(pMsg);
}

bool UITextEntry::HandleKeyMsg(KeyboardKeyMsg *pMsg) {
    return ProcessKey(pMsg->mKey);
}

void UITextEntry::Print(PrnStream &stream) {
    stream << "{UITextEntry " << mName << "}\n";
}
