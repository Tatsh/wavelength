/*
 * Assembler macros for the export table of an IOP module. The table is the magic word, a link word
 * the loader fills, the version, the library name padded to eight bytes, and the address of each
 * export in export number order, closed by a zero word. A module lists its table in an assembler
 * source that includes this file.
 */

    .set noreorder
    .set noat
    .text

/* Opens the table of the library the module provides. */
    .macro EXPORT_TABLE library, version
    .align 2
    .word 0x41c00000
    .word 0
    .word \version
1:
    .ascii "\library"
    .space 8 - (. - 1b)
    .endm

/* Emits the next export. */
    .macro EXPORT name
    .word \name
    .endm

/* A zero word closes the table. */
    .macro EXPORT_END
    .word 0
    .endm
