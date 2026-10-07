#include "app/rendererbase.h"

bool RendererBase::Router::Dispatch(Message *pMsg) {
    mTarget->DispatchPriv(pMsg);
    return false;
}

bool RendererBase::Router::DispatchPriv(Message *) {
    return false;
}

RendererBase::RendererBase() {
    mRouter.mTarget = this;
    mQueue.AddSink(&mRouter);
}

RendererBase::~RendererBase() {
}

bool RendererBase::Dispatch(Message *pMsg) {
    mQueue.Dispatch(pMsg);
    return false;
}

void RendererBase::Start() {
}

void RendererBase::Stop() {
}

void RendererBase::PollMessages() {
    mQueue.Poll();
}

void RendererBase::UpdateSimple() {
}

void RendererBase::DrawSimple() {
}
