#include "gs/scratcher.h"

#include <cmath>

#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/synth.h"

namespace {

enum Piece {
    kPieceLeft = 0,
    kPieceCentre = 1,
    kPieceRight = 2,
};

constexpr double kLeftPosition = -0.5;
constexpr double kRightPosition = 0.5;
constexpr float kPitchThreshold = 0.02f;

// A pitch bend carries 14 bits around a centre of 8192 and spans three semitones each way.
constexpr unsigned int kStatusPitchBend = 0xe0;
constexpr float kBendSemitones = 3.0f;
constexpr float kBendScale = 8191.0f;
constexpr float kBendCentre = 8192.0f;
constexpr int kDataBits = 7;
constexpr unsigned int kDataMask = 0x7f;
constexpr unsigned int kBendMask = 0xffff;
constexpr int kData1Shift = 8;
constexpr int kData2Shift = 16;

constexpr int kRestartPosition = 1;

} // namespace

void Scratcher::ScratchNoteCB::OnNote(unsigned char, int nDuration) {
    TheGfxManager.ShowScratchNote(mPlayer,
                                  static_cast<float>(TheSongScheduler.mTick + mTickOffset),
                                  static_cast<float>(nDuration));
}

Scratcher::Scratcher(
    Muse *pFirst, Muse *pSecond, Muse *pThird, int nQuantumTicks, int nLengthTicks, int nChannel)
    : mChannel(nChannel), mQuantumTicks(nQuantumTicks), mLoopers(kNumPieces, nullptr),
      mCopies(kNumPieces), mRestartCmd(NewMemFunCommand(this, &Scratcher::Restart)), mScratching(0),
      mPitch(0.0f), mReserved34(0), mPiece(0) {
    (void)pFirst->GetLength(); // The binary discards the three lengths.
    (void)pSecond->GetLength();
    (void)pThird->GetLength();
    pFirst->SetNoteCB(&mNoteCB);
    pSecond->SetNoteCB(&mNoteCB);
    pThird->SetNoteCB(&mNoteCB);
    mLoopers[kPieceLeft] = new MuseLooper(pFirst, nLengthTicks);
    mLoopers[kPieceCentre] = new MuseLooper(pSecond, nLengthTicks);
    mLoopers[kPieceRight] = new MuseLooper(pThird, nLengthTicks);
    mCopies[kPieceLeft] = Ptr<Muse>(pFirst->Clone());
    mCopies[kPieceCentre] = Ptr<Muse>(pSecond->Clone());
    mCopies[kPieceRight] = Ptr<Muse>(pThird->Clone());
}

Scratcher::~Scratcher() {
    Stop();
    for (MuseLooper *pLooper : mLoopers) {
        delete pLooper;
    }
}

void Scratcher::Start(int nPlayer, int nTickOffset) {
    SendPitchBend(mPitch);
    SwitchPiece(mPiece);
    mNoteCB.mPlayer = nPlayer;
    mNoteCB.mTickOffset = nTickOffset;
    const int nPosition = TheSongScheduler.mTick % mQuantumTicks;
    if (nPosition < mQuantumTicks / 2) {
        mLoopers[mPiece]->Play(&TheSongScheduler, nPosition);
    } else {
        TheSongScheduler.PostIn(mRestartCmd.Get(), mQuantumTicks - nPosition + 1, false);
    }
    mScratching = 1;
}

void Scratcher::Stop() {
    TheSongScheduler.Cancel(mRestartCmd.Get());
    mLoopers[mPiece]->Stop();
    SendPitchBend(0.0f);
    mScratching = 0;
}

int Scratcher::IsScratching() {
    return mScratching;
}

int Scratcher::GetLoopLength() {
    return mLoopers[kPieceLeft]->GetLength();
}

void Scratcher::SetPitch(float fPitch) {
    if (!(kPitchThreshold < std::fabs(fPitch - mPitch))) {
        return;
    }
    mPitch = fPitch;
    if (mScratching) {
        SendPitchBend(fPitch);
    }
}

void Scratcher::SetPosition(float fPosition) {
    int nPiece = kPieceCentre;
    if (static_cast<double>(fPosition) < kLeftPosition) {
        nPiece = kPieceLeft;
    } else if (static_cast<double>(fPosition) > kRightPosition) {
        nPiece = kPieceRight;
    }
    if (nPiece != mPiece) {
        SwitchPiece(nPiece);
    }
}

void Scratcher::SendPitchBend(float fPitch) {
    const unsigned int nBend = static_cast<unsigned int>(static_cast<int>(
                                   fPitch / kBendSemitones * kBendScale + kBendCentre)) &
                               kBendMask;
    const unsigned int nStatus = static_cast<unsigned char>(mChannel) | kStatusPitchBend;
    TheSynth->SendPackedMessage(nStatus | ((nBend & kDataMask) << kData1Shift) |
                                (((nBend >> kDataBits) & kDataMask) << kData2Shift));
}

void Scratcher::SwitchPiece(int nPiece) {
    if (mScratching) {
        mLoopers[mPiece]->Stop();
        const int nPosition = TheSongScheduler.mTick % mQuantumTicks;
        if (nPosition < mQuantumTicks / 2) {
            mLoopers[nPiece]->Play(&TheSongScheduler, nPosition);
        } else {
            mLoopers[nPiece]->Play(&TheSongScheduler, nPosition - mQuantumTicks);
        }
    }
    mPiece = nPiece;
}

void Scratcher::Restart() {
    mLoopers[mPiece]->Play(&TheSongScheduler, kRestartPosition);
}
