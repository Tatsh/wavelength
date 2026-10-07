#include "msg/message.h"

Message::~Message() {
}

void Message::PrintExtra(PrnStream &) const {
}

void Message::saveGuts(BinStream &) const {
}

void Message::restoreGuts(BinStream &) {
}
