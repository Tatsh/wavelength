#include "game/gametrackselector.h"

#include "game/forcefeedbackmgr.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"

namespace {

// Track of a player on no track.
constexpr int kNoTrack = -1;

// Player of a track with no player.
constexpr int kNoPlayer = -1;

// Controller of a player without one.
constexpr int kNoPad = -1;

} // namespace

GameTrackSelector::GameTrackSelector(const std::vector<Track *> &tracks,
                                     FreestyleTrack *pFreestyleTrack,
                                     int nFreestyleType,
                                     int nFreestyleInstrument)
    : mSelector(static_cast<int>(tracks.size()) + 1), mFreestyleType(nFreestyleType),
      mFreestyleInstrument(nFreestyleInstrument), mHasFreestyle(pFreestyleTrack != nullptr) {
    mTracks.insert(mTracks.begin(), tracks.begin(), tracks.end());
    if (mHasFreestyle) {
        mTracks.push_back(pFreestyleTrack);
    }
    for (size_t i = 0; i < tracks.size(); ++i) {
        TheGfxManager.SetTrackPlayers(static_cast<int>(i), 0, nullptr);
    }
}

bool GameTrackSelector::IsFreestyleTrack(int nTrack) const {
    return mHasFreestyle && nTrack == static_cast<int>(mTracks.size()) - 1;
}

void GameTrackSelector::AddPlayer(Player *pPlayer, int nTrack) {
    mPlayers.push_back(pPlayer);
    mSelector.AddPlayer(nTrack);
    Track *pTrack = mTracks[nTrack];
    pPlayer->SetTrack(pTrack);
    pTrack->SetPlayer(mPlayers[mSelector.GetFirstPlayer(nTrack)]);
    RefreshTrack(nTrack);
}

int GameTrackSelector::GetNumPlayers() const {
    return mSelector.GetNumPlayers();
}

int GameTrackSelector::GetNumTracks() const {
    return mSelector.GetNumTracks();
}

void GameTrackSelector::EnterFreestyle(int nPlayer, bool bVictory) {
    SetPlayerTrack(nPlayer, static_cast<int>(mTracks.size()) - 1, bVictory);
}

void GameTrackSelector::MovePlayer(int nPlayer, int nTrack) {
    SetPlayerTrack(nPlayer, nTrack, false);
}

void GameTrackSelector::SetPlayerTrack(int nPlayer, int nTrack, bool bVictory) {
    const int nOldTrack = mSelector.GetPlayerTrack(nPlayer);
    mSelector.SetPlayerTrack(nPlayer, nTrack);
    if (nOldTrack != kNoTrack) {
        if (mSelector.GetNumPlayersOnTrack(nOldTrack) == 0) {
            mTracks[nOldTrack]->SetPlayer(nullptr);
        } else {
            Track *pOldTrack = mTracks[nOldTrack];
            Player *pCurrent = pOldTrack->mPlayer;
            const int nFirst = mSelector.GetFirstPlayer(nOldTrack);
            Player *pFirst = mPlayers[nFirst];
            if (pCurrent != pFirst) {
                pOldTrack->SetPlayer(pFirst);
            }
            if (pFirst != nullptr) {
                const int nPad = TheGameDb->GetPlayerPad(nFirst);
                if (nPad != kNoPad) {
                    TheForceFeedbackMgr->SetBeatEnabled(nPad, true);
                }
            }
        }
        if (!IsFreestyleTrack(nOldTrack)) {
            RefreshTrack(nOldTrack);
        }
    }
    if (nTrack == kNoTrack) {
        return;
    }

    Track *pTrack = mTracks[nTrack];
    const bool bAlone = mSelector.GetNumPlayersOnTrack(nTrack) == 1;
    if (bAlone) {
        pTrack->SetPlayer(mPlayers[nPlayer]);
    }
    mPlayers[nPlayer]->SetTrack(pTrack);
    const int nPad = TheGameDb->GetPlayerPad(nPlayer);
    if (nPad != kNoPad) {
        TheForceFeedbackMgr->SetBeatEnabled(nPad, bAlone);
    }
    if (IsFreestyleTrack(nTrack)) {
        TheGfxManager.ShowPlayerOnFreestyleTrack(
            nPlayer, mFreestyleType, mFreestyleInstrument, bVictory, 0);
    } else {
        RefreshTrack(nTrack);
    }
}

