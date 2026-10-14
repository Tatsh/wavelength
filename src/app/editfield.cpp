#include "app/editfield.h"

#include "os/keyboard.h"

namespace {

// The keys of characters HandleKey() acts on besides the printable ones.
constexpr int kKeyBackspace = '\b';
constexpr int kKeyReturn = '\n';

// The printable characters, from the space to the tilde.
constexpr int kFirstPrintable = ' ';
constexpr unsigned kPrintableCount = 95;

// The text ends with a space the cursor may stand before and the terminator.
constexpr int kTailLength = 2;

} // namespace

// The static initialiser of the unit, and the routine that runs it.
// NTSC-U/C: 0x001e4bb8, PAL: 0x001ed958
// NTSC-U/C: 0x001e4bf0, PAL: 0x001ed990

EditField::EditField(int nMaxLength) : mCapacity(nMaxLength + kTailLength), mCursor(0) {
    mText.assign(nMaxLength, '\0');
    Clear();
}

EditField::~EditField() = default;

int EditField::HandleKey(KeyboardKeyMsg *pMsg) {
    const int nKey = pMsg->mKey;
    if (static_cast<unsigned>(nKey - kFirstPrintable) < kPrintableCount) {
        if (mText.size() >= static_cast<unsigned>(mCapacity)) {
            return kKeyIgnored;
        }
        const auto at = mText.begin() + mCursor;
        if (!mInsert && (at < mText.end() - kTailLength)) {
            *at = static_cast<char>(nKey);
        } else {
            mText.insert(at, static_cast<char>(nKey));
        }
        ++mCursor;
        return kKeyChanged;
    }

    switch (nKey) {
    case kKeyHome:
    case kKeyUp:
        mCursor = 0;
        return kKeyIgnored;
    case kKeyEnd:
    case kKeyDown:
        mCursor = static_cast<int>(mText.size()) - kTailLength;
        return kKeyIgnored;
    case kKeyLeft:
        if (mCursor > 0) {
            --mCursor;
        }
        return kKeyIgnored;
    case kKeyRight:
        if (static_cast<unsigned>(mCursor) < mText.size() - kTailLength) {
            ++mCursor;
        }
        return kKeyIgnored;
    case kKeyInsert:
        mInsert ^= 1;
        return kKeyIgnored;
    case kKeyDelete: {
        const auto at = mText.begin() + mCursor;
        if (at >= mText.end() - kTailLength) {
            return kKeyIgnored;
        }
        mText.erase(at);
        return kKeyChanged;
    }
    case kKeyBackspace:
        if (mCursor <= 0) {
            return kKeyIgnored;
        }
        mText.erase(mText.begin() + mCursor - 1);
        --mCursor;
        return kKeyChanged;
    case kKeyReturn:
        return kKeySubmit;
    case kKeyEscape:
        return kKeyCancel;
    default:
        return kKeyIgnored;
    }
}

const char *EditField::GetText() {
    return &mText[0];
}

void EditField::Clear() {
    mText.resize(kTailLength, '\0');
    mText[0] = ' ';
    mText[1] = '\0';
    mCursor = 0;
}
