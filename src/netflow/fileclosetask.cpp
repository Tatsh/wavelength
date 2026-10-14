#include "netflow/fileclosetask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

void FileCloseTask::OnStart() {
    rtMediusFileRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    const int nError = rtMediusFileClose(&request, OnResponse);
    LobbyRT::ReportMediusError(2271, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileCloseTask::OnResponse(int nConnection,
                               int nType,
                               int nSize,
                               const rtMediusFileResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileCloseTask *pTask = pFound != nullptr ? dynamic_cast<FileCloseTask *>(pFound) : nullptr;
    if (pTask != nullptr) {
        pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                      RepoRemixStatusMsg::kStatusFailed);
    }
}
