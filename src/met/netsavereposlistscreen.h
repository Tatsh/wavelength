#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqscreen.h"
#include "msg/reporemixstatusmsg.h"
#include "netflow/netreporemix.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen `fn_save_repos_tool` that uploads the edited list of the player's repository, then
 * returns to the upload tool.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x00463e60`. The
 * metagame registers the class for the screen type `save_repos_tool_screen`.
 */
class NetSaveReposListScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no remixes.
     *
     * @param pData The script description.
     * @ghidraAddress PAL: 0x0018ee80
     */
    explicit NetSaveReposListScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress PAL: 0x003c9f20
     */
    ~NetSaveReposListScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress PAL: 0x003c9fe0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetSaveReposListScreen(pData);
    }

    /**
     * Route the status of the upload and the end of the entry to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress PAL: 0x0018ef00
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress PAL: 0x003ca020
     */
    const char *Title() override {
        return "";
    }

    /** @param remixes The remixes to upload as the repository. */
    void SetRemixes(const std::list<NetRepoRemix> &remixes) {
        mRemixes = remixes;
    }

private:
    /**
     * Upload the remixes once this screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018ef90
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Return to `fn_upload_tool` when the upload finished, or go to `net_save_repos_list_error`
     * when it failed.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018eff0
     */
    bool HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg);

    std::list<NetRepoRemix> mRemixes; /*!< The remixes to upload as the repository. */
};

#endif
