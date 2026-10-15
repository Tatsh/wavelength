# The LoadExecPS2() correction the game installs in the kernel of an early console. InitExecPS2()
# copies the patch to kExecPatchBase and installs its system calls. The patch keeps the OSD
# configuration itself, reads the defaults from the boot ROM, and replaces LoadExecPS2() with a
# version that ends every other thread before it starts the new program. Every address the patch
# uses is the address of the copy, and the kernel routine table lists the routines of that kernel.

    .set noreorder
    .set noat

    .equ kExecPatchBase, 0x80074000
    .equ kExecRomConfig, 0x80074700
    .equ kExecKernelTable, 0x80074728
    .equ kExecEntryTable, 0x80074780
    .equ kExecConfig, 0x800747a8
    .equ kExecConfig2, 0x800747b0
    .equ kExecGetEntryAddressOffset, 0x0
    .equ kExecGetConfigOffset, 0x88
    .equ kExecSetConfigOffset, 0x138
    .equ kExecReadRomConfigOffset, 0x1e8
    .equ kExecGetConfig2Offset, 0x2a8
    .equ kExecSetConfig2Offset, 0x350
    .equ kExecResetSystemOffset, 0x3c8
    .equ kExecLoadExecOffset, 0x498
    .equ kExecRomConfigOffset, 0x700
    .equ kExecKernelTableOffset, 0x728
    .equ kExecEntryTableOffset, 0x780
    .equ kExecPatchSize, 1960
    .equ kExecEntryCount, 5
    .equ kExecRomConfigSize, 38
    .equ kExecConfig2Size, 128
    .equ kExecRomBase, 0xbc000000
    .equ kExecRomConfigPointer, 0xbc0003c0
    .equ kExecRomConfigSkip, 15
    .equ kExecLoadExecId, 0xffffc402
    .equ kJalOpcode, 0x0c000000
    .equ kJalTargetMask, 0x03ffffff
    .equ kIntcStat, 0x1000f000
    .equ kIntcVblankStart, 4
    .equ kThreadEntrySize, 76
    .equ kThreadMax, 256
    .equ kThreadStatusDormant, 16
    .equ kIntcMaskAllButGs, 0xdffd
    .equ kTimerMaskAll, 127
    .equ kSyscallSetOsdConfigParam, 74
    .equ kSyscallGetOsdConfigParam, 75
    .equ kSyscallSetOsdConfigParam2, 110
    .equ kSyscallGetOsdConfigParam2, 111

    .data
    .align 3

# NTSC-U/C: 0x003c4898, PAL: 0x0045ace0
    .globl g_execPatch
g_execPatch:
ExecPatchStart:
# 0x80074000
# Reads the boot ROM defaults, then returns the kernel address of a patched system call from the
# entry table, or zero. The LoadExecPS2() replacement is returned as the jal instruction that
# reaches it.
    .org ExecPatchStart + kExecGetEntryAddressOffset
ExecGetEntryAddress:
    addiu	$sp,$sp,-32
    addiu	$a1,$zero,kExecRomConfigSize
    sd	$s0,0($sp)
    daddu	$s0,$a0,$zero
    sd	$ra,16($sp)
    lui	$a0,%hi(kExecRomConfig)
    jal	kExecPatchBase + kExecReadRomConfigOffset
    addiu	$a0,$a0,%lo(kExecRomConfig)
    lui	$v0,%hi(kExecEntryTable)
    lui	$a1,kExecLoadExecId >> 16
    lui	$a2,kJalTargetMask >> 16
    lui	$a3,kJalOpcode >> 16
    addiu	$v1,$v0,%lo(kExecEntryTable)
    daddu	$a0,$zero,$zero
    ori	$a1,$a1,kExecLoadExecId & 0xffff
    ori	$a2,$a2,kJalTargetMask & 0xffff
.LExec0:
    lw	$v0,0($v1)
    bnel	$s0,$v0,.LExec1
    addiu	$a0,$a0,1
    bne	$s0,$a1,.LExec2
    lw	$v0,4($v1)
    srl	$v0,$v0,2
    and	$v0,$v0,$a2
    beq	$zero,$zero,.LExec2
    or	$v0,$v0,$a3
