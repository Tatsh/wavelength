#include "msg/finalscorepacket.h"

void FinalScorePacket::saveGuts(BinStream &stream) const {
    int nScore = mScore;
    stream.WriteEndian(&nScore, sizeof(nScore));
}

void FinalScorePacket::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mScore, sizeof(mScore));
}
