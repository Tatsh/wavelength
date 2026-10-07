/*
 * Import tables of synth_s.irx, in the order and with the versions and function numbers of the
 * shipped module. sysclib function 5 is imported and never called.
 */

#include "iopimports.s"

    IMPORT_TABLE sysmem, 0x0101
    IMPORT AllocSysMemory, 4
    IMPORT FreeSysMemory, 5
    IMPORT_END

    IMPORT_TABLE intrman, 0x0102
    IMPORT EnableIntr, 6
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
    IMPORT_END

    IMPORT_TABLE stdio, 0x0103
    IMPORT printf, 4
    IMPORT_END

    IMPORT_TABLE sysclib, 0x0103
    IMPORT sysclib_5, 5
    IMPORT memcpy, 12
    IMPORT memset, 14
    IMPORT strcmp, 22
    IMPORT_END

    IMPORT_TABLE thbase, 0x0102
    IMPORT CreateThread, 4
    IMPORT DeleteThread, 5
    IMPORT StartThread, 6
    IMPORT TerminateThread, 10
    IMPORT GetThreadId, 20
    IMPORT SleepThread, 24
    IMPORT WakeupThread, 25
    IMPORT iWakeupThread, 26
    IMPORT DelayThread, 33
    IMPORT GetSystemTime, 34
    IMPORT USec2SysClock, 39
    IMPORT_END

    IMPORT_TABLE thsemap, 0x0101
    IMPORT CreateSema, 4
    IMPORT DeleteSema, 5
    IMPORT SignalSema, 6
    IMPORT WaitSema, 8
    IMPORT_END

    IMPORT_TABLE timrman, 0x0103
    IMPORT AllocHardTimer, 4
    IMPORT FreeHardTimer, 6
    IMPORT SetTimerHandler, 20
    IMPORT SetupHardTimer, 22
    IMPORT StartHardTimer, 23
    IMPORT StopHardTimer, 24
    IMPORT_END

    IMPORT_TABLE libsd, 0x0105
    IMPORT sceSdInit, 4
    IMPORT sceSdSetParam, 5
    IMPORT sceSdGetParam, 6
    IMPORT sceSdSetSwitch, 7
    IMPORT sceSdGetSwitch, 8
    IMPORT sceSdSetAddr, 9
    IMPORT sceSdSetCoreAttr, 11
    IMPORT sceSdNote2Pitch, 13
    IMPORT sceSdVoiceTrans, 17
    IMPORT sceSdVoiceTransStatus, 19
    IMPORT sceSdSetEffectAttr, 23
    IMPORT sceSdClearEffectWorkArea, 25
    IMPORT sceSdSetTransIntrHandler, 26
    IMPORT_END