.LExec1:
    sltiu	$v0,$a0,kExecEntryCount
    bnez	$v0,.LExec0
    addiu	$v1,$v1,8
    daddu	$v0,$zero,$zero
.LExec2:
    ld	$ra,16($sp)
    ld	$s0,0($sp)
    jr	$ra
    addiu	$sp,$sp,32
    nop

# 0x80074088
# System call 75. Copies the stored configuration word into the caller's word one field at a time,
# then the language half.
    .org ExecPatchStart + kExecGetConfigOffset
ExecGetOsdConfigParam:
    lui	$a1,%hi(kExecConfig)
    lw	$v1,0($a0)
    lw	$v0,%lo(kExecConfig)($a1)
    addiu	$a2,$zero,-2
    and	$v1,$v1,$a2
    addiu	$a3,$zero,-7
    andi	$v0,$v0,0x1
    addiu	$8,$zero,-9
    or	$v1,$v1,$v0
    addiu	$9,$zero,-17
    sw	$v1,0($a0)
    addiu	$10,$zero,-8161
    and	$v1,$v1,$a3
    lui	$a2,0xffff
    lw	$v0,%lo(kExecConfig)($a1)
    ori	$a2,$a2,0x1fff
    addiu	$a3,$a1,%lo(kExecConfig)
    andi	$v0,$v0,0x6
    or	$v1,$v1,$v0
    sw	$v1,0($a0)
    and	$v1,$v1,$8
    lw	$v0,%lo(kExecConfig)($a1)
    andi	$v0,$v0,0x8
    or	$v1,$v1,$v0
    sw	$v1,0($a0)
    and	$v1,$v1,$9
    lw	$v0,%lo(kExecConfig)($a1)
    andi	$v0,$v0,0x10
    or	$v1,$v1,$v0
    sw	$v1,0($a0)
    and	$v1,$v1,$10
    lw	$v0,%lo(kExecConfig)($a1)
    andi	$v0,$v0,0x1fe0
    or	$v1,$v1,$v0
    sw	$v1,0($a0)
    and	$v1,$v1,$a2
    lw	$v0,%lo(kExecConfig)($a1)
    andi	$v0,$v0,0xe000
    or	$v1,$v1,$v0
    sw	$v1,0($a0)
    lhu	$v0,2($a3)
    jr	$ra
    sh	$v0,2($a0)
    nop

# 0x80074138
# System call 74. Stores the caller's configuration word one field at a time, then the language
# half.
    .org ExecPatchStart + kExecSetConfigOffset
ExecSetOsdConfigParam:
    lui	$a1,%hi(kExecConfig)
    lw	$v0,0($a0)
    lw	$v1,%lo(kExecConfig)($a1)
    addiu	$a2,$zero,-2
    andi	$v0,$v0,0x1
    addiu	$a3,$zero,-7
    and	$v1,$v1,$a2
    addiu	$8,$zero,-9
    or	$v1,$v1,$v0
    addiu	$9,$zero,-17
    sw	$v1,%lo(kExecConfig)($a1)
    addiu	$10,$zero,-8161
    and	$v1,$v1,$a3
    lui	$a2,0xffff
    lw	$v0,0($a0)
    ori	$a2,$a2,0x1fff
    addiu	$a3,$a1,%lo(kExecConfig)
    andi	$v0,$v0,0x6
    or	$v1,$v1,$v0
    sw	$v1,%lo(kExecConfig)($a1)
    and	$v1,$v1,$8
    lw	$v0,0($a0)
    andi	$v0,$v0,0x8
    or	$v1,$v1,$v0
    sw	$v1,%lo(kExecConfig)($a1)
    and	$v1,$v1,$9
    lw	$v0,0($a0)
    andi	$v0,$v0,0x10
    or	$v1,$v1,$v0
    sw	$v1,%lo(kExecConfig)($a1)
    and	$v1,$v1,$10
    lw	$v0,0($a0)
    andi	$v0,$v0,0x1fe0
    or	$v1,$v1,$v0
    sw	$v1,%lo(kExecConfig)($a1)
    and	$v1,$v1,$a2
    lw	$v0,0($a0)
    andi	$v0,$v0,0xe000
    or	$v1,$v1,$v0
    sw	$v1,%lo(kExecConfig)($a1)
    lhu	$v0,2($a0)
    jr	$ra
    sh	$v0,2($a3)
    nop

