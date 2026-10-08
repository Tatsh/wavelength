#pragma once

#include <afxwin.h>

#include <shlobj.h>

/**
 * Shell dialogue that picks a folder, showing the current selection as its status text.
 *
 * The object is 0x238 bytes. The class has a virtual table but no RTTI, so the names of its
 * members are inferred.
 */
class CFolderDialog {
public:
    /**
     * Prepare the dialogue.
     *
     * @param initialFolder The folder selected first, or null for none.
     * @param flags The `BIF_` flags. BIF_STATUSTEXT is always added.
     * @param parent The owner window, or null.
     * @ghidraAddress 0x10001000
     */
    CFolderDialog(LPCTSTR initialFolder, UINT flags, CWnd *parent);

    /** @ghidraAddress 0x10001121 */
    virtual ~CFolderDialog();

    /**
     * Show the dialogue.
     *
     * @return `IDOK` when a folder with a file system path was chosen, otherwise `IDCANCEL`.
     * @ghidraAddress 0x100011c3
     */
    virtual int DoModal();

    /**
     * Report the chosen folder, or the initial folder when none was chosen.
     *
     * @return The folder.
     * @ghidraAddress 0x10001272
     */
    CString GetSelectedFolder();

protected:
    /**
     * Select the initial folder once the dialogue is open.
     *
     * @ghidraAddress 0x1000142a
     */
    virtual void OnInitialized();

    /**
     * Record a new selection.
     *
     * @param pidl The selected item.
     * @ghidraAddress 0x100014ad
     */
    virtual void OnSelChanged(LPITEMIDLIST pidl);

    /**
     * Dispatch a message from the dialogue.
     *
     * @param hwnd The dialogue.
     * @param msg The `BFFM_` message.
     * @param lParam The message's argument.
     * @ghidraAddress 0x10001178
     */
    virtual void OnBrowseMessage(HWND hwnd, UINT msg, LPARAM lParam);

    /**
     * Enable or disable the OK button.
     *
     * @param enable Whether to enable it.
     * @ghidraAddress 0x100012a3
     */
    void EnableOK(BOOL enable);

    /**
     * Select a folder by path.
     *
     * @param path The folder.
     * @ghidraAddress 0x100012d1
     */
    void SetSelection(LPCTSTR path);

    /**
     * Select a folder by item.
     *
     * @param pidl The folder.
     * @ghidraAddress 0x100012f9
     */
    void SetSelection(LPCITEMIDLIST pidl);

    /**
     * Show text above the folder tree.
     *
     * @param text The text.
     * @ghidraAddress 0x10001321
     */
    void SetStatusText(LPCTSTR text);

    /**
     * Shorten a path to fit the status text: its first 35 characters and an ellipsis.
     *
     * @param path The path.
     * @return The path, shortened when longer than 35 characters.
     * @ghidraAddress 0x10001349
     */
    CString ShortenPath(const CString &path);

private:
    /**
     * Forward a message from the dialogue to its CFolderDialog.
     *
     * @param hwnd The dialogue.
     * @param msg The message.
     * @param lParam The message's argument.
     * @param lpData The CFolderDialog.
     * @return Zero.
     * @ghidraAddress 0x100010f8
     */
    static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData);

    BROWSEINFO mBrowseInfo;       /*!< The dialogue's parameters. */
    CString mInitialFolder;       /*!< The folder selected first. */
    CString mSelectedFolder;      /*!< The chosen folder. */
    TCHAR mDisplayName[MAX_PATH]; /*!< The display name of the chosen item. */
    TCHAR mPath[MAX_PATH];        /*!< The path of the selected folder. */
    HWND mWnd;                    /*!< The open dialogue. */
};
