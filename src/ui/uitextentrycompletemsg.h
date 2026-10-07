#pragma once

#include <cstddef>

#include "msg/message.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "os/string.h"
#include "ui/uitextentry.h"

/**
 * Identity that UITextEntryCompleteMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003afd00
 */
extern int g_nUITextEntryCompleteMsgType;

/**
 * Message a text entry sends when the player confirms the text.
 *
 * The RTTI records the class as deriving from Message. The object is 0x1c bytes. A text entry sends
 * it through its own Dispatch() when the return key arrives.
 */
class UITextEntryCompleteMsg : public Message {
public:
    /**
     * Construct the message.
     *
     * Inline. UITextEntry::ProcessKey() expands it on its stack.
     *
     * @param pszText The text.
     * @param pEntry The text entry.
     */
    UITextEntryCompleteMsg(const char *pszText, UITextEntry *pEntry)
        : mText(pszText), mEntry(pEntry) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "UITextEntryCompleteMsg", 0);
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
     * @ghidraAddress NTSC-U/C: 0x0037c450
     * @ghidraAddress PAL: 0x003eab80
     */
    Message *Clone() override {
        return new UITextEntryCompleteMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nUITextEntryCompleteMsgType.
     * @ghidraAddress NTSC-U/C: 0x0037c4b8
     * @ghidraAddress PAL: 0x003eabe8
     */
    int Type() override {
        return g_nUITextEntryCompleteMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `UITextEntryCompleteMsg`.
     * @ghidraAddress NTSC-U/C: 0x0037c4c8
     * @ghidraAddress PAL: 0x003eabf8
     */
    const char *GetName() const override {
        return "UITextEntryCompleteMsg";
    }

    /**
     * Write the text.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0020e980
     * @ghidraAddress PAL: 0x00217798
     */
    void PrintExtra(PrnStream &stream) const override;

    String mText;        /*!< The text. */
    UITextEntry *mEntry; /*!< The text entry. */
};