# 0x800741e8
# Copies the configuration defaults from the boot ROM, when the ROM has them, and clears three
# fields of a version 0 configuration.
    .org ExecPatchStart + kExecReadRomConfigOffset
ExecReadRomConfig:
    lui	$a2,%hi(kExecRomConfigPointer)
    lw	$a2,%lo(kExecRomConfigPointer)($a2)
    beqz	$a2,.LExec4
    lui	$8,%hi(kExecRomConfig)
    lui	$v0,kExecRomBase >> 16
    addu	$a2,$a2,$v0
    addiu	$a3,$8,%lo(kExecRomConfig)
    addiu	$a2,$a2,kExecRomConfigSkip
    daddu	$a1,$zero,$zero
    nop
.LExec3:
    addu	$v0,$a2,$a1
    addu	$a0,$a3,$a1
    lbu	$v1,0($v0)
    addiu	$a1,$a1,1
    slti	$v0,$a1,kExecRomConfigSize
    sb	$v1,0($a0)
    bnez	$v0,.LExec3
    nop
    beq	$zero,$zero,.LExec5
    ld	$v1,%lo(kExecRomConfig)($8)
.LExec4:
    ld	$v1,%lo(kExecRomConfig)($8)
.LExec5:
    dsll	$v0,$v1,26
    dsra32	$v0,$v0,0
    andi	$v0,$v0,0x7
    bnez	$v0,.LExec6
    nop
    addiu	$v0,$zero,-257
    dsll	$v0,$v0,16
    ori	$v0,$v0,0xffff
    dsll	$v0,$v0,16
    ori	$v0,$v0,0xffff
    addiu	$a0,$zero,-3073
    dsll	$a0,$a0,16
    ori	$a0,$a0,0xffff
    dsll	$a0,$a0,16
    ori	$a0,$a0,0xffff
    and	$v0,$v1,$v0
    lui	$v1,0xffff
    ori	$v1,$v1,0xfff
    dsll	$v1,$v1,16
    ori	$v1,$v1,0xffff
    dsll	$v1,$v1,16
    ori	$v1,$v1,0xffff
    and	$v0,$v0,$a0
    and	$v0,$v0,$v1
    sd	$v0,%lo(kExecRomConfig)($8)
.LExec6:
    jr	$ra
    nop

# 0x800742a8
# System call 111. Copies bytes of the stored extended configuration, limited to its 128 bytes,
# and returns the configuration version, or zero for version 0.
    .org ExecPatchStart + kExecGetConfig2Offset
ExecGetOsdConfigParam2:
    addu	$a3,$a1,$a2
    sltiu	$v0,$a3,kExecConfig2Size + 1
    bnez	$v0,.LExec8
    daddu	$10,$a0,$zero
    sltiu	$v0,$a2,kExecConfig2Size
    beqz	$v0,.LExec7
    addiu	$v0,$zero,kExecConfig2Size
    beq	$zero,$zero,.LExec8
    subu	$a1,$v0,$a2
.LExec7:
    addiu	$a2,$zero,kExecConfig2Size
    daddu	$a1,$zero,$zero
.LExec8:
    addu	$a1,$a1,$a2
    sltu	$v0,$a2,$a1
    beqz	$v0,.LExec10
    daddu	$8,$zero,$zero
    daddu	$a3,$a1,$zero
    lui	$9,%hi(kExecRomConfig)
    lui	$a1,%hi(kExecConfig2)
.LExec9:
    addiu	$v0,$a1,%lo(kExecConfig2)
    addu	$a0,$10,$8
    addu	$v0,$a2,$v0
    addiu	$8,$8,1
    lbu	$v1,0($v0)
    addiu	$a2,$a2,1
    sltu	$v0,$a2,$a3
    bnez	$v0,.LExec9
    sb	$v1,0($a0)
    beq	$zero,$zero,.LExec11
    ld	$v1,%lo(kExecRomConfig)($9)
