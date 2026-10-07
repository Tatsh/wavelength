#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "ui/uitextentry.h"

/**
 * Identity that UITextEntryInvalidMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afd04
 */
extern int g_nUITextEntryInvalidMsgType;

/**
 * Message a text entry sends when it refuses a typed character.
 *
 * The RTTI records the class as deriving from Message. The object is 0x10 bytes. A text entry sends
 * it through its own Dispatch() for a character of its `invalid_chars` list, and for a character
 * that does not fit.
 */
class UITextEntryInvalidMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. UITextEntry::ProcessKey() expands it on its stack.
     *
     * @param ch The refused character.
     * @param bEndOfField Whether the character was refused because the text is full.
     * @param pEntry The text entry.
     */
    UITextEntryInvalidMsg(char ch, bool bEndOfField, UITextEntry *pEntry)
        : mChar(ch), mEndOfField(bEndOfField), mEntry(pEntry) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UITextEntryInvalidMsg", 0);
    }

    /**
     * Release a message.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0037c588
     * @ghidraAddress PAL: 0x003eacb8
     */
    Message *Clone() override {
        return new UITextEntryInvalidMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUITextEntryInvalidMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037c5e8
     * @ghidraAddress PAL: 0x003ead18
     */
    int Type() override {
        return g_nUITextEntryInvalidMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UITextEntryInvalidMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037c5f8
     * @ghidraAddress PAL: 0x003ead28
     */
    const char *GetName() const override {
        return "UITextEntryInvalidMsg";
    }

    /**
     * Write the character and whether the text was full.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e9c8
     * @ghidraAddress PAL: 0x002177e0
     */
    void PrintExtra(PrnStream &stream) const override;

    char mChar;          /*!< The refused character. */
    bool mEndOfField;    /*!< Whether the character was refused because the text is full. */
    UITextEntry *mEntry; /*!< The text entry. */
};
