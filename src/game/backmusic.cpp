#include "game/backmusic.h"

#include "os/memfuncommand.h"
#include "os/scheduler.h"

BackMusic::BackMusic(const char *pszName, int nIntroBars, int nNumBars, int nTicksPerBar)
    : mName(pszName), mBars(nIntroBars + nNumBars),
      mBarCommand(NewMemFunCommand(this, &BackMusic::PlayBar)), mPlaying(nullptr),
      mIntroBars(nIntroBars), mNumBars(nNumBars), mTicksPerBar(nTicksPerBar), mPlayMap(nullptr) {
}

BackMusic::~BackMusic() {
    Stop();
}

const char *BackMusic::GetName() const {
    return mName.c_str();
}

void BackMusic::SetBar(int nBar, Muse *pMuse) {
    mBars[nBar + mIntroBars] = Ptr<Muse>(pMuse);
}

void BackMusic::Start(PlayMap *pPlayMap) {
    if (mPlaying != nullptr) {
        return;
    }
    mPlayMap = pPlayMap;
    PlayBar();
}

void BackMusic::Stop() {
    if (mPlaying == nullptr) {
        return;
    }
    mPlaying->Stop();
    mPlaying = nullptr;
    TheSongScheduler.Cancel(mBarCommand.Get());
}

void BackMusic::PlayBar() {
    const int nTick = TheSongScheduler.mTick;
    const int nBar = nTick / mTicksPerBar;
    const int nSongBar = mPlayMap->mLooping ? mPlayMap->MapBar(nBar) : nBar % mNumBars;
    mPlaying = mBars[nSongBar + mIntroBars].Get();
    mPlaying->PlayFrom(&TheSongScheduler, nTick - nBar * mTicksPerBar);
    TheSongScheduler.PostAt(mBarCommand.Get(), (nBar + 1) * mTicksPerBar, false);
}