void GameTrackSelector::RotatePrevious(int nPlayer) {
    const int nTrack = mSelector.GetPlayerTrack(nPlayer) - 1;
    if (nTrack >= 0) {
        MovePlayer(nPlayer, nTrack);
    }
}

void GameTrackSelector::RotateNext(int nPlayer) {
    const int nTrack = mSelector.GetPlayerTrack(nPlayer) + 1;
    if (static_cast<size_t>(nTrack) < mTracks.size() - 1) {
        MovePlayer(nPlayer, nTrack);
    }
}

int GameTrackSelector::GetPlayerTrack(int nPlayer) const {
    return mSelector.GetPlayerTrack(nPlayer);
}

int GameTrackSelector::GetPlayerSlot(int nPlayer) const {
    return mSelector.GetPlayerSlot(nPlayer);
}

void GameTrackSelector::SwapPlayer(int nPlayer, int nTrack, int nSlot) {
    mSelector.SwapPlayer(nPlayer, nTrack, nSlot);
}

void GameTrackSelector::Resync() {
    mSelector.Compact();
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        Player *pPlayer = mPlayers[i];
        const int nTrack = mSelector.GetPlayerTrack(static_cast<int>(i));
        if (nTrack == kNoTrack) {
            continue;
        }
        Track *pTrack = mTracks[nTrack];
        if (pPlayer->GetTrack() != pTrack) {
            pPlayer->SetTrack(pTrack);
        }
        const int nPad = TheGameDb->GetPlayerPad(static_cast<int>(i));
        if (nPad != kNoPad) {
            TheForceFeedbackMgr->SetBeatEnabled(nPad, GetPlayerSlot(static_cast<int>(i)) == 0);
        }
    }
    for (size_t i = 0; i < mTracks.size(); ++i) {
        const int nTrack = static_cast<int>(i);
        Track *pTrack = mTracks[i];
        if (mSelector.GetNumPlayersOnTrack(nTrack) == 0) {
            if (pTrack->mPlayer != nullptr) {
                pTrack->SetPlayer(nullptr);
            }
        } else {
            Player *pFirst = mPlayers[mSelector.GetFirstPlayer(nTrack)];
            if (pTrack->mPlayer != pFirst) {
                pTrack->SetPlayer(pFirst);
            }
        }
        if (IsFreestyleTrack(nTrack)) {
            const int nFirst = mSelector.GetFirstPlayer(nTrack);
            if (nFirst != kNoPlayer) {
                TheGfxManager.ShowPlayerOnFreestyleTrack(
                    nFirst, mFreestyleType, mFreestyleInstrument, false, 0);
            }
        } else {
            RefreshTrack(nTrack);
        }
    }
}

void GameTrackSelector::RemovePlayer(int nPlayer) {
    SetPlayerTrack(nPlayer, kNoTrack, false);
    TheGfxManager.SetPlayerShown(nPlayer, false);
    TheGfxManager.SetCatcherShown(nPlayer, false);
    TheGfxManager.SetScoreShown(nPlayer, false);
    TheGfxManager.SetLaneShown(nPlayer, false);
}

void GameTrackSelector::RefreshTrack(int nTrack) {
    (void)IsFreestyleTrack(nTrack); // Yes, the binary discards this call's result.
    const int nPlayers = mSelector.GetNumPlayersOnTrack(nTrack);
    const int *pPlayers = mSelector.GetPlayersOnTrack(nTrack);
    for (int i = 0; i < nPlayers; ++i) {
        TheGfxManager.ShowPlayerOnTrack(pPlayers[i], nTrack);
    }
    TheGfxManager.SetTrackPlayers(nTrack, nPlayers, pPlayers);
}
