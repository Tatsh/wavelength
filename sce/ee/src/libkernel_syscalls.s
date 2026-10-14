# The kernel system call stubs the game uses. Each loads its call number into v1 and traps. A
# negative number selects the form that is safe inside an interrupt handler.

    .set noreorder

    .macro SYSCALL_STUB name, number
    .globl \name
    .type \name, @function
    .ent \name
\name:
    addiu $v1, $zero, \number
    syscall
    jr $ra
    nop
    .end \name
    .size \name, . - \name
    .endm

    .text

# NTSC-U/C: 0x00316b20, PAL: 0x003833e0
    SYSCALL_STUB SetGsCrt, 2

# NTSC-U/C: 0x00316b40, PAL: 0x00383400
    SYSCALL_STUB _Exit, 4

# NTSC-U/C: 0x00316b60, PAL: 0x00383420
    SYSCALL_STUB _LoadExecPS2, 6

# NTSC-U/C: 0x00316c00, PAL: 0x003834c0
    SYSCALL_STUB AddIntcHandler, 16

# NTSC-U/C: 0x00316c20, PAL: 0x003834e0
    SYSCALL_STUB RemoveIntcHandler, 17

# NTSC-U/C: 0x00316c30, PAL: 0x003834f0
    SYSCALL_STUB AddDmacHandler, 18

# NTSC-U/C: 0x00316c40, PAL: 0x00383500
    SYSCALL_STUB AddDmacHandler2, 18

# NTSC-U/C: 0x00316c50, PAL: 0x00383510
    SYSCALL_STUB RemoveDmacHandler, 19

# NTSC-U/C: 0x00316c60, PAL: 0x00383520
    SYSCALL_STUB _EnableIntc, 20

# NTSC-U/C: 0x00316c70, PAL: 0x00383530
    SYSCALL_STUB _DisableIntc, 21

# NTSC-U/C: 0x00316c80, PAL: 0x00383540
    SYSCALL_STUB _EnableDmac, 22

# NTSC-U/C: 0x00316c90, PAL: 0x00383550
    SYSCALL_STUB _DisableDmac, 23

# NTSC-U/C: 0x00316ca0, PAL: 0x00383570
    SYSCALL_STUB SetAlarm, 252

# NTSC-U/C: 0x00316d00, PAL: 0x003835c0
    SYSCALL_STUB iSetAlarm, -254

# NTSC-U/C: 0x00316d20, PAL: 0x003835e0
    SYSCALL_STUB CreateThread, 32

# NTSC-U/C: 0x00316d30, PAL: 0x003835f0
    SYSCALL_STUB DeleteThread, 33

# NTSC-U/C: 0x00316d40, PAL: 0x00383630
    SYSCALL_STUB StartThread, 34

# NTSC-U/C: 0x00316d60
    SYSCALL_STUB ExitDeleteThread, 36

# NTSC-U/C: 0x00316d70
    SYSCALL_STUB TerminateThread, 37

# NTSC-U/C: 0x00316db0, PAL: 0x00383670
    SYSCALL_STUB ChangeThreadPriority, 41

# NTSC-U/C: 0x00316dd0, PAL: 0x00383690
    SYSCALL_STUB RotateThreadReadyQueue, 43

# NTSC-U/C: 0x00316e10, PAL: 0x003836d0
    SYSCALL_STUB GetThreadId, 47

# NTSC-U/C: 0x00316e20, PAL: 0x003836e0
    SYSCALL_STUB ReferThreadStatus, 48

# NTSC-U/C: 0x00316e40, PAL: 0x00383700
    SYSCALL_STUB SleepThread, 50

# NTSC-U/C: 0x00316e50, PAL: 0x00383710
    SYSCALL_STUB WakeupThread, 51

# NTSC-U/C: 0x00316e60, PAL: 0x00383720
    SYSCALL_STUB _iWakeupThread, -52

# NTSC-U/C: 0x00316e90, PAL: 0x00383750
    SYSCALL_STUB SuspendThread, 55

# NTSC-U/C: 0x00316f00, PAL: 0x003837c0
    SYSCALL_STUB EndOfHeap, 62

# NTSC-U/C: 0x00316f20, PAL: 0x003837e0
    SYSCALL_STUB CreateSema, 64

# NTSC-U/C: 0x00316f30, PAL: 0x003837f0
    SYSCALL_STUB DeleteSema, 65

# NTSC-U/C: 0x00316f40, PAL: 0x00383800
    SYSCALL_STUB SignalSema, 66

# NTSC-U/C: 0x00316f50, PAL: 0x00383810
    SYSCALL_STUB iSignalSema, -67

# NTSC-U/C: 0x00316f60, PAL: 0x00383820
    SYSCALL_STUB WaitSema, 68

# NTSC-U/C: 0x00316f70, PAL: 0x00383830
    SYSCALL_STUB PollSema, 69

# NTSC-U/C: 0x00316fc0, PAL: 0x00383880
    SYSCALL_STUB SetOsdConfigParam, 74

# NTSC-U/C: 0x00316fd0, PAL: 0x00383890
    SYSCALL_STUB GetOsdConfigParam, 75

# NTSC-U/C: 0x003171a0, PAL: 0x00383a60
    SYSCALL_STUB FlushCache, 100

# NTSC-U/C: 0x00317230, PAL: 0x00383af0
    SYSCALL_STUB GetOsdConfigParam2, 111

# NTSC-U/C: 0x00317240, PAL: 0x00383b00
    SYSCALL_STUB GsGetIMR, 112

# NTSC-U/C: 0x00317260, PAL: 0x00383b20
    SYSCALL_STUB GsPutIMR, 113

# NTSC-U/C: 0x00317290, PAL: 0x00383b50
    SYSCALL_STUB SetVSyncFlag, 115

# NTSC-U/C: 0x00317350, PAL: 0x00383c10
    SYSCALL_STUB Deci2Call, 124

# NTSC-U/C: 0x00317380, PAL: 0x00383c40
    SYSCALL_STUB GetMemorySize, 127

# NTSC-U/C: 0x00317390, PAL: 0x00383c50
    SYSCALL_STUB _InitTLB, 130
