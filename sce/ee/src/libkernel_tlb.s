# The TLB extension the game installs in the kernel. InitTLBFunctions() copies the patch to
# kTlbPatchBase and installs its system calls. Every address the patch uses is the address of the
# copy. The return handler and the two templates run from the program text.

    .set noreorder
    .set noat

    .equ kTlbPatchBase, 0x80075000
    .equ kTlbEntryTable, 0x80075300
    .equ kTlbExecArgs, 0x80075330
    .equ kTlbGetEntryAddressOffset, 0x0
    .equ kTlbPutOffset, 0x38
    .equ kTlbSetOffset, 0xc8
    .equ kTlbGetOffset, 0x108
    .equ kTlbProbeOffset, 0x158
    .equ kTlbExpandScratchPadOffset, 0x1a8
    .equ kTlbEntryTableOffset, 0x300
    .equ kTlbPatchSize, 816
    .equ kTlbEntryCount, 6
    .equ kTlbEntries, 48
    .equ kTlbUnmappedBase, 0xe0010000
    .equ kScratchPadExpansion, 0x70004000
    .equ kPageMask, 0xfffff000
    .equ kPageOffsetMask, 0xfff
    .equ kLargestExpansionPage, 0xffffe
    .equ kEntryLoFlags, 0x1f
    .equ kEntryLoPfnShift, 6
    .equ kPageSize, 4096
    .equ kSegmentShift, 24
    .equ kSegmentMask, 0xf0
    .equ kSegmentMain, 0x00
    .equ kSegmentUncached, 0x20
    .equ kSegmentUncachedAccelerated, 0x30
    .equ kSegmentScratchPad, 0x40
    .equ kSegmentRegisters, 0x10
    .equ kSegmentReserved, 0x50
    .equ kStatusIe, 0x01
    .equ kStatusExl, 0x02
    .equ kStatusUserMode, 0x10
    .equ kStatusKsuMask, 0x18
    .equ kSyscallPutTLBEntry, 85
    .equ kSyscallSetTLBEntry, 86
    .equ kSyscallGetTLBEntry, 87
    .equ kSyscallProbeTLBEntry, 88
    .equ kSyscallExpandScratchPad, 89
    .equ kTlbEntryExecArgs, 3
    .equ kSyscallReturnToUser, -84
    .equ kTemplateStackSize, 4096

# The coprocessor 0 registers are numbered: Index 0, EntryLo0 2, EntryLo1 3, PageMask 5, Wired 6,
# EntryHi 10, Status 12, and EPC 14.

    .text

# NTSC-U/C: 0x0031d180
# A template the binary does not reference. Calls the handler the first word points at on its
# own stack, then returns to the kernel through system call -84.
    .align 6
TlbCallTemplate:
    lui	$at,0
    lw	$at,0($at)
    lui	$sp,0
    jalr	$at
    addiu	$sp,$sp,kTemplateStackSize
    addiu	$v1,$zero,kSyscallReturnToUser
    syscall
    .org TlbCallTemplate + 64

# NTSC-U/C: 0x0031d1c0, PAL: 0x00389b00
# System call 84. Restores the registers of the user thread from the save area and returns to it.
    .globl HandleReturnFromExceptionToUserMode
    .type HandleReturnFromExceptionToUserMode, @function
    .ent HandleReturnFromExceptionToUserMode
