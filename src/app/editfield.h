#pragma once

#include <vector>

#include "msg/keyboardkeymsg.h"

/**
 * Line of text the keyboard edits, with a cursor and an overwrite mode.
 *
 * The class is not polymorphic and emits no RTTI, and its title is inferred. The object is 0x1c
 * bytes. Only the members the chat calls are declared.
 */
class EditField {
public:
    /** The values of HandleKey(). */
    enum KeyResult {
        kKeyCancel = -2, /*!< The key cancels the edit. */
        kKeySubmit = -1, /*!< The key submits the text. */
        kKeyIgnored = 0, /*!< The key does not change the text. */
        kKeyChanged = 1, /*!< The key changes the text. */
    };

    /**
     * Construct an empty field.
     *
     * @param nMaxLength The most characters the field takes.
     * @ghidraAddress NTSC-U/C: 0x001e4648
     * @ghidraAddress PAL: 0x001ed3e8
     */
    explicit EditField(int nMaxLength);

    /**
     * Release the text.
     *
     * @ghidraAddress NTSC-U/C: 0x001e46f8
     * @ghidraAddress PAL: 0x001ed498
     */
    ~EditField();

    /**
     * Apply a key to the text and the cursor.
     *
     * @param pMsg The key.
     * @return One of KeyResult.
     * @ghidraAddress NTSC-U/C: 0x001e4758
     * @ghidraAddress PAL: 0x001ed4f8
     */
    int HandleKey(KeyboardKeyMsg *pMsg);

    /**
     * Report the text.
     *
     * @return The text.
     * @ghidraAddress NTSC-U/C: 0x001e4b40
     * @ghidraAddress PAL: 0x001ed8e0
     */
    const char *GetText();

    /**
     * Empty the text to a single space and move the cursor to the start.
     *
     * @ghidraAddress NTSC-U/C: 0x001e4b48
     * @ghidraAddress PAL: 0x001ed8e8
     */
    void Clear();

    int mCapacity; /*!< The most characters the field takes, plus two. */
    int mCursor;   /*!< The character the cursor stands before. */
    /*!< Whether a typed character goes in before the one at the cursor rather than replacing it.
         The constructor leaves it unset. */
    int mInsert;
    std::vector<char> mText; /*!< The terminated text. */
};
