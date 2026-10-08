#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"

/**
 * Pixel rectangle with an optional palette and a chain of smaller mipmaps.
 *
 * The class is not polymorphic and emits no RTTI descriptor, so the name and the member titles are
 * inferred, the titles from the labels of its text dump. RndTex has one. A palette entry and a
 * 32-bit pixel are four bytes in the byte order mOrder selects. A bitmap of 4 or 8 bits per pixel
 * indexes its palette, and one of 16, 24, or 32 bits stores its colours directly.
 */
class RndBitmap {
public:
    /** The bits of mOrder. */
    enum Order {
        kOrderRGBA = 1,     /*!< Red in the lowest byte rather than blue. */
        kOrderGs = 2,       /*!< Alpha from 0 to 128 and a palette in the GS entry order. */
        kOrderSwizzled = 4, /*!< Pixels in the GS block order. */
    };

    /** Construct an empty bitmap. */
    RndBitmap() {
        mBuffer = nullptr;
        mMip = nullptr;
        Reset();
    }

    /** Release the pixels and the mipmaps. */
    ~RndBitmap() {
        Reset();
    }

    /**
     * Count the mipmaps that follow the bitmap.
     *
     * @return The count.
     * @ghidraAddress NTSC-U/C: 0x0021c8e0
     * @ghidraAddress PAL: 0x002256f8
     */
    int NumMips() const;

    /**
     * Report the size of the pixels in bytes.
     *
     * @return The size.
     * @ghidraAddress NTSC-U/C: 0x0021c918
     * @ghidraAddress PAL: 0x00225730
     */
    int PixelBytes() const;

    /**
     * Report the size of the palette in bytes, or 0 for a bitmap without one.
     *
     * @return The size.
     * @ghidraAddress NTSC-U/C: 0x0021c928
     * @ghidraAddress PAL: 0x00225740
     */
    int PaletteBytes() const;

    /**
     * Derive the row size when it is unset, and drop the swizzled order from a bitmap too small
     * or too deep for it.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021c950
     * @ghidraAddress PAL: 0x00225768
     */
    void NormalizeLayout();

    /**
     * Find the palette entry nearest to a colour, by the sum of the component differences.
     *
     * @param nRed The red component.
     * @param nGreen The green component.
     * @param nBlue The blue component.
     * @param nAlpha The alpha component.
     * @return The entry index.
     * @ghidraAddress NTSC-U/C: 0x0021ca10
     * @ghidraAddress PAL: 0x00225828
     */
    unsigned char NearestColor(unsigned char nRed,
                               unsigned char nGreen,
                               unsigned char nBlue,
                               unsigned char nAlpha) const;

    /**
     * Decode one stored colour, a palette entry or a direct pixel, in the bitmap's format.
     *
     * The name is inferred.
     *
     * @param pData The stored colour.
     * @param nRed Receives the red component.
     * @param nGreen Receives the green component.
     * @param nBlue Receives the blue component.
     * @param nAlpha Receives the alpha component.
     * @ghidraAddress NTSC-U/C: 0x0021cb28
     * @ghidraAddress PAL: 0x00225940
     */
    void ReadColor(const unsigned char *pData,
                   unsigned char &nRed,
                   unsigned char &nGreen,
                   unsigned char &nBlue,
                   unsigned char &nAlpha) const;

    /**
     * Encode one colour in the bitmap's format.
     *
     * The name is inferred.
     *
     * @param nRed The red component.
     * @param nGreen The green component.
     * @param nBlue The blue component.
     * @param nAlpha The alpha component.
     * @param pData Receives the stored colour.
     * @ghidraAddress NTSC-U/C: 0x0021cc80
     * @ghidraAddress PAL: 0x00225a98
     */
    void WriteColor(unsigned char nRed,
                    unsigned char nGreen,
                    unsigned char nBlue,
                    unsigned char nAlpha,
                    unsigned char *pData) const;

