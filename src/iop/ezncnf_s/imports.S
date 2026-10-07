/* Import tables of ezncnf_s.irx, in the order the shipped module lists them. */

#include "iopimports.s"

    IMPORT_TABLE sysmem, 0x0101
    IMPORT AllocSysMemory, 4
    IMPORT FreeSysMemory, 5
    IMPORT_END

    IMPORT_TABLE intrman, 0x0102
    IMPORT CpuSuspendIntr, 17
    IMPORT CpuResumeIntr, 18
    IMPORT_END

    IMPORT_TABLE sifcmd, 0x0101
    IMPORT sceSifInitRpc, 14
    IMPORT sceSifRegisterRpc, 17
    IMPORT sceSifSetRpcQueue, 19
    IMPORT sceSifRpcLoop, 22
    IMPORT sceSifRemoveRpc, 24
    IMPORT sceSifRemoveRpcQueue, 25
    IMPORT_END

    IMPORT_TABLE sifman, 0x0101
    IMPORT sceSifSetDma, 7
    IMPORT sceSifDmaStat, 8
    IMPORT_END

    IMPORT_TABLE stdio, 0x0103
    IMPORT printf, 4
    IMPORT_END

    /* The module never calls export 5 of sysclib, and its name is not identified. */
    IMPORT_TABLE sysclib, 0x0103
    IMPORT sysclib_5, 5
    IMPORT look_ctype_table, 8
    IMPORT memset, 14
    IMPORT bzero, 17
    IMPORT strcmp, 22
    IMPORT strcpy, 23
    IMPORT strncmp, 29
    IMPORT strncpy, 30
    IMPORT strtol, 36
    IMPORT_END

    IMPORT_TABLE thbase, 0x0102
    IMPORT CreateThread, 4
    IMPORT DeleteThread, 5
    IMPORT StartThread, 6
    IMPORT TerminateThread, 10
    IMPORT GetThreadId, 20
    IMPORT_END

    IMPORT_TABLE netcnf, 0x0120
    IMPORT sceNetCnfGetCount, 4
    IMPORT sceNetCnfGetList, 5
    IMPORT sceNetCnfLoadEntry, 6
    IMPORT sceNetCnfAddress2String, 16
    IMPORT netcnf_21, 21
    IMPORT netcnf_22, 22
    IMPORT_END
