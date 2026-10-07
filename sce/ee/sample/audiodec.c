#include "ezmpeg/audiodec.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <eekernel.h>
#include <ezmpeg.h>
#include <libsdr.h>
#include <sifdev.h>

enum {
    kFullMasterVolume = 0x3fff,
    kFullInputVolume = 0x7fff,
    kSilentVolume = 0,
    kBlockSize = 1024,
    kPositionMask = 0xffffff,
    kCoreCount = 2,
    kSoundChannel = 0,
    kWait = 1,
    // The sound processor address the cleared buffer is uploaded to.
    kZeroSpuAddress = 0x4000,
};

// NTSC-U/C: 0x001b02f8, PAL: 0x001b9098
static int sendToIOP(int dst, unsigned char *src, int size) {
    sceSifDmaData transfer;
    unsigned int id;

    if (size <= 0) {
        return 0;
    }
    transfer.data = (unsigned int)(uintptr_t)src;
    transfer.addr = (unsigned int)dst;
    transfer.size = (unsigned int)size;
    transfer.mode = 0;
    FlushCache(WRITEBACK_DCACHE);
    id = sceSifSetDma(&transfer, 1);
    while (sceSifDmaStat(id) >= 0) {
    }
    return size;
}

// NTSC-U/C: 0x001b0378, PAL: 0x001b9118
static void changeMasterVolume(int val) {
    int core;

    for (core = 0; core < kCoreCount; ++core) {
        sceSdRemote(kWait, rSdSetParam, core | SD_P_MVOLL, val);
        sceSdRemote(kWait, rSdSetParam, core | SD_P_MVOLR, val);
    }
}

// NTSC-U/C: 0x001b03e0, PAL: 0x001b9180
static void changeInputVolume(int val) {
    sceSdRemote(kWait, rSdSetParam, SD_CORE_0 | SD_P_AVOLL, val);
    sceSdRemote(kWait, rSdSetParam, SD_CORE_0 | SD_P_AVOLR, val);
}

// NTSC-U/C: 0x001b00e0, PAL: 0x001b8e80
// Describe the free part of the IOP ring ahead of the write offset as up to two spans, keeping one
// block between the write offset and the play position.
static void iopGetArea(int *pd0, int *d0, int *pd1, int *d1, AudioDec *ad, int pos) {
    int diff = (pos + ad->iopBuffSize - ad->iopLastPos - kBlockSize) % ad->iopBuffSize;

    if ((unsigned int)(pos - ad->iopLastPos) < kBlockSize) {
        *pd0 = ad->iopBuff;
        *d0 = 0;
        *pd1 = ad->iopBuff;
        *d1 = 0;
        return;
    }
    diff = diff / kBlockSize * kBlockSize;
    if (ad->iopBuffSize - ad->iopLastPos >= diff) {
        *pd0 = ad->iopBuff + ad->iopLastPos;
        *d0 = diff;
        *pd1 = 0;
        *d1 = 0;
    } else {
        *pd0 = ad->iopBuff + ad->iopLastPos;
        *d0 = ad->iopBuffSize - ad->iopLastPos;
        *pd1 = ad->iopBuff;
        *d1 = diff - (ad->iopBuffSize - ad->iopLastPos);
    }
}

// NTSC-U/C: 0x001b01b0, PAL: 0x001b8f50
// Copy two source spans into two IOP spans, trimming the source to the room the IOP spans offer.
static int sendToIOP2area(
    int pd0, int d0, int pd1, int d1, unsigned char *ps0, int s0, unsigned char *ps1, int s1) {
    if (d0 + d1 < s0 + s1) {
        const int diff = (s0 + s1) - (d0 + d1);
        if (diff < s1) {
            s1 -= diff;
        } else {
            s0 -= diff - s1;
            s1 = 0;
        }
    }
    if (s0 >= d0) {
        sendToIOP(pd0, ps0, d0);
        sendToIOP(pd1, ps0 + d0, s0 - d0);
        sendToIOP(pd1 + s0 - d0, ps1, s1);
    } else {
        const int rest = d0 - s0;
        if (s1 >= rest) {
            sendToIOP(pd0, ps0, s0);
            sendToIOP(pd0 + s0, ps1, rest);
            sendToIOP(pd1, ps1 + rest, s1 - rest);
        } else {
            sendToIOP(pd0, ps0, s0);
            sendToIOP(pd0 + s0, ps1, s1);
        }
    }
    return s0 + s1;
}

int audioDecCreate(AudioDec *ad,
                   unsigned char *buff,
                   int buffSize,
                   void *iopBuff,
                   int iopBuffSize,
                   unsigned char *zeroBuff,
                   void *iopZeroBuff,
                   int zeroBuffSize) {
    ad->data = buff;
    ad->size = buffSize;
    ad->zeroBuffSize = zeroBuffSize;
    ad->state = AU_STATE_INIT;
    ad->hdrCount = 0;
    ad->put = 0;
    ad->count = 0;
    ad->totalBytes = 0;
    ad->totalBytesSent = 0;
    ad->iopLastPos = 0;
    ad->iopPausePos = 0;
    ad->iopBuffSize = iopBuffSize;
    ad->iopBuff = (int)(uintptr_t)iopBuff;
    ad->iopZero = (int)(uintptr_t)iopZeroBuff;
    memset(zeroBuff, 0, zeroBuffSize);
    sendToIOP(ad->iopZero, zeroBuff, zeroBuffSize);
    changeMasterVolume(kFullMasterVolume);
    return 1;
}

