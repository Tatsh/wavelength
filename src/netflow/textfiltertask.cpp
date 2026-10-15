#include "netflow/textfiltertask.h"

#include <cstring>

#include "msg/createaccountresultmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"
#include "os/utf8.h"

namespace {

// The strictness of the filter.
constexpr int kFilterLevel = 1;

// The character the server writes for a filtered word, and the character it becomes.
constexpr char kFilteredCharacter = '*';
constexpr char kReplacementCharacter = '#';

} // namespace

void TextFilterTask::OnStart() {
    rtMediusTextFilterRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.filterLevel = kFilterLevel;
#ifdef VIDEO_STANDARD_PAL
    strncpy(request.text, Utf8String(mText->c_str()).c_str(), RT_MEDIUS_TEXT_SIZE);
    const int nError = rtMediusTextFilter(&request, OnResponse);
    LobbyRT::ReportMediusError(633, nError);
#else
    strncpy(request.text, mText->c_str(), RT_MEDIUS_TEXT_SIZE);
    const int nError = rtMediusTextFilter(&request, OnResponse);
    LobbyRT::ReportMediusError(611, nError);
#endif
    if (nError != 0) {
        Finish(false, CreateAccountResultMsg::kResultVulgarName);
    }
}

void TextFilterTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusTextFilterResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    TextFilterTask *pTask = pFound != nullptr ? dynamic_cast<TextFilterTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->result >= 0) {
#ifdef VIDEO_STANDARD_PAL
        const String text = Utf8DecodedString(pResponse->text);
        *pTask->mFiltered = text.c_str();
#else
        *pTask->mFiltered = pResponse->text;
#endif
        if (pTask->mReplaceStars != 0) {
            for (char *pszText = pTask->mFiltered->mBuffer; *pszText != '\0'; ++pszText) {
                if (*pszText == kFilteredCharacter) {
                    *pszText = kReplacementCharacter;
                }
            }
        }
    }
    pTask->Finish(true, 0);
}
