#include "netflow/getannouncementstask.h"

#include <cstring>

#include "netflow/lobbyrt.h"

namespace {

// The tag of a line break in the news, and its length.
const char *const kBreakTag = "<br>";
constexpr int kBreakTagLength = 4;

} // namespace

void GetAnnouncementsTask::OnStart() {
    TheLobbyRT->mAnnouncement = "";
    rtMediusIdRequest request;
    memset(&request, 0, sizeof(request));
    strcpy(request.header.sessionKey, TheLobbyRT->mSessionKey.c_str());
    MakeTag(this, request.header.messageId);
    request.id = LobbyRT::kApplicationId;
    const int nError = rtMediusGetAnnouncements(&request, OnResponse);
    LobbyRT::ReportMediusError(1048, nError);
    if (nError != 0) {
        Finish(true, 0);
    }
}

void GetAnnouncementsTask::OnResponse(int nConnection,
                                      int nType,
                                      int nSize,
                                      rtMediusAnnouncementResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pTask = FindTag(pResponse->header.messageId);
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->header.status == RT_MEDIUS_STATUS_SUCCESS) {
        ReplaceBreaks(pResponse->announcement);
        TheLobbyRT->mAnnouncement = pResponse->announcement;
    }
    pTask->Finish(true, 0);
}

void GetAnnouncementsTask::ReplaceBreaks(char *pszText) {
    char *pszOut = pszText;
    const char *pszIn = pszText;
    while (*pszIn != '\0') {
        if (strncmp(pszIn, kBreakTag, kBreakTagLength) == 0) {
            *pszOut = '\n';
            pszIn += kBreakTagLength - 1;
        } else {
            *pszOut = *pszIn;
        }
        ++pszIn;
        ++pszOut;
    }
    *pszOut = '\0';
}