.LExec10:
    lui	$9,%hi(kExecRomConfig)
    ld	$v1,%lo(kExecRomConfig)($9)
.LExec11:
    dsll	$v0,$v1,26
    dsra32	$v0,$v0,0
    andi	$v0,$v0,0x7
    bnez	$v0,.LExec12
    nop
    jr	$ra
    daddu	$v0,$zero,$zero
.LExec12:
    dsrl32	$v0,$v1,12
    jr	$ra
    andi	$v0,$v0,0xf
    nop

# 0x80074350
# System call 110. Stores bytes into the extended configuration, limited to its 128 bytes.
    .org ExecPatchStart + kExecSetConfig2Offset
ExecSetOsdConfigParam2:
    daddu	$v1,$a1,$zero
    addu	$a1,$v1,$a2
    sltiu	$v0,$a1,kExecConfig2Size + 1
    bnez	$v0,.LExec15
    daddu	$9,$a0,$zero
    sltiu	$v0,$a2,kExecConfig2Size
    beqz	$v0,.LExec13
    addiu	$v0,$zero,kExecConfig2Size
    beq	$zero,$zero,.LExec14
    subu	$v1,$v0,$a2
.LExec13:
    addiu	$a2,$zero,kExecConfig2Size
    daddu	$v1,$zero,$zero
.LExec14:
    addu	$a1,$v1,$a2
.LExec15:
    sltu	$v0,$a2,$a1
    beqz	$v0,.LExec17
    daddu	$a3,$zero,$zero
    lui	$8,%hi(kExecConfig2)
    nop
.LExec16:
    addu	$v0,$9,$a3
    addiu	$v1,$8,%lo(kExecConfig2)
    addu	$v1,$a2,$v1
    lbu	$a0,0($v0)
    addiu	$a2,$a2,1
    addiu	$a3,$a3,1
    sltu	$v0,$a2,$a1
    bnez	$v0,.LExec16
    sb	$a0,0($v1)
.LExec17:
    jr	$ra
    nop
    nop

# 0x800743c8
# Waits for the start of a vertical blank and resets the devices through the kernel routines.
    .org ExecPatchStart + kExecResetSystemOffset
ExecResetSystem:
    addiu	$sp,$sp,-16
    lui	$v0,kIntcStat >> 16
    sd	$ra,0($sp)
    addiu	$v1,$zero,kIntcVblankStart
    ori	$v0,$v0,kIntcStat & 0xffff
    lui	$a0,kIntcStat >> 16
    sw	$v1,0($v0)
    ori	$a0,$a0,kIntcStat & 0xffff
.LExec18:
    lw	$v0,0($a0)
    andi	$v0,$v0,kIntcVblankStart
    nop
    nop
    nop
    beqz	$v0,.LExec18
    nop
    lui	$v0,kIntcStat >> 16
    addiu	$a0,$zero,kIntcVblankStart
    ori	$v0,$v0,kIntcStat & 0xffff
    lui	$v1,%hi(kExecKernelTable + 56)
    sw	$a0,0($v0)
    lw	$v0,%lo(kExecKernelTable + 56)($v1)
    jalr	$v0
    nop
    lui	$v0,%hi(kExecKernelTable + 60)
    addiu	$a1,$zero,2
    lw	$v1,%lo(kExecKernelTable + 60)($v0)
    addiu	$a2,$zero,1
    jalr	$v1
    addiu	$a0,$zero,1
    lui	$v0,%hi(kExecKernelTable + 36)
    lw	$v1,%lo(kExecKernelTable + 36)($v0)
    jalr	$v1
    ori	$a0,$zero,kIntcMaskAllButGs
    lui	$v0,%hi(kExecKernelTable + 40)
    lw	$v1,%lo(kExecKernelTable + 40)($v0)
    jalr	$v1
    nop
    lui	$v0,%hi(kExecKernelTable + 52)
    lw	$v1,%lo(kExecKernelTable + 52)($v0)
    jalr	$v1
    addiu	$a0,$zero,kTimerMaskAll
    lui	$v0,%hi(kExecKernelTable + 44)
    lw	$v1,%lo(kExecKernelTable + 44)($v0)
    jalr	$v1
    nop
    lui	$v0,%hi(kExecKernelTable + 48)
    lw	$v1,%lo(kExecKernelTable + 48)($v0)
    jalr	$v1
    nop
    ld	$ra,0($sp)
    jr	$ra
    addiu	$sp,$sp,16

