#include "app/cutscene.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <eeregs.h>
#include <ezmpeg.h>
#include <libcdvd.h>
#include <libdma.h>
#include <libgraph.h>
#include <libmpeg.h>
#include <libpad.h>
#include <libsdr.h>
#include <sifdev.h>
#include <sifrpc.h>

#include "os/mem.h"
#include "os/system.h"

namespace {

// The buffers PlayMovieFile() hands the player.
constexpr int kMpegWorkSize = 0x13c76c;
constexpr int kRgb32Size = 0x118000;
constexpr int kPath3TagsSize = 0x34a00;
constexpr int kDemuxSize = 0x30000;
constexpr int kAudioSize = 0xc000;
constexpr int kIopAudioSize = 0x6000;
constexpr int kZeroSize = 0x800;
constexpr int kBufferAlign = 64;

// The decoder, the display registers, and the pad report sit on quadword boundaries of the stack.
constexpr int kQwordAlign = 16;

// The display. The second frame buffer, and the DMA chain that loads it, follow the first.
constexpr int kDisplayWidth = 640;
constexpr int kDisplayHeight = 448;
constexpr int kClearHeight = kDisplayHeight * 2;
constexpr int kPsmCt32 = 0;
constexpr int kGsInit = 0;
constexpr int kGsInterlace = 1;
constexpr int kGsNtsc = 2;
constexpr int kGsField = 0;
constexpr int kDmaResetEnable = 1;
constexpr int kSecondChain = 0x1a50;
constexpr unsigned long long kDispfbFbwMask = 0x7e00;
constexpr unsigned long long kDispfbFbw = 10 << 9;
constexpr unsigned long long kDispfbFbpMask = 0x1ff;
constexpr unsigned long long kSecondFrameFbp = 0x8c;
constexpr int kVsyncOddField = 1;

// The picture buffer holds 0x460 macroblocks, a 640 by 448 picture.
constexpr int kPictureMacroblocks = 0x460;

// Timer 0 counts horizontal blanks while a frame is serviced. Reading carries on until the frame
// has taken 500 of them, and the idle callback reads for as long as a picture of the decoded size
// is estimated to take.
constexpr unsigned int kTimerCountHblank = 0x83;
constexpr unsigned int kFrameHblanks = 500;
constexpr float kHblanksPerPixel = 0.000151f;

// Cross or Start stops the movie once it has played this long.
constexpr float kSkipDelayMs = 5272.0f;
constexpr int kPadReportSize = 32;
constexpr int kPadButtonsHigh = 2;
constexpr int kPadButtonsLow = 3;
constexpr int kPadButtonsMask = 0xffff;
constexpr int kPadSkipButtons = 0x840;

// The stream file. Disc files are read in whole sectors.
constexpr int kReadChunk = 0x4000;
constexpr int kCdSectorShift = 11;
constexpr int kCdStreamSectors = 0x50;
constexpr int kCdStreamBanks = 5;
constexpr int kCdStreamBufferSize = 0x28010;
constexpr int kCdStreamAlign = 16;
constexpr int kPathSize = 256;
constexpr int kDeviceSize = 64;
constexpr char kDeviceSeparator = ':';
constexpr char kVersionSeparator = ';';
constexpr char kDiscDevice[] = "cdrom0";
constexpr char kHostDevice[] = "host0";

// The input ring of the IPU. Each transfer to the IPU moves at most 0x1000 bytes, and the stream
// is presumed to have ended when less than that arrives.
constexpr int kIpuChunk = 0x1000;
constexpr int kIpuQwordMask = ~15;
constexpr int kIpuRoundUp = 15;
constexpr int kQwordShift = 4;
constexpr unsigned int kIpuChainStart = 0x101;
constexpr int kRingStartOffset = 16;

// The clear colour tile is one macroblock of 32-bit pixels.
constexpr int kClearTileWords = 256;

constexpr uintptr_t kPhysicalAddressMask = 0x0fffffff;
constexpr uintptr_t kUncachedSegment = 0x20000000;

/** The buffers PlayMovieFile() allocates for playPss(). */
struct PlayPssParam {
    unsigned char *mpegWork; /*!< Work area of the decoder. */
    int mpegWorkSize;        /*!< Size of the decoder work area in bytes. */
    unsigned char *rgb32;    /*!< The decoded picture. */
    LoadImageTag *path3Tags; /*!< The two DMA chains that load a picture. */
    unsigned char *demux;    /*!< The input ring of the IPU. */
    int demuxSize;           /*!< Size of the IPU input ring in bytes. */
    unsigned char *audio;    /*!< The Emotion Engine audio ring. */
    int audioSize;           /*!< Size of the Emotion Engine audio ring in bytes. */
    void *iopAudio;          /*!< The IOP audio ring. */
    int iopAudioSize;        /*!< Size of the IOP audio ring in bytes. */
    unsigned char *zero;     /*!< The cleared buffer the audio decoder uploads. */
    void *iopZero;           /*!< The IOP copy of the cleared buffer. */
};

/** An open movie file. */
struct StrFile {
    int isOnCD;      /*!< Nonzero for a file read through the disc stream functions. */
    int size;        /*!< Size of the file in bytes. */
    sceCdlFILE fp;   /*!< The disc file record. */
    void *iopBuffer; /*!< The IOP buffer of the disc stream. */
    int fd;          /*!< The descriptor of a file read through the host link. */
};

// NTSC-U/C: 0x00436980
alignas(kBufferAlign) unsigned int g_clearTile[kClearTileWords];

// NTSC-U/C: 0x00436d80
bool g_bAudioStarted;

// NTSC-U/C: 0x00436d88
AudioDec g_audioDec;

// NTSC-U/C: 0x00436de8
int (*g_pfnFetch)(unsigned char *pBuffer, int nSize);

// NTSC-U/C: 0x00436dec
int g_nServiceHblanks;

// NTSC-U/C: 0x00436e00
alignas(kBufferAlign) unsigned char g_demuxBuffer[kReadChunk];

// NTSC-U/C: 0x0043ae00
int g_nDemuxRemaining;

// NTSC-U/C: 0x0043ae04
LoadImageTag *g_pPath3Tags;

// NTSC-U/C: 0x0043ae08
unsigned char *g_pRingBase;

// NTSC-U/C: 0x0043ae0c
int g_nRingSize;

// NTSC-U/C: 0x0043ae10
unsigned char *g_pRingPut;

// NTSC-U/C: 0x0043ae14
unsigned char *g_pRingDmaStart;

// NTSC-U/C: 0x0043ae18
unsigned char *g_pRingDmaEnd;

// NTSC-U/C: 0x0043ae40
StrFile g_inFile;

// NTSC-U/C: 0x003af8ec
bool g_bCdInitialized = false;

// NTSC-U/C: 0x003af8f0
bool g_bReadPending = false;

template <typename T>
inline T *Uncached(T *pAddress) {
    return reinterpret_cast<T *>((reinterpret_cast<uintptr_t>(pAddress) & kPhysicalAddressMask) |
                                 kUncachedSegment);
}

// NTSC-U/C: 0x001aea28, PAL: 0x001b7708
void *memalign64(int nSize, const char *pszName) {
    return PoolMemAlloc(nSize, pszName, kBufferAlign);
}

// NTSC-U/C: 0x001aea48, PAL: 0x001b7728
void *iopalloc(int nSize, const char *pszName) {
    void *pBuffer = sceSifAllocIopHeap(nSize);
    if (pBuffer == nullptr) {
        printf("Cannot allocate %d bytes of IOP memory for %s\n", nSize, pszName);
        for (;;) {
        }
    }
    printf("Allocated %7d bytes of IOP memory for %s\n", nSize, pszName);
    return pBuffer;
}

// NTSC-U/C: 0x001af548, PAL: 0x001b82e8
// Demultiplex the read buffer, reading the next chunk once it is used up. A non-blocking call
// returns at once when the file has no data ready.
int fillBuff(sceMpeg *mp, int block) {
    unsigned char *pData;
    if (g_nDemuxRemaining == 0) {
        for (;;) {
            g_nDemuxRemaining = g_pfnFetch(g_demuxBuffer, kReadChunk);
            if (g_nDemuxRemaining != 0) {
                break;
            }
            if (block == 0) {
                return 0;
            }
        }
        pData = g_demuxBuffer;
    } else {
        pData = &g_demuxBuffer[kReadChunk - g_nDemuxRemaining];
    }
    const int nConsumed = sceMpegDemuxPss(mp, pData, g_nDemuxRemaining);
    g_nDemuxRemaining -= nConsumed;
    return nConsumed;
}

// NTSC-U/C: 0x001af118
// Copy a video packet into the IPU input ring, or refuse it while the ring is too full.
int videoCallback([[maybe_unused]] sceMpeg *mp, void *cbdata, [[maybe_unused]] void *data) {
    sceMpegCbDataStr *pStr = static_cast<sceMpegCbDataStr *>(cbdata);
    int nFree = g_pRingDmaStart - g_pRingPut;
    if (nFree < 0) {
        nFree += g_nRingSize;
    }
    if (nFree == 0 && g_pRingDmaStart == g_pRingDmaEnd) {
        nFree = g_nRingSize;
    }
    if (static_cast<unsigned int>(nFree) < pStr->len) {
        return 0;
    }

    const int nOverflow = (g_pRingPut - g_pRingBase) + pStr->len - g_nRingSize;
    if (nOverflow > 0) {
        memcpy(Uncached(g_pRingPut), pStr->data, pStr->len - nOverflow);
        memcpy(Uncached(g_pRingBase), &pStr->data[pStr->len - nOverflow], nOverflow);
        g_pRingPut = &g_pRingBase[nOverflow];
    } else {
        memcpy(Uncached(g_pRingPut), pStr->data, pStr->len);
        g_pRingPut = &g_pRingPut[pStr->len];
    }
    return 1;
}

// NTSC-U/C: 0x001af260, PAL: 0x001b8000
// Copy an audio packet, less its four-byte prefix, into the audio decoder, or refuse it while the
// decoder is too full.
int audioCallback([[maybe_unused]] sceMpeg *mp, void *cbdata, [[maybe_unused]] void *data) {
    sceMpegCbDataStr *pStr = static_cast<sceMpegCbDataStr *>(cbdata);
    unsigned char *pPut0;
    int nPut0;
    unsigned char *pPut1;
    int nPut1;
    audioDecBeginPut(&g_audioDec, &pPut0, &nPut0, &pPut1, &nPut1);

    constexpr int kPrefixSize = 4;
    pStr->len -= kPrefixSize;
    pStr->data = &pStr->data[kPrefixSize];
    if (static_cast<unsigned int>(nPut0 + nPut1) < pStr->len) {
        return 0;
    }

    const int nRest = pStr->len - nPut0;
    if (nRest > 0) {
        memcpy(pPut0, pStr->data, pStr->len - nRest);
        memcpy(pPut1, &pStr->data[pStr->len - nRest], nRest);
    } else {
        memcpy(pPut0, pStr->data, pStr->len);
    }
    audioDecEndPut(&g_audioDec, pStr->len);
    return 1;
}

// Report the bytes of the IPU input ring not yet sent to the IPU.
inline int RingPending() {
    int nPending = g_pRingPut - g_pRingDmaEnd;
    if (nPending < 0) {
        nPending += g_nRingSize;
    }
    return nPending;
}

// NTSC-U/C: 0x001af338, PAL: 0x001b80d8
// Send the next chunk of the IPU input ring to the IPU, reading more of the file first when less
// than a chunk is waiting.
int nodataCallback(sceMpeg *mp, [[maybe_unused]] void *cbdata, [[maybe_unused]] void *data) {
    g_pRingDmaStart = g_pRingDmaEnd;
    int nSize = RingPending();
    while (nSize < kIpuChunk) {
        if (fillBuff(mp, 1) == 0) {
            nSize += kIpuRoundUp;
            printf("End of stream presumed\n");
            break;
        }
        nSize = RingPending();
    }

    nSize &= kIpuQwordMask;
    if (nSize > kIpuChunk) {
        nSize = kIpuChunk;
    }
    unsigned char *pEnd = &g_pRingBase[g_nRingSize];
    if (pEnd < &g_pRingDmaStart[nSize]) {
        nSize = pEnd - g_pRingDmaStart;
    }
    *D4_QWC = nSize / (1 << kQwordShift);
    *D4_MADR = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(g_pRingDmaStart));
    *D4_CHCR = kIpuChainStart;
    g_pRingDmaEnd = &g_pRingDmaStart[nSize];
    if (!(g_pRingDmaEnd < pEnd)) {
        g_pRingDmaEnd = &g_pRingDmaEnd[-g_nRingSize];
    }
    return 1;
}

