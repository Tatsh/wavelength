#include "met/keyboardrequest.h"

namespace {

// The limits of a request built with no arguments.
constexpr int kDefaultMaxChars = 256;
constexpr int kDefaultMaxWidth = 512;

} // namespace

KeyboardRequest::KeyboardRequest() : mUser(nullptr), mReturnScreen(nullptr), mText("") {
    mMaxWidth = kDefaultMaxWidth;
    mMaxChars = kDefaultMaxChars;
    mFunctionKeys = 1;
    mPassword = 0;
    mInvalidChars = 0;
    mPad = 0;
    mNumLines = 0;
}

KeyboardRequest::KeyboardRequest(KeyboardUser *pUser,
                                 UIScreen *pReturnScreen,
                                 const char *pszText,
                                 int nPad,
                                 int nMaxChars,
                                 int nMaxWidth,
                                 int nFunctionKeys,
                                 int nInvalidChars,
                                 int nPassword,
                                 int nNumLines)
    : mUser(pUser), mReturnScreen(pReturnScreen), mText(pszText), mMaxChars(nMaxChars),
      mMaxWidth(nMaxWidth), mFunctionKeys(nFunctionKeys), mPassword(nPassword),
      mInvalidChars(nInvalidChars), mPad(nPad), mNumLines(nNumLines) {
}
