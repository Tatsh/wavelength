#include "msg/chatmsg.h"

#include <cstring>
#include <list>
#include <vector>

#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"
#include "netflow/netinet.h"
#include "netflow/netlaunchpadplayer.h"
#include "os/string.h"
#include "os/system.h"

// The sink of one channel of chat.
struct ChatHandler {
    int mChannel;   // The channel.
    MsgSink *mSink; // The sink.
};

// The text received so far of a line that arrives in parts.
struct PartialMsg {
    int mAccountId; // The sender.
    int mChannel;   // The channel.
    String mText;   // The parts received.
};

namespace {

// The rate of the sent lines AllowSend() lets through. The level of the bucket drains by
// kDrainPerSecond each second, rises by kLineCost with each line, and blocks a line above
// kMaxLevel.
constexpr float kMillisecondsToSeconds = 0.001f;
constexpr float kDrainPerSecond = -0.33f;
constexpr float kLineCost = 1.0f;
constexpr float kMaxLevel = 4.0f;

// The characters of each part of a line.
constexpr int kPartLength = 32;

// The kinds of line, the first character of the identifier of a request.
constexpr char kKindFinal = 'f';
constexpr char kKindPartial = 'p';
constexpr char kKindPowerOff = 'd';

// The first channel, the second character of the identifier of a request.
constexpr char kChannelBase = 'a';

// The characters of the identifier of a request before the name of the sender, and the size of
// the name there.
constexpr int kSenderNameOffset = 2;
constexpr int kSenderNameSize = 19;

// The message types of a request.
constexpr int kMessageTypeChannel = 0;
constexpr int kMessageTypeAccount = 1;

// The recipient of a line to the chat room.
constexpr int kNoTarget = 0;

} // namespace

// NTSC-U/C: 0x0043ca48
// The sink of each channel.
static std::vector<ChatHandler> gHandlers;

// NTSC-U/C: 0x0043ca58
// The lines that arrived in part.
static std::vector<PartialMsg> gPartials;

MsgSink *ChatMsg::FindSink(int nChannel) {
    for (const ChatHandler &handler : gHandlers) {
        if (handler.mChannel == nChannel) {
            return handler.mSink;
        }
    }
    return nullptr;
}

void ChatMsg::AddHandler(int nChannel, MsgSink *pHandler) {
    (void)FindSink(nChannel); // Yes, the binary discards the sink already registered.
    gHandlers.push_back(ChatHandler{nChannel, pHandler});
}

void ChatMsg::RemoveSink(int nChannel) {
    for (auto it = gHandlers.begin(); it != gHandlers.end(); ++it) {
        if (it->mChannel == nChannel) {
            gHandlers.erase(it);
            return;
        }
    }
}

bool ChatMsg::AllowSend() {
    // NTSC-U/C: 0x003b0aac
    static float sLevel;
    // NTSC-U/C: 0x003b0ab0
    static float sLastSeconds;
    const float fSeconds = SystemMs() * kMillisecondsToSeconds;
    float fLevel = sLevel + (fSeconds - sLastSeconds) * kDrainPerSecond;
    if (fLevel < 0.0f) {
        fLevel = 0.0f;
    }
    if (kMaxLevel < fLevel) {
        return false;
    }
    sLastSeconds = fSeconds;
    sLevel = fLevel + kLineCost;
    return true;
}

void ChatMsg::SendPrivate(int nChannel, int nAccountId, const char *pszText) {
    rtMediusChatRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.sessionKey, TheLobbyRT->mSessionKey.c_str());
    request.messageId[1] = static_cast<char>(kChannelBase + nChannel);
    request.targetId = nAccountId;
    request.messageType = kMessageTypeAccount;
    (void)strlen(pszText); // Yes, the binary discards the length.
    request.messageId[0] = kKindFinal;
    strcpy(request.text, pszText);
    (void)rtMediusSendChat(&request);
}

