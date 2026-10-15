#pragma once

#include <cstddef>
#include <list>
#include <vector>

#include "os/mem.h"
#include "rnd/dmapacket.h"
#include "rnd/rndbitmap.h"
#include "rnd/rndtex.h"

/**
 * Texture, PlayStation 2.
 *
 * The RTTI includes the class name and records RndTex as the one base. GS memory is a cache of
 * 16384 blocks of 256 bytes. Each texture has one UsedRange for its CLUT, when it has a palette,
 * and one for each mipmap. A range receives blocks when the texture is next selected, from the
 * free list or, failing that, from a range of another texture that was not selected since the
 * last BeginFrame(). A texture also builds the TEX0, TEX1, MIPTBP1, and MIPTBP2 images it selects.
 */
class PsTex : public RndTex {
public:
    /** A run of free GS blocks. The name is inferred. */
    struct FreeRange {
        unsigned short mStart; /*!< The first block. */
        unsigned short mCount; /*!< The number of blocks. */
    };

    /** The GS blocks of the CLUT or of one mipmap. The name is inferred. */
    struct UsedRange {
        /**
         * Report the alignment the range requires, as a power of two of blocks.
         *
         * A pinned range, and a range of 17 blocks or more, aligns to a whole page of 32 blocks.
         *
         * @return The power of two.
         * @ghidraAddress NTSC-U/C: 0x00216eb0
         * @ghidraAddress PAL: 0x0021fcc8
         */
        int AlignShift() const;

        /**
         * Take the free run that wastes the fewest blocks once aligned.
         *
         * A run that fits exactly is taken at once. The alignment padding before the range
         * becomes a separate free run.
         *
         * @return Whether the range received blocks.
         * @ghidraAddress NTSC-U/C: 0x00216f10
         * @ghidraAddress PAL: 0x0021fd28
         */
        bool AllocFree();

        /**
         * Take the blocks of the range of another texture that wastes the fewest of them.
         *
         * Pinned ranges and ranges selected since the last stamp are passed over. Of two ranges
         * that waste as many blocks, the one selected more recently wins. The other texture's
         * range is evicted, and the blocks this range does not need are freed.
         *
         * @return Whether the range received blocks.
         * @ghidraAddress NTSC-U/C: 0x00217150
         * @ghidraAddress PAL: 0x0021ff68
         */
        bool AllocUsed();

        /**
         * Free the blocks and remove the range from sUsedRanges.
         *
         * A range without blocks is unchanged.
         *
         * @ghidraAddress NTSC-U/C: 0x00217f58
         * @ghidraAddress PAL: 0x00220d70
         */
        void Release();

        unsigned short mStart : 15; /*!< The first block, or 0 while the range has none. */
        unsigned short mPinned
            : 1;               /*!< Set for a render target. A pinned range is never evicted. */
        unsigned short mCount; /*!< The number of blocks. */
        int mFrame;            /*!< The value of sStamp when the texture was last selected. */
    };

    /**
     * Construct a texture without GS blocks.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00218278
     * @ghidraAddress PAL: 0x00221090
     */
    explicit PsTex(const char *pszName);

    /**
     * Release the GS blocks and the movie packet.
     *
     * @ghidraAddress NTSC-U/C: 0x002182c8
     * @ghidraAddress PAL: 0x002210e0
     */
    ~PsTex() override;

    /**
     * Allocate a texture from its pool.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(PsTex), "PsTex", 0);
    }

    /**
     * Return a texture to its pool.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolFree(sizeof(PsTex), pBlock);
    }

    /**
     * Give access to the bitmap, recording the access for UnlockBitmap().
     *
     * @param nFlags The access the caller needs. Bit 2 marks the palette as changed.
     * @return The bitmap.
     * @ghidraAddress NTSC-U/C: 0x00218fa8
     * @ghidraAddress PAL: 0x00221dc0
     */
    RndBitmap *LockBitmap(int nFlags) override;

    /**
     * Mark the first range for upload, and the second as well when the palette changed.
     *
     * @ghidraAddress NTSC-U/C: 0x00218fb8
     * @ghidraAddress PAL: 0x00221dd0
     */
    void UnlockBitmap() override;

    /**
     * Load the bitmap, then build the GS registers and the ranges for it.
     *
     * @ghidraAddress NTSC-U/C: 0x0037d0b0
     * @ghidraAddress PAL: 0x003eb7e0
     */
    void SyncBitmap() override {
        RndTex::SyncBitmap();
        SyncRegisters();
    }

    /**
     * Release the GS blocks, then the bitmap.
     *
     * @ghidraAddress NTSC-U/C: 0x0037d0e0
     * @ghidraAddress PAL: 0x003eb810
     */
    void ResetBitmap() override {
        ReleaseSurfaces();
        RndTex::ResetBitmap();
    }

    /**
     * Give each range without blocks its blocks and mark it for upload.
     *
     * When neither the free list nor another texture can supply a range, every texture is evicted
     * and the free list starts again. The first range is the CLUT when the bitmap has a palette.
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00217380
     * @ghidraAddress PAL: 0x00220198
     */
    void Place();

    /**
     * Release the movie packet, then build one for the current bitmap when requested.
     *
     * The packet uploads the bitmap in blocks of 16 by 16 pixels. UploadMovie() copies it into the
     * frame. The name is inferred.
     *
     * @param bBuild Whether to build a packet.
     * @ghidraAddress NTSC-U/C: 0x00217628
     * @ghidraAddress PAL: 0x00220440
     */
    void BuildMoviePacket(bool bBuild);

    /**
     * Copy the movie packet into the frame, flushing the frame whenever the packet fills it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00217830
     * @ghidraAddress PAL: 0x00220648
     */
    void UploadMovie();

