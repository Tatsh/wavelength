#include "netflow/allocatebuffertask.h"

#include "os/mem.h"

void AllocateBufferTask::OnStart() {
    *mBuffer = static_cast<char *>(PoolMemAlloc(*mSize, "FileDownload", 0));
    Finish(true, 0);
}
