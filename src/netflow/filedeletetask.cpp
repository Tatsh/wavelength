#include "netflow/filedeletetask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

void FileDeleteTask::OnStart() {
    if (*mEnabled == 0) {
        Finish(true, 0);
        return;
    }
    rtMediusFileRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    const int nError = rtMediusFileDelete(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(2638, nError);
#else
    LobbyRT::ReportMediusError(2364, nError);
#endif
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileDeleteTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusFileDeleteResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileDeleteTask *pTask = pFound != nullptr ? dynamic_cast<FileDeleteTask *>(pFound) : nullptr;
    if (pTask != nullptr) {
        pTask->Finish(true, 0);
    }
}