HandleReturnFromExceptionToUserMode:
    mfc0	$at,$12
    addiu	$k0,$zero,~(kStatusKsuMask | kStatusExl | kStatusIe)
    and	$at,$at,$k0
    mtc0	$at,$12
    sync.p
    lui	$v0,%hi(g_tlbSavedEpc)
    lw	$v0,%lo(g_tlbSavedEpc)($v0)
    mtc0	$v0,$14
    sync.p
    lui	$v0,%hi(g_tlbSavedHi)
    ld	$v0,%lo(g_tlbSavedHi)($v0)
    mthi	$v0
    lui	$v0,%hi(g_tlbSavedHi1)
    ld	$v0,%lo(g_tlbSavedHi1)($v0)
    mthi1	$v0
    lui	$v0,%hi(g_tlbSavedLo)
    ld	$v0,%lo(g_tlbSavedLo)($v0)
    mtlo	$v0
    lui	$v0,%hi(g_tlbSavedLo1)
    ld	$v0,%lo(g_tlbSavedLo1)($v0)
    mtlo1	$v0
    lui	$v0,%hi(g_tlbSavedSa)
    ld	$v0,%lo(g_tlbSavedSa)($v0)
    mtsa	$v0
    sync.p
    lui	$k0,%hi(g_tlbSavedRegisters)
    addiu	$k0,$k0,%lo(g_tlbSavedRegisters)
    lq	$at,16($k0)
    lq	$v0,32($k0)
    lq	$v1,48($k0)
    lq	$a0,64($k0)
    lq	$a1,80($k0)
    lq	$a2,96($k0)
    lq	$a3,112($k0)
    lq	$8,128($k0)
    lq	$9,144($k0)
    lq	$10,160($k0)
    lq	$11,176($k0)
    lq	$12,192($k0)
    lq	$13,208($k0)
    lq	$14,224($k0)
    lq	$15,240($k0)
    lq	$s0,256($k0)
    lq	$s1,272($k0)
    lq	$s2,288($k0)
    lq	$s3,304($k0)
    lq	$s4,320($k0)
    lq	$s5,336($k0)
    lq	$s6,352($k0)
    lq	$s7,368($k0)
    lq	$t8,384($k0)
    lq	$t9,400($k0)
    lq	$gp,448($k0)
    lq	$sp,464($k0)
    lq	$fp,480($k0)
    lq	$ra,496($k0)
    mfc0	$k0,$12
    ori	$k0,$k0,kStatusUserMode | kStatusExl | kStatusIe
    mtc0	$k0,$12
    sync.p
    eret
    .end HandleReturnFromExceptionToUserMode
    .size HandleReturnFromExceptionToUserMode, . - HandleReturnFromExceptionToUserMode

# NTSC-U/C: 0x0031d2c0
# A template the binary does not reference. Calls the handler at the table entry v0 selects on
# its own stack, and stops at a break should the handler return.
    .align 6
TlbDispatchTemplate:
    lui	$at,0
    addu	$at,$at,$v0
    lw	$at,0($at)
    lui	$sp,0
    jalr	$at
    addiu	$sp,$sp,kTemplateStackSize
    break	0x3ff,0x3ff
    nop

    .data
    .align 3

# NTSC-U/C: 0x003c5808
    .globl g_tlbPatch
g_tlbPatch:
TlbPatchStart:
# 0x80075000
# Returns the kernel address of a TLB system call from the entry table, or zero.
    .org TlbPatchStart + kTlbGetEntryAddressOffset
TlbGetEntryAddress:
    lui	$v0,%hi(kTlbEntryTable)
    daddu	$a1,$zero,$zero
    addiu	$v1,$v0,%lo(kTlbEntryTable)
    nop
.LTlb0:
    lw	$v0,0($v1)
    bne	$a0,$v0,.LTlb1
    addiu	$a1,$a1,1
    jr	$ra
    lw	$v0,4($v1)
.LTlb1:
    sltiu	$v0,$a1,kTlbEntryCount
    bnez	$v0,.LTlb0
    addiu	$v1,$v1,8
    jr	$ra
    daddu	$v0,$zero,$zero

# 0x80075038
# System call 85. Writes a random entry for main memory, its uncached views, or the scratchpad,
# and returns the index the entry landed at, or -1 for another segment.
    .org TlbPatchStart + kTlbPutOffset
TlbPutEntry:
    srl	$v0,$a1,kSegmentShift
    andi	$v1,$v0,kSegmentMask
    addiu	$v0,$zero,kSegmentUncachedAccelerated
    beq	$v1,$v0,.LTlb5
    sltiu	$v0,$v1,kSegmentUncachedAccelerated + 1
    beqz	$v0,.LTlb2
    addiu	$v0,$zero,kSegmentRegisters
    beq	$v1,$v0,.LTlb4
    sltiu	$v0,$v1,kSegmentRegisters + 1
    beqz	$v0,.LTlb3
    addiu	$v0,$zero,kSegmentUncached
    beqz	$v1,.LTlb5
    addiu	$v0,$zero,-1
    beq	$zero,$zero,.LTlb6
    nop