    /**
     * Add an image transfer of pixels in main memory to the frame.
     *
     * A null image adds nothing. The texture is not read. The name is inferred.
     *
     * @param nDbp The destination base pointer in blocks.
     * @param nPsm The GS pixel storage mode.
     * @param nBpp The bits per pixel.
     * @param nWidth The width.
     * @param nHeight The height.
     * @param pPixels The pixels, or null.
     * @ghidraAddress NTSC-U/C: 0x00217a18
     * @ghidraAddress PAL: 0x00220830
     */
    void UploadImage(
        int nDbp, int nPsm, int nBpp, int nWidth, int nHeight, const unsigned char *pPixels);

    /**
     * Place the ranges, upload those marked for upload, and flush the texture cache.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00217be0
     * @ghidraAddress PAL: 0x002209f8
     */
    void Upload();

    /**
     * Make the first mipmap the surface the GS draws into.
     *
     * FRAME_1 addresses the mipmap, XYOFFSET_1 centres it, and a 16-bit texture is dithered.
     *
     * @ghidraAddress NTSC-U/C: 0x00218028
     * @ghidraAddress PAL: 0x00220e40
     */
    void BindAsRenderTarget();

    /**
     * Build TEX0, TEX1, and the buffer widths of MIPTBP1 and MIPTBP2, and add a range for the
     * CLUT and for each mipmap.
     *
     * A rendered texture of fewer than 16 bits per pixel is first made 16 bits. The name is
     * inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002184f8
     * @ghidraAddress PAL: 0x00221310
     */
    void SyncRegisters();

    /**
     * Release the bitmap and the GS blocks, and empty the ranges.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00218f00
     * @ghidraAddress PAL: 0x00221d18
     */
    void ReleaseSurfaces();

    /**
     * Upload the texture when needed, then select it on the GS with a texture function.
     *
     * Each range is stamped with sStamp, and sStamp then advances. A texture without a bitmap
     * selects nothing.
     *
     * @param nTexFunction The TFX field of TEX0.
     * @ghidraAddress NTSC-U/C: 0x00218fe0
     * @ghidraAddress PAL: 0x00221df8
     */
    void Select(int nTexFunction);

    /**
     * Evict every range not selected since the last call, and record the stamp.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216c98
     * @ghidraAddress PAL: 0x0021fab0
     */
    static void BeginFrame();

    /**
     * Count the free GS blocks.
     *
     * The name is inferred.
     *
     * @return The count.
     * @ghidraAddress NTSC-U/C: 0x00216d38
     * @ghidraAddress PAL: 0x0021fb50
     */
    static int FreeBlockCount();

    /**
     * Free every block after the display and depth buffers, and evict every range.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00216d88
     * @ghidraAddress PAL: 0x0021fba0
     */
    static void ReserveDisplayMemory();

    /**
     * Return a run of blocks to the free list, merging it with its neighbours.
     *
     * The name is inferred.
     *
     * @param nStart The first block.
     * @param nCount The number of blocks, or 0 to change nothing.
     * @ghidraAddress NTSC-U/C: 0x00217d88
     * @ghidraAddress PAL: 0x00220ba0
     */
    static void FreeBlocks(int nStart, int nCount);

    /**
     * Report the GS blocks a bitmap occupies, rounded up to the shapes the GS pages allow.
     *
     * The name is inferred.
     *
     * @param bitmap The bitmap.
     * @return The block count.
     * @ghidraAddress NTSC-U/C: 0x002183d0
     * @ghidraAddress PAL: 0x002211e8
     */
    static int NumBlocks(const RndBitmap &bitmap);

    /**
     * Install the texture factory for the class `Tex` of a `.rnd` file, and reset both stamps.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00218168
     * @ghidraAddress PAL: 0x00220f80
     */
    static void Init();

    /**
     * Create a texture.
     *
     * @param pszName The registry key.
     * @return The texture.
     * @ghidraAddress NTSC-U/C: 0x0037d068
     * @ghidraAddress PAL: 0x003eb798
     */
    static RndObject *New(const char *pszName) {
        return new PsTex(pszName);
    }

    /**
     * The free runs, in block order.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c600
     */
    static std::list<FreeRange> sFreeRanges;

    /**
     * The ranges with blocks, in the order they received them.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c608
     */
    static std::list<UsedRange *> sUsedRanges;

    /**
     * The count of selections, the stamp Select() records on each range.
     *
     * @ghidraAddress NTSC-U/C: 0x003b07d8
     * @ghidraAddress PAL: 0x0042f670
     */
    static int sStamp;

    /**
     * The value of sStamp at the last BeginFrame(). A range stamped before it is evicted.
     *
     * @ghidraAddress NTSC-U/C: 0x003b07dc
     * @ghidraAddress PAL: 0x0042f674
     */
    static int sEvictStamp;

    unsigned long long mTex0;       /*!< The TEX0_1 image. */
    unsigned long long mTex1;       /*!< The TEX1_1 image. */
    unsigned long long mMipTbp1;    /*!< The MIPTBP1_1 image, mipmaps 1 to 3. */
    unsigned long long mMipTbp2;    /*!< The MIPTBP2_1 image, mipmaps 4 to 6. */
    std::vector<UsedRange> mRanges; /*!< The CLUT range, then one range for each mipmap. */
    int mLockFlags;                 /*!< The flags of the open LockBitmap(), or 0. */
    unsigned int mDirty;            /*!< One bit for each range to upload. */
    DmaQuadword *mMoviePacket;      /*!< The packet BuildMoviePacket() built, or null. */
};
