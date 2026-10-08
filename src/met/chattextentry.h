#pragma once

#include "msg/keyboardkeymsg.h"
#include "script/dataarray.h"
#include "ui/uitextentry.h"

/**
 * Chat entry field whose function keys type the texts `fkey_f1` to `fkey_f12` of the locale.
 *
 * The RTTI records the class as deriving from UITextEntry, and the vtable is at `0x003cffd0`. The
 * class adds no members. The destructor at `0x003613a0` (PAL `0x003cf8b8`) is compiler-generated.
 */
class ChatTextEntry : public UITextEntry {
public:
    /**
     * Construct the entry from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the entry belongs to.
     * @ghidraAddress NTSC-U/C: 0x001a28c0
     * @ghidraAddress PAL: 0x001aa5a0
     */
    ChatTextEntry(DataArray *pData, const char *pszPanel);

    /**
     * Create an entry from its script description.
     *
     * Metagame::RegisterScreenClasses() registers the routine for the entry type
     * `chat_entry_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the entry belongs to.
     * @return The new entry.
     * @ghidraAddress NTSC-U/C: 0x00361438
     * @ghidraAddress PAL: 0x003cf950
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new ChatTextEntry(pData, pszPanel);
    }

    /**
     * Handle a keyboard key, or pass any other message to UITextEntry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a2a10
     * @ghidraAddress PAL: 0x001aa6f0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Replace the text with the text of a function key, then pass the key to UITextEntry.
     *
     * @param pMsg The key.
     * @return The result of UITextEntry::HandleKeyMsg().
     * @ghidraAddress NTSC-U/C: 0x001a28f8
     * @ghidraAddress PAL: 0x001aa5d8
     */
    bool HandleKeyboardKey(KeyboardKeyMsg *pMsg);
};
