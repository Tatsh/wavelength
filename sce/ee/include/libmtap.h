#ifndef LIBMTAP_H
#define LIBMTAP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Multitap access through the mtapman server on the IOP. */

/**
 * Bind the five mtapman servers and check the mtapman version.
 *
 * @return 1, or zero when a bind request cannot be sent or on a version mismatch.
 * @ghidraAddress NTSC-U/C: 0x00312b68
 * @ghidraAddress PAL: 0x0037f200
 */
int sceMtapInit(void);

/**
 * Start watching a port for a multitap.
 *
 * @param nPort Port.
 * @return mtapman's result, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312da8
 * @ghidraAddress PAL: 0x0037f440
 */
int sceMtapPortOpen(int nPort);

/**
 * Stop watching a port for a multitap.
 *
 * @param nPort Port.
 * @return mtapman's result, or zero when the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312e18
 * @ghidraAddress PAL: 0x0037f4b0
 */
int sceMtapPortClose(int nPort);

/**
 * Report whether a multitap is connected to a port.
 *
 * @param nPort Port.
 * @return 1 when a multitap is connected, or zero when none is or the call failed.
 * @ghidraAddress NTSC-U/C: 0x00312e88
 * @ghidraAddress PAL: 0x0037f520
 */
int sceMtapGetConnection(int nPort);

/**
 * Report the mtapman version.
 *
 * @return The major version in bits 8 to 15 and the minor in bits 0 to 7, or zero when the call
 * failed.
 * @ghidraAddress NTSC-U/C: 0x00312ef8
 * @ghidraAddress PAL: 0x0037f590
 */
int sceMtapGetModVersion(void);

#ifdef __cplusplus
}
#endif

#endif
