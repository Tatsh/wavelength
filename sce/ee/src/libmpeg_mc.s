# The motion compensation kernels of libmpeg. Each kernel takes one prediction descriptor that
# libmpeg.c builds and writes the prediction rows of a macroblock to the scratchpad. The add and put
# routines saturate the prediction, with or without the decoded block, into the frame.

    .set noreorder
    .set noat

    .text
    # The clamp constant inside sceMpegMcPutBlock() must stay quadword aligned.
    .align 4
    nop
    nop

# NTSC-U/C: 0x0030a308, PAL: 0x00376798
    .globl sceMpegMcLumaCopy
    .type sceMpegMcLumaCopy, @function
sceMpegMcLumaCopy:
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    sll	$11,$12,0x1
    addiu	$15,$0,-1
    mtsab	$13,0
.LMc0:
    lq	$8,0($5)
    addi	$7,$7,-1
    lq	$9,0($6)
    addu	$5,$5,$12
    qfsrv	$10,$9,$8
    pextlb	$8,$0,$10
    pextub	$9,$0,$10
    sq	$8,0($14)
    addu	$6,$6,$12
    sq	$9,16($14)
    bgtz	$7,.LMc0
    addu	$14,$14,$11
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$10,$15,$7
    bnez	$10,.LMc0
    daddu	$15,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaCopy, .-sceMpegMcLumaCopy

# NTSC-U/C: 0x0030a380, PAL: 0x00376810
    .globl sceMpegMcChromaCopy
    .type sceMpegMcChromaCopy, @function
sceMpegMcChromaCopy:
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    sll	$11,$12,0x1
    mtsab	$13,0
    addiu	$24,$0,-1
.LMc1:
    lw	$7,8($4)
    addiu	$15,$0,-1
.LMc2:
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$9,$8,$8
    pextlb	$8,$0,$9
    sq	$8,0($14)
    addi	$7,$7,-1
    addu	$5,$5,$12
    addu	$14,$14,$11
    bgtz	$7,.LMc2
    addu	$6,$6,$12
    addiu	$5,$5,320
    addiu	$6,$6,320
    lw	$7,12($4)
    and	$10,$15,$7
    bnez	$10,.LMc2
    daddu	$15,$0,$0
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    bnez	$24,.LMc1
    daddu	$24,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcChromaCopy, .-sceMpegMcChromaCopy

# NTSC-U/C: 0x0030a418, PAL: 0x003768a8
    .globl sceMpegMcLumaHalfV
    .type sceMpegMcLumaHalfV, @function
sceMpegMcLumaHalfV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$24,16($4)
    lq	$8,0($5)
    sll	$12,$24,0x1
    lq	$9,0($6)
    mtsab	$13,0
    qfsrv	$10,$9,$8
    pextlb	$8,$0,$10
    addiu	$11,$0,-1
    beqz	$7,.LMc4
    pextub	$9,$0,$10
.LMc3:
    addu	$5,$5,$24
    addu	$6,$6,$24
    lq	$10,0($5)
    lq	$15,0($6)
    qfsrv	$2,$15,$10
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    paddh	$2,$8,$10
    paddh	$3,$9,$15
    por	$8,$10,$0
    por	$9,$15,$0
    paddh	$2,$2,$25
    paddh	$3,$3,$25
    psrlh	$2,$2,0x1
    psrlh	$3,$3,0x1
    sq	$2,0($14)
    sq	$3,16($14)
    bgtz	$7,.LMc3
    addu	$14,$14,$12
.LMc4:
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc3
    daddu	$11,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaHalfV, .-sceMpegMcLumaHalfV

# NTSC-U/C: 0x0030a4d0, PAL: 0x00376960
    .globl sceMpegMcChromaHalfV
    .type sceMpegMcChromaHalfV, @function
sceMpegMcChromaHalfV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    addiu	$11,$0,1
    sll	$24,$12,0x1
    mtsab	$13,0
.LMc5:
    lw	$7,8($4)
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$8,$8,$8
    ori	$11,$11,0x8000
    beqz	$7,.LMc7
    pextlb	$15,$0,$8
.LMc6:
    addu	$5,$5,$12
    addu	$6,$6,$12
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$8,$8,$8
    pextlb	$10,$0,$8
    addi	$7,$7,-1
    paddh	$9,$10,$15
    por	$15,$10,$0
    paddh	$10,$9,$25
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc6
    addu	$14,$14,$24