void audioDecPause(AudioDec *ad) {
    int position;

    ad->state = AU_STATE_PAUSE;
    changeInputVolume(kSilentVolume);
    position = sceSdRemote(kWait, rSdBlockTrans, kSoundChannel, SD_TRANS_MODE_STOP, 0, 0);
    ad->iopPausePos = (position & kPositionMask) - ad->iopBuff;
    sceSdRemote(kWait,
                rSdVoiceTrans,
                kSoundChannel,
                SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA,
                ad->iopZero,
                kZeroSpuAddress,
                ad->zeroBuffSize);
}

void audioDecResume(AudioDec *ad) {
    changeInputVolume(kFullInputVolume);
    sceSdRemote(kWait,
                rSdBlockTrans,
                kSoundChannel,
                SD_TRANS_MODE_WRITE_FROM | SD_BLOCK_LOOP,
                ad->iopBuff,
                ad->iopBuffSize / kBlockSize * kBlockSize,
                ad->iopBuff + ad->iopPausePos);
    ad->state = AU_STATE_PLAY;
}

void audioDecStart(AudioDec *ad) {
    audioDecResume(ad);
}

void audioDecReset(AudioDec *ad) {
    audioDecPause(ad);
    ad->iopPausePos = 0;
    ad->state = AU_STATE_INIT;
    ad->hdrCount = 0;
    ad->put = 0;
    ad->count = 0;
    ad->totalBytes = 0;
    ad->totalBytesSent = 0;
    ad->iopLastPos = 0;
}

void audioDecBeginPut(
    AudioDec *ad, unsigned char **ptr0, int *len0, unsigned char **ptr1, int *len1) {
    int free;

    if (ad->state == AU_STATE_INIT) {
        *ptr0 = &ad->header.bytes[ad->hdrCount];
        *len0 = AUDIO_HEADER_SIZE - ad->hdrCount;
        *ptr1 = ad->data;
        *len1 = ad->size;
        return;
    }
    free = ad->size - ad->count;
    if (ad->size - ad->put >= free) {
        *ptr0 = ad->data + ad->put;
        *len0 = free;
        *ptr1 = NULL;
        *len1 = 0;
    } else {
        *ptr0 = ad->data + ad->put;
        *len0 = ad->size - ad->put;
        *ptr1 = ad->data;
        *len1 = free - (ad->size - ad->put);
    }
}

void audioDecEndPut(AudioDec *ad, int size) {
    if (ad->state == AU_STATE_INIT) {
        int header = AUDIO_HEADER_SIZE - ad->hdrCount;
        if ((unsigned int)size < (unsigned int)header) {
            header = size;
        }
        ad->hdrCount += header;
        if ((unsigned int)ad->hdrCount >= AUDIO_HEADER_SIZE) {
            const SpuStreamHeader *sshd = &ad->header.blocks.sshd;
            const SpuStreamBody *ssbd = &ad->header.blocks.ssbd;

            ad->state = AU_STATE_PRESET;
            printf("-------- audio information --------------------\n");
            printf("[%c%c%c%c]\n"
                   "header size:                            %d\n"
                   "type(0:PCM big, 1:PCM little, 2:ADPCM): %d\n"
                   "sampling rate:                          %dHz\n"
                   "channels:                               %d\n"
                   "interleave size:                        %d\n"
                   "interleave start block address:         %d\n"
                   "interleave end block address:           %d\n",
                   sshd->id[0],
                   sshd->id[1],
                   sshd->id[2],
                   sshd->id[3],
                   sshd->size,
                   sshd->type,
                   sshd->rate,
                   sshd->ch,
                   sshd->interSize,
                   sshd->loopStart,
                   sshd->loopEnd);
            printf("[%c%c%c%c]\n"
                   "data size:                              %d\n",
                   ssbd->id[0],
                   ssbd->id[1],
                   ssbd->id[2],
                   ssbd->id[3],
                   ssbd->size);
        }
        size -= header;
    }
    ad->put = (ad->put + size) % ad->size;
    ad->count += size;
    ad->totalBytes += size;
}

int audioDecIsPreset(AudioDec *ad) {
    return ad->totalBytesSent >= ad->iopBuffSize;
}

int audioDecSendToIOP(AudioDec *ad) {
    // A stage outside the four leaves the spans as they were, so they start cleared.
    int pd0 = 0;
    int d0 = 0;
    int pd1 = 0;
    int d1 = 0;
    int ret = 0;
    int counted;
    int pos;
    int s0;
    int s1;

    switch (ad->state) {
    case AU_STATE_INIT:
        return 0;
    case AU_STATE_PRESET:
        pd0 = ad->iopBuff + ad->totalBytesSent % ad->iopBuffSize;
        d0 = ad->iopBuffSize - ad->totalBytesSent;
        pd1 = 0;
        d1 = 0;
        break;
    case AU_STATE_PLAY: {
        const int position = sceSdRemote(kWait, rSdBlockTransStatus, kSoundChannel);
        iopGetArea(&pd0, &d0, &pd1, &d1, ad, (position & kPositionMask) - ad->iopBuff);
        break;
    }
    case AU_STATE_PAUSE:
        return 0;
    default:
        break;
    }

    counted = ad->count / kBlockSize * kBlockSize;
    pos = (ad->put - ad->count + ad->size) % ad->size;
    s0 = ad->size - pos;
    if (s0 >= counted) {
        s0 = counted;
    }
    s1 = counted - s0;
    if (d0 + d1 >= kBlockSize && s0 + s1 >= kBlockSize) {
        ret = sendToIOP2area(pd0, d0, pd1, d1, ad->data + pos, s0, ad->data, s1);
    }
    ad->count -= ret;
    ad->totalBytesSent += ret;
    ad->iopLastPos = (ad->iopLastPos + ret) % ad->iopBuffSize;
    return ret;
}
