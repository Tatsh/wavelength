#include "game/notefinder.h"

#include "msg/message.h"

bool NoteFinder::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == static_cast<int>(NoteMsg::sID)) {
        OnNote(static_cast<NoteMsg *>(pMsg));
    }
    return false;
}
