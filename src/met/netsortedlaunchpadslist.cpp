#include "met/netsortedlaunchpadslist.h"

#include <iterator>

#include "met/netsorteddatapanel.h"
#include "met/netsortedlaunchpadspanel.h"
#include "netflow/netlobby.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDataPanel[] = "fn_sorted_pic";
constexpr char kSortedPanel[] = "fn_sorted";
constexpr char kSortedScreen[] = "fn_sorted";

NetSortedLaunchpadsPanel *FindSortedPanel() {
    return static_cast<NetSortedLaunchpadsPanel *>(TheUI.FindPanel(kSortedPanel, false));
}

} // namespace

void NetSortedLaunchpadsList::UpdateCursor() {
    UIList::UpdateCursor();
    auto *pPanel = static_cast<NetSortedDataPanel *>(TheUI.FindPanel(kDataPanel, false));
    pPanel->Reset();
    if (mLaunchpads.empty()) {
        return;
    }
    NetLaunchpadInfo *pLaunchpad = &*std::next(mLaunchpads.begin(), mSelected);
    pPanel->ShowLaunchpad(pLaunchpad);
    TheNetLobby->RequestLaunchpadPlayers(TheUI.FindScreen(kSortedScreen, false),
                                         pLaunchpad->mLaunchpadId);
}

void NetSortedLaunchpadsList::ScrollUp() {
    if (mSelected == 0) {
        FindSortedPanel()->Request(NetSortedLaunchpadsPanel::kPagePrevious);
    } else {
        FreqList::ScrollUp();
    }
}

void NetSortedLaunchpadsList::ScrollDown() {
    if (mSelected < mItemCount - 1) {
        FreqList::ScrollDown();
    } else {
        FindSortedPanel()->Request(NetSortedLaunchpadsPanel::kPageNext);
    }
}
