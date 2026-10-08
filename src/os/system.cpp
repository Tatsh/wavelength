#include "os/system.h"

#include <cstdio>
#include <cstring>

#include <eekernel.h>
#include <libcdvd.h>
#include <libmtap.h>
#include <libscf.h>
#include <sifdev.h>
#include <sifrpc.h>

#include "math/rand.h"
#include "math/sine.h"
#include "memcard/memcard.h"
#include "os/archive.h"
#include "os/asynctask.h"
#include "os/cheatsmanager.h"
#include "os/datetime.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/irx.h"
#include "os/joypad.h"
#include "os/keyboard.h"
#include "os/locale.h"
#include "os/mem.h"
#include "os/namedobjectregistry.h"
#include "os/optionprocessor.h"
#include "os/string.h"

namespace {

// Controller ports a multitap can be connected to.
constexpr int kMultitapPortCount = 2;

// Ports MultitapInit() opens.
constexpr int kMultitapOpenCount = 4;

// The sceMtapGetConnection() result of a connected multitap.
constexpr int kMultitapConnected = 1;

// Seconds between two multitap checks.
constexpr float kMultitapCheckSeconds = 1.5F;

constexpr float kMillisecondsPerSecondFloat = 1000.0F;

// Bytes of the configuration path.
constexpr int kConfigPathSize = 256;

// The priority main() runs at.
constexpr int kMainThreadPriority = 2;

constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerHour = 3600;

// Settings a tool console reports, for a console in the United States.
constexpr short kToolTimezone = -300;
constexpr unsigned char kToolReserved2 = 1;
constexpr unsigned char kToolReserved3 = 1;
constexpr unsigned char kToolReserved5 = 0x50;
constexpr unsigned char kToolSummerTime = 1;

// The language keys of the configuration and of the console settings, by language code.
const char *const kLanguageKeys[] = {
    "japanese", "english", "french", "spanish", "german", "italian"};

// NTSC-U/C: 0x003b2120
int gPadCheck;

// Whether a multitap was connected to each port at the last check.
// NTSC-U/C: 0x00481980
int gMultitaps[kMultitapPortCount];

// The system time in seconds of the next multitap check.
// NTSC-U/C: 0x003b211c
float gNextMultitapCheck;

// NTSC-U/C: 0x0028c6b0, PAL: 0x00296038 (static initialiser)
// NTSC-U/C: 0x0028c768, PAL: 0x002960f0 (constructor call)
// NTSC-U/C: 0x0028c788, PAL: 0x00296110 (destructor call)
// NTSC-U/C: 0x00481988
OptionProcessor gOptions;

// The `file_order` option, the path of the file read order log.
// NTSC-U/C: 0x004819a0
String gFileOrder;

// NTSC-U/C: 0x003b2230
int gUsingCD;

// NTSC-U/C: 0x003b2130
char gConfigPath[kConfigPathSize];

// NTSC-U/C: 0x003b2128
DataArray *gConfig;

// NTSC-U/C: 0x003b2238
const char *gLanguage;

// NTSC-U/C: 0x003b223c
char *gDebugBuffer; // Yes, the binary never sets it.

// Open the multitap ports and check for multitaps from now on.
// NTSC-U/C: 0x0028c2a8, PAL: 0x00295bf8
void MultitapInit() {
    sceMtapInit();
    for (int i = 0; i < kMultitapOpenCount; ++i) {
        sceMtapPortOpen(i);
    }
    gMultitaps[0] = 0;
    gMultitaps[1] = 0;
    gPadCheck = 1;
    gNextMultitapCheck = SystemMs() / kMillisecondsPerSecondFloat;
}

// NTSC-U/C: 0x0028c378, PAL: 0x00295cc8
void MultitapTerminate() {
    for (int i = 0; i < kMultitapOpenCount; ++i) {
        sceMtapPortClose(i);
    }
}

// Every 1.5 seconds, remap the controllers and the memory cards when a multitap was connected or
// disconnected.
// NTSC-U/C: 0x0028c3b0, PAL: 0x00295d00
void MultitapPoll() {
    if (gPadCheck == 0) {
        return;
    }
    const float fNow = SystemMs() / kMillisecondsPerSecondFloat;
    if (fNow < gNextMultitapCheck) {
        return;
    }
    gNextMultitapCheck = fNow + kMultitapCheckSeconds;
    const int bPort0 = sceMtapGetConnection(0) == kMultitapConnected;
    const int bPort1 = sceMtapGetConnection(1) == kMultitapConnected;
    if (gMultitaps[0] == bPort0 && gMultitaps[1] == bPort1) {
        return;
    }
    gMultitaps[0] = bPort0;
    gMultitaps[1] = bPort1;
    JoypadMapDefault(bPort0 != 0, bPort1 != 0);
    MemcardAssignSlots(bPort0 != 0, bPort1 != 0);
}

// NTSC-U/C: 0x0028c4e0, PAL: 0x00295e30
void ParseCommandLine(int argc, char **argv) {
    gOptions.AddBool("host_config", &g_bHostConfig, 1);
    gOptions.AddString("file_order", &gFileOrder);
    gOptions.Process(argc, argv);
}

// Read the configuration file at gConfigPath, from the host when `host_config` asks for it.
// NTSC-U/C: 0x0028c5c8, PAL: 0x00295f50
DataArray *ReadConfigFile() {
    const bool bUsingCD = UsingCD();
    if (g_bHostConfig != 0) {
        SetUsingCD(false);
    }
    if (bUsingCD && g_bHostConfig != 0) {
        DebugPrint("WARNING: reading Config files from host0\n");
    }
    DataArray *pConfig = DataArray::Read(gConfigPath, nullptr);
    SetUsingCD(bUsingCD);
    return pConfig;
}

// NTSC-U/C: 0x0028c648, PAL: 0x00295fd0
void LoadConfigFile(const char *pszDirectory, const char *pszFile) {
    if (pszFile == nullptr) {
        pszFile = "default_config.txt";
    }
    sprintf(gConfigPath, "%s/%s", pszDirectory, pszFile);
    gConfig = ReadConfigFile();
}

// NTSC-U/C: 0x0028c7a8, PAL: 0x00296130
void UseCD() {
    SetUsingCD(true);
}

// Start the disc drive, reboot the IOP with the game's IOP image, and lower the main thread's
// priority.
// NTSC-U/C: 0x0028c7c8, PAL: 0x00296150
void PrepareIop() {
    int nMedia = SCECdCD;
    sceSifInitRpc(0);
    sceCdInit(SCECdINIT);
    sceCdMmode(SCECdCD);
    switch (sceCdGetDiskType()) {
    case SCECdPS2CD:
        SetUsingCD(true);
        break;
    case SCECdPS2DVD:
        nMedia = SCECdDVD;
        sceCdMmode(SCECdDVD);
        SetUsingCD(true);
        break;
    case SCECdNODISC:
        SetUsingCD(false);
        break;
    default:
        break;
    }
    if (kImageFirstWord == 0) {
        String image;
        MakeDevicePath(image, "iop/ioprp260.img");
        while (sceSifRebootIop(image.c_str()) == 0) {
        }
        while (sceSifSyncIop() == 0) {
        }
        sceFsReset();
        sceSifInitRpc(0);
        sceSifInitIopHeap();
        sceCdInit(SCECdINIT);
        sceCdMmode(nMedia);
    }
    UseCD(); // Yes, the binary overrides the disc type check.
    ChangeThreadPriority(GetThreadId(), kMainThreadPriority);
}

// Choose the language of the `language` setting of the `system` block, report it to the console
// library as a tool console's setting, and take the console's language.
// NTSC-U/C: 0x0028c940, PAL: 0x002962f0
void ChooseLanguage() {
    const DataArray *pSystem = SystemConfig()->FindArray("system", true);
    sceScfT10kConfig config;
    config.nTimezone = kToolTimezone;
    config.abReserved2[0] = kToolReserved2;
    config.abReserved2[1] = kToolReserved3;
    config.nReserved5 = kToolReserved5;
    config.nSummerTime = kToolSummerTime;
    config.nReserved7 = 0;
    const char *pszLanguage = nullptr;
    pSystem->FindSymbol("language", &pszLanguage, false);
    config.nLanguage = SCE_ENGLISH_LANGUAGE;
    if (pszLanguage != nullptr) {
        for (int i = SCE_JAPANESE_LANGUAGE; i <= SCE_ITALIAN_LANGUAGE; ++i) {
            if (i != SCE_ENGLISH_LANGUAGE && strcmp(kLanguageKeys[i], pszLanguage) == 0) {
                config.nLanguage = static_cast<unsigned char>(i);
                break;
            }
        }
    }
    sceScfSetT10kConfig(&config);
    const int nLanguage = sceScfGetLanguage();
    gLanguage = nLanguage >= SCE_JAPANESE_LANGUAGE && nLanguage <= SCE_ITALIAN_LANGUAGE ?
                    kLanguageKeys[nLanguage] :
                    kLanguageKeys[SCE_ENGLISH_LANGUAGE];
}

// NTSC-U/C: 0x0028cb08, PAL: 0x002964b8
void RunStartupHook() {
}

// NTSC-U/C: 0x0028cb10, PAL: 0x002964c0
void FreeDebugBuffer() {
    if (gDebugBuffer != nullptr) {
        delete[] gDebugBuffer;
    }
}

} // namespace

