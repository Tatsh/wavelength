#include "os/CDReader.h"

#include <cstdio>

#include "os/Block.h"
#include "os/Debug.h"

int CDReadSectors(int sector, int numSectors, void *buffer) {
    FILE *f = fopen("gen/main.ark", "rb");
    ASSERT(f);
    fseek(f, sector * kSectorSize, SEEK_SET);
    fread(buffer, 1, numSectors * kSectorSize, f);
    fclose(f);
    return 0;
}

bool CDReadDone() {
    return true;
}

int CDGetError() {
    return 0;
}
