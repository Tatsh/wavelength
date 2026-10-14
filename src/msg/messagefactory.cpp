#include "msg/messagefactory.h"

#include <algorithm>

#include "msg/messageio.h"
#include "os/debug.h"

std::vector<MessageFactory::Entry> *MessageFactory::List() {
    // The destruction registered with atexit() on first use is at
    // NTSC-U/C: 0x0029cc08, PAL: 0x002a68d0
    static std::vector<Entry> sList;
    return &sList;
}

MessageFactory::MessageFactory(int nType, MessageFactoryProc pfnCreate) {
    Entry entry;
    entry.mType = nType;
    const auto it = std::lower_bound(List()->begin(), List()->end(), entry);
    entry.mCreate = pfnCreate;
    List()->insert(it, entry);
}

Message *Message::NewMessage(int nType) {
    const auto it = std::lower_bound(MessageFactory::List()->begin(),
                                     MessageFactory::List()->end(),
                                     MessageFactory::Entry{nType, nullptr});
    if (it == MessageFactory::List()->end() || it->mType != nType) {
        return nullptr;
    }
    return it->mCreate();
}

BinStream &operator<<(BinStream &stream, Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    const short nType = static_cast<short>(pMsg->Type());
    stream.WriteEndian(&nType, sizeof(nType));
    pMsg->saveGuts(stream);
    return stream;
}

BinStream &operator>>(BinStream &stream, Message *&pMsg) {
    short nType;
    stream.ReadEndian(&nType, sizeof(nType));
    pMsg = Message::NewMessage(nType);
    if (pMsg == nullptr) {
        DebugPrint("Cannot find ID %ld in Message Factory List", nType);
        return stream;
    }
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    pMsg->restoreGuts(stream);
    return stream;
}
