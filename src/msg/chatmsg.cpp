#include "msg/chatmsg.h"

#include <list>
#include <string.h>
#include <vector>

#include <librtmedius.h>

#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"
#include "netflow/netlaunchpadplayer.h"
#include "os/string.h"
#include "os/system.h"

namespace {

// The identifier of a sender outside a session.
constexpr int kNoPlayer = -2;

// The first letter of the channel letters.
constexpr char kChannelBase = 'a';

// Kinds of chat line, the first character of the identifier of a request.
constexpr char kLineFinal = 'f';
constexpr char kLinePart = 'p';
constexpr char kLinePowerOff = 'd';

// The channel whose senders the local player can mute.
constexpr int kMutableChannel = 1;

// The characters of one part of a line.
constexpr int kPartLength = 32;

// The characters of the sender's name a request carries.
constexpr int kRequestNameLength = 19;

// The flood level above which a line is refused, the level a line adds, and the drain per second.
constexpr float kFloodLimit = 4.0f;
constexpr float kFloodPerLine = 1.0f;
constexpr float kFloodDrainPerSecond = -0.33f;

// Seconds per millisecond.
constexpr float kSecondsPerMs = 0.001f;

// The sink of one channel.
struct ChatHandler {
    int mChannel;   // The channel.
    MsgSink *mSink; // The sink.
};

// A line from the lobby server whose remaining parts have not arrived.
struct PartialMsg {
    int mAccountId; // The sender's account.
    int mChannel;   // The channel.
    String mText;   // The parts so far.
};

} // namespace

// NTSC-U/C: 0x003b0aac
// How many lines were sent recently, draining with time.
static float sFloodLevel = 0.0f;

// NTSC-U/C: 0x003b0ab0
// The seconds of the system clock at the last line sent.
static float sLastLineSeconds = 0.0f;

// The two vectors below are constructed and destroyed by one routine at 0x002532d0. The image's
// static construction and destruction entries at 0x002533f8 and 0x00253418 call it.
// NTSC-U/C: 0x002532d0, PAL: 0x0025bc78
// NTSC-U/C: 0x002533f8, PAL: 0x0025bda0
// NTSC-U/C: 0x00253418, PAL: 0x0025bdc0

// NTSC-U/C: 0x0043ca48
// The sink of each channel.
static std::vector<ChatHandler> sHandlers;

// NTSC-U/C: 0x0043ca58
// The lines waiting for their remaining parts.
static std::vector<PartialMsg> sPartials;

MsgSink *ChatMsg::FindHandler(int nChannel) {
    for (const auto &handler : sHandlers) {
        if (handler.mChannel == nChannel) {
            return handler.mSink;
        }
    }
    return nullptr;
}

void ChatMsg::AddHandler(int nChannel, MsgSink *pHandler) {
    (void)FindHandler(nChannel); // Yes, the binary discards the result.
    sHandlers.push_back(ChatHandler{nChannel, pHandler});
}

void ChatMsg::RemoveSink(int nChannel) {
    for (auto it = sHandlers.begin(); it != sHandlers.end(); ++it) {
        if (it->mChannel == nChannel) {
            sHandlers.erase(it);
            return;
        }
    }
}

bool ChatMsg::AllowLine() {
    const float fSeconds = SystemMs() * kSecondsPerMs;
    float fLevel = sFloodLevel + (fSeconds - sLastLineSeconds) * kFloodDrainPerSecond;
    if (fLevel < 0.0f) {
        fLevel = 0.0f;
    }
    if (kFloodLimit < fLevel) {
        return false;
    }
    sLastLineSeconds = fSeconds;
    sFloodLevel = fLevel + kFloodPerLine;
    return true;
}

void ChatMsg::SendPrivate(int nChannel, int nAccountId, const char *pszText) {
    rtMediusChatRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    request.header.messageId[1] = static_cast<char>(nChannel + kChannelBase);
    request.targetAccountId = nAccountId;
    request.toAccount = 1;
    request.header.messageId[0] = kLineFinal;
    (void)strlen(pszText); // Yes, the binary discards the length.
    strcpy(request.text, pszText);
    rtMediusSendChatMessage(&request);
}

