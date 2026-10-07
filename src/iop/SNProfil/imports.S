/* Import tables of SNProfil.irx, in the order the shipped module lists them. */

#include "iopimports.s"

    IMPORT_TABLE intrman, 0x0102
    IMPORT CpuEnableIntr, 9
    IMPORT_END

    IMPORT_TABLE sifcmd, 0x0101
    IMPORT sceSifInitRpc, 14
    IMPORT sceSifRegisterRpc, 17
    IMPORT sceSifSetRpcQueue, 19
    IMPORT sceSifRpcLoop, 22
    IMPORT_END

    IMPORT_TABLE stdio, 0x0103
    IMPORT printf, 4
    IMPORT_END

    IMPORT_TABLE thbase, 0x0101
    IMPORT CreateThread, 4
    IMPORT StartThread, 6
    IMPORT GetThreadId, 20
    IMPORT_END
