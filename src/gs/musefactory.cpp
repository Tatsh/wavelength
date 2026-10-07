#include "gs/musefactory.h"

#include <algorithm>

#include "gs/notemuse.h"
#include "gs/stdmidimuse.h"
#include "os/debug.h"

namespace {

constexpr int kNoChannel = -1;
constexpr int kUnpaired = -1;
constexpr int kNoIndex = -1;
constexpr int kReservedMessages = 500;
constexpr int kReservedOpenNotes = 10;

constexpr unsigned char kTypeMask = 0xf0;
constexpr unsigned char kChannelMask = 0x0f;
constexpr unsigned char kNoteOff = 0x80;
constexpr unsigned char kNoteOn = 0x90;

bool IsNoteOn(const MuseFactory::TickMuse &message) {
    return (message.mStatus & kTypeMask) == kNoteOn;
}

bool TickBefore(const MuseFactory::TickMuse &message, int nTick) {
    return message.mTick < nTick;
}

bool TickAfter(int nTick, const MuseFactory::TickMuse &message) {
    return nTick < message.mTick;
}

} // namespace

MuseFactory::MuseFactory()
    : mMessages(), mOpenNotes(), mChannel(kNoChannel), mState(nullptr), mResumeIndex(kNoIndex) {
    mMessages.reserve(kReservedMessages);
    mOpenNotes.reserve(kReservedOpenNotes);
}

MuseFactory::~MuseFactory() {
    delete mState;
}

void MuseFactory::AddMidi(int nTick,
                          unsigned char nStatus,
                          unsigned char nData1,
                          unsigned char nData2) {
    if (mChannel == kNoChannel) {
        mChannel = nStatus & kChannelMask;
        mState = new MidiChannelState(static_cast<unsigned char>(mChannel));
    }
    switch (nStatus & kTypeMask) {
    case kNoteOn:
        AddNoteOn(nTick, nStatus, nData1, nData2);
        break;
    case kNoteOff:
        AddNoteOff(nTick, nStatus, nData1, nData2);
        break;
    default:
        AddOther(nTick, nStatus, nData1, nData2);
        break;
    }
}

unsigned char MuseFactory::GetChannel() const {
    return static_cast<unsigned char>(mChannel);
}

int MuseFactory::CountMessages(int nStart, int nEnd) {
    const auto first = std::lower_bound(mMessages.begin(), mMessages.end(), nStart, TickBefore);
    if (first == mMessages.end()) {
        return 0;
    }
    const auto last = std::upper_bound(first, mMessages.end(), nEnd - 1, TickAfter);
    if (last == first) {
        return 0;
    }
    int nCount = 0;
    for (auto it = first; it != last; ++it) {
        if (!IsNoteOn(*it) || it->mDuration >= 0) {
            ++nCount;
        }
    }
    return nCount;
}

MultiMuse *MuseFactory::ExtractAll() {
    MultiMuse *pMulti = new MultiMuse(0);
    for (const TickMuse &message : mMessages) {
        if (Muse *pMuse = MakeMuse(message)) {
            pMulti->Add(pMuse, message.mTick);
        }
    }
    return pMulti;
}

MultiMuse *MuseFactory::Extract(int nStart, int nEnd) {
    MultiMuse *pMulti = new MultiMuse(0);
    if (mMessages.empty() || mMessages.back().mTick < nStart || !(mMessages.front().mTick < nEnd)) {
        return pMulti;
    }

    // Bring the controller state up to the start, resuming from the last extraction when it lies
    // before the start.
    auto it = mMessages.begin();
    if (mResumeIndex != kNoIndex && !(nStart < mMessages[mResumeIndex].mTick)) {
        it = mMessages.begin() + mResumeIndex;
    } else {
        mState->Reset();
    }
    for (; it != mMessages.end() && !(nStart < it->mTick); ++it) {
        mState->OnMessage(it->mStatus, it->mData1, it->mData2);
    }

    // Step back to the last message before the start, then on to the first one at or after it.
    while (it != mMessages.begin() && (it == mMessages.end() || !(it->mTick < nStart))) {
        --it;
    }
    if (it != mMessages.end()) {
        while (it != mMessages.end() && it->mTick < nStart) {
            ++it;
        }
        // Skip the messages before the first note on of the span.
        while (it != mMessages.end() && !IsNoteOn(*it) && it->mTick < nEnd) {
            ++it;
        }
    }
    mResumeIndex = static_cast<int>(it - mMessages.begin());

    int nAdded = 0;
    for (; it != mMessages.end() && it->mTick < nEnd; ++it) {
        Muse *pMuse = MakeMuse(*it);
        if (pMuse == nullptr) {
            continue;
        }
        if (nAdded == 0) {
            if (Muse *pState = mState->GetMuse()) {
                pMulti->Add(pState, 0);
            }
        }
        pMulti->Add(pMuse, it->mTick - nStart);
        ++nAdded;
    }
    return pMulti;
}

void MuseFactory::AddNoteOn(int nTick,
                            unsigned char nStatus,
                            unsigned char nData1,
                            unsigned char nData2) {
    mMessages.push_back(TickMuse{nTick, nStatus, nData1, nData2, kUnpaired});
    mOpenNotes.push_back(static_cast<int>(mMessages.size()) - 1);
}

void MuseFactory::AddNoteOff(int nTick, unsigned char, unsigned char nData1, unsigned char) {
    for (auto it = mOpenNotes.begin(); it != mOpenNotes.end(); ++it) {
        TickMuse &noteOn = mMessages[*it];
        if (noteOn.mData1 == nData1) {
            noteOn.mDuration = nTick - noteOn.mTick;
            mOpenNotes.erase(it);
            return;
        }
    }
    DebugWarn("got NoteOff message without corresponding NoteOn");
}

void MuseFactory::AddOther(int nTick,
                           unsigned char nStatus,
                           unsigned char nData1,
                           unsigned char nData2) {
    mMessages.push_back(TickMuse{nTick, nStatus, nData1, nData2, kUnpaired});
}

Muse *MuseFactory::MakeMuse(const TickMuse &message) {
    if (IsNoteOn(message)) {
        if (message.mDuration < 0) {
            return nullptr;
        }
        return new NoteMuse(message.mData1,
                            message.mData2,
                            message.mDuration,
                            static_cast<unsigned char>(mChannel));
    }
    return new StdMidiMuse(message.mStatus, message.mData1, message.mData2);
}
