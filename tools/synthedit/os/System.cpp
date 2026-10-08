#include "os/System.h"

#include <cstdio>

#include "os/Debug.h"
#include "utl/Data.h"
#include "utl/DataFile.h"

namespace {

// 0x1003fdb0
OptionProcessor gOptionProcessor;

// 0x1003fdd4
DataArray *gSystemConfig;

// 0x1003fdd8
char gConfigFile[256];

// 0x1003fed8
int gUsingCD;

// 0x1003fedc
bool gHostConfig;

// 0x1000b6e0
DataArray *ReadSystemConfig() {
    const bool usingCD = UsingCD();
    if (gHostConfig) {
        SetUsingCD(false);
    }
    if (usingCD && gHostConfig) {
        TheDebug.Printf("WARNING: reading Config files from host0\n");
    }
    DataArray *config = DataReadFile(gConfigFile, NULL);
    SetUsingCD(usingCD);
    return config;
}

} // namespace

String gFileOrder;

void SystemProcessOptions(int argc, char **argv) {
    gOptionProcessor.AddBoolOption("host_config", &gHostConfig, true);
    gOptionProcessor.AddStringOption("file_order", &gFileOrder);
    gOptionProcessor.Process(argc, argv);
}

bool UsingCD() {
    return gUsingCD != 0;
}

void SetUsingCD(bool usingCD) {
    gUsingCD = usingCD;
}

void SystemConfigInit(const char *path, const char *file) {
    if (file == NULL) {
        file = "default_config.txt";
    }
    ASSERT(path);
    sprintf(gConfigFile, "%s/%s", path, file);
    gSystemConfig = ReadSystemConfig();
    ASSERT(gSystemConfig);
}

OptionProcessor *SystemOptions() {
    return &gOptionProcessor;
}

DataArray *SystemConfig() {
    return gSystemConfig;
}
