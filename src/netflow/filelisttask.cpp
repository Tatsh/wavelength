#include "netflow/filelisttask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"

namespace {

// The largest size of a file the request lists, exclusive, which is no limit.
constexpr int kNoSizeLimit = -1;

// The files of the page the request lists.
constexpr int kPageSize = 1000;

} // namespace

void FileListTask::OnStart() {
    mFiles->clear();
    rtMediusFileListRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.fileNameBeginsWith, mPrefix->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    request.fileSizeLessThan = kNoSizeLimit;
    request.pageSize = kPageSize;
    request.fileSizeGreaterThan = 0;
    request.startingEntry = 0;
    const int nError = rtMediusFileList(&request, OnResponse);
    LobbyRT::ReportMediusError(2312, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileListTask::OnResponse(int nConnection,
                              int nType,
                              int nSize,
                              const rtMediusFileResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileListTask *pTask = pFound != nullptr ? dynamic_cast<FileListTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS) {
        if (pTask->mPrefix->Compare(0, pTask->mPrefix->mLength, pResponse->file.fileName) == 0) {
            pTask->mFiles->push_back(String(pResponse->file.fileName));
        }
        if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
    pTask->Finish(true, 0);
}
