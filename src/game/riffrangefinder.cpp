#include "game/riffrangefinder.h"

#include "msg/notemsg.h"

bool RiffRangeFinder::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() != static_cast<int>(NoteMsg::sID)) {
        return false;
    }
    const unsigned int nNote = static_cast<NoteMsg *>(pMsg)->mNote;
    if (nNote < mLow) {
        mLow = nNote;
    }
    if (mHigh < nNote) {
        mHigh = nNote;
    }
    return false;
}
