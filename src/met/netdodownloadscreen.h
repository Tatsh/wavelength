#pragma once

#include "met/freqscreen.h"
#include "msg/reporemixstatusmsg.h"
#ifdef VIDEO_STANDARD_PAL
#include "msg/remixdownloadmsg.h"
#endif
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that downloads a remix, shows the progress, and goes on to save it.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetDoDownloadScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no remix.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017f9e8
     * @ghidraAddress PAL: 0x00183778
     */
    explicit NetDoDownloadScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035b2b0
     * @ghidraAddress PAL: 0x003c8e08
     */
    ~NetDoDownloadScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035b368
     * @ghidraAddress PAL: 0x003c8ec0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetDoDownloadScreen(pData);
    }

    /**
     * Route the status of the download and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017fa68
     * @ghidraAddress PAL: 0x00183800
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x0035b3a8
     * @ghidraAddress PAL: 0x003c8f00
     */
    const char *Title() override {
        return "";
    }

    String mRemixName; /*!< The name the remix is saved under. */
    String mFile;      /*!< The file of the remix on the server. */

#ifdef VIDEO_STANDARD_PAL
    /** The value of mUploadIndex when the file does not belong to the upload tool. */
    static constexpr int kNoUploadIndex = -1;

    /** The upload tool's file being downloaded, or kNoUploadIndex. */
    int mUploadIndex = kNoUploadIndex;
#endif

private:
    /**
     * Show the part of the remix downloaded on the dialog.
     *
     * The name is inferred.
     *
     * @param fProgress The part downloaded, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x0017fb68
     * @ghidraAddress PAL: 0x00183920
     */
    void ShowProgress(float fProgress);

    /**
     * Start the download once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017faf8
     * @ghidraAddress PAL: 0x001838b0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Show the progress, or go on to save the remix when the download ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017fc20
     * @ghidraAddress PAL: 0x00183e20
     */
    bool HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg);

#ifdef VIDEO_STANDARD_PAL
    /**
     * Act on a downloaded file of the upload tool. A failed download removes the file from the
     * upload tool. Otherwise the screen `fn_do_download_tool` adds the remix to the repository
     * tool, and the screens `fn_do_download_preview_game` and `fn_do_download_preview_remix`
     * start the remix as a practice game or in the remix editor.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress PAL: 0x001839d8
     */
    bool HandleRemixDownload(RemixDownloadMsg *pMsg);
#endif
};