    /**
     * Make a bitmap of half the size, with the same palette, the next mipmap.
     *
     * A bitmap of the wrong size or palette is refused with a notice.
     *
     * @param pMip The mipmap, or null to change nothing.
     * @ghidraAddress NTSC-U/C: 0x0021cda8
     * @ghidraAddress PAL: 0x00225bc0
     */
    void SetMip(RndBitmap *pMip);

    /**
     * Release the pixels and the mipmaps and empty the bitmap.
     *
     * @ghidraAddress NTSC-U/C: 0x0021ce70
     * @ghidraAddress PAL: 0x00225c88
     */
    void Reset();

    /**
     * Make a copy of another bitmap and its mipmaps in another format.
     *
     * The mipmaps share this bitmap's palette.
     *
     * @param source The bitmap to copy.
     * @param nBpp The bits per pixel.
     * @param nOrder The Order bits.
     * @ghidraAddress NTSC-U/C: 0x0021cef0
     * @ghidraAddress PAL: 0x00225d08
     */
    void Create(const RndBitmap &source, int nBpp, int nOrder);

    /**
     * Report whether a size and a depth are valid, with a notice when they are not.
     *
     * @param nWidth The width.
     * @param nHeight The height.
     * @param nBpp The bits per pixel.
     * @return Whether they are valid.
     * @ghidraAddress NTSC-U/C: 0x0021d078
     * @ghidraAddress PAL: 0x00225e90
     */
    static bool CheckDims(int nWidth, int nHeight, int nBpp);

    /**
     * Allocate a bitmap of a size and a format, with a palette when it has 8 bits per pixel or
     * fewer.
     *
     * @param nWidth The width.
     * @param nHeight The height.
     * @param nRowBytes The row size in bytes, or 0 to derive it.
     * @param nBpp The bits per pixel.
     * @param nOrder The Order bits.
     * @ghidraAddress NTSC-U/C: 0x0021d0f0
     * @ghidraAddress PAL: 0x00225f08
     */
    void Create(int nWidth, int nHeight, int nRowBytes, int nBpp, int nOrder);

    /**
     * Describe pixels and a palette that already exist.
     *
     * An invalid size or depth changes nothing. A palette on a bitmap of more than 8 bits per
     * pixel is dropped with a notice.
     *
     * @param nWidth The width.
     * @param nHeight The height.
     * @param nRowBytes The row size in bytes, or 0 to derive it.
     * @param nBpp The bits per pixel.
     * @param nOrder The Order bits.
     * @param pPalette The palette, or null.
     * @param pPixels The pixels.
     * @param pBuffer The block the bitmap releases with Reset(), or null.
     * @ghidraAddress NTSC-U/C: 0x0021d168
     * @ghidraAddress PAL: 0x00225f80
     */
    void Create(int nWidth,
                int nHeight,
                int nRowBytes,
                int nBpp,
                int nOrder,
                unsigned char *pPalette,
                unsigned char *pPixels,
                unsigned char *pBuffer);

    /**
     * Describe a bitmap that Save() wrote into a buffer, without copying it.
     *
     * The bitmap takes the buffer. The data after the 16-byte header has to be aligned to 16
     * bytes. The name is inferred.
     *
     * @param pBuffer The buffer.
     * @ghidraAddress NTSC-U/C: 0x0021d258
     * @ghidraAddress PAL: 0x00226070
     */
    void Create(unsigned char *pBuffer);

    /**
     * Find the byte of the pixels with a pixel, in the GS block order for a swizzled bitmap.
     *
     * The name is inferred.
     *
     * @param nX The column.
     * @param nY The row.
     * @param nHighNibble Receives whether a 4-bit pixel is in the high half of the byte.
     * @return The byte offset.
     * @ghidraAddress NTSC-U/C: 0x0021d410
     * @ghidraAddress PAL: 0x00226228
     */
    int PixelOffset(int nX, int nY, int &nHighNibble) const;

