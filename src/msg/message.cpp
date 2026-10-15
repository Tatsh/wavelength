#include "msg/message.h"

Message::~Message() {
}

void Message::PrintExtra(PrnStream &) const {
}

void Message::saveGuts(BinStream &) const {
}

#ifdef VIDEO_STANDARD_PAL
void Message::Print(PrnStream &stream) const {
    stream << "{" << GetName() << " ";
    PrintExtra(stream);
    stream << "}";
}
#endif

void Message::restoreGuts(BinStream &) {
}
