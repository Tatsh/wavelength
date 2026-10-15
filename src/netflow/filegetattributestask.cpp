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
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(2216, nError);
#else
    LobbyRT::ReportMediusError(2016, nError);
#endif
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
#ifdef VIDEO_STANDARD_PAL
        *pTask->mPermissions = static_cast<unsigned short>(pResponse->file.ownerPermissions);
#endif
    }
    pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS, RepoRemixStatusMsg::kStatusFailed);
}