    /**
     * Report the palette index of a pixel.
     *
     * @param nX The column.
     * @param nY The row.
     * @return The index.
     * @ghidraAddress NTSC-U/C: 0x0021d6d0
     * @ghidraAddress PAL: 0x002264e8
     */
    int PixelIndex(int nX, int nY) const;

    /**
     * Set the palette index of a pixel.
     *
     * @param nX The column.
     * @param nY The row.
     * @param nIndex The index.
     * @ghidraAddress NTSC-U/C: 0x0021d730
     * @ghidraAddress PAL: 0x00226548
     */
    void SetPixelIndex(int nX, int nY, unsigned char nIndex);

    /**
     * Report the highest palette index any pixel uses.
     *
     * @return The index, or -1 for a bitmap without a palette.
     * @ghidraAddress NTSC-U/C: 0x0021d7b0
     * @ghidraAddress PAL: 0x002265c8
     */
    int MaxPixelIndex() const;

    /**
     * Convert the pixels to 8 bits per pixel, keeping the palette entries.
     *
     * Only as many palette bytes as the old depth has are copied. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021d868
     * @ghidraAddress PAL: 0x00226680
     */
    void ConvertTo8Bpp();

    /**
     * Load a BMP file and add it as the last mipmap.
     *
     * @param pszPath The file.
     * @return Whether the file loaded.
     * @ghidraAddress NTSC-U/C: 0x0021d9a0
     * @ghidraAddress PAL: 0x002267b8
     */
    bool LoadMip(const char *pszPath);

    /**
     * Take the alpha of each pixel from the red of a BMP file of the same size and palette use.
     *
     * Palette bitmaps combine the two palettes, growing to 8 bits per pixel when the combination
     * needs it.
     *
     * @param pszPath The file.
     * @ghidraAddress NTSC-U/C: 0x0021da58
     * @ghidraAddress PAL: 0x00226870
     */
    void LoadAlpha(const char *pszPath);

    /**
     * Load an uncompressed BMP file of 4 or more bits per pixel.
     *
     * A file whose path includes `_tb` makes black transparent, `_gw` takes the alpha from the red
     * and turns the colour white, and `_ga` takes the alpha from the red.
     *
     * @param pszPath The file.
     * @return Whether the file loaded.
     * @ghidraAddress NTSC-U/C: 0x0021dd68
     * @ghidraAddress PAL: 0x00226b80
     */
    bool LoadBmp(const char *pszPath);

    /**
     * Write the bitmap as a BMP file.
     *
     * The bitmap has to be in the default byte order.
     *
     * @param pszPath The file.
     * @ghidraAddress NTSC-U/C: 0x0021e398
     * @ghidraAddress PAL: 0x002271b0
     */
    void SaveBmp(const char *pszPath) const;

    /**
     * Report whether another bitmap has the same palette entries.
     *
     * @param other The other bitmap.
     * @return Whether the palettes match.
     * @ghidraAddress NTSC-U/C: 0x0021e5e0
     * @ghidraAddress PAL: 0x002273f8
     */
    bool SamePalette(const RndBitmap &other) const;

    /**
     * Report whether another bitmap stores pixels the same way, so rows copy unchanged.
     *
     * @param other The other bitmap.
     * @return Whether the formats match.
     * @ghidraAddress NTSC-U/C: 0x0021e6b0
     * @ghidraAddress PAL: 0x002274c8
     */
    bool SameFormat(const RndBitmap &other) const;

    /**
     * Copy a rectangle of another bitmap into this one, converting the colours.
     *
     * @param source The bitmap to copy from.
     * @param nDestX The left column of the destination.
     * @param nDestY The top row of the destination.
     * @param nSourceX The left column of the source.
     * @param nSourceY The top row of the source.
     * @param nWidth The width.
     * @param nHeight The height.
     * @ghidraAddress NTSC-U/C: 0x0021e710
     * @ghidraAddress PAL: 0x00227528
     */
    void Blit(const RndBitmap &source,
              int nDestX,
              int nDestY,
              int nSourceX,
              int nSourceY,
              int nWidth,
              int nHeight);

