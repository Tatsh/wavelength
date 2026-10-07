#include "gs/mutablenotemuse.h"

MutableNoteMuse::MutableNoteMuse(unsigned char nNote,
                                 unsigned char nVelocity,
                                 int nDuration,
                                 unsigned char nChannel)
    : NoteMuseBase(nNote, nVelocity, nDuration, nChannel), mNextNote(nNote) {
}

MutableNoteMuse::~MutableNoteMuse() {
}

void MutableNoteMuse::Play(Scheduler *pScheduler) {
    mNote = mNextNote;
    NoteMuseBase::Play(pScheduler);
}

void MutableNoteMuse::PlayFrom(Scheduler *pScheduler, int nOffset) {
    mNote = mNextNote;
    NoteMuseBase::PlayFrom(pScheduler, nOffset);
}

void MutableNoteMuse::PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) {
    mNote = mNextNote;
    NoteMuseBase::PlayWindow(pScheduler, nStart, nEnd);
}

Muse *MutableNoteMuse::Clone() {
    return new MutableNoteMuse(mNote, mVelocity, GetLength(), mChannel);
}

void MutableNoteMuse::SetNote(unsigned char nNote) {
    mNextNote = nNote;
}