# 0x80074498
# The LoadExecPS2() replacement. Ends every thread but the caller, resets the system, copies the
# arguments, and makes the caller the thread of the new program.
    .org ExecPatchStart + kExecLoadExecOffset
ExecLoadExecPS2:
    addiu	$sp,$sp,-176
    lui	$v0,%hi(kExecKernelTable)
    sd	$fp,144($sp)
    lui	$v1,%hi(kExecKernelTable + 8)
    sd	$s7,128($sp)
    addiu	$fp,$zero,kThreadStatusDormant
    sd	$s6,112($sp)
    lui	$s7,%hi(kExecKernelTable + 28)
    sd	$s5,96($sp)
    lui	$s6,%hi(kExecKernelTable + 32)
    sd	$s4,80($sp)
    daddu	$s5,$a0,$zero
    sd	$s2,48($sp)
    daddu	$s4,$a2,$zero
    sd	$s1,32($sp)
    lui	$s2,%hi(kExecKernelTable + 24)
    sd	$s0,16($sp)
    addiu	$s1,$zero,kThreadEntrySize
    lw	$8,%lo(kExecKernelTable)($v0)
    addiu	$s0,$zero,1
    sd	$ra,160($sp)
    sw	$a1,0($sp)
    sd	$s3,64($sp)
    lw	$s3,0($8)
    lw	$v0,%lo(kExecKernelTable + 8)($v1)
    sw	$a3,4($sp)
    jalr	$v0
    daddu	$a0,$s3,$zero
    lui	$v1,%hi(kExecKernelTable + 12)
    daddu	$a0,$s3,$zero
    lw	$v0,%lo(kExecKernelTable + 12)($v1)
    jalr	$v0
    daddu	$a1,$zero,$zero
    nop
.LExec19:
    lw	$v0,%lo(kExecKernelTable + 32)($s6)
    addu	$v0,$s1,$v0
    lw	$v0,8($v0)
    beqzl	$v0,.LExec21
    addiu	$s0,$s0,1
    beql	$s0,$s3,.LExec21
    addiu	$s0,$s0,1
    bne	$v0,$fp,.LExec20
    lw	$v0,%lo(kExecKernelTable + 28)($s7)
    lw	$v0,%lo(kExecKernelTable + 24)($s2)
    jalr	$v0
    daddu	$a0,$s0,$zero
    beq	$zero,$zero,.LExec21
    addiu	$s0,$s0,1
.LExec20:
    jalr	$v0
    daddu	$a0,$s0,$zero
    lw	$v0,%lo(kExecKernelTable + 24)($s2)
    jalr	$v0
    daddu	$a0,$s0,$zero
    addiu	$s0,$s0,1
.LExec21:
    slti	$v0,$s0,kThreadMax
    bnez	$v0,.LExec19
    addiu	$s1,$s1,kThreadEntrySize
    lui	$v1,%hi(kExecKernelTable + 20)
    lw	$v0,%lo(kExecKernelTable + 20)($v1)
    jalr	$v0
    daddu	$s0,$zero,$zero
    lui	$v1,%hi(kExecKernelTable + 16)
    lw	$v0,%lo(kExecKernelTable + 16)($v1)
    jalr	$v0
    nop
    lui	$v1,%hi(kExecKernelTable + 4)
    lw	$v0,%lo(kExecKernelTable + 4)($v1)
    jal	kExecPatchBase + kExecResetSystemOffset
    sw	$zero,0($v0)
    lui	$v0,%hi(kExecKernelTable + 80)
    blez	$s4,.LExec23
    lw	$a0,%lo(kExecKernelTable + 80)($v0)
    lui	$s1,%hi(kExecKernelTable + 72)
    lw	$a1,4($sp)
