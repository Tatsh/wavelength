#pragma once

#include "met/freqscreen.h"
#include "met/netrankedplayerslist.h"
#include "msg/lobbyplayersmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen of the full ranking. The list pages through ten players at a time.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetRankingScreen : public FreqScreen {
public:
    /** The pages RequestPage() takes, as NetLobby::RequestRanks() takes them. */
    enum Page {
        kPageTop = 0,      /*!< The top of the ranking. */
        kPageNext = 1,     /*!< The page after the list. */
        kPagePrevious = 3, /*!< The page before the list. */
    };

    /**
     * Construct a screen.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017e498
     * @ghidraAddress PAL: 0x00182158
     */
    explicit NetRankingScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035ac88
     * @ghidraAddress PAL: 0x003c87d8
     */
    ~NetRankingScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035ad20
     * @ghidraAddress PAL: 0x003c8870
     */
    static UIScreen *New(DataArray *pData) {
        return new NetRankingScreen(pData);
    }

    /**
     * Route the list of players to HandleLobbyPlayers().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017e608
     * @ghidraAddress PAL: 0x001822c8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter with an empty list and request the top of the ranking.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x0017e4e0
     * @ghidraAddress PAL: 0x001821a0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Empty the list and request another page, unless the previous page is requested and no
     * players precede the list.
     *
     * The name is inferred.
     *
     * @param nPage One of Page.
     * @ghidraAddress NTSC-U/C: 0x0017e670
     * @ghidraAddress PAL: 0x00182330
     */
    void RequestPage(int nPage);

private:
    /**
     * Fill the list with a page of the ranking, while the screen is current.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017e7d8
     * @ghidraAddress PAL: 0x00182498
     */
    bool HandleLobbyPlayers(LobbyPlayersMsg *pMsg);

    NetRankedPlayersList *mList; /*!< The `list` of the `fn_rank` panel. */
    int mReserved74;             // +0x74, not yet identified.
    int mLoading;                /*!< Non-zero while a page is requested. */
    int mSelectFirst;            /*!< Non-zero when the next page selects its first player. */
    int mHasPrevious;            /*!< Non-zero when players precede the last page received. */
};
