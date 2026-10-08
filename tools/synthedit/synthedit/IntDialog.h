#pragma once

#include <afxwin.h>

#include "synthedit/Resource.h"
#include "utl/Str.h"

/**
 * Dialogue that prompts for a whole number from 0 to 10000.
 *
 * The class name is from the message map. The names of its members are inferred.
 */
class CIntDialog : public CDialog {
public:
    /**
     * Prepare the dialogue with a value of zero.
     *
     * @param parent The owner window, or null.
     * @ghidraAddress 0x100015f0
     */
    CIntDialog(CWnd *parent);

    /** @ghidraAddress 0x100018b0 */
    virtual ~CIntDialog();

    /**
     * Show the dialogue.
     *
     * @return The number entered, or -1 when the dialogue was cancelled.
     * @ghidraAddress 0x100016e7
     */
    virtual int DoModal();

    /**
     * Set the caption the dialogue shows.
     *
     * @param title The caption.
     * @ghidraAddress 0x100069f0
     */
    void SetTitle(String title) {
        mTitle = title;
    }

    /** The dialogue template. */
    enum { IDD = IDD_INTDIALOG };

protected:
    /**
     * Exchange the number with the edit control, limiting it to 0 to 10000.
     *
     * @param pDX The exchange.
     * @ghidraAddress 0x1000165c
     */
    virtual void DoDataExchange(CDataExchange *pDX);

    /**
     * Show the caption.
     *
     * @return One, to let the system set the focus.
     * @ghidraAddress 0x100016bb
     */
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    int mValue;    /*!< The number entered. */
    String mTitle; /*!< The caption. */
};