bool ChatMsg::Send(int nChannel, int nRecipient, const char *pszText, int nEcho) {
    if (!AllowSend()) {
        return false;
    }
    const char *pszName = TheLobbyRT->GetName();
    if (nEcho != 0) {
        MsgSink *pSink = FindSink(nChannel);
        int nPlayer = LaunchpadRT::kNoId;
        if (TheLaunchpadRT != nullptr) {
            nPlayer = TheLaunchpadRT->mLocalId;
        }
        ChatMsg msg(TheLobbyRT->mLocalPlayer.mId, nPlayer, pszText, TheLobbyRT->GetName());
        pSink->Dispatch(&msg); // Yes, the binary does not test for a missing sink.
    }

    rtMediusChatRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.sessionKey, TheLobbyRT->mSessionKey.c_str());
    // NTSC-U/C: 0x0043ca30
    static std::vector<int> sRecipients;
    sRecipients.clear();
    if (nRecipient != 0) {
        request.messageType = kMessageTypeChannel;
        sRecipients.push_back(kNoTarget);
    } else {
        request.messageType = kMessageTypeAccount;
        if (TheLaunchpadRT != nullptr) {
            const std::list<NetLaunchpadPlayer> players = *TheLaunchpadRT->GetPlayers();
            for (const NetLaunchpadPlayer &player : players) {
                if (player.mPlayer.mId != TheLobbyRT->mLocalPlayer.mId) {
                    sRecipients.push_back(player.mPlayer.mId);
                }
            }
        }
    }

    if (sRecipients.size() != 0) {
        const String text(pszText);
        const int nLength = text.mLength;
        for (int nPos = 0; nPos < nLength; nPos += kPartLength) {
            request.messageId[0] = nPos + kPartLength < nLength ? kKindPartial : kKindFinal;
            request.messageId[1] = static_cast<char>(kChannelBase + nChannel);
            // The name fills the rest of the identifier, as retail does.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
            strncpy(&request.messageId[kSenderNameOffset], pszName, kSenderNameSize);
#pragma GCC diagnostic pop
            strcpy(request.text, text.Substring(nPos, kPartLength).c_str());
            for (const int *pTarget = sRecipients.data();
                 pTarget < sRecipients.data() + sRecipients.size();
                 ++pTarget) {
                request.targetId = *pTarget;
                (void)rtMediusSendChat(&request);
            }
        }
    }
    return true;
}

void ChatMsg::ClearPartialLines() {
    gPartials.clear();
}

void ChatMsg::OnLobbyChat(int nConnection,
                          int nType,
                          int nSize,
                          const rtMediusChatMessage *pMessage) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    if (pMessage->messageType == kKindPowerOff) {
        TheNetInet->PowerOff();
    }
    const int nChannel = pMessage->channel - kChannelBase;
    const bool bPartial = pMessage->messageType == kKindPartial;
    const int nSender = pMessage->senderAccountId;
    const char *pszSender = pMessage->senderName;
    auto it = gPartials.begin();
    while (it != gPartials.end() && it->mAccountId != nSender) {
        ++it;
    }

    if (nChannel == kChannelLobby && TheLobbyRT->IsMuted(nSender)) {
        if (it != gPartials.end()) {
            gPartials.erase(it);
        }
        return;
    }

    if (bPartial) {
        if (it == gPartials.end()) {
            gPartials.push_back(PartialMsg{nSender, nChannel, String(pMessage->text)});
        } else {
            it->mText += pMessage->text;
        }
        return;
    }

    MsgSink *pSink = FindSink(nChannel);
    int nPlayer = LaunchpadRT::kNoId;
    if (TheLaunchpadRT != nullptr) {
        nPlayer = TheLaunchpadRT->FindIdByAccount(nSender);
    }
    if (it == gPartials.end()) {
        if (pSink != nullptr) {
            ChatMsg msg(nSender, nPlayer, pMessage->text, pszSender);
            pSink->Dispatch(&msg);
        }
        return;
    }
    if (pSink != nullptr) {
        it->mText += pMessage->text;
        ChatMsg msg(nSender, nPlayer, it->mText.c_str(), pszSender);
        pSink->Dispatch(&msg);
    }
    gPartials.erase(it);
}
