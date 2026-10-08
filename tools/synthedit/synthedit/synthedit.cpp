#include "synthedit/synthedit.h"

// 0x10031658
const GUID CDECL BASED_CODE _tlid = {
    0xb26cee76, 0xeb96, 0x4863, { 0x82, 0xac, 0xa2, 0x4d, 0x7f, 0x64, 0xcb, 0x93 }
};

// 0x10031668
const WORD _wVerMajor = 1;

// 0x1003166a
const WORD _wVerMinor = 0;

CSyntheditApp NEAR theApp;

BOOL CSyntheditApp::InitInstance() {
    BOOL bInit = COleControlModule::InitInstance();
    return bInit;
}

int CSyntheditApp::ExitInstance() {
    return COleControlModule::ExitInstance();
}

// 0x1000200d
STDAPI DllRegisterServer(void) {
    AFX_MANAGE_STATE(_afxModuleAddrThis);
    if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid)) {
        return ResultFromScode(SELFREG_E_TYPELIB);
    }
    if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE)) {
        return ResultFromScode(SELFREG_E_CLASS);
    }
    return NOERROR;
}

// 0x10002082
STDAPI DllUnregisterServer(void) {
    AFX_MANAGE_STATE(_afxModuleAddrThis);
    if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor)) {
        return ResultFromScode(SELFREG_E_TYPELIB);
    }
    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE)) {
        return ResultFromScode(SELFREG_E_CLASS);
    }
    return NOERROR;
}
