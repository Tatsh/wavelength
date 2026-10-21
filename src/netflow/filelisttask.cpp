#include "netflow/filelisttask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"

namespace {

// The largest size of a file the request lists, exclusive, which is no limit.
constexpr int kNoSizeLimit = -1;

// The files of the page the request lists.
#ifdef VIDEO_STANDARD_PAL
constexpr int kPageSize = 50;
#else
constexpr int kPageSize = 1000;
#endif

} // namespace

#ifdef VIDEO_STANDARD_PAL
void FileListTask::OnStart() {
    mFiles->clear();
    mPage = 0;
    mReceived = 0;
    mPageDone = 0;
    RequestPage();
}

void FileListTask::RequestPage() {
    mReceived = 0;
    mPageDone = 0;
    rtMediusFileListRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.fileNameBeginsWith, mPrefix->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    request.fileSizeLessThan = kNoSizeLimit;
    request.startingEntry = mPage + 1;
    request.pageSize = kPageSize;
    request.fileSizeGreaterThan = 0;
    const int nError = rtMediusFileList(&request, OnResponse);
    LobbyRT::ReportMediusError(2522, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileListTask::OnPoll() {
    if (mPageDone == 0) {
        return;
    }
    mPageDone = 0;
    if (mReceived != kPageSize) {
        Finish(true, 0);
        return;
    }
    ++mPage;
    RequestPage();
}
#else
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
#endif

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
#ifdef VIDEO_STANDARD_PAL
        ++pTask->mReceived;
#endif
        if (pTask->mPrefix->Compare(0, pTask->mPrefix->GetLength(), pResponse->file.fileName) ==
            0) {
            pTask->mFiles->push_back(String(pResponse->file.fileName));
        }
        if (pResponse->status == RT_MEDIUS_STATUS_SUCCESS && pResponse->endOfList == 0) {
            return;
        }
    }
#ifdef VIDEO_STANDARD_PAL
    pTask->mPageDone = 1;
#else
    pTask->Finish(true, 0);
#endif
}
