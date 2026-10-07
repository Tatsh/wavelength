#include "met/netfoundplayerpanel.h"

#include <cstdio>
#include <cstring>

#include "met/metagameutil.h"
#include "os/locale.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kNameComponent[] = "name";
constexpr char kRankComponent[] = "rank";
constexpr char kGamesComponent[] = "games";
constexpr char kNetComponent[] = "net";
constexpr char kPanelName[] = "fn_find_p";
constexpr char kOnlineComponent[] = "online";
constexpr char kLobbyComponent[] = "lobby";
constexpr char kRankMesh[] = "fn_find_p_rank.mesh";
constexpr char kNewbieToken[] = "newbie";
constexpr char kRankToken[] = "fn_find_p_02";
constexpr char kGamesToken[] = "fn_find_p_03";
constexpr char kNetToken[] = "fn_find_p_04";
constexpr char kOnlineToken[] = "fn_find_p_05";
constexpr char kYesToken[] = "yes";
constexpr char kRankFormat[] = "%d";
constexpr char kNoText[] = "";

constexpr int kRankTextSize = 32;

} // namespace

NetFoundPlayerPanel::NetFoundPlayerPanel(DataArray *pData, const char *pszDir)
    : AvatarPanel(pData, pszDir) {
}

void NetFoundPlayerPanel::Enter(bool bForce, float fTime) {
    AvatarPanel::Enter(bForce, fTime);
    FindComponent(kNameComponent, false)->SetText(mPlayer.mName.c_str());

    UIComponent *pRank = FindComponent(kRankComponent, false);
    char szRank[kRankTextSize];
    if (mPlayer.IsNewbie()) {
        std::strcpy(szRank, TheLocale.Localize(kNewbieToken, true));
    } else {
        std::sprintf(szRank, kRankFormat, mPlayer.mRank);
    }
    pRank->SetText(FormatString(TheLocale.Localize(kRankToken, true), szRank));

    FindComponent(kGamesComponent, false)
        ->SetText(FormatString(TheLocale.Localize(kGamesToken, true), mPlayer.mGames));

    UIComponent *pNet = FindComponent(kNetComponent, false);
    const String connection(GetConnectionTypeName(mPlayer.mConnectionType));
    pNet->SetText(FormatString(TheLocale.Localize(kNetToken, true), connection.c_str()));

    String text;
    UIComponent *pOnline = TheUI.FindComponent(kPanelName, kOnlineComponent, false);
    if (mInChatroom != 0) {
        const char *pszFormat = TheLocale.Localize(kOnlineToken, true);
        text = FormatString(pszFormat, TheLocale.Localize(kYesToken, true));
    } else {
        String date;
        mPlayer.mLastOnline.FormatDate(date);
        text = FormatString(TheLocale.Localize(kOnlineToken, true), date.c_str());
    }
    pOnline->SetText(text.c_str());

    UIComponent *pLobby = TheUI.FindComponent(kPanelName, kLobbyComponent, false);
    if (mInChatroom != 0) {
        text = mChatroomName;
    } else {
        text = kNoText;
    }
    pLobby->SetText(text.c_str());

    dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(kRankMesh))
        ->SetMat(FindRankMaterial(mPlayer.mRankIcon));
    SetAvatar(&mPlayer.mAvatar);
}
