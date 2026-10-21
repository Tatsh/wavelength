#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqscreen.h"
#include "met/repostoollist.h"
#include "met/uploadinfo.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netreporemix.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen `fn_repos_tool` that edits the list of the player's repository of remixes.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x00463ec0`. The
 * buttons move a remix up or down, remove it, edit its note on `fn_upload_note_tool`, and save
 * the list on `fn_save_repos_tool`. The metagame registers the class for the screen type
 * `repos_tool_screen`.
 */
class NetReposToolScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no remixes and no edited note.
     *
     * @param pData The script description.
     * @ghidraAddress PAL: 0x0018e328
     */
    explicit NetReposToolScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress PAL: 0x003c9e08
     */
    ~NetReposToolScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress PAL: 0x003c9ed0
     */
    static UIScreen *New(DataArray *pData) {
        return new NetReposToolScreen(pData);
    }

    /**
     * Route the controller to its handler.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress PAL: 0x0018e5b0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * List the remixes, and give the remix whose note was edited the new note.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0018e3e0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Exit, and give `fn_upload_note_tool` the note of the selected remix to edit.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0018e4c0
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Remove the remix whose file is an uploaded file.
     *
     * The name is inferred.
     *
     * @param info The uploaded file.
     * @return Whether a remix was removed.
     * @ghidraAddress PAL: 0x0018e618
     */
    bool RemoveRemix(const UploadInfo &info);

    /**
     * Add a remix at the end of the list.
     *
     * The name is inferred.
     *
     * @param remix The remix.
     * @ghidraAddress PAL: 0x0018e730
     */
    void AddRemix(const NetRepoRemix &remix);

    /** @return The remixes of the repository. */
    const std::list<NetRepoRemix> &GetRemixes() const {
        return mRemixes;
    }

    /** @param remixes The remixes of the repository. */
    void SetRemixes(const std::list<NetRepoRemix> &remixes) {
        mRemixes = remixes;
    }

    /** @param pszNote The note being edited. */
    void SetNote(const char *pszNote) {
        mNote = pszNote;
    }

private:
    /** The value of mSelection when no note is being edited. */
    static constexpr int kNoSelection = -1;

    /**
     * Move, remove, or save the remixes, or edit the note of the selected one.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of UIScreen::HandleJoypad().
     * @ghidraAddress PAL: 0x0018e808
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    ReposToolList *mList;             /*!< The list, from the last entry. */
    std::list<NetRepoRemix> mRemixes; /*!< The remixes of the repository. */
    String mNote;                     /*!< The note being edited. */
    int mSelection = kNoSelection;    // The remix whose note is being edited.
};

#endif
