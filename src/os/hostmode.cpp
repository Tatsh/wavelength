#include "os/hostmode.h"

#include <libscf.h>
#include <sifdev.h>

#include "os/iop.h"
#include "os/log.h"
#include "os/mem.h"
#include "os/zone.h"

namespace {

// The boot options are nine consecutive words from 0x0070bf10, each with an accessor of the same
// three-instruction shape, followed by the version string. Both writers of the block are in this
// translation unit, ConfigureRetailBoot() and ForceCdOnlyBoot().

// The value of the file-service open flag that opens for reading only.
constexpr int kOpenReadOnly = 1;

int g_nHostMode = 0;

int g_nUsingArkFiles = 0;

int g_nWarningsEnabled = 1;

// No routine writes this word. ConfigureRetailBoot() omits it from the block.
int g_nScreenMessagesEnabled = 1;

int g_nUsingCdMedia = 0;

int g_nDebugKeysEnabled = 0;

int g_nMidiErrorLogEnabled = 0;

int g_nMemAccountingEnabled = 0;

int g_nIntroMovieEnabled = 1;

#ifdef VIDEO_STANDARD_PAL
HxStr g_versionString("197");
#else
HxStr g_versionString("198");
#endif

#ifdef VIDEO_STANDARD_PAL
// The language code GetLanguage() reports. The name is inferred.
// PAL: 0x0074fad0
int g_nLanguage = SCE_FRENCH_LANGUAGE;
#endif

// Report that no configuration file was found, force the disc configuration, and open the memory
// report. InitBootConfig() is the one caller.
void ForceCdOnlyBoot() {
    printf(" Running from CD only, since we couldn't find the config file\n");
    g_nHostMode = kHostModeCdOnly;
    printf(" Running from CD ONLY, forcing arkfiles ON and async ON\n");
    g_nUsingArkFiles = 1;
    g_nUsingCdMedia = 1;
    MemOpenLog(nullptr);
}

} // namespace

HostMode GetHostMode() {
    return static_cast<HostMode>(g_nHostMode);
}

int UsingArkFiles() {
    return g_nUsingArkFiles;
}

int WarningsEnabled() {
    return g_nWarningsEnabled;
}

int ScreenMessagesEnabled() {
    return g_nScreenMessagesEnabled;
}

int UsingCdMedia() {
    return g_nUsingCdMedia;
}

HxStr GetFreqRoot() {
    return HxStr("");
}

HxStr MakeFreqPath(const HxStr &name) {
    HxStr path(GetFreqRoot());
    path += name;
    return path;
}

int DebugKeysEnabled() {
    return g_nDebugKeysEnabled;
}

int MidiErrorLogEnabled() {
    return g_nMidiErrorLogEnabled;
}

int MemAccountingEnabled() {
    return g_nMemAccountingEnabled;
}

int IntroMovieEnabled() {
    return g_nIntroMovieEnabled;
}

#ifdef VIDEO_STANDARD_PAL
int GetLanguage() {
    return g_nLanguage;
}

void SetLanguage(int nLanguage) {
    g_nLanguage = nLanguage;
}
#endif

HxStr GetVersionString() {
    return g_versionString;
}

void ConfigureRetailBoot() {
    g_nHostMode = kHostModeCdOnly;
    g_nIntroMovieEnabled = 1;
    g_nUsingArkFiles = 1;
    g_nWarningsEnabled = 0;
    g_nUsingCdMedia = 1;
    g_nDebugKeysEnabled = 0;
    g_nMidiErrorLogEnabled = 0;
    g_nMemAccountingEnabled = 0;
}

void InitBootConfig() {
    ForceCdOnlyBoot();
    InitializeZoneList();
#ifdef VIDEO_STANDARD_PAL
    SetLanguage(sceScfGetLanguage());
#endif
}

void TerminateBootConfig() {
    ReleaseAllZoneSlots();
}

bool CheckCDFile(const char *pszPath) {
    const int nDescriptor = sceOpen(pszPath, kOpenReadOnly);
    if (nDescriptor < 0) {
        return false;
    }
    sceClose(nDescriptor);
    return true;
}
