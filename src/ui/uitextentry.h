#pragma once

#include "msg/keyboardkeymsg.h"
#include "os/prnstream.h"
#include "os/string.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/transformable.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uistyle.h"

/**
 * Field the player types text into from a USB keyboard or the on-screen keyboard.
 *
 * The RTTI records the class as deriving from UIComponent. The object is 0xb0 bytes, and the
 * vtable is at `0x003d21f0`. The panel's file provides the text object `<panel>_<name>.txt`, the
 * caret `<panel>_<name>_cursor.txt`, the marker of the field's right edge
 * `<panel>_<name>_lastcursor.txt`, and optionally the highlight mesh `<panel>_<name>.mesh`. The
 * description may set `scroll`, `num_lines`, `word_wrap_lines`, `max_entry_width`,
 * `max_num_chars`, `password`, `invalid_chars`, and, with a mesh, `hilight_style`.
 *
 * A character that is refused sends a UITextEntryInvalidMsg, and the return key sends a
 * UITextEntryCompleteMsg, both through the entry's own Dispatch(). While the entry edits, the caret
 * blinks.
 */
class UITextEntry : public UIComponent {
public:
    /**
     * Construct a text entry from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the entry belongs to.
     * @ghidraAddress NTSC-U/C: 0x002073d0
     * @ghidraAddress PAL: 0x00210188
     */
    UITextEntry(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the text entry.
     *
     * @ghidraAddress NTSC-U/C: 0x00207838
     * @ghidraAddress PAL: 0x00210610
     */
    ~UITextEntry() override;

    /**
     * Create a text entry from its script description.
     *
     * UIManager::Init() registers the routine for the entry type `text_entry_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the entry belongs to.
     * @return The new text entry.
     * @ghidraAddress NTSC-U/C: 0x00377708
     * @ghidraAddress PAL: 0x003e5e38
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new UITextEntry(pData, pszPanel);
    }

    /**
     * Handle a keyboard key, or pass any other message to UIComponent.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x002085d8
     * @ghidraAddress PAL: 0x002113d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the text the player has entered.
     *
     * @return The text.
     * @ghidraAddress NTSC-U/C: 0x00207ae8
     * @ghidraAddress PAL: 0x002108c0
     */
    const char *Text() const override;

    /**
     * Replace the text and put the caret after it.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x002079e0
     * @ghidraAddress PAL: 0x002107b8
     */
    void SetText(const char *pszText) override;

    /**
     * Change the state, showing the material and the font of the highlight style.
     *
     * @param nState One of UIComponent::State.
     * @param bForce Apply the state even when it does not change.
     * @ghidraAddress NTSC-U/C: 0x00207af0
     * @ghidraAddress PAL: 0x002108c8
     */
    void SetState(int nState, bool bForce) override;

    /**
     * Blink the caret while the entry edits, and hide it otherwise.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00207c80
     * @ghidraAddress PAL: 0x00210a58
     */
    void Poll(float fTime) override;

    /**
     * Write the entry's name.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00208660
     * @ghidraAddress PAL: 0x00211460
     */
    void Print(PrnStream &stream) override;

    /**
     * Report a copy of a text with every character replaced by an asterisk.
     *
     * @param pszText The text.
     * @return The masked copy.
     * @ghidraAddress NTSC-U/C: 0x002078a8
     * @ghidraAddress PAL: 0x00210680
     */
    String MaskText(const char *pszText) const;

    /**
     * Report the width one character of the text takes, including the font's tracking.
     *
     * A password entry measures an asterisk instead.
     *
     * @param nIndex The character's position in mText.
     * @return The width.
     * @ghidraAddress NTSC-U/C: 0x00207960
     * @ghidraAddress PAL: 0x00210738
     */
    float CharWidth(int nIndex);

    /**
     * Start or stop editing.
     *
     * Editing blinks the caret from the current front-end time. Stopping hides it.
     *
     * @param bEditing Whether the entry edits.
     * @ghidraAddress NTSC-U/C: 0x00207bb0
     * @ghidraAddress PAL: 0x00210988
     */
    void SetEditing(bool bEditing);

    /**
     * Act on a key as if it were typed.
     *
     * @param nKey The key.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x00207c08
     * @ghidraAddress PAL: 0x002109e0
     */
    bool HandleKey(int nKey);

    /**
     * Show the caret when it is hidden, and hide it when it shows.
     *
     * @ghidraAddress NTSC-U/C: 0x00207c28
     * @ghidraAddress PAL: 0x00210a00
     */
    void ToggleCaret();

    /**
     * Show the part of the text that fits, and place the caret.
     *
     * A scrolling entry moves its first visible character to keep the caret inside the field.
     *
     * @ghidraAddress NTSC-U/C: 0x00207d00
     * @ghidraAddress PAL: 0x00210ad8
     */
    void Layout();

    /**
     * Move the caret object to mCaretPos.
     *
     * @ghidraAddress NTSC-U/C: 0x00208038
     * @ghidraAddress PAL: 0x00210e10
     */
    void UpdateCaret();

    /**
     * Edit the text for a key.
     *
     * Return completes the entry, backspace and delete remove a character, the left and right
     * arrows move the caret, and a printable character is inserted at the caret when it is allowed
     * and fits.
     *
     * @param nKey The key.
     * @return True, for every key.
     * @ghidraAddress NTSC-U/C: 0x00208058
     * @ghidraAddress PAL: 0x00210e30
     */
    bool ProcessKey(int nKey);

    /**
     * Edit the text for a keyboard message.
     *
     * @param pMsg The message.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x00208640
     * @ghidraAddress PAL: 0x00211440
     */
    bool HandleKeyMsg(KeyboardKeyMsg *pMsg);

    String mText;             /*!< The text the player has entered. */
    Rnd::Text *mTextObj;      /*!< The text object that shows the text. */
    Rnd::Text *mCaret;        /*!< The caret. */
    Rnd::Text *mLastCaret;    /*!< The marker of the field's right edge. */
    Rnd::Mesh *mMesh;         /*!< The highlight mesh, or null. */
    UIStyle *mHighlightStyle; /*!< The style of the mesh and the text. */
    int mCursor;              /*!< The caret's position in mText. */
    int mScrollStart;         /*!< The first visible character. */
    alignas(16) float mCaretOrigin[Rnd::kXfmRowFloatCount]; /*!< The caret's starting position. */
    alignas(16) float mLastCaretOrigin[Rnd::kXfmRowFloatCount]; /*!< The right edge's position. */
    int mScroll;                                                /*!< The `scroll` setting. */
    String mVisibleText;                                 /*!< The part of the text that shows. */
    alignas(16) float mCaretPos[Rnd::kXfmRowFloatCount]; /*!< The caret's position. */
    float mMaxEntryWidth;     /*!< The `max_entry_width` setting, or -1 for no limit. */
    int mNumLines;            /*!< The `num_lines` setting. */
    int mMaxNumChars;         /*!< The `max_num_chars` setting, or -1 for no limit. */
    int mWordWrapLines;       /*!< The `word_wrap_lines` setting, or 0 for no wrapping. */
    int mPassword;            /*!< The `password` setting, which masks the text. */
    bool mEditing;            /*!< Whether the caret blinks. */
    float mNextBlink;         /*!< The time the caret next changes. */
    DataArray *mInvalidChars; /*!< The `invalid_chars` array, or null. */

private:
    /**
     * Milliseconds between changes of the blinking caret.
     *
     * @ghidraAddress NTSC-U/C: 0x003afcc8
     */
    static float sBlinkMs;
};
