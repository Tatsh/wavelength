#include "netflow/filedownloadtask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

namespace {

// The progress of a finished transfer.
constexpr float kProgressDone = 1.0f;

// The estimated progress the step waits for after the last part.
constexpr float kProgressFinish = 0.99f;

} // namespace

void FileDownloadTask::OnStart() {
    mReceived = 0;
    mDone = 0;
    mPolls = 0.0f;
    mRate = 0.0f;
    mProgress = 0.0f;
    rtMediusFileRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    const int nError = rtMediusFileDownload(&request, OnResponse);
    LobbyRT::ReportMediusError(__LINE__, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileDownloadTask::OnPoll() {
    mPolls += 1.0f;
    const float fEstimate = mPolls * mRate / static_cast<float>(*mSize);
    if (mProgress < fEstimate) {
        mProgress = fEstimate;
    }
    if (kProgressDone < mProgress) {
        mProgress = kProgressDone;
    }
    TheLobbyRT->ReportProgress(mProgress);
    if (mDone != 0 && kProgressFinish < mProgress) {
        Finish(true, 0);
    }
}

void FileDownloadTask::OnResponse(int nConnection,
                                  int nType,
                                  int nSize,
                                  const rtMediusFileTransferResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileDownloadTask *pTask =
        pFound != nullptr ? dynamic_cast<FileDownloadTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    if (pResponse->status != RT_MEDIUS_STATUS_SUCCESS) {
        pTask->Finish(false, RepoRemixStatusMsg::kStatusFailed);
        return;
    }
    pTask->Append(pResponse, pResponse->dataSize);
    if (pResponse->transferStatus == RT_MEDIUS_TRANSFER_DONE) {
        pTask->mDone = 1;
    }
}

void FileDownloadTask::Append(const rtMediusFileTransferResponse *pResponse, int nBytes) {
    if (*mSize < mReceived + nBytes) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
        return;
    }
    memcpy(&(*mBuffer)[mReceived], pResponse->data, nBytes);
    mReceived += nBytes;
    mRate = static_cast<float>(mReceived) / mPolls;
}