.LMc7:
    psrah	$10,$11,0xf
    addiu	$5,$5,320
    lw	$7,12($4)
    addiu	$6,$6,320
    and	$10,$10,$7
    bnez	$10,.LMc6
    andi	$11,$11,0x7fff
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    andi	$10,$11,0x1
    bnez	$10,.LMc5
    andi	$11,$11,0xfffe
    jr	$31
    nop
    nop
    .size sceMpegMcChromaHalfV, .-sceMpegMcChromaHalfV

# NTSC-U/C: 0x0030a5a0, PAL: 0x00376a30
    .globl sceMpegMcLumaHalfH
    .type sceMpegMcLumaHalfH, @function
sceMpegMcLumaHalfH:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    addiu	$24,$0,1
    lw	$9,16($4)
    sll	$8,$9,0x1
    addiu	$11,$0,-1
.LMc8:
    lq	$10,0($5)
    lq	$15,0($6)
    mtsab	$13,0
    qfsrv	$2,$15,$10
    qfsrv	$3,$10,$15
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    mtsab	$24,0
    qfsrv	$3,$3,$2
    pextlb	$2,$0,$3
    pextub	$3,$0,$3
    paddh	$10,$10,$2
    paddh	$15,$15,$3
    paddh	$2,$10,$25
    paddh	$3,$15,$25
    psrlh	$2,$2,0x1
    psrlh	$3,$3,0x1
    sq	$2,0($14)
    sq	$3,16($14)
    addu	$5,$5,$9
    addu	$6,$6,$9
    bgtz	$7,.LMc8
    addu	$14,$14,$8
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$12,$11,$7
    bnez	$12,.LMc8
    daddu	$11,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaHalfH, .-sceMpegMcLumaHalfH

# NTSC-U/C: 0x0030a650, PAL: 0x00376ae0
    .globl sceMpegMcChromaHalfH
    .type sceMpegMcChromaHalfH, @function
sceMpegMcChromaHalfH:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    addiu	$24,$0,1
    addiu	$12,$0,-1
    lw	$3,16($4)
    sll	$2,$3,0x1
.LMc9:
    lw	$7,8($4)
    addiu	$11,$0,-1
.LMc10:
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addi	$7,$7,-1
    addu	$5,$5,$3
    addu	$6,$6,$3
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    paddh	$10,$9,$8
    paddh	$10,$10,$25
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc10
    addu	$14,$14,$2
    addiu	$5,$5,320
    addiu	$6,$6,320
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc10
    daddu	$11,$0,$0
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    bnez	$12,.LMc9
    daddu	$12,$0,$0
    jr	$31
    nop
    .size sceMpegMcChromaHalfH, .-sceMpegMcChromaHalfH

# NTSC-U/C: 0x0030a708, PAL: 0x00376b98
    .globl sceMpegMcLumaHalfHV
    .type sceMpegMcLumaHalfHV, @function
sceMpegMcLumaHalfHV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    psllh	$25,$25,0x1
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    addiu	$24,$0,1
    lq	$8,0($5)
    lq	$9,0($6)
    mtsab	$13,0
    qfsrv	$10,$9,$8
    qfsrv	$15,$8,$9
    pextlb	$8,$0,$10
    pextub	$9,$0,$10
    addiu	$11,$0,-1
    mtsab	$24,0
    qfsrv	$15,$15,$10
    pextlb	$10,$0,$15
    pextub	$15,$0,$15
    paddh	$8,$8,$10
    beqz	$7,.LMc12
    paddh	$9,$9,$15
.LMc11:
    addu	$5,$5,$12
    addu	$6,$6,$12
    lq	$10,0($5)
    lq	$15,0($6)
    mtsab	$13,0
    qfsrv	$2,$15,$10
    qfsrv	$3,$10,$15
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    mtsab	$24,0
    qfsrv	$3,$3,$2
    pextlb	$2,$0,$3
    pextub	$3,$0,$3
    paddh	$10,$10,$2
    paddh	$15,$15,$3
    paddh	$2,$8,$10
    paddh	$3,$9,$15
    por	$8,$10,$0
    por	$9,$15,$0
    paddh	$2,$2,$25
    paddh	$3,$3,$25
    psrlh	$2,$2,0x2
    psrlh	$3,$3,0x2
    sq	$2,0($14)
    sll	$10,$12,0x1
    sq	$3,16($14)
    bgtz	$7,.LMc11
    addu	$14,$14,$10
