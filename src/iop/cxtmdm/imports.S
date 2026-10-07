/*
 * Import tables of cxtmdm.irx, in the order and with the versions and function numbers of the
 * shipped module.
 */

#include "iopimports.s"

    IMPORT_TABLE usbd, 0x0101
    IMPORT sceUsbdRegisterLdd, 4
    IMPORT sceUsbdUnregisterLdd, 5
    IMPORT sceUsbdScanStaticDescriptor, 6
    IMPORT sceUsbdSetPrivateData, 7
    IMPORT sceUsbdGetPrivateData, 8
    IMPORT sceUsbdOpenPipe, 9
    IMPORT sceUsbdTransferPipe, 11
    IMPORT sceUsbdGetDeviceLocation, 13
    IMPORT_END

    IMPORT_TABLE modem, 0x0101
    IMPORT ModemRegisterDevice, 4
    IMPORT ModemUnregisterDevice, 5
    IMPORT_END

    IMPORT_TABLE netdev, 0x0101
    IMPORT NetdevAllocMemory, 6
    IMPORT NetdevFreeMemory, 7
    IMPORT_END

    IMPORT_TABLE intrman, 0x0102
    IMPORT CpuSuspendIntr, 17
    IMPORT CpuResumeIntr, 18
    IMPORT_END

    IMPORT_TABLE stdio, 0x0103
    IMPORT printf, 4
    IMPORT_END

    IMPORT_TABLE sysclib, 0x0102
    IMPORT memcmp, 11
    IMPORT memcpy, 12
    IMPORT memset, 14
    IMPORT bcopy, 16
    IMPORT bzero, 17
    IMPORT sprintf, 19
    IMPORT strcmp, 22
    IMPORT strcpy, 23
    IMPORT strlen, 27
    IMPORT strncmp, 29
    IMPORT_END

    IMPORT_TABLE thbase, 0x0101
    IMPORT CreateThread, 4
    IMPORT DeleteThread, 5
    IMPORT StartThread, 6
    IMPORT TerminateThread, 10
    IMPORT ChangeThreadPriority, 14
    IMPORT DelayThread, 33
    IMPORT SetAlarm, 35
    IMPORT CancelAlarm, 37
    IMPORT USec2SysClock, 39
    IMPORT_END

    IMPORT_TABLE thevent, 0x0101
    IMPORT CreateEventFlag, 4
    IMPORT DeleteEventFlag, 5
    IMPORT SetEventFlag, 6
    IMPORT iSetEventFlag, 7
    IMPORT WaitEventFlag, 10
    IMPORT_END

    IMPORT_TABLE thsemap, 0x0101
    IMPORT CreateSema, 4
    IMPORT DeleteSema, 5
    IMPORT SignalSema, 6
    IMPORT WaitSema, 8
    IMPORT_END