bool ChatMsg::Send(int nChannel, int nRecipient, const char *pszText, int nEcho) {
    if (!AllowLine()) {
        return false;
    }
    const char *pszName = TheLobbyRT->GetName();
    if (nEcho) {
        MsgSink *pSink = FindHandler(nChannel);
        int nPlayer = kNoPlayer;
        if (TheLaunchpadRT != nullptr) {
            nPlayer = TheLaunchpadRT->mLocalId;
        }
        ChatMsg msg(TheLobbyRT->mLocalPlayer.mId, nPlayer, pszText, TheLobbyRT->GetName());
        pSink->Dispatch(&msg); // Yes, the binary does not test for a missing sink.
    }
    rtMediusChatRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    // NTSC-U/C: 0x0043ca30
    // The function-local vector is destroyed at exit by the routine at 0x00252618.
    // NTSC-U/C: 0x00252618, PAL: 0x0025afc0
    static std::vector<int> recipients;
    recipients.clear();
    if (nRecipient != 0) {
        request.toAccount = 0;
        recipients.push_back(0);
    } else if (TheLaunchpadRT != nullptr) {
        request.toAccount = 1;
        const std::list<NetLaunchpadPlayer> players = *TheLaunchpadRT->GetPlayers();
        for (const auto &player : players) {
            if (player.mPlayer.mId != TheLobbyRT->mLocalPlayer.mId) {
                recipients.push_back(player.mPlayer.mId);
            }
        }
    }
    if (recipients.size() != 0) {
        const String text(pszText);
        for (int nOffset = 0; nOffset < text.mLength; nOffset += kPartLength) {
            request.header.messageId[0] =
                nOffset + kPartLength < text.mLength ? kLinePart : kLineFinal;
            request.header.messageId[1] = static_cast<char>(nChannel + kChannelBase);
            strncpy(&request.header.messageId[2], pszName, kRequestNameLength);
            strcpy(request.text, text.Substring(nOffset, kPartLength).c_str());
            for (const int nAccount : recipients) {
                request.targetAccountId = nAccount;
                rtMediusSendChatMessage(&request);
            }
        }
    }
    return true;
}

void ChatMsg::ClearPartialMessages() {
    sPartials.clear();
}

void ChatMsg::OnLobbyChat(int, int, int, const rtMediusChatMessage *pMessage) {
    if (pMessage->messageType == kLinePowerOff) {
        TheNetInet->RequestPowerOff();
    }
    const int nChannel = pMessage->channel - kChannelBase;
    const bool bPart = pMessage->messageType == kLinePart;
    const int nAccountId = pMessage->senderAccountId;
    auto partial = sPartials.begin();
    while (partial != sPartials.end() && partial->mAccountId != nAccountId) {
        ++partial;
    }
    if (nChannel == kMutableChannel && TheLobbyRT->IsMuted(nAccountId)) {
        if (partial != sPartials.end()) {
            sPartials.erase(partial);
        }
        return;
    }
    if (bPart) {
        if (partial == sPartials.end()) {
            sPartials.push_back(PartialMsg{nAccountId, nChannel, String(pMessage->text)});
        } else {
            partial->mText += pMessage->text;
        }
        return;
    }
    MsgSink *pSink = FindHandler(nChannel);
    int nPlayer = kNoPlayer;
    if (TheLaunchpadRT != nullptr) {
        nPlayer = TheLaunchpadRT->FindIdByAccount(nAccountId);
    }
    if (partial == sPartials.end()) {
        if (pSink != nullptr) {
            ChatMsg msg(nAccountId, nPlayer, pMessage->text, pMessage->senderName);
            pSink->Dispatch(&msg);
        }
        return;
    }
    if (pSink != nullptr) {
        partial->mText += pMessage->text;
        ChatMsg msg(nAccountId, nPlayer, partial->mText.c_str(), pMessage->senderName);
        pSink->Dispatch(&msg);
    }
    sPartials.erase(partial);
}
