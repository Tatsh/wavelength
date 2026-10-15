#pragma once

#include "met/keyboarduser.h"
#include "os/string.h"
#include "ui/uiscreen.h"

/**
 * What a screen requests of the front-end keyboard: who receives the text, the text to start from,
 * and the limits of the entry.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is 0x38 bytes.
 * A screen builds one on its stack and hands it to KeyboardPanel::SetRequest() before it opens
 * `kb_screen`.
 */
class KeyboardRequest {
public:
    /**
     * Construct a request with no user and the default limits.
     *
     * @ghidraAddress NTSC-U/C: 0x001a8830
     * @ghidraAddress PAL: 0x001b0520
     */
    KeyboardRequest();

    /**
     * Construct a request.
     *
     * @param pUser The user that receives the text.
     * @param pReturnScreen The screen the keyboard returns to.
     * @param pszText The text the entry starts with.
     * @param nPad The controller that types.
     * @param nMaxChars The most characters the entry accepts.
     * @param nMaxWidth The widest the text may be.
     * @param nFunctionKeys Non-zero when the function keys type their texts.
     * @param nInvalidChars Non-zero when the `invalid_chars` of the metagame configuration apply.
     * @param nPassword Non-zero when the entry hides the text.
     * @param nNumLines The number of lines of a scrolling entry, or 0 for one line.
     * @ghidraAddress NTSC-U/C: 0x001a8898
     */
#ifdef VIDEO_STANDARD_PAL
    KeyboardRequest(KeyboardUser *pUser,
                    UIScreen *pReturnScreen,
                    const char *pszText,
                    int nPad,
                    int nMaxChars,
                    int nMaxWidth,
                    int nFunctionKeys,
                    int nInvalidChars,
                    int nPassword,
                    int nNumLines)
        : KeyboardRequest(pUser,
                          pReturnScreen,
                          pszText,
                          nPad,
                          nMaxChars,
                          nMaxChars,
                          nMaxWidth,
                          nFunctionKeys,
                          nInvalidChars,
                          nPassword,
                          nNumLines) {
    }

    /**
     * Construct a request, as the PAL build does.
     *
     * Every caller passes the most characters twice, and the constructor does not store the
     * second copy.
     *
     * @param pUser The user that receives the text.
     * @param pReturnScreen The screen the keyboard returns to.
     * @param pszText The text the entry starts with.
     * @param nPad The controller that types.
     * @param nMaxChars The most characters the entry accepts.
     * @param nMaxCharsCopy The most characters again.
     * @param nMaxWidth The widest the text may be.
     * @param nFunctionKeys Non-zero when the function keys type their texts.
     * @param nInvalidChars Non-zero when the `invalid_chars` of the metagame configuration apply.
     * @param nPassword Non-zero when the entry hides the text.
     * @param nNumLines The number of lines of a scrolling entry, or 0 for one line.
     * @ghidraAddress PAL: 0x001b0588
     */
    KeyboardRequest(KeyboardUser *pUser,
                    UIScreen *pReturnScreen,
                    const char *pszText,
                    int nPad,
                    int nMaxChars,
                    int nMaxCharsCopy,
                    int nMaxWidth,
                    int nFunctionKeys,
                    int nInvalidChars,
                    int nPassword,
                    int nNumLines);
#else
    KeyboardRequest(KeyboardUser *pUser,
                    UIScreen *pReturnScreen,
                    const char *pszText,
                    int nPad,
                    int nMaxChars,
                    int nMaxWidth,
                    int nFunctionKeys,
                    int nInvalidChars,
                    int nPassword,
                    int nNumLines);
#endif

    KeyboardUser *mUser;     /*!< The user that receives the text, or null. */
    UIScreen *mReturnScreen; /*!< The screen the keyboard returns to, or null. */
    String mText;            /*!< The text the entry starts with. */
    int mMaxChars;           /*!< The most characters the entry accepts. +0x1c */
#ifdef VIDEO_STANDARD_PAL
    int mReserved20; // +0x20 Set to 256 by the default constructor, otherwise unset and never read.
#endif
    int mMaxWidth;     /*!< The widest the text may be. */
    int mFunctionKeys; /*!< Non-zero when the function keys type their texts. */
    int mPassword;     /*!< Non-zero when the entry hides the text. */
    int mInvalidChars; /*!< Non-zero when the `invalid_chars` apply. */
    int mPad;          /*!< The controller that types. */
    int mNumLines;     /*!< The lines of a scrolling entry, or 0 for one line. +0x34 */
};
