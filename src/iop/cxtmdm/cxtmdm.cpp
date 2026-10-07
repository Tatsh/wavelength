#include "cxtmdm/cxtmdm.h"

#include <kernel.h>
#include <sysclib.h>
#include <usbd.h>

#include "cxtmdm/cxtmodem.h"

namespace {

constexpr unsigned short kModuleVersion = 0x1016;
constexpr int kTestLoadFound = 5;
constexpr char kDialOption[] = "dial=";
constexpr int kDialOptionLength = 5;
constexpr char kLoadModeOption[] = "lmode";
constexpr char kAutoLoadValue[] = "AUTOLOAD";
constexpr char kTestLoadValue[] = "TESTLOAD";

// NTSC-U/C: 0x00002a9c
[[gnu::used]] constexpr char kRevisionId[] = "$Id: USB_Modem_Driver/Conexant 0x1016 2001/09/17 $";

// NTSC-U/C: 0x000034a4
UsbdLddOps g_driver = {
    nullptr,
    nullptr,
    "Conexant USB Modem Driver",
    CxtModem::Probe,
    CxtModem::Connect,
    CxtModem::Disconnect,
    {},
};

} // namespace

// NTSC-U/C: 0x000034e0
ModuleInfo Module = {"USB_Modem_Driver/Conexant", kModuleVersion};

int start(int argc, char **argv) {
    CxtModem::sLoadMode = CxtModem::kLoadModeNormal;
    CxtModem::sDialString[0] = '\0';
    for (int i = 0; i < argc; ++i) {
        if (strncmp(kDialOption, argv[i], kDialOptionLength) == 0) {
            strcpy(CxtModem::sDialString, &argv[i][kDialOptionLength]);
        }
        // Split the argument at its first equals sign.
        int value = 0;
        for (char *c = argv[i]; *c != '\0'; ++c) {
            ++value;
            if (*c == '=') {
                *c = '\0';
                break;
            }
        }
        if (strcmp(argv[i], kLoadModeOption) != 0) {
            continue;
        }
        if (strcmp(&argv[i][value], kAutoLoadValue) == 0) {
            CxtModem::sLoadMode = CxtModem::kLoadModeAuto;
            break;
        }
        if (strcmp(&argv[i][value], kTestLoadValue) == 0) {
            CxtModem::sLoadMode = CxtModem::kLoadModeTest;
            break;
        }
    }
    switch (CxtModem::sLoadMode) {
    case CxtModem::kLoadModeNormal:
        CxtModem::sDeviceFound = true;
        break;
    case CxtModem::kLoadModeAuto:
    case CxtModem::kLoadModeTest:
        CxtModem::sDeviceFound = false;
        break;
    }
    // Registration probes the attached devices, and CxtModem::Probe() records a modem it finds.
    sceUsbdRegisterLdd(&g_driver);
    if (CxtModem::sLoadMode == CxtModem::kLoadModeTest) {
        sceUsbdUnregisterLdd(&g_driver);
        return CxtModem::sDeviceFound ? kTestLoadFound : NO_RESIDENT_END;
    }
    if (!CxtModem::sDeviceFound) {
        sceUsbdUnregisterLdd(&g_driver);
        return NO_RESIDENT_END;
    }
    return RESIDENT_END;
}
