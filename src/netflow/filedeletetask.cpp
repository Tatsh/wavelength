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
    LobbyRT::ReportMediusError(__LINE__, nError);
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
