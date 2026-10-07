/*
 * Export table of libnetb.irx. Export 0 is the module entry and exports 1 to 3 share a routine
 * that returns at once.
 */

#include "iopexports.s"

    EXPORT_TABLE Libnet, 0x0101
    EXPORT start
    EXPORT LibnetExportStub
    EXPORT LibnetExportStub
    EXPORT LibnetExportStub
    EXPORT_END

/* NTSC-U/C: 0x00003a68, PAL: 0x000038d8 */
    .globl LibnetExportStub
    .type LibnetExportStub, @function
LibnetExportStub:
    jr $31
    nop
