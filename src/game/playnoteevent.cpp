#include "game/playnoteevent.h"

PlayNoteEvent::PlayNoteEvent(
    unsigned char nPlayer, unsigned char nButton, int nState, float fX, float fY)
    : mPlayer(nPlayer), mButton(nButton), mState(nState), mX(fX), mY(fY) {
}

PlayNoteEvent::PlayNoteEvent(BinStream &stream) {
    Load(stream);
}

void PlayNoteEvent::Save(BinStream &stream) const {
    const unsigned char nPlayer = mPlayer;
    stream.Write(&nPlayer, sizeof(nPlayer));
    const unsigned char nButton = mButton;
    stream.Write(&nButton, sizeof(nButton));
    const unsigned char nState = static_cast<unsigned char>(mState);
    stream.Write(&nState, sizeof(nState));
    const float fX = mX;
    stream.WriteEndian(&fX, sizeof(fX));
    const float fY = mY;
    stream.WriteEndian(&fY, sizeof(fY));
}

void PlayNoteEvent::Load(BinStream &stream) {
    stream.Read(&mPlayer, sizeof(mPlayer));
    stream.Read(&mButton, sizeof(mButton));
    unsigned char nState;
    stream.Read(&nState, sizeof(nState));
    mState = nState != 0;
    stream.ReadEndian(&mX, sizeof(mX));
    stream.ReadEndian(&mY, sizeof(mY));
}