// NTSC-U/C: 0x001af498, PAL: 0x001b8238
// Read the file while the decoder waits, for as long as a picture is estimated to take.
int backgroundCallback(sceMpeg *mp, [[maybe_unused]] void *cbdata, [[maybe_unused]] void *data) {
    const unsigned int nUntil = *T0_COUNT + g_nServiceHblanks;
    while (*T0_COUNT < nUntil) {
        fillBuff(mp, 0);
    }
    return 1;
}

// NTSC-U/C: 0x001af510, PAL: 0x001b82b0
int errorCallback([[maybe_unused]] sceMpeg *mp, void *cbdata, [[maybe_unused]] void *data) {
    printf("MPEG decoding error: '%s'\n", static_cast<sceMpegCbDataError *>(cbdata)->errMessage);
    for (;;) {
    }
}

// NTSC-U/C: 0x001af608, PAL: 0x001b83a8
// Fill both frame buffers with one colour.
void clearBackground(unsigned int nColor) {
    unsigned int *pTile = Uncached(g_clearTile);
    for (int i = kClearTileWords - 1; i >= 0; --i) {
        pTile[i] = nColor;
    }
    setLoadImageTagsTile(g_pPath3Tags, g_clearTile, 0, 0, kDisplayWidth, kClearHeight);
    loadImage(g_pPath3Tags);
    sceGsSyncPath(0, 0);
}

