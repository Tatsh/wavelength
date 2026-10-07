/* Import tables of libnetb.irx, in the order the shipped module lists them. */

#include "iopimports.s"

    IMPORT_TABLE msifrpc, 0x0101
    IMPORT msifrpc_4, 4
    IMPORT msifrpc_17, 17
    IMPORT_END

    IMPORT_TABLE inet, 0x0101
    IMPORT sceInetName2Address, 4
    IMPORT sceInetAddress2String, 5
    IMPORT inet_6, 6
    IMPORT inet_7, 7
    IMPORT inet_8, 8
    IMPORT sceInetRecv, 9
    IMPORT sceInetSend, 10
    IMPORT inet_11, 11
    IMPORT sceInetRecvFrom, 12
    IMPORT sceInetSendTo, 13
    IMPORT inet_14, 14
    IMPORT sceInetControl, 15
    IMPORT inet_16, 16
    IMPORT inet_24, 24
    IMPORT sceInetInterfaceControl, 25
    IMPORT inet_27, 27
    IMPORT inet_30, 30
    IMPORT inet_36, 36
    IMPORT inet_38, 38
    IMPORT inet_41, 41
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
    IMPORT_END

    IMPORT_TABLE sifman, 0x0101
    IMPORT sceSifSetDma, 7
    IMPORT sceSifDmaStat, 8
    IMPORT_END

    IMPORT_TABLE stdio, 0x0103
    IMPORT printf, 4
    IMPORT_END

    IMPORT_TABLE sysclib, 0x0103
    IMPORT memset, 14
    IMPORT sprintf, 19
    IMPORT strcmp, 22
    IMPORT strlen, 27
    IMPORT strncmp, 29
    IMPORT strncpy, 30
    IMPORT strstr, 34
    IMPORT strtol, 36
    IMPORT_END

    IMPORT_TABLE thbase, 0x0102
    IMPORT CreateThread, 4
    IMPORT DeleteThread, 5
    IMPORT StartThread, 6
    IMPORT ExitThread, 8
    IMPORT TerminateThread, 10
    IMPORT GetThreadId, 20
    IMPORT CheckThreadStack, 21
    IMPORT ReferThreadStatus, 22
    IMPORT SleepThread, 24
    IMPORT DelayThread, 33
    IMPORT_END

    IMPORT_TABLE thsemap, 0x0101
    IMPORT CreateSema, 4
    IMPORT DeleteSema, 5
    IMPORT SignalSema, 6
    IMPORT WaitSema, 8
    IMPORT_END
