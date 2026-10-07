#include "gs/axecontour.h"

#include "os/scheduler.h"

namespace {

constexpr int kNoNote = -1;

} // namespace

AxeContour::AxeContour(int nLength, unsigned char nChannel)
    : mChannel(nChannel), mMuse(new MultiMuse(0)), mLooper(mMuse.Get(), nLength), mNotes(),
      mLowestKey(0), mHighestKey(0), mCurrentNote(kNoNote), mNoteCB(new AxeNoteCB(this)) {
}

AxeContour::~AxeContour() {
    delete mNoteCB;
}

int AxeContour::GetLength() {
    return mLooper.GetLength();
}

unsigned char AxeContour::GetChannel() const {
    return mChannel;
}

void AxeContour::AddNote(int nTick, unsigned char nKey, unsigned char nVelocity, int nDuration) {
    MutableNoteMuse *pMuse = new MutableNoteMuse(nKey, nVelocity, nDuration, mChannel);
    pMuse->SetNoteCB(mNoteCB);
    mMuse->Add(pMuse, nTick);
    if (mNotes.empty()) {
        mHighestKey = nKey;
        mLowestKey = nKey;
    } else {
        mHighestKey = nKey < mHighestKey ? mHighestKey : nKey;
        mLowestKey = mLowestKey < nKey ? mLowestKey : nKey;
    }
    mNotes.push_back(ContourNote{nKey, pMuse});
}

int AxeContour::GetNumNotes() const {
    return static_cast<int>(mNotes.size());
}

void AxeContour::AddMuse(int nTick, Muse *pMuse) {
    mMuse->Add(pMuse, nTick);
}

unsigned char AxeContour::GetNoteKey(int nIndex) const {
    return mNotes[nIndex].mKey;
}

void AxeContour::SetNoteKey(int nIndex, unsigned char nKey) {
    mNotes[nIndex].mMuse->SetNote(nKey);
}

unsigned char AxeContour::GetHighestKey() const {
    return mHighestKey;
}

unsigned char AxeContour::GetLowestKey() const {
    return mLowestKey;
}

void AxeContour::Play(int nPosition) {
    mLooper.Play(&TheSongScheduler, nPosition);
}

void AxeContour::Stop() {
    mCurrentNote = kNoNote;
    mLooper.Stop();
}

bool AxeContour::IsPlaying() const {
    return mLooper.IsPlaying();
}

int AxeContour::GetPosition() const {
    return mLooper.GetPosition();
}
