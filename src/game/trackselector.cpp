#include "game/trackselector.h"

#include <algorithm>

namespace {

constexpr int kEmptySlot = -1;
constexpr int kNoTrack = -1;
constexpr int kNoPlayer = -1;

} // namespace

TrackSelector::TrackInfo::TrackInfo() : mPlayers(kNumSlots, kEmptySlot) {
}

int TrackSelector::TrackInfo::Find(int nPlayer) const {
    return static_cast<int>(std::find(mPlayers.begin(), mPlayers.end(), nPlayer) -
                            mPlayers.begin());
}

int TrackSelector::TrackInfo::Count() const {
    int nEmpty = 0;
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        if (*it == kEmptySlot) {
            ++nEmpty;
        }
    }
    return static_cast<int>(mPlayers.size()) - nEmpty;
}

void TrackSelector::TrackInfo::Add(int nPlayer) {
    mPlayers[Count()] = nPlayer;
}

void TrackSelector::TrackInfo::Compact() {
    for (unsigned int i = 1; i < mPlayers.size(); ++i) {
        if (mPlayers[i] == kEmptySlot) {
            continue;
        }
        int nTarget = static_cast<int>(i);
        if (mPlayers[i - 1] == kEmptySlot) {
            nTarget = static_cast<int>(i) - 1;
            while (nTarget > 0 && mPlayers[nTarget - 1] == kEmptySlot) {
                --nTarget;
            }
        }
        if (mPlayers[nTarget] == kEmptySlot) {
            mPlayers[nTarget] = mPlayers[i];
            mPlayers[i] = kEmptySlot;
        }
    }
}

void TrackSelector::TrackInfo::Remove(int nPlayer) {
    int nSlot = Find(nPlayer);
    int nCount = Count();
    for (; nSlot < nCount - 1; ++nSlot) {
        mPlayers[nSlot] = mPlayers[nSlot + 1];
    }
    mPlayers[nCount - 1] = kEmptySlot;
}

TrackSelector::TrackSelector(int nTracks) : mTrackInfos(nTracks, TrackInfo()), mNeedsCompact(0) {
}

void TrackSelector::AddPlayer(int nTrack) {
    mPlayerTracks.push_back(nTrack);
    mTrackInfos[nTrack].Add(static_cast<int>(mPlayerTracks.size()) - 1);
}

void TrackSelector::AddPlayer(int nTrack, int nSlot) {
    mPlayerTracks.push_back(nTrack);
    mTrackInfos[nTrack].mPlayers[nSlot] = static_cast<int>(mPlayerTracks.size()) - 1;
}

int TrackSelector::GetNumPlayers() const {
    return static_cast<int>(mPlayerTracks.size());
}

int TrackSelector::GetNumTracks() const {
    return static_cast<int>(mTrackInfos.size());
}

void TrackSelector::SetPlayerTrack(int nPlayer, int nTrack) {
    int nOldTrack = mPlayerTracks[nPlayer];
    if (nOldTrack != kNoTrack) {
        mTrackInfos[nOldTrack].Remove(nPlayer);
    }
    mPlayerTracks[nPlayer] = nTrack;
    if (nTrack != kNoTrack) {
        mTrackInfos[nTrack].Add(nPlayer);
    }
}

void TrackSelector::SwapPlayer(int nPlayer, int nTrack, int nSlot) {
    mNeedsCompact = 1;
    int nOldTrack = mPlayerTracks[nPlayer];
    int nOldSlot = mTrackInfos[nOldTrack].Find(nPlayer);
    int nOther = mTrackInfos[nTrack].mPlayers[nSlot];
    mTrackInfos[nOldTrack].mPlayers[nOldSlot] = nOther;
    mTrackInfos[nTrack].mPlayers[nSlot] = nPlayer;
    mPlayerTracks[nPlayer] = nTrack;
    if (nOther != kNoPlayer) {
        mPlayerTracks[nOther] = nOldTrack;
    }
}

void TrackSelector::Compact() {
    if (!mNeedsCompact) {
        return;
    }
    for (unsigned int i = 0; i < mTrackInfos.size(); ++i) {
        mTrackInfos[i].Compact();
    }
    mNeedsCompact = 0;
}

int TrackSelector::GetPlayerTrack(int nPlayer) const {
    return mPlayerTracks[nPlayer];
}

int TrackSelector::GetPlayerSlot(int nPlayer) const {
    return mTrackInfos[mPlayerTracks[nPlayer]].Find(nPlayer);
}

int TrackSelector::GetFirstPlayer(int nTrack) const {
    return mTrackInfos[nTrack].mPlayers.front();
}

int TrackSelector::GetNumPlayersOnTrack(int nTrack) const {
    return mTrackInfos[nTrack].Count();
}

const int *TrackSelector::GetPlayersOnTrack(int nTrack) const {
    return mTrackInfos[nTrack].mPlayers.data();
}
