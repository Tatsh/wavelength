#include "met/multiendgamescreen.h"

#include <string.h>

#include "game/avatarpartset.h"
#include "game/gamedb.h"
#include "game/songentry.h"
#include "math/rand.h"
#include "met/metagame.h"
#include "met/multigamestatspanel.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/movie.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kBandPanel[] = "m_g_end_band";

// The rank of a winner.
constexpr int kWinnerRank = 0;

// With three winners or more, the first two rows use the panels `1o4` and `2o4`.
constexpr int kManyWinners = 3;
constexpr int kRowsWithFourPlayerPanel = 2;

// Two winners or more share the `tie` panels.
constexpr int kTieWinners = 2;

// A game of one player is laid out as one of two players, and a game of two players uses the
// layout `3`.
constexpr int kOnePlayer = 1;
constexpr int kTwoPlayers = 2;

// The dances of the Freqs cycle through loops 1 to 3 from a random start from 1 to 4.
constexpr int kFirstDance = 1;
constexpr int kLastDance = 4;

// Passed through to the avatar player. The meaning is not recovered.
constexpr int kDanceAnimFlags = 1;

inline int NextDance(int nDance) {
    ++nDance;
    return nDance == kLastDance ? kFirstDance : nDance;
}

} // namespace

MultiEndGameScreen::MultiEndGameScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindSymbol("m_stats", &mStats, false);
}

void MultiEndGameScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mFocusPanel->SetFocus(mFocusPanel->FindComponent("exit", false), kPadNone);

    std::vector<int> ranking;
    for (int nRank = 0; nRank < TheGameDb->GetNumPlayers(); ++nRank) {
        for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
            if (TheGameDb->GetPlayerRank(nPlayer) == nRank) {
                ranking.push_back(nPlayer);
            }
        }
    }
    mPlayerRows.clear();
    mPlayerRows.resize(TheGameDb->GetNumPlayers(), 0);

    int nWinners = 0;
    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        if (TheGameDb->GetPlayerRank(nPlayer) == kWinnerRank) {
            ++nWinners;
        }
    }

    for (int nRow = 0; nRow < TheGameDb->GetNumPlayers(); ++nRow) {
        int nPlayers = TheGameDb->GetNumPlayers();
        String row(FormatString("%d", nRow + 1));
        if (nPlayers == kOnePlayer) {
            nPlayers = kTwoPlayers;
        }
        if (nWinners >= kManyWinners && nRow < kRowsWithFourPlayerPanel) {
            row = FormatString("%do4", nRow + 1);
        }
        const bool bNoTie = nWinners < kTieWinners;
        if (!bNoTie) {
            mLayout = "tie";
        } else if (nPlayers == kTwoPlayers) {
            mLayout = "3";
        } else {
            mLayout = FormatString("%d", nPlayers);
        }
        String panelName(FormatString("m_g_end_%s_%s", row.c_str(), mLayout.c_str()));
        MultiGameStatsPanel *pPanel =
            dynamic_cast<MultiGameStatsPanel *>(TheUI.FindPanel(panelName.c_str(), false));
        // The neighbours are compared by player, not by the ranking, as the binary does.
        if (nRow == 0) {
            pPanel->mTied = !bNoTie;
        } else if (nRow == static_cast<int>(ranking.size()) - 1) {
            pPanel->mTied = TheGameDb->GetPlayerRank(nRow) == TheGameDb->GetPlayerRank(nRow - 1);
        } else {
            pPanel->mTied = TheGameDb->GetPlayerRank(nRow) == TheGameDb->GetPlayerRank(nRow - 1) ||
                            TheGameDb->GetPlayerRank(nRow) == TheGameDb->GetPlayerRank(nRow + 1);
        }
        pPanel->SetPlayer(ranking[nRow]);
        mPlayerRows[ranking[nRow]] = nRow;
    }

    SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    UIComponent *pComponent = TheUI.FindComponent(kBandPanel, "song", false);
    pComponent->SetText(FormatString(pComponent->Text(), entry.GetTitleShort()));
    pComponent = TheUI.FindComponent(kBandPanel, "band", false);
    pComponent->SetText(FormatString(pComponent->Text(), entry.GetArtistShort()));
    pComponent = TheUI.FindComponent(kBandPanel, "skill", false);
    pComponent->SetText(FormatString(pComponent->Text(), TheGameDb->GetDifficultyName()));

    int nLoseDance = RandomInt(kFirstDance, kLastDance);
    int nWinDance = RandomInt(kFirstDance, kLastDance);
    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        AvatarPartSet *pAvatar = TheGameDb->GetAvatar(nPlayer);
        if (TheGameDb->GetPlayerRank(nPlayer) == kWinnerRank) {
            pAvatar->SetBaseAnim(FormatString("win%d_loop", nWinDance), kDanceAnimFlags);
            nWinDance = NextDance(nWinDance);
        } else {
            pAvatar->SetBaseAnim(FormatString("lose%d_loop", nLoseDance), kDanceAnimFlags);
            nLoseDance = NextDance(nLoseDance);
        }
    }
}

bool MultiEndGameScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        const char *pszName = pMsg->mComponent->mName;
        Metagame::DialogAction action;
        if (strcmp(pszName, "replay") == 0) {
            action = Metagame::kDialogActionQuit;
        } else if (strcmp(pszName, "exit") == 0) {
            action = Metagame::kDialogActionEnd;
        } else {
            action = static_cast<Metagame::DialogAction>(0);
        }
        TheMetagame.ShowEndGameScreens(action);
    }
    return UIScreen::HandleSelect(pMsg);
}

bool MultiEndGameScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void MultiEndGameScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    dynamic_cast<Rnd::Movie *>(Rnd::TheManager.Find("freq_winner.mov"))
        ->SetFrame(TheGameDb->mSongTick);
}
