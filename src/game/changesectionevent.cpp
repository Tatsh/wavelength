#include "game/changesectionevent.h"

ChangeSectionEvent::ChangeSectionEvent(BinStream &stream) {
    Load(stream);
}

void ChangeSectionEvent::Save(BinStream &stream) const {
    const unsigned char nPlayer = mPlayer;
    stream.Write(&nPlayer, sizeof(nPlayer));
    stream.Write(&mDirection, sizeof(mDirection));
}

void ChangeSectionEvent::Load(BinStream &stream) {
    stream.Read(&mPlayer, sizeof(mPlayer));
    stream.Read(&mDirection, sizeof(mDirection));
}
