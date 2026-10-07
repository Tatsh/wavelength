#include "softfx_s/streambuffer.h"

#include <kernel.h>
#include <sif.h>
#include <sysclib.h>

namespace {

// The free space a newly set status address receives.
constexpr int kInitialStatus = 1;
constexpr int kStatusTransferSize = 16;
constexpr int kStatusTransferCount = 1;

} // namespace

StreamBuffer::StreamBuffer([[maybe_unused]] unsigned int size)
    : RingBuffer(kSize), mReserved(0), mStatusAddress(nullptr) {
}

int StreamBuffer::SendStatus(int value, void *address) {
    StatusPacket packet;
    memset(&packet, 0, sizeof(packet));
    packet.mValue = value;
    sceSifDmaData transfer;
    memset(&transfer, 0, sizeof(transfer));
    transfer.data = &packet;
    transfer.addr = address;
    transfer.size = kStatusTransferSize;
    transfer.mode = 0;
    int state;
    CpuSuspendIntr(&state);
    const int id = static_cast<int>(sceSifSetDma(&transfer, kStatusTransferCount));
    CpuResumeIntr(state);
    return id;
}

void StreamBuffer::SetStatusAddress(void *address) {
    mStatusAddress = address;
    SendStatus(kInitialStatus, address);
}

void StreamBuffer::Write(const void *data, unsigned int size, bool endOfStream) {
    RingBuffer::Write(data, size, endOfStream);
    mNotifyPending = true;
}

void StreamBuffer::Notify() {
    if (!mNotifyPending) {
        return;
    }
    const int free = static_cast<int>(FreeSpace());
    if (free > 0) {
        SendStatus(free, mStatusAddress);
        mNotifyPending = false;
    }
}

StreamBuffer::~StreamBuffer() {
}