.LTlb2:
    addiu	$v0,$zero,kSegmentReserved
    beq	$v1,$v0,.LTlb4
    sltiu	$v0,$v1,kSegmentReserved + 1
    beqz	$v0,.LTlb4
    addiu	$v0,$zero,kSegmentScratchPad
.LTlb3:
    beq	$v1,$v0,.LTlb5
    nop
.LTlb4:
    jr	$ra
    addiu	$v0,$zero,-1
.LTlb5:
    mtc0	$a0,$5
    mtc0	$a1,$10
    mtc0	$a2,$2
    mtc0	$a3,$3
    sync.p
    tlbwr
    sync.p
    tlbp
    sync.p
    mfc0	$v0,$0
.LTlb6:
    jr	$ra
    nop

# 0x800750c8
# System call 86. Writes the entry at an index and returns the index, or -1 past the last entry.
    .org TlbPatchStart + kTlbSetOffset
TlbSetEntry:
    sltiu	$v0,$a0,kTlbEntries
    beqz	$v0,.LTlb7
    addiu	$v0,$zero,-1
    mtc0	$a0,$0
    mtc0	$a1,$5
    mtc0	$a2,$10
    mtc0	$a3,$2
    mtc0	$8,$3
    sync.p
    tlbwi
    sync.p
    jr	$ra
    daddu	$v0,$a0,$zero
.LTlb7:
    jr	$ra
    nop

# 0x80075108
# System call 87. Reads the entry at an index into four words and returns the index, or -1 past
# the last entry.
    .org TlbPatchStart + kTlbGetOffset
TlbGetEntry:
    sltiu	$v0,$a0,kTlbEntries
    bnez	$v0,.LTlb8
    nop
    jr	$ra
    addiu	$v0,$zero,-1
.LTlb8:
    mtc0	$a0,$0
    sync.p
    tlbr
    sync.p
    mfc0	$v0,$5
    sw	$v0,0($a1)
    mfc0	$v1,$10
    sw	$v1,0($a2)
    mfc0	$v0,$2
    sw	$v0,0($a3)
    mfc0	$v1,$3
    sw	$v1,0($8)
    jr	$ra
    daddu	$v0,$a0,$zero

# 0x80075158
# System call 88. Finds the entry for a virtual address, reads it into three words, and returns
# its index, or -1 when no entry maps the address.
    .org TlbPatchStart + kTlbProbeOffset
TlbProbeEntry:
    mtc0	$a0,$10
    sync.p
    tlbp
    sync.p
    mfc0	$a0,$0
    bgez	$a0,.LTlb9
    nop
    beq	$zero,$zero,.LTlb10
    addiu	$a0,$zero,-1
.LTlb9:
    tlbr
    sync.p
    mfc0	$v0,$5
    sw	$v0,0($a1)
    mfc0	$v1,$2
    sw	$v1,0($a2)
    mfc0	$v0,$3
    sw	$v0,0($a3)
.LTlb10:
    jr	$ra
    daddu	$v0,$a0,$zero

# 0x800751a8
# System call 89. Maps two pages of main memory after the scratchpad, wiring a new entry the
# first time, or removes the mapping when the page is zero. Returns the index, zero after a
# removal, or -1 for a page that is not aligned or lies below 1 MB.
    .org TlbPatchStart + kTlbExpandScratchPadOffset
TlbExpandScratchPad:
    addiu	$sp,$sp,-48
    sd	$s0,16($sp)
    daddu	$s0,$a0,$zero
    andi	$v0,$s0,kPageOffsetMask
    bnez	$v0,.LTlb11
    sd	$ra,32($sp)
    lui	$v0,kLargestExpansionPage >> 16
    addiu	$v1,$s0,-1
    ori	$v0,$v0,kLargestExpansionPage & 0xffff
    sltu	$v0,$v0,$v1
    bnez	$v0,.LTlb12
    lui	$a0,kScratchPadExpansion >> 16
