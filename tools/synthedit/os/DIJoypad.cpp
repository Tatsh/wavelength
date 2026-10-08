#include "os/DIJoypad.h"

#include "os/Debug.h"

namespace {

const DWORD kUnsetJoystickId = 12345;
const LONG kAxisMax = 254;
const int kDeadzoneScale = 20000;
const int kDeadzoneDivisor = 256;
const DWORD kSaturation = 10000;
const DWORD kBufferSize = 256;

} // namespace

DIJoypad::DIJoypad(const DIDEVICEINSTANCE *instance) {
    mReserved0c = 0;
    mReserved10 = false;
    mDevice = NULL;
    mDevice2 = NULL;
    ASSERT(gLpdi != NULL);
    HRESULT hr = gLpdi->CreateDevice(instance->guidInstance, &mDevice, NULL);
    ASSERT(SUCCEEDED(hr));
    mDevice->QueryInterface(IID_IDirectInputDevice2, reinterpret_cast<void **>(&mDevice2));
    hr = mDevice->SetDataFormat(&c_dfDIJoystick);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(mDevice->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND))) {
        TheDebug.Fail("SetCooperativeLevel failed");
    }

    DIPROPDWORD dword;
    dword.diph.dwSize = sizeof(DIPROPDWORD);
    dword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dword.diph.dwHow = DIPH_DEVICE;
    dword.diph.dwObj = 0;
    dword.dwData = kUnsetJoystickId;
    hr = mDevice->GetProperty(DIPROP_JOYSTICKID, &dword.diph);
    ASSERT(SUCCEEDED(hr));
    mJoystickId = dword.dwData;

    DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYOFFSET;
    range.lMin = 0;
    range.lMax = kAxisMax;
    SetAxesProperty(DIPROP_RANGE, &range.diph);

    dword.diph.dwSize = sizeof(DIPROPDWORD);
    dword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dword.diph.dwHow = DIPH_BYOFFSET;
    dword.dwData = gDeadzone * kDeadzoneScale / kDeadzoneDivisor;
    SetAxesProperty(DIPROP_DEADZONE, &dword.diph);

    dword.diph.dwSize = sizeof(DIPROPDWORD);
    dword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dword.diph.dwHow = DIPH_BYOFFSET;
    dword.dwData = kSaturation;
    SetAxesProperty(DIPROP_SATURATION, &dword.diph);

    dword.diph.dwSize = sizeof(DIPROPDWORD);
    dword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dword.diph.dwObj = 0;
    dword.diph.dwHow = DIPH_DEVICE;
    dword.dwData = kBufferSize;
    hr = mDevice->SetProperty(DIPROP_BUFFERSIZE, &dword.diph);
    ASSERT(SUCCEEDED(hr));
    Acquire();
}

DIJoypad::~DIJoypad() {
    try {
        if (mDevice != NULL) {
            mDevice->Release();
            mDevice = NULL;
        }
        if (mDevice2 != NULL) {
            mDevice2->Release();
            mDevice2 = NULL;
        }
    } catch (...) {
    }
}

bool DIJoypad::Acquire() {
    if (mDevice == NULL) {
        return false;
    }
    const HRESULT hr = mDevice->Acquire();
    if (FAILED(hr)) {
        TheDebug << "Acquire error: ";
        PrintDIError(hr);
    }
    return SUCCEEDED(hr);
}

void DIJoypad::SetAxesProperty(REFGUID property, LPDIPROPHEADER header) {
    header->dwObj = DIJOFS_X;
    mDevice->SetProperty(property, header);
    header->dwObj = DIJOFS_Y;
    mDevice->SetProperty(property, header);
    header->dwObj = DIJOFS_Z;
    mDevice->SetProperty(property, header);
    header->dwObj = DIJOFS_RZ;
    mDevice->SetProperty(property, header);
}