// NTSC-U/C: 0x003b2234
int g_bHostConfig;

void SystemSetPadCheck(bool bEnabled) {
    gPadCheck = bEnabled;
}

int JoypadMultitapConnected() {
    return gMultitaps[0];
}

void SystemPoll() {
    MultitapPoll();
    JoypadPoll();
    KeyboardPoll();
    MemPoll();
    DateTime::PollClock();
    PollAsyncTasks();
}

bool UsingCD() {
    return gUsingCD != 0;
}

void SetUsingCD(bool bUsingCD) {
    gUsingCD = bUsingCD;
}

DataArray *SystemConfig() {
    return gConfig;
}

const char *GetSystemLanguage() {
    return gLanguage;
}

void SystemInit(int argc, char **argv, const char *pszConfigFile) {
    DebugError("SystemInit(%s)\n", pszConfigFile);
    ParseCommandLine(argc, argv);
    TimerInit();
    PrepareIop();
    DataReserveSymbols();
    Archive::Init();
    DateTime now{};
    now.ReadClock();
    SeedRand(now.mSecond + (now.mMinute * kSecondsPerMinute) + (now.mHour * kSecondsPerHour));
    FileOpenTraceLog(gFileOrder.c_str());
    if (gFileOrder.mLength != 0) {
        DataSetCompiled();
    }
    LoadConfigFile(".", pszConfigFile);
    MemConfigureHeaps();
    InitAsyncTasks();
    SinTableInit();
    TimerLoadNames();
    NamedObjectRegistryInit();
    FileInit();
    IrxLoadAll(SystemConfig()->FindArray("system", true)->FindArray("iop_modules", true));
    MultitapInit();
    JoypadInit();
    KeyboardInit();
    CheatsManager::Init();
    MemcardInit();
    LocalePrepare();
    RunStartupHook();
    ChooseLanguage();
    FileSetBootPath(argv[0]);
}

void SystemTerminate() {
    TheLocale.Terminate();
    MemcardTerminate();
    CheatsManager::Terminate();
    KeyboardTerminate();
    JoypadTerminate();
    MultitapTerminate();
    NamedObjectRegistryTerminate();
    TimerTerminate();
    SinTableTerminate();
    FileTerminate();
    MemTerminate();
    TerminateAsyncTasks();
    SystemConfig()->Release();
    DataTerminate();
    FreeDebugBuffer();
}