.LMc12:
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc11
    daddu	$11,$0,$0
    jr	$31
    nop
    .size sceMpegMcLumaHalfHV, .-sceMpegMcLumaHalfHV

# NTSC-U/C: 0x0030a800, PAL: 0x00376c90
    .globl sceMpegMcChromaHalfHV
    .type sceMpegMcChromaHalfHV, @function
sceMpegMcChromaHalfHV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    psllh	$25,$25,0x1
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    addiu	$24,$0,1
    addiu	$11,$0,1
.LMc13:
    lw	$7,8($4)
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addu	$5,$5,$12
    ori	$11,$11,0x8000
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    beqz	$7,.LMc15
    paddh	$15,$9,$8
.LMc14:
    addu	$6,$6,$12
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addi	$7,$7,-1
    addu	$5,$5,$12
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    paddh	$10,$9,$8
    paddh	$9,$10,$15
    por	$15,$10,$0
    paddh	$10,$9,$25
    sll	$8,$12,0x1
    psrlh	$10,$10,0x2
    sq	$10,0($14)
    bgtz	$7,.LMc14
    addu	$14,$14,$8
.LMc15:
    psrah	$10,$11,0xf
    addiu	$5,$5,320
    lw	$7,12($4)
    addiu	$6,$6,320
    and	$10,$10,$7
    bnez	$10,.LMc14
    andi	$11,$11,0x7fff
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    andi	$10,$11,0x1
    bnez	$10,.LMc13
    andi	$11,$11,0xfffe
    jr	$31
    nop
    nop
    .size sceMpegMcChromaHalfHV, .-sceMpegMcChromaHalfHV

# NTSC-U/C: 0x0030a900, PAL: 0x00376d90
    .globl sceMpegMcLumaAverageCopy
    .type sceMpegMcLumaAverageCopy, @function
sceMpegMcLumaAverageCopy:
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$9,16($4)
    sll	$8,$9,0x1
    addiu	$11,$0,-1
    mtsab	$13,0
.LMc16:
    lq	$10,0($5)
    lq	$15,0($6)
    qfsrv	$2,$15,$10
    pextlb	$10,$0,$2
    pextub	$15,$0,$2
    lq	$2,0($14)
    lq	$3,16($14)
    paddh	$2,$2,$10
    paddh	$3,$3,$15
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    paddh	$10,$2,$25
    psrlh	$2,$10,0x1
    paddh	$10,$3,$25
    psrlh	$3,$10,0x1
    sq	$2,0($14)
    sq	$3,16($14)
    addi	$7,$7,-1
    addu	$5,$5,$9
    addu	$14,$14,$8
    bgtz	$7,.LMc16
    addu	$6,$6,$9
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$12,$11,$7
    bnez	$12,.LMc16
    daddu	$11,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaAverageCopy, .-sceMpegMcLumaAverageCopy

# NTSC-U/C: 0x0030a9a0, PAL: 0x00376e30
    .globl sceMpegMcChromaAverageCopy
    .type sceMpegMcChromaAverageCopy, @function
sceMpegMcChromaAverageCopy:
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    addiu	$12,$0,-1
    lw	$3,16($4)
    sll	$2,$3,0x1
    mtsab	$13,0
.LMc17:
    lw	$7,8($4)
    addiu	$11,$0,-1
.LMc18:
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addi	$7,$7,-1
    addu	$5,$5,$3
    addu	$6,$6,$3
    lq	$8,0($14)
    paddh	$10,$9,$8
    pcgth	$9,$10,$0
    psrlh	$9,$9,0xf
    paddh	$10,$10,$9
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc18
    addu	$14,$14,$2
    addiu	$5,$5,320
    addiu	$6,$6,320
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc18
    daddu	$11,$0,$0
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    bnez	$12,.LMc17
    daddu	$12,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcChromaAverageCopy, .-sceMpegMcChromaAverageCopy

# NTSC-U/C: 0x0030aa50, PAL: 0x00376ee0
    .globl sceMpegMcLumaAverageHalfV
    .type sceMpegMcLumaAverageHalfV, @function
