#pragma once

#include "game/triggeraction.h"
#include "rnd/text.h"
#include "script/dataarray.h"

/**
 * The `change_lyric` action, which shows the lyric of the last lyric event in a text object.
 *
 * The RTTI records the class as deriving from TriggerAction. The object is 0x10 bytes. Node 1 names
 * the text object.
 */
class ChangeLyricAction : public TriggerAction {
public:
    /**
     * Read the action.
     *
     * @param pAction The node.
     * @ghidraAddress NTSC-U/C: 0x00202250
     * @ghidraAddress PAL: 0x0020aff0
     */
    explicit ChangeLyricAction(DataArray *pAction);

    /**
     * Give the text object TriggerMgr::mLyric.
     *
     * @ghidraAddress NTSC-U/C: 0x00202e98
     */
    void Exec() override;

private:
    Rnd::Text *mText;
};
