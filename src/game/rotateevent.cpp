#include "game/rotateevent.h"

RotateEvent::RotateEvent(BinStream &stream) {
    Load(stream);
}

void RotateEvent::Save(BinStream &stream) const {
    const unsigned char nPlayer = mPlayer;
    stream.Write(&nPlayer, sizeof(nPlayer));
    stream.Write(&mDirection, sizeof(mDirection));
}

void RotateEvent::Load(BinStream &stream) {
    stream.Read(&mPlayer, sizeof(mPlayer));
    stream.Read(&mDirection, sizeof(mDirection));
}
