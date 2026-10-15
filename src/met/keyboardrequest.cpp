#include "met/keyboardrequest.h"

namespace {

// The limits of a request built with no arguments.
constexpr int kDefaultMaxChars = 256;
constexpr int kDefaultMaxWidth = 512;

} // namespace

#ifdef VIDEO_STANDARD_PAL
KeyboardRequest::KeyboardRequest() : mUser(nullptr), mReturnScreen(nullptr), mText("") {
    mMaxWidth = kDefaultMaxWidth;
    mReserved20 = kDefaultMaxChars;
    mFunctionKeys = 1;
    mMaxChars = kDefaultMaxChars;
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
                                 [[maybe_unused]] int nMaxCharsCopy,
                                 int nMaxWidth,
                                 int nFunctionKeys,
                                 int nInvalidChars,
                                 int nPassword,
                                 int nNumLines)
    : mUser(pUser), mReturnScreen(pReturnScreen), mText(pszText), mMaxChars(nMaxChars),
      mMaxWidth(nMaxWidth), mFunctionKeys(nFunctionKeys), mPassword(nPassword),
      mInvalidChars(nInvalidChars), mPad(nPad), mNumLines(nNumLines) {
} // Yes, the binary does not store nMaxCharsCopy, and mReserved20 stays unset.
#else
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
#endif
