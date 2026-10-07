#include "memcard/mcgetinfotask.h"

#include "memcard/memcard.h"

void MCGetInfoTask::Set(MemcardSerialTask *pOwner, int nPort, int nAcceptNewCard) {
    SetPort(nPort);
    mOwner = pOwner;
    mAcceptNewCard = nAcceptNewCard;
}

void MCGetInfoTask::OnGetInfo(int nResult) {
    SetStatus(nResult);
    if (sStatus == kStatusUnformatted) {
        sStatus = kStatusChangedCard;
    }
    if (mAcceptNewCard != 0 && sStatus == kStatusChangedCard) {
        sStatus = kStatusOk;
    }
    Finish(sStatus == kStatusOk);
    if (mOwner != nullptr) {
        mOwner->OnCardInfo();
    }
}

void MCGetInfoTask::OnStart() {
    MemcardGetInfo(this, mPort, &mType, &mFree, &mFormat);
}
