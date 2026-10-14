#ifndef RTRECORD_H
#define RTRECORD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Little-endian fields of the records of the SCE-RT channels. The routines report 0 for success, or
 * 2 for a null argument.
 */

/**
 * Write a byte.
 *
 * @param pbField The field.
 * @param uValue The value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5a68
 * @ghidraAddress PAL: 0x002cf9d8
 */
int RtRecordWriteByte(unsigned char *pbField, unsigned char uValue);

/**
 * Read a byte.
 *
 * @param pbField The field.
 * @param puValue Receives the value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5a88
 * @ghidraAddress PAL: 0x002cf9f8
 */
int RtRecordReadByte(const unsigned char *pbField, unsigned char *puValue);

/**
 * Write a 16-bit value, low byte first.
 *
 * @param pbField The field.
 * @param uValue The value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5ab0
 * @ghidraAddress PAL: 0x002cfa20
 */
int RtRecordWriteWord(unsigned char *pbField, unsigned short uValue);

/**
 * Read a 16-bit value, low byte first.
 *
 * @param pbField The field.
 * @param puValue Receives the value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5ad8
 * @ghidraAddress PAL: 0x002cfa48
 */
int RtRecordReadWord(const unsigned char *pbField, unsigned short *puValue);

/**
 * Write a 32-bit value, low byte first.
 *
 * @param pbField The field.
 * @param uValue The value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5b10
 * @ghidraAddress PAL: 0x002cfa80
 */
int RtRecordWriteDword(unsigned char *pbField, unsigned int uValue);

/**
 * Read a 32-bit value, low byte first.
 *
 * @param pbField The field.
 * @param puValue Receives the value.
 * @return 0, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c5b48
 * @ghidraAddress PAL: 0x002cfab8
 */
int RtRecordReadDword(const unsigned char *pbField, unsigned int *puValue);

#ifdef __cplusplus
}
#endif

#endif
