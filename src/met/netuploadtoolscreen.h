#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <list>

#include "met/freqscreen.h"
#include "met/uploadinfo.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netreporemix.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen `fn_upload_tool` that lists the files of the player's upload directory on the server.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x00463f20`. The
 * buttons sort the files, download a file into the repository, preview a file as a remix or as a
 * game, delete a file, and go to the repository tool. The metagame registers the class for the
 * screen type `upload_tool_screen`.
 */
class NetUploadToolScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no files.
     *
     * @param pData The script description.
     * @ghidraAddress PAL: 0x0018d0b8
     */
    explicit NetUploadToolScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress PAL: 0x003c9d08
     */
    ~NetUploadToolScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress PAL: 0x003c9dc8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetUploadToolScreen(pData);
    }

    /**
     * Route the choice and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress PAL: 0x0018d618
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter as an online game, and list the files with the item selected on exit.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0018d478
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Remember the selected item and exit.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress PAL: 0x0018d4e0
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Replace the files, marking each one whose name is the file of a remix of the repository.
     *
     * The name is inferred.
     *
     * @param files The names of the files.
     * @param remixes The remixes of the repository.
     * @ghidraAddress PAL: 0x0018d140
     */
    void SetFiles(std::list<String> &files, std::list<NetRepoRemix> &remixes);

    /**
     * Mark again each file whose name is the file of a remix of the repository.
     *
     * The name is inferred.
     *
     * @param remixes The remixes of the repository.
     * @ghidraAddress PAL: 0x0018d340
     */
    void MarkRemixes(std::list<NetRepoRemix> &remixes);

    /**
     * Remove a file from the list, when the list has it.
     *
     * The name is inferred.
     *
     * @param nIndex The file.
     * @ghidraAddress PAL: 0x0018d550
     */
    void RemoveFile(int nIndex);

private:
    /** The parts of two file names ParseNames() compares. */
    enum NamePart {
        kNamePartDate = 0,  /*!< The dates. */
        kNamePartSong = 1,  /*!< The songs. */
        kNamePartTitle = 2, /*!< The titles. */
    };

    /**
     * Copy one part of each of two file names, without terminators, into zeroed buffers.
     *
     * The name is inferred.
     *
     * @param nPart The part, one of NamePart.
     * @param pszA The first file name.
     * @param pszB The second file name.
     * @param pszPartA The buffer for the part of the first name.
     * @param pszPartB The buffer for the part of the second name.
     * @ghidraAddress PAL: 0x0018d6a8
     */
    static void
    ParseNames(int nPart, const char *pszA, const char *pszB, char *pszPartA, char *pszPartB);

    /**
     * Order two files by their songs.
     *
     * The name is inferred.
     *
     * @param a The first file.
     * @param b The second file.
     * @return Whether a sorts before b.
     * @ghidraAddress PAL: 0x0018d9f8
     */
    static bool LessBySong(const UploadInfo &a, const UploadInfo &b);

    /**
     * Order two files by their titles.
     *
     * The name is inferred.
     *
     * @param a The first file.
     * @param b The second file.
     * @return Whether a sorts before b.
     * @ghidraAddress PAL: 0x0018dab0
     */
    static bool LessByTitle(const UploadInfo &a, const UploadInfo &b);

    /**
     * Order two files by their dates.
     *
     * The name is inferred.
     *
     * @param a The first file.
     * @param b The second file.
     * @return Whether a sorts before b.
     * @ghidraAddress PAL: 0x0018db68
     */
    static bool LessByDate(const UploadInfo &a, const UploadInfo &b);

    /**
     * Order the files of the repository before the others.
     *
     * The name is inferred.
     *
     * @param a The first file.
     * @param b The second file.
     * @return Whether a is in the repository and b is not.
     * @ghidraAddress PAL: 0x0018dc20
     */
    static bool InReposFirst(const UploadInfo &a, const UploadInfo &b);

    /**
     * Download the chosen file into the repository, or sort the files by the chosen button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress PAL: 0x0018dc30
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Delete, preview, or page through the files, or go to the repository tool.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition or when the focus moved to the list, otherwise the result
     * of UIScreen::HandleJoypad().
     * @ghidraAddress PAL: 0x0018de98
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    std::list<UploadInfo> mFiles; // The files of the upload directory.
    int mSelection = 0;           // The item selected on exit.
};

#endif
