#include "os/Archive.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/Cheats.h"
#include "utl/Data.h"
#include "utl/DataFile.h"
#include "utl/DataString.h"
#include "utl/EmbeddedFileTable.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/OptionProcessor.h"
#include "utl/Rand.h"
#include "utl/Trig.h"

namespace {

// 0x1003f92c
const char *gSystemLanguage;

// 0x1003f930
char **gArgv;

// 0x1000a640
void SystemLanguageInit() {
    SystemConfig()->FindArray("system", true)->FindSymbol("language", &gSystemLanguage, false);
    if (gSystemLanguage == NULL) {
        gSystemLanguage = "english";
    }
}

} // namespace

void SystemInit(const char *commandLine, const char *configFile) {
    ASSERT(commandLine);
    int argc;
    gArgv = OptionProcessor::ParseCommandLine(commandLine, &argc);
    SystemInit(argc, gArgv, configFile);
}

void SystemInit(int argc, char **argv, const char *configFile) {
    DebugPrint("SystemInit(%s)\n", configFile);
    ASSERT(argv);
    bool cd = false;
    SystemOptions()->AddBoolOption("cd", &cd, true);
    SystemProcessOptions(argc, argv);
    SetUsingCD(cd);
    SeedRand(0);
    TimerInit();
    DataStringInit();
    FileLogInit(gFileOrder.c_str());
    if (gFileOrder.size() != 0) {
        DataSetUseCompiled();
    }
    ArchiveInit();
    const char *path = argc != 0 ? FileGetPath(argv[0]) : ".";
    SystemConfigInit(path, configFile);
    MemInit();
    ArchivePreInit();
    TrigInit();
    TimerConfigInit();
    EmptyRoutine();
    FileInit();
    TimerPoll();
    JoypadInit();
    CheatsInit();
    PadMapInit();
    EmptyRoutine();
    SystemLanguageInit();
}

void SystemTerminate() {
    MemFree(gArgv);
    TheLocale.Terminate();
    PadMapTerminate();
    CheatsTerminate();
    JoypadTerminate();
    TimerTerminate();
    EmbeddedFileTable::ClearAll();
    EmptyRoutine();
    EmptyRoutine();
    TrigTerminate();
    FileTerminate();
    SystemConfig()->Release();
    DataStringTerminate();
    EmptyRoutine();
}
