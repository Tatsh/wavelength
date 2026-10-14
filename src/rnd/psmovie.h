#pragma once

#include <libipu.h>

#include "rnd/rndmovie.h"

/**
 * Movie, PlayStation 2.
 *
 * The RTTI includes the class name and records RndMovie as the one base. The movie file is an IPU
 * stream that the IPU decodes one frame at a time straight into the bitmap of the texture, as
 * 16-bit colour. One movie decodes at a time. A movie that requests a frame while another decodes
 * waits in the queue that mNext links, and the interrupt handler of the IPU output channel starts
 * the next one.
 */
class PsMovie : public RndMovie {
public:
    /** The header of an IPU movie file. The stream follows the header. */
    struct Header {
        char mMagic[4];         /*!< The magic number, `ipum`. */
        int mSize;              /*!< The size of the stream in bytes. */
        unsigned short mWidth;  /*!< The width in pixels. */
        unsigned short mHeight; /*!< The height in pixels. */
        int mNumFrames;         /*!< The number of frames. */
    };

    /**
     * Construct a movie without data.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00216678
     * @ghidraAddress PAL: 0x0021f490
     */
    explicit PsMovie(const char *pszName);

    /**
     * Release the data.
     *
     * @ghidraAddress NTSC-U/C: 0x0037cd28
     * @ghidraAddress PAL: 0x003eb458
     */
    ~PsMovie() override {
        CloseData();
    }

    /**
     * Report the last frame, fixed at 10 whatever the length of the file.
     *
     * @return 10.
     * @ghidraAddress NTSC-U/C: 0x0037ce68
     * @ghidraAddress PAL: 0x003eb598
     */
    float EndFrame() override {
        return kEndFrame;
    }

    /**
     * Queue the decode of the frame after the one decoded last, or of the first frame when the
     * requested frame is earlier.
     *
     * The frame wraps to the length of the movie. A movie still queued from an earlier request
     * waits for its decode first. The request does not seek.
     *
     * @param fFrame The frame.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x00216b60
     * @ghidraAddress PAL: 0x0021f978
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Take a reference on the texture, then load the data.
     *
     * @ghidraAddress NTSC-U/C: 0x0037ced0
     * @ghidraAddress PAL: 0x003eb600
     */
    void Open() override {
        RndMovie::Open();
        OpenData();
    }

    /**
     * Release the data, then drop the reference on the texture.
     *
     * @ghidraAddress NTSC-U/C: 0x0037cf00
     * @ghidraAddress PAL: 0x003eb630
     */
    void Close() override {
        CloseData();
        RndMovie::Close();
    }

    /**
     * Wait while the movie is queued when the texture is replaced, then replace it.
     *
     * mNext is read once. A queued movie waits forever.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00216370
     * @ghidraAddress PAL: 0x0021f188
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Load the file, size the texture for it, and build the IPU input chain.
     *
     * A movie of 2048 macroblocks or more is refused and its data released. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002167b8
     * @ghidraAddress PAL: 0x0021f5d0
     */
    void OpenData();

    /**
     * Wait while the movie is queued, then release the data and the IPU input chain.
     *
     * mNext is read once. A queued movie waits forever. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216910
     * @ghidraAddress PAL: 0x0021f728
     */
    void CloseData();

    /**
     * End the decode of a frame, mark the texture for upload, and start the next movie queued.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216990
     * @ghidraAddress PAL: 0x0021f7a8
     */
    void FinishFrame();

    /**
     * Start the IPU decoding the next frame into the bitmap of the texture.
     *
     * The first frame resets the IPU and starts the input chain from the beginning. The name is
     * inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216a18
     * @ghidraAddress PAL: 0x0021f830
     */
    void StartFrame();

    /**
     * Install the movie factory for the class `Movie` of a `.rnd` file, reset the IPU, and
     * install the interrupt handler.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002163c0
     * @ghidraAddress PAL: 0x0021f1d8
     */
    static void Init();

    /**
     * Remove the interrupt handler and restore the generic movie factory.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002164e0
     * @ghidraAddress PAL: 0x0021f2f8
     */
    static void Terminate();

    /**
     * Remove the interrupt handler.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002165f8
     * @ghidraAddress PAL: 0x0021f410
     */
    static void RemoveHandler();

    /**
     * Install the interrupt handler unless it is installed.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216630
     * @ghidraAddress PAL: 0x0021f448
     */
    static void InstallHandler();

    /**
     * End the frame of the movie decoding, the interrupt handler of the IPU output channel.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00216340
     * @ghidraAddress PAL: 0x0021f158
     */
    static int DmacHandler(int nChannel);

    /**
     * Create a movie.
     *
     * @param pszName The registry key.
     * @return The movie.
     * @ghidraAddress NTSC-U/C: 0x0037ce78
     * @ghidraAddress PAL: 0x003eb5a8
     */
    static RndObject *New(const char *pszName) {
        return new PsMovie(pszName);
    }

    /** The frame EndFrame() reports. */
    static constexpr float kEndFrame = 10.0f;

    /**
     * The interrupt handler, or -1 while it is removed.
     *
     * @ghidraAddress NTSC-U/C: 0x003b07d0
     */
    static int sHandlerId;

    /**
     * The movie decoding, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b07d4
     */
    static PsMovie *sActive;

    void *mData;                  /*!< The file, a Header and the stream, or null. */
    int mDataSize;                /*!< The size of the file in bytes. */
    int mWidth;                   /*!< The width in pixels. */
    int mHeight;                  /*!< The height in pixels. */
    int mNumFrames;               /*!< The number of frames. */
    int mFrame;                   /*!< The frame decoded last. */
    int mNumBlocks;               /*!< The number of macroblocks of 16 by 16 pixels. */
    unsigned long long *mIpuTags; /*!< The DMA chain of the stream to the IPU, or null. */
    sceIpuDmaEnv mIpuEnv;         /*!< The IPU state between two frames. */
    PsMovie *mNext;               /*!< The movie queued after this one, or null. */
};
