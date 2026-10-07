/*
 * Assembler macros for the import tables of an IOP module. The IOP loader finds each table by its
 * magic word, resolves the library by name and version, and rewrites every two-instruction stub
 * of the table into a jump to the export with the number that the stub's second instruction
 * records. A module lists its tables in an assembler source that includes this file.
 */

    .set noreorder
    .set noat
    .text

/* Opens the table of one library. The header is the magic word, a link word the loader fills, the
   version, and the library name padded to eight bytes. */
    .macro IMPORT_TABLE library, version
    .align 2
    .word 0x41e00000
    .word 0
    .word \version
1:
    .ascii "\library"
    .space 8 - (. - 1b)
    .endm

/* Emits one stub. The loader patches its jr into a j to the export numbered index. */
    .macro IMPORT name, index
    .globl \name
    .type \name, @function
\name:
    jr $31
    addiu $0, $0, \index
    .endm

/* Two zero words close a table. */
    .macro IMPORT_END
    .word 0
    .word 0
    .endm
