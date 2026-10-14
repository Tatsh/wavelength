#include "netflow/filegetattributestask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

void FileGetAttributesTask::OnStart() {
    rtMediusFileRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    const int nError = rtMediusFileGetAttributes(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileGetAttributesTask::OnResponse(int nConnection,
                                       int nType,
                                       int nSize,
                                       const rtMediusFileAttributesResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileGetAttributesTask *pTask =
        pFound != nullptr ? dynamic_cast<FileGetAttributesTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS) {
        *pTask->mSize = pResponse->file.fileSize;
    }
    pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS, RepoRemixStatusMsg::kStatusFailed);
}
