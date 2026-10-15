#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqscreen.h"
#include "msg/filelistmsg.h"
#include "msg/reporemixstatusmsg.h"
#include "netflow/reporemixesmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that waits while the files of the player's upload directory and the remixes of the
 * repository are requested, then goes to the upload tool `fn_upload_tool`.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x00463f80`. The
 * metagame registers the class for the screen type `get_all_uploads_screen`.
 */
class NetGetAllUploadsScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no files.
     *
     * @param pData The script description.
     * @ghidraAddress PAL: 0x0018ccf8
     */
    explicit NetGetAllUploadsScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress PAL: 0x003c9bf8
     */
    ~NetGetAllUploadsScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress PAL: 0x003c9cb8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetGetAllUploadsScreen(pData);
    }

    /**
     * Route the file list, the remixes of the repository, the end of the entry, and the status
     * of a transfer to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress PAL: 0x0018cd78
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress PAL: 0x003c9cf8
     */
    const char *Title() override {
        return "";
    }

private:
    /**
     * Keep the uploaded files and request the remixes of the repository.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018cef0
     */
    bool HandleFileList(FileListMsg *pMsg);

    /**
     * Give the remixes of the repository to `fn_repos_tool`, give the files and the remixes to
     * `fn_upload_tool`, and go to `fn_upload_tool`. A failed request gives no remixes.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018cf40
     */
    bool HandleRepoRemixes(RepoRemixesMsg *pMsg);

    /**
     * Request the uploaded files once this screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018ce48
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Write the status of a transfer to the diagnostic stream.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x0018cea8
     */
    bool HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg);

    std::list<String> mFiles; // The files of the upload directory.
};

#endif