// NTSC-U/C: 0x001af6a0, PAL: 0x001b8440
// Open the movie through the disc stream functions, or through the host link.
int strFileOpen(const char *pszName) {
    char szPath[kPathSize];
    char szDevice[kDeviceSize];

    const char *pszFile = strchr(pszName, kDeviceSeparator);
    if (pszFile != nullptr) {
        const int nDeviceLength = pszFile - pszName;
        strncpy(szDevice, pszName, nDeviceLength);
        ++pszFile;
        szDevice[nDeviceLength] = '\0';
        if (strcmp(szDevice, kDiscDevice) == 0) {
            g_inFile.isOnCD = 1;
            // Yes, the binary rewrites the caller's string in place.
            char *pszWrite = const_cast<char *>(pszFile);
            for (int i = strlen(pszFile); i > 0; --i, ++pszWrite) {
                if (*pszWrite == '/') {
                    *pszWrite = '\\';
                }
                *pszWrite = static_cast<char>(toupper(*pszWrite));
            }
            sprintf(
                szPath, "%s%s", pszFile, strchr(pszName, kVersionSeparator) != nullptr ? "" : ";1");
        } else {
            g_inFile.isOnCD = 0;
            sprintf(szPath, "%s:%s", szDevice, pszFile);
        }
    } else {
        strcpy(szDevice, kHostDevice);
        g_inFile.isOnCD = 0;
        sprintf(szPath, "%s:%s", szDevice, pszName);
    }
    printf("file: %s\n", szPath);

    if (g_inFile.isOnCD != 0) {
        if (!g_bCdInitialized) {
            sceCdInit(SCECdINIT);
            if (sceCdGetDiskType() == SCECdPS2CD) {
                sceCdMmode(SCECdCD);
            } else {
                sceCdMmode(SCECdDVD);
            }
            sceCdDiskReady(SCECdBlock);
            g_bCdInitialized = true;
        }
        g_inFile.iopBuffer = sceSifAllocIopHeap(kCdStreamBufferSize);
        sceCdStInit(kCdStreamSectors,
                    kCdStreamBanks,
                    (reinterpret_cast<uintptr_t>(g_inFile.iopBuffer) + kCdStreamAlign - 1) /
                        kCdStreamAlign * kCdStreamAlign);
        if (sceCdSearchFile(&g_inFile.fp, szPath) == 0) {
            printf("Cannot open '%s'(sceCdSearchFile)\n", szPath);
            return 0;
        }
        g_inFile.size = g_inFile.fp.size;
        sceCdRMode mode;
        mode.trycount = 0;
        mode.spindlctrl = 0;
        mode.datapattern = SCECdSecS2048;
        sceCdStStart(g_inFile.fp.lsn, &mode);
        return 1;
    }

    g_inFile.fd = sceOpen(szPath, SCE_RDONLY | SCE_NOWAIT);
    int nExecuting;
    do {
        sceIoctl(g_inFile.fd, SCE_FS_EXECUTING, &nExecuting);
    } while (nExecuting == 1);
    if (g_inFile.fd < 0) {
        printf("Cannot open '%s'(sceOpen)\n", szPath);
        return 0;
    }
    g_inFile.size = sceLseek(g_inFile.fd, 0, SCE_SEEK_END);
    if (g_inFile.size < 0) {
        printf("sceLseek() fails (%s): %d\n", szPath, g_inFile.size);
        sceClose(g_inFile.fd);
        return 0;
    }
    if (sceLseek(g_inFile.fd, 0, SCE_SEEK_SET) < 0) {
        printf("sceLseek() fails (%s)\n", szPath);
        sceClose(g_inFile.fd);
        return 0;
    }
    g_bReadPending = false;
    return 1;
}