sceMpegMcLumaAverageHalfV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    lq	$8,0($5)
    lq	$9,0($6)
    mtsab	$13,0
    qfsrv	$10,$9,$8
    sll	$24,$12,0x1
    pextlb	$8,$0,$10
    addiu	$11,$0,-1
    beqz	$7,.LMc20
    pextub	$9,$0,$10
.LMc19:
    addu	$5,$5,$12
    addu	$6,$6,$12
    lq	$10,0($5)
    lq	$15,0($6)
    qfsrv	$2,$15,$10
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    paddh	$2,$8,$10
    paddh	$3,$9,$15
    por	$8,$10,$0
    por	$9,$15,$0
    paddh	$2,$2,$25
    paddh	$3,$3,$25
    psrlh	$2,$2,0x1
    psrlh	$3,$3,0x1
    lq	$10,0($14)
    lq	$15,16($14)
    paddh	$2,$2,$10
    paddh	$3,$3,$15
    paddh	$10,$2,$25
    psrlh	$2,$10,0x1
    pcgth	$10,$3,$0
    psrlh	$10,$10,0xf
    paddh	$10,$3,$10
    psrlh	$3,$10,0x1
    sq	$2,0($14)
    sq	$3,16($14)
    bgtz	$7,.LMc19
    addu	$14,$14,$24
.LMc20:
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc19
    daddu	$11,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaAverageHalfV, .-sceMpegMcLumaAverageHalfV

# NTSC-U/C: 0x0030ab30, PAL: 0x00376fc0
    .globl sceMpegMcChromaAverageHalfV
    .type sceMpegMcChromaAverageHalfV, @function
sceMpegMcChromaAverageHalfV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    addiu	$11,$0,1
    sll	$24,$12,0x1
    mtsab	$13,0
.LMc21:
    lw	$7,8($4)
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$8,$8,$8
    ori	$11,$11,0x8000
    beqz	$7,.LMc23
    pextlb	$15,$0,$8
.LMc22:
    addu	$5,$5,$12
    addu	$6,$6,$12
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    qfsrv	$8,$8,$8
    pextlb	$10,$0,$8
    addi	$7,$7,-1
    paddh	$9,$10,$15
    por	$15,$10,$0
    paddh	$10,$9,$25
    psrlh	$10,$10,0x1
    lq	$8,0($14)
    paddh	$10,$10,$8
    pcgth	$9,$10,$0
    psrlh	$9,$9,0xf
    paddh	$10,$10,$9
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc22
    addu	$14,$14,$24
.LMc23:
    psrah	$10,$11,0xf
    addiu	$5,$5,320
    lw	$7,12($4)
    addiu	$6,$6,320
    and	$10,$10,$7
    bnez	$10,.LMc22
    andi	$11,$11,0x7fff
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    andi	$10,$11,0x1
    bnez	$10,.LMc21
    andi	$11,$11,0xfffe
    jr	$31
    nop
    nop
    .size sceMpegMcChromaAverageHalfV, .-sceMpegMcChromaAverageHalfV

# NTSC-U/C: 0x0030ac18, PAL: 0x003770a8
    .globl sceMpegMcLumaAverageHalfH
    .type sceMpegMcLumaAverageHalfH, @function
sceMpegMcLumaAverageHalfH:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    addiu	$24,$0,1
    lw	$9,16($4)
    sll	$8,$9,0x1
    addiu	$11,$0,-1
.LMc24:
    lq	$10,0($5)
    lq	$15,0($6)
    mtsab	$13,0
    qfsrv	$2,$15,$10
    qfsrv	$3,$10,$15
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    mtsab	$24,0
    qfsrv	$3,$3,$2
    pextlb	$2,$0,$3
    pextub	$3,$0,$3
    paddh	$10,$10,$2
    paddh	$15,$15,$3
    paddh	$2,$10,$25
    paddh	$3,$15,$25
    psrlh	$2,$2,0x1
    psrlh	$3,$3,0x1
    lq	$10,0($14)
    lq	$15,16($14)
    paddh	$2,$2,$10
    paddh	$3,$3,$15
    paddh	$10,$2,$25
    psrlh	$2,$10,0x1
    pcgth	$10,$3,$0
    psrlh	$10,$10,0xf
    paddh	$10,$3,$10
    psrlh	$3,$10,0x1
    sq	$2,0($14)
    sq	$3,16($14)
    addu	$5,$5,$9
    addu	$6,$6,$9
    bgtz	$7,.LMc24
    addu	$14,$14,$8
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$12,$11,$7
    bnez	$12,.LMc24
    daddu	$11,$0,$0
    jr	$31
    nop
    nop
    .size sceMpegMcLumaAverageHalfH, .-sceMpegMcLumaAverageHalfH

