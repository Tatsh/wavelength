#include "netflow/filecreatetask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

namespace {

// The access to read, write, and delete the file.
constexpr short kPermissionAll = 7;

} // namespace

void FileCreateTask::OnStart() {
    rtMediusFileCreateRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    request.file.globalPermissions = kPermissionAll;
    request.file.fileSize = *mSize;
    request.file.ownerPermissions = kPermissionAll;
    request.file.groupPermissions = kPermissionAll;
    const int nError = rtMediusFileCreate(&request, OnResponse);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(2352, nError);
#else
    LobbyRT::ReportMediusError(2151, nError);
#endif
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileCreateTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusFileResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileCreateTask *pTask = pFound != nullptr ? dynamic_cast<FileCreateTask *>(pFound) : nullptr;
    if (pTask != nullptr) {
        pTask->Finish(pResponse->status == RT_MEDIUS_STATUS_SUCCESS,
                      RepoRemixStatusMsg::kStatusFailed);
    }
}