.LExec22:
    sll	$v1,$s0,2
    lw	$v0,%lo(kExecKernelTable + 72)($s1)
    addiu	$s0,$s0,1
    addu	$v1,$v1,$a1
    jalr	$v0
    lw	$a1,0($v1)
    daddu	$a0,$v0,$zero
    slt	$v0,$s0,$s4
    bnez	$v0,.LExec22
    lw	$a1,4($sp)
.LExec23:
    addiu	$v0,$zero,kThreadEntrySize
    lui	$v1,%hi(kExecKernelTable + 80)
    mult	$v0,$s3,$v0
    lw	$a1,%lo(kExecKernelTable + 32)($s6)
    lw	$a0,%lo(kExecKernelTable + 80)($v1)
    lui	$v1,%hi(kExecKernelTable + 68)
    lw	$a2,%lo(kExecKernelTable + 68)($v1)
    addu	$v0,$v0,$a1
    sw	$a0,56($v0)
    lw	$a1,0($sp)
    sw	$s4,52($v0)
    sw	$s5,12($v0)
    sw	$s5,48($v0)
    sw	$a1,20($v0)
    sh	$zero,26($v0)
    sh	$zero,24($v0)
    sw	$zero,36($v0)
    sw	$zero,28($v0)
    jalr	$a2
    sw	$zero,32($v0)
    lui	$v0,%hi(kExecKernelTable + 64)
    lw	$v1,%lo(kExecKernelTable + 64)($v0)
    jalr	$v1
    nop
    daddu	$v0,$s5,$zero
    ld	$ra,160($sp)
    ld	$fp,144($sp)
    ld	$s7,128($sp)
    ld	$s6,112($sp)
    ld	$s5,96($sp)
    ld	$s4,80($sp)
    ld	$s3,64($sp)
    ld	$s2,48($sp)
    ld	$s1,32($sp)
    ld	$s0,16($sp)
    jr	$ra
    addiu	$sp,$sp,176

# The configuration defaults ExecReadRomConfig() reads from the boot ROM.
    .org ExecPatchStart + kExecRomConfigOffset
    .word 0x40
    .org ExecPatchStart + kExecKernelTableOffset
# The routines and data of the kernel the patch reaches through this table, by the use the patch
# makes of each.
    .word 0x800125ec # The identifier of the running thread.
    .word 0x800125f4 # A flag the replacement clears before the reset.
    .word 0x80004970 # Called with the running thread.
    .word 0x80004288 # Called with the running thread and zero.
    .word 0x800021b0 # Called after every other thread ends.
    .word 0x80004e68 # Called after every other thread ends, first.
    .word 0x80003f00 # Deletes a thread.
    .word 0x80003e00 # Ends a thread that is not dormant.
    .word 0x80017400 # The thread records, 76 bytes each.
    .word 0x8000b8d0 # Called with the interrupt causes to mask.
    .word 0x8000b900 # Called during the reset.
    .word 0x8000b7a8 # Called during the reset.
    .word 0x8000b840 # Called during the reset.
    .word 0x8000ad68 # Called with the timers to stop.
    .word 0x8000aa60 # Called first in the reset.
    .word 0x8000a060 # Called with 1, 2, and 1.
    .word 0x80002a80 # Called last, to start the new program.
    .word 0x80002ac0 # Called once the caller's record is set.
    .word 0x80005560 # Copies an argument string and returns the end of the copy.
    .word 0x80012600
    .word 0x80012608 # Where the argument strings are copied.
    .org ExecPatchStart + kExecEntryTableOffset
    .word kSyscallSetOsdConfigParam, kExecPatchBase + kExecSetConfigOffset
    .word kSyscallGetOsdConfigParam, kExecPatchBase + kExecGetConfigOffset
    .word kSyscallSetOsdConfigParam2, kExecPatchBase + kExecSetConfig2Offset
    .word kSyscallGetOsdConfigParam2, kExecPatchBase + kExecGetConfig2Offset
    .word kExecLoadExecId, kExecPatchBase + kExecLoadExecOffset
    .org ExecPatchStart + kExecPatchSize