# NTSC-U/C: 0x0030acf0, PAL: 0x00377180
    .globl sceMpegMcChromaAverageHalfH
    .type sceMpegMcChromaAverageHalfH, @function
sceMpegMcChromaAverageHalfH:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    addiu	$24,$0,1
    addiu	$12,$0,-1
    lw	$3,16($4)
    sll	$2,$3,0x1
.LMc25:
    lw	$7,8($4)
    addiu	$11,$0,-1
.LMc26:
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addi	$7,$7,-1
    addu	$5,$5,$3
    addu	$6,$6,$3
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    paddh	$10,$9,$8
    paddh	$10,$10,$25
    psrlh	$10,$10,0x1
    lq	$8,0($14)
    paddh	$10,$10,$8
    pcgth	$9,$10,$0
    psrlh	$9,$9,0xf
    paddh	$10,$10,$9
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc26
    addu	$14,$14,$2
    addiu	$5,$5,320
    addiu	$6,$6,320
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc26
    daddu	$11,$0,$0
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    bnez	$12,.LMc25
    daddu	$12,$0,$0
    jr	$31
    nop
    .size sceMpegMcChromaAverageHalfH, .-sceMpegMcChromaAverageHalfH

# NTSC-U/C: 0x0030adc0, PAL: 0x00377250
    .globl sceMpegMcLumaAverageHalfHV
    .type sceMpegMcLumaAverageHalfHV, @function
