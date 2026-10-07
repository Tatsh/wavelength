#include "met/multigamestatspanel.h"

#include "game/gamedb.h"
#include "met/metagameutil.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "rnd/text.h"

namespace {

constexpr char kRankOrderEntry[] = "rank_order";
constexpr char kTiedPlaceFormat[] = "%d_place_tie";
constexpr char kPlaceFormat[] = "%d_place";
constexpr char kNameTextFormat[] = "%s.txt";
constexpr char kPlaceTextFormat[] = "%s_p.txt";
constexpr char kScoreTextFormat[] = "%s_s.txt";
constexpr char kColorMeshFormat[] = "%s_c.mesh";
constexpr char kScoreFormat[] = "%d";

template <typename T>
T *FindObject(const char *pszFormat, const char *pszPanel) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(FormatString(pszFormat, pszPanel)));
}

} // namespace

MultiGameStatsPanel::MultiGameStatsPanel(DataArray *pData, const char *pszDir)
    : AvatarPanel(pData, pszDir), mTied(0) {
    pData->FindInt(kRankOrderEntry, &mRankOrder, false);
}

const char *MultiGameStatsPanel::RankLabel(int nRank, bool bTied) {
    String token;
    if (bTied) {
        token = FormatString(kTiedPlaceFormat, nRank + 1);
    } else {
        token = FormatString(kPlaceFormat, nRank + 1);
    }
    return TheLocale.Localize(token.c_str(), true);
}

void MultiGameStatsPanel::SetPlayer(int nPlayer) {
    mPlayer = nPlayer;
    SetAvatar(TheGameDb->GetAvatar(nPlayer));
    FindObject<Rnd::Text>(kNameTextFormat, mName)->SetText(TheGameDb->GetPlayerName(mPlayer));
    FindObject<Rnd::Text>(kPlaceTextFormat, mName)
        ->SetText(RankLabel(TheGameDb->GetPlayerRank(mPlayer), mTied != 0));

    const String score(FormatString(kScoreFormat, TheGameDb->GetPlayerScore(mPlayer)));
    FindObject<Rnd::Text>(kScoreTextFormat, mName)->SetText(score.c_str());

    const char *pszColor = TheGameDb->GetPlayerColor(mPlayer);
    Rnd::Mesh *pMesh = FindObject<Rnd::Mesh>(kColorMeshFormat, mName);
    pMesh->SetShowing(true);
    pMesh->SetMat(FindColorMaterial(pszColor));
}
