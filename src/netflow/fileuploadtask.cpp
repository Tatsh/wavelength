#include "netflow/fileuploadtask.h"

#include <cstring>

#include "msg/reporemixstatusmsg.h"
#include "netflow/lobbyrt.h"
#include "os/string.h"

namespace {

// The progress of a finished transfer.
constexpr float kProgressDone = 1.0f;

} // namespace

void FileUploadTask::OnStart() {
    mDone = 0;
    mPolls = 0.0f;
    mRate = 0.0f;
    mProgress = 0.0f;
    rtMediusFileUploadRequest request;
    memset(&request, 0, sizeof(request));
    MakeTag(this, request.messageId);
    strncpy(request.file.fileName, mName->c_str(), RT_MEDIUS_FILE_NAME_SIZE);
    request.file.fileSize = *mSize;
    request.buffer = *mBuffer;
    request.bufferSize = *mSize;
    const int nError = rtMediusFileUpload(&request, OnResponse);
    LobbyRT::ReportMediusError(2198, nError);
    if (nError != 0) {
        Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}

void FileUploadTask::OnPoll() {
    mPolls += 1.0f;
    const float fEstimate = mPolls * mRate / static_cast<float>(*mSize);
    if (mProgress < fEstimate) {
        mProgress = fEstimate;
    }
    if (kProgressDone < mProgress) {
        mProgress = kProgressDone;
    }
    TheLobbyRT->ReportProgress(mProgress);
    if (mDone != 0) {
        Finish(true, 0);
    }
}

void FileUploadTask::OnResponse(int nConnection,
                                int nType,
                                int nSize,
                                const rtMediusFileTransferResponse *pResponse) {
    (void)nConnection;
    (void)nType;
    (void)nSize;
    NetTask *pFound = FindTag(pResponse->messageId);
    FileUploadTask *pTask = pFound != nullptr ? dynamic_cast<FileUploadTask *>(pFound) : nullptr;
    if (pTask == nullptr) {
        return;
    }
    pTask->mRate = static_cast<float>(pResponse->startByteIndex) / pTask->mPolls;
    if (pResponse->transferStatus == RT_MEDIUS_TRANSFER_DONE) {
        pTask->mDone = 1;
    } else if (pResponse->status != RT_MEDIUS_STATUS_SUCCESS) {
        pTask->Finish(false, RepoRemixStatusMsg::kStatusFailed);
    }
}