    /**
     * Report the colour of a pixel.
     *
     * @param nX The column.
     * @param nY The row.
     * @param nRed Receives the red component.
     * @param nGreen Receives the green component.
     * @param nBlue Receives the blue component.
     * @param nAlpha Receives the alpha component.
     * @ghidraAddress NTSC-U/C: 0x0021e9a0
     * @ghidraAddress PAL: 0x002277b8
     */
    void PixelColor(int nX,
                    int nY,
                    unsigned char &nRed,
                    unsigned char &nGreen,
                    unsigned char &nBlue,
                    unsigned char &nAlpha) const;

    /**
     * Set the colour of a pixel, or its nearest palette entry.
     *
     * @param nX The column.
     * @param nY The row.
     * @param nRed The red component.
     * @param nGreen The green component.
     * @param nBlue The blue component.
     * @param nAlpha The alpha component.
     * @ghidraAddress NTSC-U/C: 0x0021ea70
     * @ghidraAddress PAL: 0x00227888
     */
    void SetPixelColor(int nX,
                       int nY,
                       unsigned char nRed,
                       unsigned char nGreen,
                       unsigned char nBlue,
                       unsigned char nAlpha);

    /**
     * Map a palette index to its position in the palette, which the GS order of an 8-bit palette
     * permutes.
     *
     * The name is inferred.
     *
     * @param nIndex The index.
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x0021eb10
     * @ghidraAddress PAL: 0x00227928
     */
    int PaletteOffset(int nIndex) const;

    /**
     * Report a palette entry.
     *
     * @param nIndex The index.
     * @param nRed Receives the red component.
     * @param nGreen Receives the green component.
     * @param nBlue Receives the blue component.
     * @param nAlpha Receives the alpha component.
     * @ghidraAddress NTSC-U/C: 0x0021eb50
     * @ghidraAddress PAL: 0x00227968
     */
    void PaletteColor(int nIndex,
                      unsigned char &nRed,
                      unsigned char &nGreen,
                      unsigned char &nBlue,
                      unsigned char &nAlpha) const;

    /**
     * Set a palette entry.
     *
     * @param nIndex The index.
     * @param nRed The red component.
     * @param nGreen The green component.
     * @param nBlue The blue component.
     * @param nAlpha The alpha component.
     * @ghidraAddress NTSC-U/C: 0x0021ebc8
     * @ghidraAddress PAL: 0x002279e0
     */
    void SetPaletteColor(int nIndex,
                         unsigned char nRed,
                         unsigned char nGreen,
                         unsigned char nBlue,
                         unsigned char nAlpha);

    /**
     * Write the size, the format, and the mipmap count of the bitmap.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021ec40
     * @ghidraAddress PAL: 0x00227a58
     */
    void Print(PrnStream &stream) const;

    /**
     * Write a 16-byte header, the palette, and the pixels of the bitmap and each mipmap.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021ed20
     */
    void Save(BinStream &stream) const;

    /**
     * The format version Save() writes.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0978
     */
    static unsigned char sRev;

    unsigned short mWidth;    /*!< The width in pixels. */
    unsigned short mHeight;   /*!< The height in pixels. */
    unsigned short mRowBytes; /*!< The size of a row in bytes. */
    unsigned char mBpp;       /*!< The bits per pixel. */
    unsigned char mOrder;     /*!< The Order bits. */
    unsigned char *mPixels;   /*!< The pixels. */
    unsigned char *mPalette;  /*!< The palette, or null. */
    unsigned char *mBuffer;   /*!< The block Reset() releases, or null. */
    RndBitmap *mMip;          /*!< The next smaller mipmap, or null. */
};