// NTSC-U/C: 0x001af9e8, PAL: 0x001b8788
int strFileClose() {
    if (g_inFile.isOnCD != 0) {
        sceCdStStop();
        sceSifFreeIopHeap(g_inFile.iopBuffer);
    } else {
        sceClose(g_inFile.fd);
    }
    return 1;
}

// NTSC-U/C: 0x001afa38, PAL: 0x001b87d8
// Read the next chunk of the movie. A host file is read without waiting, and the call reports no
// data until the read it started has finished.
int strFileRead(unsigned char *pBuffer, int nSize) {
    if (g_inFile.isOnCD != 0) {
        unsigned int nError;
        return sceCdStRead(nSize >> kCdSectorShift,
                           reinterpret_cast<unsigned int *>(pBuffer),
                           STMNBLK,
                           &nError)
               << kCdSectorShift;
    }
    if (!g_bReadPending) {
        sceRead(g_inFile.fd, pBuffer, nSize);
        g_bReadPending = true;
    }
    int nExecuting;
    sceIoctl(g_inFile.fd, SCE_FS_EXECUTING, &nExecuting);
    if (nExecuting != 0) {
        return 0;
    }
    g_bReadPending = false;
    return nSize;
}

// NTSC-U/C: 0x001aec48, PAL: 0x001b7948
// Play the movie, showing each decoded picture centred in alternate frame buffers.
void playPss(int (*pfnFetch)(unsigned char *pBuffer, int nSize), const PlayPssParam *pParam) {
    alignas(kQwordAlign) sceMpeg mp;
    alignas(kQwordAlign) sceGsDispEnv disp;
    alignas(kQwordAlign) unsigned char pad[kPadReportSize];

    sceGsResetPath();
    sceDmaReset(kDmaResetEnable);
    sceGsSyncPath(0, 0);
    sceGsSyncV(0);
    sceGsResetGraph(kGsInit, kGsInterlace, kGsNtsc, kGsField);
    g_pPath3Tags = pParam->path3Tags;
    sceMpegInit();
    sceSdRemoteInit();
    audioDecCreate(&g_audioDec,
                   pParam->audio,
                   pParam->audioSize,
                   pParam->iopAudio,
                   pParam->iopAudioSize,
                   pParam->zero,
                   pParam->iopZero,
                   kZeroSize);
    g_bAudioStarted = false;
    clearBackground(0);

    sceMpegCreate(&mp, pParam->mpegWork, pParam->mpegWorkSize);
    sceMpegAddStrCallback(&mp, sceMpegStrM2V, 0, videoCallback, nullptr);
    sceMpegAddStrCallback(&mp, sceMpegStrPCM, 0, audioCallback, nullptr);
    sceMpegAddCallback(&mp, sceMpegCbNodata, nodataCallback, nullptr);
    sceMpegAddCallback(&mp, sceMpegCbBackground, backgroundCallback, nullptr);
    sceMpegAddCallback(&mp, sceMpegCbError, errorCallback, nullptr);

    // Yes, the binary starts writing and sending 16 bytes into the ring.
    g_nRingSize = pParam->demuxSize;
    g_pRingDmaStart = pParam->demux;
    g_pRingDmaEnd = &pParam->demux[kRingStartOffset];
    g_pfnFetch = pfnFetch;
    g_nDemuxRemaining = 0;
    g_pRingBase = pParam->demux;
    g_pRingPut = &pParam->demux[kRingStartOffset];
    while (fillBuff(&mp, 1) != 0) {
    }

    sceGsSetDefDispEnv(&disp, kPsmCt32, kDisplayWidth, kDisplayHeight, 0, 0);
    disp.dispfb = (disp.dispfb & ~kDispfbFbwMask) | kDispfbFbw;
    sceGsPutDispEnv(&disp);
    while (sceGsSyncV(0) == kVsyncOddField) {
    }

    const float flSkipAfter = SystemMs() + kSkipDelayMs;
    bool bSecondFrame = true;
    while (sceMpegIsEnd(&mp) == 0) {
        if (flSkipAfter < SystemMs() && scePadRead(0, 0, pad) > 0) {
            const int nButtons =
                kPadButtonsMask ^ ((pad[kPadButtonsHigh] << 8) | pad[kPadButtonsLow]);
            if ((nButtons & kPadSkipButtons) != 0) {
                break;
            }
        }

        *T0_MODE = kTimerCountHblank;
        *T0_COUNT = 0;
        if (sceMpegGetPicture(&mp, pParam->rgb32, kPictureMacroblocks) < 0) {
            printf("sceMpegGetPicture failed\n");
            for (;;) {
            }
        }
        if (mp.frameCount == 0) {
            const int x = (kDisplayWidth - mp.width) / 2;
            const int y = (kDisplayHeight - mp.height) / 2;
            setLoadImageTags(g_pPath3Tags, pParam->rgb32, x, y, mp.width, mp.height);
            setLoadImageTags(&g_pPath3Tags[kSecondChain],
                             pParam->rgb32,
                             x,
                             y + kDisplayHeight,
                             mp.width,
                             mp.height);
            g_nServiceHblanks = static_cast<int>(static_cast<float>(mp.width) *
                                                 static_cast<float>(mp.height) * kHblanksPerPixel);
        }
        sceGsSyncPath(0, 0);
        loadImage(bSecondFrame ? &g_pPath3Tags[kSecondChain] : g_pPath3Tags);

        audioDecSendToIOP(&g_audioDec);
        if (!g_bAudioStarted && audioDecIsPreset(&g_audioDec) != 0) {
            audioDecStart(&g_audioDec);
            g_bAudioStarted = true;
        }
        while (*T0_COUNT < kFrameHblanks) {
            fillBuff(&mp, 0);
        }

        while (sceGsSyncV(0) == kVsyncOddField) {
        }
        if (bSecondFrame) {
            bSecondFrame = false;
            disp.dispfb = (disp.dispfb & ~kDispfbFbpMask) | kSecondFrameFbp;
        } else {
            bSecondFrame = true;
            disp.dispfb &= ~kDispfbFbpMask;
        }
        sceGsPutDispEnv(&disp);
    }

    audioDecReset(&g_audioDec);
    sceMpegReset(&mp);
    sceMpegDelete(&mp);
}

} // namespace

