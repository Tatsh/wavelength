#pragma once

extern "C" {

/**
 * Mark the gzip module initialized.
 *
 * Only GzipInit() reads the flag.
 *
 * @ghidraAddress NTSC-U/C: 0x002834b8
 * @ghidraAddress PAL: 0x0028cd68
 */
void GzipInit();

/**
 * Decompress or compress the gzip input to a buffer.
 *
 * Decompression reads the member header and inflates the data without checking the trailer.
 * Compression writes a whole member. The file is not closed.
 *
 * The name is inferred.
 *
 * @param nFile The file to read, or -1 for the memory source.
 * @param pDest The destination.
 * @param bCompress Non-zero to compress, zero to decompress.
 * @return The number of bytes written, or -1 on failure.
 * @ghidraAddress NTSC-U/C: 0x002834d8
 * @ghidraAddress PAL: 0x0028cd88
 */
int GzipFdToRam(int nFile, void *pDest, int bCompress);

/**
 * Inflate a gzip member that is already in memory, naming the input `(in-memory)`.
 *
 * The source and destination windows may overlap, and every caller in the async layer relies on
 * the overlap. The stored bytes sit against the end of the destination, and the inflate runs
 * forward over the whole destination.
 *
 * @param pSource The stored bytes.
 * @param nSourceLength The number of bytes the inflate may read.
 * @param pDest The destination.
 * @return The number of bytes written, or -1 on failure.
 * @ghidraAddress NTSC-U/C: 0x002835b0
 * @ghidraAddress PAL: 0x0028ce60
 */
int GzipDecompressRamToRam(const void *pSource, int nSourceLength, void *pDest);

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
int GzipSizeDecompressed(const void *pSource, int nSourceLength);

/**
 * Deflate bytes in memory into a gzip member, naming the input `(in-memory)`.
 *
 * The name is inferred.
 *
 * @param pSource The bytes.
 * @param nSourceLength The number of bytes.
 * @param pDest The destination, which may be the source.
 * @return The length of the member, or -1 on failure.
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

} // extern "C"
