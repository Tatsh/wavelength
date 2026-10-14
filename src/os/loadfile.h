#pragma once

/** Extension that routes a load through the decompressing path. */
constexpr char kGzExtension[] = ".gz";

/**
 * Read a whole file.
 *
 * A path whose last three characters match kGzExtension, tested without regard to case, is handed
 * to LoadGzFile() instead. Any other path is opened, measured, and read in one call.
 *
 * With no buffer supplied the routine allocates one, from the selected zone when there is one and
 * from MemAllocTagged() otherwise. With a buffer supplied that is too small the file is closed and
 * the load reports null.
 *
 * @param pszPath The file to read.
 * @param pBuffer The destination, or null to have one allocated.
 * @param nBufferSize The destination size, which is ignored when pBuffer is null.
 * @param pnSize Receives the file size, and is written even when the allocation failed.
 * @return The destination, or null when the file could not be opened or read.
 */
void *LoadWholeFile(const char *pszPath, void *pBuffer, unsigned nBufferSize, unsigned *pnSize);

/**
 * Read a whole compressed file, decompressing it.
 *
 * The size queried and reported is the decompressed size, and the reader inflates the whole file
 * in one call. Whether the query and the read address an ark stream or a loose file depends on
 * UsingArkFiles().
 *
 * A supplied buffer that is too small is fatal here rather than a null report, the one behavioural
 * difference from LoadWholeFile(). The file is not closed on the success path.
 *
 * @param pszPath The file to read.
 * @param pBuffer The destination, or null to have one allocated.
 * @param nBufferSize The destination size, which is ignored when pBuffer is null.
 * @param pnSize Receives the decompressed size.
 * @return The destination, or null when the file could not be opened.
 */
void *LoadGzFile(const char *pszPath, void *pBuffer, unsigned nBufferSize, unsigned *pnSize);

/**
 * Open a file, whether it resolves to an ark stream or a loose file.
 *
 * The CD drive is synchronised first, and the path is opened read-only through sceOpen().
 *
 * @param pszPath The path to open, device prefix included.
 * @return The handle, or a negative value on failure.
 */
int OpenStreamByPath(const char *pszPath);

/**
 * Read one chunk of a file into a buffer.
 *
 * The chunk index is mapped through ArkfileLogicalToPhysicalSector() before the seek, even though
 * ReadArkStreamThroughCache(), the one caller, has already mapped it. An optimized archive is
 * therefore read at a doubly mapped position.
 *
 * @param nFile The file to read.
 * @param nSector The chunk index.
 * @param pBuffer The destination.
 * @param nLength The number of bytes to read.
 */
void ReadStreamChunk(int nFile, int nSector, void *pBuffer, unsigned nLength);

/**
 * Append a component to a device path.
 *
 * A backslash is appended first when the component is not empty, and the component is normalised
 * as it is copied. The argument order (the component before the buffer) matches the image.
 *
 * @param pszComponent The component to append.
 * @param pszPath The buffer to append to.
 */
void FilenameToISO9660(const char *pszComponent, char *pszPath);

/**
 * Close an open file, whether it is an ark stream or a loose file.
 *
 * The whole body forwards to the SDK primitive at 0x0056af88 with the argument passed through.
 *
 * @param nFile The file to close.
 */
void CloseLoadFile(int nFile);

/**
 * Report the decompressed size of the file an ark stream reads.
 *
 * The routine clears bit 0x4000 from the handle, finds the matching record in the stream table,
 * and reports its directory entry's decompressed size. Nothing about it is specific to
 * compression. LoadGzFile() simply needs that size, and the ark reader at 0x0055a280 uses the same
 * entry's stored size to measure how much of the stream remains.
 *
 * @param nStream The ark stream handle.
 * @return The decompressed size in bytes, or -1 when no record has that handle.
 */
int FindOpenFileInArkTrueSize(int nStream);

/**
 * Report the decompressed size of a compressed loose file.
 *
 * @param nFile The file.
 * @return The decompressed size in bytes.
 */
unsigned GetGzFileSize(int nFile);

/**
 * Decompress a whole file into a buffer.
 *
 * Vendored gzip glue, declared by title only. The body stores the file in the bundled gzip code's
 * input-descriptor global at `0x00761488`, copies `unknown` into its input-name global at
 * `0x00761490`, and drives the gzip routines at `0x006125b8`, `0x00612468`, `0x00562f88`,
 * `0x0061d778`, and `0x006125d0`. Both exits close the file with close() unless the descriptor
 * reads -1. The failure exit puts -1 in the result register. LoadGzFile(), the one caller, ignores
 * the result.
 *
 * @param nFile The file to read.
 * @param pBuffer The destination, which must take the whole decompressed size.
 */
extern "C" void GzipDecompressFdToRam(int nFile, void *pBuffer);

