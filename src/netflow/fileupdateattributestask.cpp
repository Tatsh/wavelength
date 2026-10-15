#include "netflow/fileupdateattributestask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

namespace {

// The access of every account to the file once the attributes are updated.
constexpr short kNoAccess = 0;

} // namespace

void FileUpdateAttributesTask::OnStart() {
    rtMediusFileUpdateAttributesRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    request.file.ownerPermissions = kNoAccess;
    request.file.groupPermissions = kNoAccess;
    request.file.globalPermissions = kNoAccess;
    const int nError = rtMediusFileUpdateAttributes(&request, OnResponse);
    LobbyRT::ReportMediusError(2599, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileUpdateAttributesTask::OnResponse(int nConnection,
                                          int nType,
                                          int nSize,
                                          const rtMediusFileResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileUpdateAttributesTask *pTask =
        pFound != nullptr ? dynamic_cast<FileUpdateAttributesTask *>(pFound) : nullptr;
    if (pTask != nullptr) {
        pTask->Finish(true, 0);
    }
}
