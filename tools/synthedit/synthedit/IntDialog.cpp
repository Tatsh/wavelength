#include "synthedit/IntDialog.h"

namespace {

// The range of the number.
const int kMinValue = 0;
const int kMaxValue = 10000;

// DoModal() reports a cancelled dialogue with this value.
const int kCancelled = -1;

} // namespace

CIntDialog::CIntDialog(CWnd *parent) : CDialog(IDD, parent) {
    mValue = 0;
}

void CIntDialog::DoDataExchange(CDataExchange *pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, mValue);
    DDV_MinMaxInt(pDX, mValue, kMinValue, kMaxValue);
}

BEGIN_MESSAGE_MAP(CIntDialog, CDialog)
END_MESSAGE_MAP()

BOOL CIntDialog::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(mTitle);
    return TRUE;
}

int CIntDialog::DoModal() {
    int result = CDialog::DoModal();
    if (result == IDOK) {
        result = mValue;
    } else {
        result = kCancelled;
    }
    return result;
}

CIntDialog::~CIntDialog() {
}
