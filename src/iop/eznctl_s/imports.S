/* Import tables of eznctl_s.irx, in the order the shipped module lists them. */

#include "iopimports.s"

    /* The module never calls the sysmem exports. */
    IMPORT_TABLE sysmem, 0x0101
    IMPORT AllocSysMemory, 4
    IMPORT FreeSysMemory, 5
    IMPORT_END

    IMPORT_TABLE sifcmd, 0x0101
    IMPORT sceSifInitRpc, 14
    IMPORT sceSifRegisterRpc, 17
    IMPORT sceSifSetRpcQueue, 19
    IMPORT sceSifRpcLoop, 22
    IMPORT sceSifRemoveRpc, 24
    IMPORT sceSifRemoveRpcQueue, 25
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
    IMPORT strcat, 20
    IMPORT strcmp, 22
    IMPORT strcpy, 23
    IMPORT strlen, 27
    IMPORT strncmp, 29
    IMPORT_END

    IMPORT_TABLE thbase, 0x0102
    IMPORT CreateThread, 4
    IMPORT DeleteThread, 5
    IMPORT StartThread, 6
    IMPORT TerminateThread, 10
    IMPORT iReleaseWaitThread, 19
    IMPORT GetThreadId, 20
    IMPORT SetAlarm, 35
    IMPORT CancelAlarm, 37
    IMPORT USec2SysClock, 39
    IMPORT_END

    IMPORT_TABLE thevent, 0x0101
    IMPORT CreateEventFlag, 4
    IMPORT DeleteEventFlag, 5
    IMPORT SetEventFlag, 6
    IMPORT WaitEventFlag, 10
    IMPORT_END

    IMPORT_TABLE netcnf, 0x0120
    IMPORT sceNetCnfLoadEntry, 6
    IMPORT sceNetCnfAllocMem, 10
    IMPORT sceNetCnfInitInterface, 11
    IMPORT sceNetCnfName2Address, 15
    IMPORT netcnf_20, 20
    IMPORT_END

    IMPORT_TABLE inet, 0x0101
    IMPORT sceInetName2Address, 4
    IMPORT sceInetAddress2String, 5
    IMPORT sceInetInterfaceControl, 25
    IMPORT_END

    IMPORT_TABLE inetctl, 0x0101
    IMPORT inetctl_4, 4
    IMPORT inetctl_5, 5
    IMPORT inetctl_6, 6
    IMPORT inetctl_7, 7
    IMPORT sceInetCtlRegisterEventHandler, 8
    IMPORT sceInetCtlUnregisterEventHandler, 9
    IMPORT sceInetCtlGetState, 10
    IMPORT_END