/**
 * Inflate a gzip member that is already in memory.
 *
 * The source and destination windows may overlap, and every caller in the async layer relies on
 * the overlap. The stored bytes sit against the end of the destination, and the inflate runs
 * forward over the whole destination.
 *
 * @param pSource The stored bytes.
 * @param nSourceLength The number of stored bytes.
 * @param pDest The destination.
 * @return Positive once the data is in place, and not positive on failure.
 * @ghidraAddress NTSC-U/C: 0x002835b0
 * @ghidraAddress PAL: 0x0028ce60
 */
int GzipDecompressRamToRam(const void *pSource, int nSourceLength, void *pDest);

/**
 * Inflate a gzip member in memory into a buffer.
 *
 * The name is inferred.
 *
 * @param pSource The stored bytes.
 * @param nLength The number of bytes the inflate may read.
 * @param pDest The destination.
 * @return The status of the inflate.
 * @ghidraAddress NTSC-U/C: 0x002835b0
 * @ghidraAddress PAL: 0x0028ce60
 */
int InflateMemoryBuffer(const void *pSource, int nLength, void *pDest);

/**
 * Report the uncompressed length a gzip member in memory records in its trailer.
 *
 * The length is the last four bytes of the member, read without regard to alignment.
 *
 * @param pSource The stored bytes.
 * @param nSourceLength The number of stored bytes.
 * @return The uncompressed length.
 * @ghidraAddress NTSC-U/C: 0x00283620
 * @ghidraAddress PAL: 0x0028ced0
 */
int GzipInflatedSize(const void *pSource, int nSourceLength);

/**
 * Deflate bytes in memory into a gzip member, naming the source `(in-memory)`.
 *
 * The name is inferred.
 *
 * @param pSource The bytes.
 * @param nSourceLength The number of bytes.
 * @param pDest The destination, which may be the source.
 * @return The length of the member.
 * @ghidraAddress NTSC-U/C: 0x00283648
 * @ghidraAddress PAL: 0x0028cef8
 */
int GzipCompressRamToRam(const void *pSource, int nSourceLength, void *pDest);

/**
 * Write the name of the compressed copy of a file.
 *
 * The name is inferred.
 *
 * @param pszFile The file.
 * @param bGenerated Non-zero for the copy under `gen` beside the file, `<dir>/gen/<base>.<ext>.gz`.
 *                   Otherwise the copy is the file with `.gz` appended.
 * @param pszOut Receives the name.
 * @ghidraAddress NTSC-U/C: 0x002836b8
 * @ghidraAddress PAL: 0x0028cf68
 */
void GzipFileName(const char *pszFile, int bGenerated, char *pszOut);

/**
 * Report the stored length of a file.
 *
 * The path is opened, measured by seeking to its end, rewound, and closed. A gzip file reports its
 * compressed size, unlike FileTrueSize().
 *
 * @param pszPath The file to measure.
 * @return The length in bytes, or 0 when the file could not be opened.
 */
int FileSize(const char *pszPath);

/**
 * Report the uncompressed length of a file.
 *
 * It opens the path, measures it from the ark directory record, the gzip trailer, or the file size,
 * and reports the length without reporting the handle. A caller that needs the file opens it again.
 *
 * @param pszPath The file to measure.
 * @return The uncompressed length, or zero or less when the file could not be opened.
 */
int FileTrueSize(const char *pszPath);

/** The window length the decompressor fills before each flush. */
constexpr unsigned kGzipWindowSize = 0x8000;

extern "C" {

/**
 * The decompressor window.
 *
 * The image reserves two window lengths, and only the first is written.
 */
extern unsigned char gzipWindow[2 * kGzipWindowSize];

/**
 * The write position in gzipWindow.
 */
extern unsigned gzipOutcnt;

/**
 * The staging buffer refilled from the memory source or the file.
 */
extern unsigned char gzipInbuf[];

/**
 * The valid byte count in gzipInbuf.
 */
extern unsigned gzipInsize;

/**
 * The read position in gzipInbuf.
 */
extern unsigned gzipInptr;

/**
 * Refill the staging buffer from the memory source or the file and report its first byte.
 *
 * @param nSilentEof Nonzero to report -1 rather than an error when a file has no more bytes.
 * @return The first byte of the refilled buffer, or -1 at a silent end of file.
 */
int GzipRefillInputBuffer(int nSilentEof);

/**
 * Checksum the filled part of gzipWindow, copy it to the output, and rewind the window.
 *
 * An empty window is not flushed.
 */
void GzipFlushWindow();

} // extern "C"

/**
 * Read one byte through the staging buffer, refilling the buffer when it is exhausted.
 *
 * @return The byte.
 */
inline int GzipGetByte() {
    if (gzipInptr < gzipInsize) {
        return gzipInbuf[gzipInptr++];
    }
    return GzipRefillInputBuffer(0);
}