bool PlayMovieFile(const char *pszPath) {
    if (strFileOpen(pszPath) == 0) {
        printf("Can't open file %s\n", pszPath);
        return false;
    }
    sceSifInitRpc(0);
    sceSifInitIopHeap();

    PlayPssParam param;
    param.mpegWork = static_cast<unsigned char *>(memalign64(kMpegWorkSize, "mpeg work area"));
    param.mpegWorkSize = kMpegWorkSize;
    param.rgb32 = static_cast<unsigned char *>(memalign64(kRgb32Size, "rgb32 buffer"));
    param.path3Tags = static_cast<LoadImageTag *>(memalign64(kPath3TagsSize, "path 3 tags"));
    param.demux = static_cast<unsigned char *>(memalign64(kDemuxSize, "demux buffer"));
    param.demuxSize = kDemuxSize;
    param.audio = static_cast<unsigned char *>(memalign64(kAudioSize, "audio buffer"));
    param.audioSize = kAudioSize;
    param.iopAudio = iopalloc(kIopAudioSize, "audio buffer");
    param.iopAudioSize = kIopAudioSize;
    param.zero = static_cast<unsigned char *>(memalign64(kZeroSize, "zero buffer"));
    param.iopZero = iopalloc(kZeroSize, "zero buffer");

    playPss(strFileRead, &param);
    strFileClose();

    PoolMemFree(param.mpegWork);
    PoolMemFree(param.rgb32);
    PoolMemFree(param.path3Tags);
    PoolMemFree(param.demux);
    PoolMemFree(param.audio);
    sceSifFreeIopHeap(param.iopAudio);
    PoolMemFree(param.zero);
    sceSifFreeIopHeap(param.iopZero);
    return true;
}
