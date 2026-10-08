#pragma once

#include <cstddef>

#include "os/binstream.h"
#include "os/filepath.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndbitmap.h"
#include "rnd/rndobject.h"

/**
 * Texture, a bitmap loaded from a file or rendered to.
 *
 * The RTTI includes the class name and records RndObject as the one base.
 */
class RndTex : public RndObject {
public:
    /**
     * Construct an empty 32-bit texture without a file.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00240eb8
     * @ghidraAddress PAL: 0x002499e8
     */
    explicit RndTex(const char *pszName);

    /**
     * Release the bitmap.
     *
     * @ghidraAddress NTSC-U/C: 0x003910b0
     * @ghidraAddress PAL: 0x003ff7b8
     */
    ~RndTex() override;

    /**
     * Allocate a texture, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Tex", 0);
    }

    /**
     * Release a texture.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Write a description of the texture, and of its bitmap at a dump level of 2 or more.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00241218
     * @ghidraAddress PAL: 0x00249d48
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the texture.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00241378
     * @ghidraAddress PAL: 0x00249ea8
     */
    void Save(BinStream &stream) override;

    /**
     * Do nothing, for a texture refers to no object.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00391098
     * @ghidraAddress PAL: 0x003ff7a0
     */
    void Replace([[maybe_unused]] RndObject *pFrom, [[maybe_unused]] RndObject *pTo) override {
    }

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x003910a0
     * @ghidraAddress PAL: 0x003ff7a8
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy the size, the format, and the file of another texture, then load the bitmap again.
     *
     * @param pSource The texture to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00241728
     * @ghidraAddress PAL: 0x0024a230
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it, then load the bitmap.
     *
     * An early version's flag word becomes a file name suffix where one exists.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00241460
     * @ghidraAddress PAL: 0x00249f80
     */
    void Load(BinStream &stream) override;

    /**
     * Give access to the bitmap.
     *
     * The base body ignores the flags. The name is inferred.
     *
     * @param nFlags The access the caller needs.
     * @return The bitmap.
     * @ghidraAddress NTSC-U/C: 0x00391088
     */
    virtual RndBitmap *LockBitmap([[maybe_unused]] int nFlags) {
        return &mBitmap;
    }

    /**
     * End an access LockBitmap() gave.
     *
     * The base body does nothing. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00391090
     */
    virtual void UnlockBitmap() {
    }

    /**
     * Load the bitmap from the file, or allocate a blank one of the size and the format when there
     * is no file.
     *
     * An invalid size leaves the bitmap empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00241140
     * @ghidraAddress PAL: 0x00249c70
     */
    virtual void SyncBitmap();

    /**
     * Release the bitmap.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002411f8
     * @ghidraAddress PAL: 0x00249d28
     */
    virtual void ResetBitmap();

    /**
     * Set the size, the format, and the file, then load the bitmap.
     *
     * @param nWidth The width.
     * @param nHeight The height.
     * @param nBpp The bits per pixel.
     * @param file The file, or an empty path for none.
     * @param nMipMapK The mipmap distance factor.
     * @param nRendered Non-zero for a texture rendered to.
     * @ghidraAddress NTSC-U/C: 0x00240f50
     * @ghidraAddress PAL: 0x00249a80
     */
    void
    SetBitmap(int nWidth, int nHeight, int nBpp, const FilePath &file, int nMipMapK, int nRendered);

    /**
     * Report what is wrong with a texture dimension.
     *
     * @param nSize The dimension.
     * @return A notice format with one `%s` for the name, or null for a valid dimension.
     * @ghidraAddress NTSC-U/C: 0x00241000
     * @ghidraAddress PAL: 0x00249b30
     */
    static const char *CheckDim(int nSize);

    /**
     * Report whether the size and the format are valid, with a notice when they are not unless a
     * dimension is 0.
     *
     * @return Whether they are valid.
     * @ghidraAddress NTSC-U/C: 0x00241080
     * @ghidraAddress PAL: 0x00249bb0
     */
    bool CheckSize();

    /**
     * Create a texture.
     *
     * @param pszName The registry key.
     * @return The texture.
     * @ghidraAddress NTSC-U/C: 0x00391120
     * @ghidraAddress PAL: 0x003ff828
     */
    static RndObject *New(const char *pszName) {
        return new RndTex(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Tex`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a48
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a4c
     */
    static int sRev;

    RndBitmap mBitmap; /*!< The bitmap. */
    int mMipMapK;      /*!< The mipmap distance factor. */
    int mRendered;     /*!< Non-zero for a texture rendered to. */
    int mWidth;        /*!< Width in pixels. */
    int mHeight;       /*!< Height in pixels. */
    int mBpp;          /*!< Bits per pixel. */
    FilePath mFile;    /*!< The file the bitmap loads from. */
};