sceMpegMcLumaAverageHalfHV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    psllh	$25,$25,0x1
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$7,8($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$24,16($4)
    addiu	$12,$0,1
    lq	$8,0($5)
    lq	$9,0($6)
    mtsab	$13,0
    qfsrv	$10,$9,$8
    qfsrv	$15,$8,$9
    pextlb	$8,$0,$10
    pextub	$9,$0,$10
    addiu	$11,$0,-1
    mtsab	$12,0
    qfsrv	$15,$15,$10
    pextlb	$10,$0,$15
    pextub	$15,$0,$15
    paddh	$8,$8,$10
    beqz	$7,.LMc28
    paddh	$9,$9,$15
.LMc27:
    addu	$5,$5,$24
    addu	$6,$6,$24
    lq	$10,0($5)
    lq	$15,0($6)
    mtsab	$13,0
    qfsrv	$2,$15,$10
    qfsrv	$3,$10,$15
    pextlb	$10,$0,$2
    addi	$7,$7,-1
    pextub	$15,$0,$2
    mtsab	$12,0
    qfsrv	$3,$3,$2
    pextlb	$2,$0,$3
    pextub	$3,$0,$3
    paddh	$10,$10,$2
    paddh	$15,$15,$3
    paddh	$2,$8,$10
    paddh	$3,$9,$15
    por	$8,$10,$0
    por	$9,$15,$0
    paddh	$2,$2,$25
    paddh	$3,$3,$25
    psrlh	$2,$2,0x2
    psrlh	$3,$3,0x2
    lq	$10,0($14)
    lq	$15,16($14)
    paddh	$2,$2,$10
    paddh	$3,$3,$15
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    paddh	$10,$2,$25
    psrlh	$2,$10,0x1
    paddh	$10,$3,$25
    psrlh	$3,$10,0x1
    sq	$2,0($14)
    sll	$10,$24,0x1
    sq	$3,16($14)
    bgtz	$7,.LMc27
    addu	$14,$14,$10
.LMc28:
    addiu	$5,$5,128
    addiu	$6,$6,128
    lw	$7,12($4)
    and	$10,$11,$7
    bnez	$10,.LMc27
    daddu	$11,$0,$0
    jr	$31
    nop
    .size sceMpegMcLumaAverageHalfHV, .-sceMpegMcLumaAverageHalfHV

# NTSC-U/C: 0x0030aee0, PAL: 0x00377370
    .globl sceMpegMcChromaAverageHalfHV
    .type sceMpegMcChromaAverageHalfHV, @function
sceMpegMcChromaAverageHalfHV:
    pnor	$25,$0,$0
    psrlh	$25,$25,0xf
    psllh	$25,$25,0x1
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    lw	$13,4($4)
    lw	$12,16($4)
    addiu	$24,$0,1
    addiu	$11,$0,1
.LMc29:
    lw	$7,8($4)
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addu	$5,$5,$12
    ori	$11,$11,0x8000
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    beqz	$7,.LMc31
    paddh	$15,$9,$8
.LMc30:
    addu	$6,$6,$12
    ld	$8,0($5)
    ld	$9,0($6)
    pcpyld	$8,$9,$8
    mtsab	$13,0
    qfsrv	$8,$8,$8
    pextlb	$9,$0,$8
    addi	$7,$7,-1
    addu	$5,$5,$12
    mtsab	$24,0
    qfsrv	$10,$0,$8
    pextlb	$8,$0,$10
    paddh	$10,$9,$8
    paddh	$9,$10,$15
    por	$15,$10,$0
    paddh	$10,$9,$25
    psrlh	$10,$10,0x2
    lq	$8,0($14)
    paddh	$10,$10,$8
    pcgth	$9,$10,$0
    psrlh	$9,$9,0xf
    paddh	$10,$10,$9
    sll	$8,$12,0x1
    psrlh	$10,$10,0x1
    sq	$10,0($14)
    bgtz	$7,.LMc30
    addu	$14,$14,$8
.LMc31:
    psrah	$10,$11,0xf
    addiu	$5,$5,320
    lw	$7,12($4)
    addiu	$6,$6,320
    and	$10,$10,$7
    bnez	$10,.LMc30
    andi	$11,$11,0x7fff
    lw	$5,20($4)
    lw	$6,24($4)
    lw	$14,0($4)
    addiu	$5,$5,64
    addiu	$6,$6,64
    addiu	$14,$14,128
    andi	$10,$11,0x1
    bnez	$10,.LMc29
    andi	$11,$11,0xfffe
    jr	$31
    nop
    nop
    .size sceMpegMcChromaAverageHalfHV, .-sceMpegMcChromaAverageHalfHV

# NTSC-U/C: 0x0030aff8, PAL: 0x00377488
    .globl sceMpegMcAddBlock
    .type sceMpegMcAddBlock, @function
sceMpegMcAddBlock:
    addiu	$12,$0,24
    lui	$10,%hi(.LMcClamp)
    addiu	$10,$10,%lo(.LMcClamp)
    lq	$11,0($10)
.LMc32:
    lq	$8,0($5)
    addi	$12,$12,-1
    lq	$13,0($6)
    addiu	$4,$4,16
    lq	$9,16($5)
    paddh	$8,$8,$13
    lq	$2,16($6)
    pminh	$8,$8,$11
    paddh	$9,$9,$2
    pmaxh	$8,$8,$0
    pminh	$9,$9,$11
    addiu	$5,$5,32
    pmaxh	$9,$9,$0
    addiu	$6,$6,32
    ppacb	$10,$9,$8
    bnez	$12,.LMc32
    sq	$10,-16($4)
    jr	$31
    nop
    nop
    .size sceMpegMcAddBlock, .-sceMpegMcAddBlock

# NTSC-U/C: 0x0030b058, PAL: 0x003774e8
    .globl sceMpegMcPutBlock
    .type sceMpegMcPutBlock, @function
sceMpegMcPutBlock:
    addiu	$12,$0,24
    lui	$10,%hi(.LMcClamp)
    addiu	$10,$10,%lo(.LMcClamp)
    lq	$11,0($10)
.LMc33:
    lq	$8,0($5)
    addi	$12,$12,-1
    pminh	$8,$8,$11
    lq	$9,16($5)
    pmaxh	$8,$8,$0
    pminh	$9,$9,$11
    addiu	$5,$5,32
    pmaxh	$9,$9,$0
    addiu	$4,$4,16
    ppacb	$10,$9,$8
    bnez	$12,.LMc33
    sq	$10,-16($4)
    nop
    nop
    # The clamp words run as shifts into the zero register when the loop exits.
.LMcClamp:
    .word	0x00ff00ff
    .word	0x00ff00ff
    .word	0x00ff00ff
    .word	0x00ff00ff
    jr	$31
    nop
    .size sceMpegMcPutBlock, .-sceMpegMcPutBlock
