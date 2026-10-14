#pragma once

/**
 * Blowfish in CBC mode over the packets of an online session.
 *
 * The routines share one cipher state, keyed once by NetCryptInit(). The names are inferred.
 */

/** The bytes of the key and of the initial vector NetCryptSetKey() takes. */
constexpr int kNetCryptKeySize = 16;

/** The words NetCryptMakeKey() draws. */
constexpr int kNetCryptKeyWords = 4;

/**
 * Register Blowfish and start the shared CBC state with the default key and initial vector.
 *
 * @ghidraAddress NTSC-U/C: 0x0026b4d0
 * @ghidraAddress PAL: 0x00275060
 */
void NetCryptInit();

/**
 * Draw a random key and initial vector for NetCryptSetKey().
 *
 * The routine stores kNetCryptKeyWords random words.
 *
 * @param pKey Receives the bytes.
 * @ghidraAddress NTSC-U/C: 0x0026b578
 * @ghidraAddress PAL: 0x00275108
 */
void NetCryptMakeKey(unsigned char pKey[kNetCryptKeySize]);

/**
 * Replace the stored key and initial vector.
 *
 * The cipher state keeps the key NetCryptInit() set up, and only the initial vector takes effect.
 *
 * @param pKey The eight bytes of the key followed by the eight bytes of the initial vector.
 * @ghidraAddress NTSC-U/C: 0x0026b5c0
 * @ghidraAddress PAL: 0x00275150
 */
void NetCryptSetKey(const unsigned char pKey[kNetCryptKeySize]);

/**
 * Encrypt the whole eight-byte blocks of a buffer in place, from the stored initial vector.
 *
 * @param pData The buffer.
 * @param nSize The size of the buffer in bytes.
 * @ghidraAddress NTSC-U/C: 0x0026b5f8
 * @ghidraAddress PAL: 0x00275188
 */
void NetCryptEncrypt(void *pData, int nSize);

/**
 * Decrypt the whole eight-byte blocks of a buffer in place, from the stored initial vector.
 *
 * @param pData The buffer.
 * @param nSize The size of the buffer in bytes.
 * @ghidraAddress NTSC-U/C: 0x0026b688
 * @ghidraAddress PAL: 0x00275218
 */
void NetCryptDecrypt(void *pData, int nSize);