.LTlb11:
    beq	$zero,$zero,.LTlb15
    addiu	$v0,$zero,-1
.LTlb12:
    daddu	$a1,$sp,$zero
    ori	$a0,$a0,kScratchPadExpansion & 0xffff
    ori	$a2,$sp,4
    jal	kTlbPatchBase + kTlbProbeOffset
    ori	$a3,$sp,8
    daddu	$a1,$v0,$zero
    bgez	$a1,.LTlb13
    nop
    beqz	$s0,.LTlb15
    daddu	$v0,$zero,$zero
    mfc0	$a1,$6
    addiu	$v0,$a1,1
    mtc0	$v0,$6
    sync.p
    beq	$zero,$zero,.LTlb14
    nop
.LTlb13:
    bnez	$s0,.LTlb14
    addiu	$v0,$a1,-1
    lui	$v1,kTlbUnmappedBase >> 16
    sll	$v0,$v0,13
    addu	$a2,$v0,$v1
    mfc0	$v0,$6
    addiu	$v0,$v0,-1
    mtc0	$v0,$6
    mtc0	$a1,$0
    mtc0	$zero,$5
    mtc0	$a2,$10
    mtc0	$zero,$2
    mtc0	$zero,$3
    sync.p
    tlbwi
    sync.p
    beq	$zero,$zero,.LTlb15
    daddu	$v0,$zero,$zero
.LTlb14:
    lui	$v0,kPageMask >> 16
    addiu	$a0,$s0,kPageSize
    ori	$v0,$v0,kPageMask & 0xffff
    lui	$a2,kScratchPadExpansion >> 16
    and	$a0,$a0,$v0
    sw	$zero,0($sp)
    and	$v0,$s0,$v0
    srl	$a0,$a0,kEntryLoPfnShift
    srl	$v0,$v0,kEntryLoPfnShift
    ori	$a0,$a0,kEntryLoFlags
    ori	$v0,$v0,kEntryLoFlags
    ori	$a2,$a2,kScratchPadExpansion & 0xffff
    sw	$v0,4($sp)
    sw	$a0,8($sp)
    mtc0	$a1,$0
    daddu	$v1,$zero,$zero
    mtc0	$v1,$5
    mtc0	$a2,$10
    mtc0	$v0,$2
    mtc0	$a0,$3
    sync.p
    tlbwi
    sync.p
    daddu	$v0,$a1,$zero
.LTlb15:
    ld	$ra,32($sp)
    ld	$s0,16($sp)
    jr	$ra
    addiu	$sp,$sp,48

    .org TlbPatchStart + kTlbEntryTableOffset
    .word kSyscallPutTLBEntry, kTlbPatchBase + kTlbPutOffset
    .word kSyscallSetTLBEntry, kTlbPatchBase + kTlbSetOffset
    .word kSyscallGetTLBEntry, kTlbPatchBase + kTlbGetOffset
    .word kSyscallProbeTLBEntry, kTlbPatchBase + kTlbProbeOffset
    .word kSyscallExpandScratchPad, kTlbPatchBase + kTlbExpandScratchPadOffset
    .word kTlbEntryExecArgs, kTlbExecArgs
    .org TlbPatchStart + kTlbPatchSize

    .bss
    .align 7

# NTSC-U/C: 0x00537f80
# The registers of the user thread HandleReturnFromExceptionToUserMode restores, one quadword
# each in register order.
g_tlbSavedRegisters:
    .space 512

# NTSC-U/C: 0x00538180
g_tlbSavedHi:
    .space 8

# NTSC-U/C: 0x00538188
g_tlbSavedHi1:
    .space 8

# NTSC-U/C: 0x00538190
g_tlbSavedLo:
    .space 8

# NTSC-U/C: 0x00538198
g_tlbSavedLo1:
    .space 8

# NTSC-U/C: 0x005381a0
g_tlbSavedSa:
    .space 8

# NTSC-U/C: 0x005381a8
g_tlbSavedEpc:
    .space 4
