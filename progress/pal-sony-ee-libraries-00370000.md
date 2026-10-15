# Sony EE libraries, PAL `0x00370000` to `0x00380000`

212 of 212 routines done.

Sorted by status (remaining first), then length, then reference count, then
name. Names and signatures are the current Ghidra prototypes of the PAL
program and are preliminary. The NTSC-U/C column is empty for a routine only
the PAL executable has.

| Name                                              |       Status       | # xref | Length | PAL          | NTSC-U/C     | Signature                                                                    |
| ------------------------------------------------- | :----------------: | -----: | -----: | ------------ | ------------ | ---------------------------------------------------------------------------- |
| `mpc___getAllRefs`                                | :white_check_mark: |      1 |   1796 | `0x00375a50` | `0x003095c0` | `undefined mpc___getAllRefs()`                                               |
| `sceGsExecStoreImage`                             | :white_check_mark: |      1 |   1676 | `0x0037c3d0` | `0x0030fe20` | `undefined sceGsExecStoreImage()`                                            |
| `parsePacket`                                     | :white_check_mark: |      1 |   1456 | `0x00373e08` | `0x00307978` | `undefined ParseMpegPesPacketHeader()`                                       |
| `libusbkb__SetResult_Read`                        | :white_check_mark: |      0 |   1396 | `0x0037d370` | `0x00310dc0` | `undefined libusbkb__SetResult_Read()`                                       |
| `DecodeMpegMacroblockType`                        | :white_check_mark: |      1 |   1192 | `0x00377fd0` | `0x0030bb40` | `undefined DecodeMpegMacroblockType()`                                       |
| `BuildMpegMotionBlockDescriptor`                  | :white_check_mark: |     13 |   1052 | `0x00376158` | `0x00309cc8` | `undefined BuildMpegMotionBlockDescriptor()`                                 |
| `ParseMpegSequenceHeader`                         | :white_check_mark: |      1 |    972 | `0x00375020` | `0x00308b90` | `undefined ParseMpegSequenceHeader()`                                        |
| `sceGsSetDefDispEnv`                              | :white_check_mark: |      3 |    884 | `0x0037b9e8` | `0x0030f438` | `undefined sceGsSetDefDispEnv()`                                             |
| `sceMpegDemuxPssRing`                             | :white_check_mark: |      1 |    756 | `0x00373838` | `0x003073a8` | `undefined DemuxMpegProgramStream()`                                         |
| `ReconstructMpegMacroblockPixels`                 | :white_check_mark: |      2 |    728 | `0x00376578` | `0x0030a0e8` | `undefined ReconstructMpegMacroblockPixels()`                                |
| `SetupMpegReferenceFrameBuffers`                  | :white_check_mark: |      4 |    692 | `0x00379920` | `0x0030d490` | `undefined SetupMpegReferenceFrameBuffers()`                                 |
| `SetupMpegPictureBuffers`                         | :white_check_mark: |      1 |    680 | `0x00375148` | `0x00308cb8` | `void SetupMpegPictureBuffers(sceMpeg * mp)`                                 |
| `WaitForIpuBlockDecode`                           | :white_check_mark: |      2 |    668 | `0x00377570` | `0x0030b0e0` | `undefined WaitForIpuBlockDecode()`                                          |
| `CopyMpegMacroblocksToFrameViaSpr`                | :white_check_mark: |      2 |    656 | `0x00379c78` | `0x0030d7e8` | `undefined CopyMpegMacroblocksToFrameViaSpr()`                               |
| `BlitMpegFrameViaIpuCsc`                          | :white_check_mark: |      2 |    648 | `0x0037aa88` | `0x0030e5f8` | `undefined BlitMpegFrameViaIpuCsc()`                                         |
| `DispatchMpegMacroblockDecode`                    | :white_check_mark: |      1 |    592 | `0x00375800` | `0x00309370` | `undefined DispatchMpegMacroblockDecode()`                                   |
| `sceMtapInit`                                     | :white_check_mark: |      1 |    576 | `0x0037f200` | `0x00312b68` | `undefined InitMultitapLibrary()`                                            |
| `sceIpuInit`                                      | :white_check_mark: |      2 |    568 | `0x0037b560` | `0x0030efb0` | `undefined ResetAndInitIpuDecoder()`                                         |
| `sceMpegCreate`                                   | :white_check_mark: |      1 |    524 | `0x00374468` | `0x00307fd8` | `undefined sceMpegCreate()`                                                  |
| `DecodeMpegSliceMacroblocks`                      | :white_check_mark: |      1 |    504 | `0x00377d18` | `0x0030b888` | `undefined DecodeMpegSliceMacroblocks()`                                     |
| `sceUsbKbInit`                                    | :white_check_mark: |      1 |    504 | `0x0037cbc8` | `0x00310618` | `undefined sceUsbKbInit()`                                                   |
| `ParseMpegPictureCodingExtension`                 | :white_check_mark: |      4 |    496 | `0x00379220` | `0x0030cd90` | `undefined ParseMpegPictureCodingExtension()`                                |
| `sceGsSetDefDrawEnv`                              | :white_check_mark: |      2 |    484 | `0x0037be20` | `0x0030f870` | `undefined sceGsSetDefDrawEnv()`                                             |
| `scePadPortOpen`                                  | :white_check_mark: |      1 |    484 | `0x0037e598` | `0x00311fe8` | `undefined scePadPortOpen()`                                                 |
| `sceDmaPutEnv`                                    | :white_check_mark: |      1 |    472 | `0x0037e098` | `0x00311ae8` | `undefined sceDmaPutEnv()`                                                   |
| `sceMcInitLibrary`                                | :white_check_mark: |      1 |    460 | `0x0037f5f8` | `0x00312f60` | `undefined sceMcInitLibrary()`                                               |
| `csc___doCSC2`                                    | :white_check_mark: |      2 |    448 | `0x0037a7f0` | `0x0030e360` | `undefined csc___doCSC2()`                                                   |
| `DisplayMpegFieldPicturePair`                     | :white_check_mark: |      2 |    440 | `0x0037a1d0` | `0x0030dd40` | `undefined DisplayMpegFieldPicturePair()`                                    |
| `mpc___motionVectors`                             | :white_check_mark: |      2 |    412 | `0x00378500` | `0x0030c070` | `undefined mpc___motionVectors()`                                            |
| `sceGsResetGraph`                                 | :white_check_mark: |      2 |    400 | `0x0037b798` | `0x0030f1e8` | `undefined sceGsResetGraph()`                                                |
| `sceMpegGetPictureStamps`                         | :white_check_mark: |      3 |    396 | `0x00379f30` | `0x0030daa0` | `undefined sceMpegGetPictureStamps()`                                        |
| `mpc___dualPrimeVector`                           | :white_check_mark: |      2 |    388 | `0x00377830` | `0x0030b3a0` | `undefined mpc___dualPrimeVector()`                                          |
| `sceMcGetInfo`                                    | :white_check_mark: |      1 |    380 | `0x0037ffa8` | `0x00313918` | `undefined SendMcGetInfoRpc()`                                               |
| `DecodeIpuVlc`                                    | :white_check_mark: |      5 |    376 | `0x00378978` | `0x0030c4e8` | `undefined DecodeIpuVlc()`                                                   |
| `sceMcWrite`                                      | :white_check_mark: |      1 |    372 | `0x0037fcb8` | `0x003135f8` | `undefined SendMcWriteRpc()`                                                 |
| `BuildMpegInputDmaChainAndKick`                   | :white_check_mark: |      1 |    364 | `0x0037a388` | `0x0030def8` | `undefined BuildMpegInputDmaChainAndKick()`                                  |
| `scePadRead`                                      | :white_check_mark: |      2 |    356 | `0x0037e7e0` | `0x00312230` | `undefined FUN_0037e7e0()`                                                   |
| `PrepareNextMpegFrame`                            | :white_check_mark: |      1 |    348 | `0x00374be0` | `0x00308750` | `undefined PrepareNextMpegFrame()`                                           |
| `sceUsbKbCnvRawCode`                              | :white_check_mark: |      1 |    344 | `0x0037dc58` | `0x003116a8` | `undefined sceUsbKbCnvRawCode()`                                             |
| `ReadMpegBits`                                    | :white_check_mark: |     82 |    336 | `0x00378ce8` | `0x0030c858` | `undefined ReadMpegBits()`                                                   |
| `sceIpuRestartDMA`                                | :white_check_mark: |      2 |    336 | `0x0037b340` | `0x0030ed90` | `undefined sceIpuRestartDMA()`                                               |
| `parsePackHeader`                                 | :white_check_mark: |      1 |    336 | `0x00373c48` | `0x003077b8` | `undefined parsePackHeader()`                                                |
| `DecodeMpegMotionVectorPair`                      | :white_check_mark: |      5 |    332 | `0x003786a0` | `0x0030c210` | `undefined DecodeMpegMotionVectorPair()`                                     |
| `libusbkb__cnv_keycode`                           | :white_check_mark: |      3 |    332 | `0x0037db08` | `0x00311558` | `undefined libusbkb__cnv_keycode()`                                          |
| `csc___ch3dmaCSC`                                 | :white_check_mark: |      1 |    328 | `0x0037a6a8` | `0x0030e218` | `undefined csc___ch3dmaCSC()`                                                |
| `sceGsSetDefStoreImage`                           | :white_check_mark: |      1 |    320 | `0x0037c290` | `0x0030fce0` | `undefined sceGsSetDefStoreImage()`                                          |
| `scePadInfoMode`                                  | :white_check_mark: |      3 |    312 | `0x0037ebb0` | `0x00312518` | `undefined scePadInfoMode()`                                                 |
| `DisplayMpegFramePicture`                         | :white_check_mark: |      2 |    312 | `0x0037a0c0` | `0x0030dc30` | `undefined DisplayMpegFramePicture()`                                        |
| `scePadInit`                                      | :white_check_mark: |      1 |    312 | `0x0037e3b8` | `0x00311e08` | `undefined scePadInit()`                                                     |
| `ParseMpegSequenceExtension`                      | :white_check_mark: |      2 |    304 | `0x003755b0` | `0x00309120` | `undefined ParseMpegSequenceExtension()`                                     |
| `ParseMpegSliceHeader`                            | :white_check_mark: |      1 |    300 | `0x00377be8` | `0x0030b758` | `undefined ParseMpegSliceHeader()`                                           |
| `IssueIpuCscAndReadResult`                        | :white_check_mark: |      2 |    296 | `0x0037a580` | `0x0030e0f0` | `undefined IssueIpuCscAndReadResult()`                                       |
| `ParseMpegPictureExtensions`                      | :white_check_mark: |      3 |    292 | `0x003790f8` | `0x0030cc68` | `undefined ParseMpegPictureExtensions()`                                     |
| `sceMcOpen`                                       | :white_check_mark: |      2 |    292 | `0x0037f820` | `0x00313160` | `undefined SendMcOpenRpc()`                                                  |
| `sceMpegMcLumaAverageHalfHV`                      | :white_check_mark: |      3 |    288 | `0x00377250` | `0x0030adc0` | `void sceMpegMcLumaAverageHalfHV(MpegMcDescriptor * pDescriptor)`            |
| `DecodeMpegPictureSlices`                         | :white_check_mark: |      1 |    288 | `0x00377ac8` | `0x0030b638` | `undefined DecodeMpegPictureSlices()`                                        |
| `scePadInfoAct`                                   | :white_check_mark: |      1 |    288 | `0x0037ea88` | `0x003123f0` | `undefined scePadInfoAct()`                                                  |
| `sceMpegMcChromaAverageHalfHV`                    | :white_check_mark: |      3 |    276 | `0x00377370` | `0x0030aee0` | `void sceMpegMcChromaAverageHalfHV(MpegMcDescriptor * pDescriptor)`          |
| `DecodeNextMpegFrame`                             | :white_check_mark: |      1 |    276 | `0x00374a80` | `0x003085f0` | `undefined DecodeNextMpegFrame()`                                            |
| `libpad___send_to_iop`                            | :white_check_mark: |      1 |    276 | `0x0037e2a0` | `0x00311cf0` | `undefined libpad___send_to_iop()`                                           |
| `sceMcRead`                                       | :white_check_mark: |      1 |    276 | `0x0037fba0` | `0x003134e0` | `undefined SendMcReadRpc()`                                                  |
| `ReadMpegMacroblockAddressIncrement`              | :white_check_mark: |      2 |    272 | `0x003779b8` | `0x0030b528` | `undefined ReadMpegMacroblockAddressIncrement()`                             |
| `ParseMpegToNextPicture`                          | :white_check_mark: |      2 |    268 | `0x00378f20` | `0x0030ca90` | `undefined ParseMpegToNextPicture()`                                         |
| `PeekIpuBits`                                     | :white_check_mark: |      5 |    264 | `0x00378af0` | `0x0030c660` | `undefined PeekIpuBits()`                                                    |
| `sceGsSetDefClear`                                | :white_check_mark: |      2 |    260 | `0x0037c008` | `0x0030fa58` | `undefined sceGsSetDefClear()`                                               |
| `sceMpegMcChromaHalfHV`                           | :white_check_mark: |      3 |    252 | `0x00376c90` | `0x0030a800` | `void sceMpegMcChromaHalfHV(MpegMcDescriptor * pDescriptor)`                 |
| `sceUsbKbSetLEDMode`                              | :white_check_mark: |      1 |    252 | `0x0037d018` | `0x00310a68` | `undefined sceUsbKbSetLEDMode()`                                             |
| `sceMpegMcLumaHalfHV`                             | :white_check_mark: |      3 |    248 | `0x00376b98` | `0x0030a708` | `void sceMpegMcLumaHalfHV(MpegMcDescriptor * pDescriptor)`                   |
| `ParseMpegPictureDisplayExtension`                | :white_check_mark: |      2 |    244 | `0x00379638` | `0x0030d1a8` | `undefined ParseMpegPictureDisplayExtension()`                               |
| `sceMpegAddStrCallback`                           | :white_check_mark: |      2 |    244 | `0x00373b50` | `0x003076c0` | `undefined sceMpegAddStrCallback()`                                          |
| `ConsumeIpuBits`                                  | :white_check_mark: |      9 |    240 | `0x00378bf8` | `0x0030c768` | `undefined ConsumeIpuBits()`                                                 |
| `sceGsPutDrawEnv`                                 | :white_check_mark: |      2 |    232 | `0x0037c110` | `0x0030fb60` | `undefined sceGsPutDrawEnv()`                                                |
| `RunImageDecodeStateMachine`                      | :white_check_mark: |      1 |    232 | `0x00374910` | `0x00308480` | `undefined RunImageDecodeStateMachine()`                                     |
| `sceUsbKbClearRbuf`                               | :white_check_mark: |      1 |    232 | `0x0037de90` | `0x003118e0` | `undefined sceUsbKbClearRbuf()`                                              |
| `sceIpuStopDMA`                                   | :white_check_mark: |      3 |    228 | `0x0037b258` | `0x0030eca8` | `undefined sceIpuStopDMA()`                                                  |
| `sceMpegMcChromaAverageHalfV`                     | :white_check_mark: |      3 |    228 | `0x00376fc0` | `0x0030ab30` | `void sceMpegMcChromaAverageHalfV(MpegMcDescriptor * pDescriptor)`           |
| `UploadQuantMatrixToIpu`                          | :white_check_mark: |      2 |    224 | `0x003754d0` | `0x00309040` | `undefined UploadQuantMatrixToIpu()`                                         |
| `InitPlaneDmaAddressSlots`                        | :white_check_mark: |      1 |    224 | `0x003753f0` | `0x00308f60` | `undefined InitPlaneDmaAddressSlots()`                                       |
| `sceMpegMcLumaAverageHalfV`                       | :white_check_mark: |      3 |    220 | `0x00376ee0` | `0x0030aa50` | `void sceMpegMcLumaAverageHalfV(MpegMcDescriptor * pDescriptor)`             |
| `sceDmaReset`                                     | :white_check_mark: |      2 |    220 | `0x0037dfb8` | `0x00311a08` | `undefined sceDmaReset()`                                                    |
| `sceMcSync`                                       | :white_check_mark: |      2 |    220 | `0x0037fe70` | `0x003137e0` | `undefined sceMcSync()`                                                      |
| `sceUsbKbRead`                                    | :white_check_mark: |      1 |    220 | `0x0037cf30` | `0x00310980` | `undefined sceUsbKbRead()`                                                   |
| `sceMcSeek`                                       | :white_check_mark: |      1 |    216 | `0x0037fa38` | `0x00313378` | `undefined SendMcSeekRpc()`                                                  |
| `scePadSetActAlign`                               | :white_check_mark: |      1 |    216 | `0x0037ee60` | `0x003127c8` | `undefined scePadSetActAlign()`                                              |
| `sceMpegMcLumaAverageHalfH`                       | :white_check_mark: |      3 |    212 | `0x003770a8` | `0x0030ac18` | `void sceMpegMcLumaAverageHalfH(MpegMcDescriptor * pDescriptor)`             |
| `csc___ch4dma`                                    | :white_check_mark: |      1 |    212 | `0x0037a9b0` | `0x0030e520` | `undefined csc___ch4dma()`                                                   |
| `sceMpegMcChromaAverageHalfH`                     | :white_check_mark: |      3 |    208 | `0x00377180` | `0x0030acf0` | `void sceMpegMcChromaAverageHalfH(MpegMcDescriptor * pDescriptor)`           |
| `DecodeMpegPictureToFrameBuffer`                  | :white_check_mark: |      3 |    204 | `0x003797c0` | `0x0030d330` | `undefined DecodeMpegPictureToFrameBuffer()`                                 |
| `sceMpegMcChromaHalfV`                            | :white_check_mark: |      3 |    204 | `0x00376960` | `0x0030a4d0` | `void sceMpegMcChromaHalfV(MpegMcDescriptor * pDescriptor)`                  |
| `sceGszbufaddr`                                   | :white_check_mark: |      2 |    200 | `0x0037cb00` | `0x00310550` | `undefined sceGszbufaddr()`                                                  |
| `mpeg___clearEach`                                | :white_check_mark: |      1 |    200 | `0x00374e98` | `0x00308a08` | `undefined mpeg___clearEach()`                                               |
| `sceUsbKbEnd`                                     | :white_check_mark: |      1 |    200 | `0x0037cdc0` | `0x00310810` | `undefined sceUsbKbEnd()`                                                    |
| `LoadMpegQuantizerMatrices`                       | :white_check_mark: |      2 |    196 | `0x00379570` | `0x0030d0e0` | `undefined LoadMpegQuantizerMatrices()`                                      |
| `ParseMpegPictureHeader`                          | :white_check_mark: |      1 |    196 | `0x00379030` | `0x0030cba0` | `undefined ParseMpegPictureHeader()`                                         |
| `scePadSetActDirect`                              | :white_check_mark: |      1 |    192 | `0x0037eda0` | `0x00312708` | `undefined SetPadActuatorDirect()`                                           |
| `sceGsPutDispEnv`                                 | :white_check_mark: |      2 |    188 | `0x0037bd60` | `0x0030f7b0` | `undefined sceGsPutDispEnv()`                                                |
| `HandleMpegSkippedMacroblock`                     | :white_check_mark: |      1 |    188 | `0x00377f10` | `0x0030ba80` | `undefined HandleMpegSkippedMacroblock()`                                    |
| `sceMpegMcChromaHalfH`                            | :white_check_mark: |      3 |    184 | `0x00376ae0` | `0x0030a650` | `void sceMpegMcChromaHalfH(MpegMcDescriptor * pDescriptor)`                  |
| `sceMcClose`                                      | :white_check_mark: |      1 |    184 | `0x0037f980` | `0x003132c0` | `undefined SendMcCloseRpc()`                                                 |
| `sceMpegMcLumaHalfV`                              | :white_check_mark: |      3 |    180 | `0x003768a8` | `0x0030a418` | `void sceMpegMcLumaHalfV(MpegMcDescriptor * pDescriptor)`                    |
| `scePadGetButtonMask`                             | :white_check_mark: |      1 |    180 | `0x0037ef38` | `0x003128a0` | `undefined scePadGetButtonMask()`                                            |
| `scePadSetMainMode`                               | :white_check_mark: |      1 |    180 | `0x0037ece8` | `0x00312650` | `undefined scePadSetMainMode()`                                              |
| `WaitForIpuCommandResult`                         | :white_check_mark: |      4 |    176 | `0x003788c8` | `0x0030c438` | `undefined WaitForIpuCommandResult()`                                        |
| `sceMpegInit`                                     | :white_check_mark: |      1 |    176 | `0x003743b8` | `0x00307f28` | `undefined sceMpegInit()`                                                    |
| `scePadSetButtonInfo`                             | :white_check_mark: |      1 |    176 | `0x0037eff0` | `0x00312958` | `undefined scePadSetButtonInfo()`                                            |
| `sceMpegMcChromaAverageCopy`                      | :white_check_mark: |      3 |    172 | `0x00376e30` | `0x0030a9a0` | `void sceMpegMcChromaAverageCopy(MpegMcDescriptor * pDescriptor)`            |
| `sceMpegMcLumaHalfH`                              | :white_check_mark: |      3 |    172 | `0x00376a30` | `0x0030a5a0` | `void sceMpegMcLumaHalfH(MpegMcDescriptor * pDescriptor)`                    |
| `sceUsbKbGetInfo`                                 | :white_check_mark: |      2 |    168 | `0x0037ce88` | `0x003108d8` | `undefined sceUsbKbGetInfo()`                                                |
| `WaitForIpuIdle`                                  | :white_check_mark: |     15 |    164 | `0x00378820` | `0x0030c390` | `undefined WaitForIpuIdle()`                                                 |
| `CheckMpegOutputBufferCapacity`                   | :white_check_mark: |      2 |    160 | `0x00379bd8` | `0x0030d748` | `undefined CheckMpegOutputBufferCapacity()`                                  |
| `sceGsResetPath`                                  | :white_check_mark: |      2 |    160 | `0x0037b040` | `0x0030ea90` | `undefined sceGsResetPath()`                                                 |
| `sceGsSyncVCallback`                              | :white_check_mark: |      2 |    160 | `0x0037ca60` | `0x003104b0` | `undefined sceGsSyncVCallback()`                                             |
| `sceUsbKbSync`                                    | :white_check_mark: |      2 |    160 | `0x0037d200` | `0x00310c50` | `undefined sceUsbKbSync()`                                                   |
| `ParseMpegGroupOfPicturesHeader`                  | :white_check_mark: |      1 |    160 | `0x003794d0` | `0x0030d040` | `undefined ParseMpegPictureHeader()`                                         |
| `graph003__checkModelVersion`                     | :white_check_mark: |      1 |    160 | `0x0037b938` | `0x0030f388` | `undefined graph003__checkModelVersion()`                                    |
| `sceMpegMcLumaAverageCopy`                        | :white_check_mark: |      3 |    156 | `0x00376d90` | `0x0030a900` | `void sceMpegMcLumaAverageCopy(MpegMcDescriptor * pDescriptor)`              |
| `scePadInit2`                                     | :white_check_mark: |      1 |    156 | `0x0037e4f8` | `0x00311f48` | `undefined scePadInit2()`                                                    |
| `AdvanceMpegBitReader`                            | :white_check_mark: |      4 |    152 | `0x0037ad68` | `0x0030e8d8` | `undefined AdvanceMpegBitReader()`                                           |
| `sceGsSyncV`                                      | :white_check_mark: |      4 |    148 | `0x0037c1f8` | `0x0030fc48` | `undefined sceGsSyncV()`                                                     |
| `sceMpegMcChromaCopy`                             | :white_check_mark: |      3 |    148 | `0x00376810` | `0x0030a380` | `void sceMpegMcChromaCopy(MpegMcDescriptor * pDescriptor)`                   |
| `SkipMpegCopyrightExtension`                      | :white_check_mark: |      3 |    144 | `0x00379730` | `0x0030d2a0` | `undefined SkipMpegCopyrightExtension()`                                     |
| `OutputDecodedMpegPicture`                        | :white_check_mark: |      2 |    144 | `0x00379890` | `0x0030d400` | `undefined OutputDecodedMpegPicture()`                                       |
| `mceIntrReadFixAlign`                             | :white_check_mark: |      1 |    144 | `0x0037fb10` | `0x00313450` | `undefined CopyBackMcReadRpcResult()`                                        |
| `ParseMpegSequenceRateInfo`                       | :white_check_mark: |      3 |    140 | `0x003756e0` | `0x00309250` | `undefined ParseMpegSequenceRateInfo()`                                      |
| `sceGsSyncPath`                                   | :white_check_mark: |      8 |    136 | `0x0037b0e0` | `0x0030eb30` | `undefined sceGsSyncPath()`                                                  |
| `PredictMpegMotionVectorComponent`                | :white_check_mark: |      2 |    136 | `0x00378478` | `0x0030bfe8` | `undefined PredictMpegMotionVectorComponent()`                               |
| `buildStreamKey`                                  | :white_check_mark: |      1 |    136 | `0x003737a8` | `0x00307318` | `undefined BuildMpegStreamIdKey()`                                           |
| `StartIpuFromDmaTransfer`                         | :white_check_mark: |      1 |    132 | `0x0037a4f8` | `0x0030e068` | `undefined StartIpuFromDmaTransfer()`                                        |
| `libusbkb__vblank_e_handler`                      | :white_check_mark: |      1 |    132 | `0x0037d978` | `0x003113c8` | `undefined libusbkb__vblank_e_handler()`                                     |
| `AlignToMpegStartCode`                            | :white_check_mark: |      5 |    124 | `0x00378e38` | `0x0030c9a8` | `undefined AlignToMpegStartCode()`                                           |
| `EmitDecodedMpegFrame`                            | :white_check_mark: |      1 |    124 | `0x00374db8` | `0x00308928` | `undefined EmitDecodedMpegFrame()`                                           |
| `scePadGetState`                                  | :white_check_mark: |      1 |    120 | `0x0037e948` | `0x003122b0` | `undefined scePadGetState()`                                                 |
| `sceMpegMcLumaCopy`                               | :white_check_mark: |      3 |    116 | `0x00376798` | `0x0030a308` | `void sceMpegMcLumaCopy(MpegMcDescriptor * pDescriptor)`                     |
| `libipu__setD3_CHCR`                              | :white_check_mark: |      2 |    116 | `0x0037b168` | `0x0030ebb8` | `undefined libipu__setD3_CHCR()`                                             |
| `libipu__setD4_CHCR`                              | :white_check_mark: |      2 |    116 | `0x0037b1e0` | `0x0030ec30` | `undefined libipu__setD4_CHCR()`                                             |
| `UpdateMpegPictureDisplayOrder`                   | :white_check_mark: |      1 |    116 | `0x00379458` | `0x0030cfc8` | `undefined UpdateMpegPictureDisplayOrder()`                                  |
| `libusbkb__cnv_keypad`                            | :white_check_mark: |      1 |    112 | `0x0037ddb0` | `0x00311800` | `undefined libusbkb__cnv_keypad()`                                           |
| `libusbkb__push_repbuf`                           | :white_check_mark: |      5 |    108 | `0x0037da30` | `0x00311480` | `undefined libusbkb__push_repbuf()`                                          |
| `_sceMpegAlalcAlloc`                              | :white_check_mark: |      4 |    108 | `0x003748a0` | `0x00308410` | `undefined _sceMpegAlalcAlloc()`                                             |
| `sceMtapGetConnection`                            | :white_check_mark: |      2 |    108 | `0x0037f520` | `0x00312e88` | `undefined sceMtapGetConnection()`                                           |
| `sceMtapPortClose`                                | :white_check_mark: |      1 |    108 | `0x0037f4b0` | `0x00312e18` | `undefined sceMtapPortClose()`                                               |
| `sceMtapPortOpen`                                 | :white_check_mark: |      1 |    108 | `0x0037f440` | `0x00312da8` | `undefined sceMtapPortOpen()`                                                |
| `sceUsbKbSetReadMode`                             | :white_check_mark: |      1 |    108 | `0x0037de20` | `0x00311870` | `undefined sceUsbKbSetReadMode()`                                            |
| `sceIpuSync`                                      | :white_check_mark: |      8 |    104 | `0x0037b490` | `0x0030eee0` | `undefined sceIpuSync()`                                                     |
| `FlushPendingMpegFrame`                           | :white_check_mark: |      2 |    104 | `0x00374d40` | `0x003088b0` | `undefined FlushPendingMpegFrame()`                                          |
| `libusbkb__pop_repbuf`                            | :white_check_mark: |      1 |    104 | `0x0037daa0` | `0x003114f0` | `undefined libusbkb__pop_repbuf()`                                           |
| `parseSystemHeader`                               | :white_check_mark: |      1 |    104 | `0x00373d98` | `0x00307908` | `undefined parseSystemHeader()`                                              |
| `scePadSetReqState`                               | :white_check_mark: |      3 |    100 | `0x0037e9c8` | `0x00312330` | `undefined scePadSetReqState()`                                              |
| `DispatchUsbKbRpcResult`                          | :white_check_mark: |      2 |    100 | `0x0037d2b0` | `0x00310d00` | `undefined DispatchUsbKbRpcResult()`                                         |
| `ReadMpegSliceQuantizer`                          | :white_check_mark: |      1 |    100 | `0x00378eb8` | `0x0030ca28` | `undefined ReadMpegSliceQuantizer()`                                         |
| `sceMtapGetModVersion`                            | :white_check_mark: |      1 |    100 | `0x0037f590` | `0x00312ef8` | `undefined sceMtapGetModVersion()`                                           |
| `scePadGetModVersion`                             | :white_check_mark: |      1 |    100 | `0x0037f170` | `0x00312ad8` | `undefined scePadGetModVersion()`                                            |
| `setD4_CHCR`                                      | :white_check_mark: |      1 |    100 | `0x0037b4f8` | `0x0030ef48` | `undefined SetIpuToChcrForced()`                                             |
| `scePadGetDmaStr`                                 | :white_check_mark: |      8 |     96 | `0x0037e780` | `0x003121d0` | `undefined scePadGetDmaStr()`                                                |
| `sceMpegMcPutBlock`                               | :white_check_mark: |      2 |     96 | `0x003774e8` | `0x0030b058` | `void sceMpegMcPutBlock(uchar * pDest, uchar * pSource)`                     |
| `InitDecoderScratchpadPointers`                   | :white_check_mark: |      1 |     96 | `0x00374e38` | `0x003089a8` | `undefined InitDecoderScratchpadPointers()`                                  |
| `sceMpegMcAddBlock`                               | :white_check_mark: |      1 |     92 | `0x00377488` | `0x0030aff8` | `void sceMpegMcAddBlock(uchar * pDest, uchar * pPrediction, uchar * pBlock)` |
| `scePadInfoPressMode`                             | :white_check_mark: |      1 |     92 | `0x0037f0a0` | `0x00312a08` | `undefined scePadInfoPressMode()`                                            |
| `sceMpegReset`                                    | :white_check_mark: |      2 |     88 | `0x003746f0` | `0x00308260` | `undefined sceMpegReset()`                                                   |
| `SetResult_GetInfo`                               | :white_check_mark: |      0 |     88 | `0x0037d318` | `0x00310d68` | `undefined SetResult_GetInfo()`                                              |
| `_sceMpegError`                                   | :white_check_mark: |     25 |     84 | `0x00374fa8` | `0x00308b18` | `undefined _sceMpegError()`                                                  |
| `SkipMpegBitReaderBytes`                          | :white_check_mark: |      4 |     84 | `0x0037ae98` | `0x0030ea08` | `undefined SkipMpegBitReaderBytes()`                                         |
| `mceGetInfoApdx`                                  | :white_check_mark: |      1 |     84 | `0x0037ff50` | `0x003138c0` | `undefined CopyBackMcGetInfoRpcResult()`                                     |
| `sceMpegClearRefBuff`                             | :white_check_mark: |      1 |     84 | `0x00374748` | `0x003082b8` | `undefined ClearMpegStreamBufferFlags()`                                     |
| `scePadEnterPressMode`                            | :white_check_mark: |      1 |     84 | `0x0037f100` | `0x00312a68` | `undefined EnablePadAllButtonPressure()`                                     |
| `_sceMpegCallback`                                | :white_check_mark: |     12 |     80 | `0x003747c8` | `0x00308338` | `undefined DispatchMpegDecoderEvent()`                                       |
| `scePadGetReqState`                               | :white_check_mark: |      7 |     80 | `0x0037ea30` | `0x00312398` | `undefined scePadGetReqState()`                                              |
| `getBits`                                         | :white_check_mark: |     47 |     76 | `0x0037ae00` | `0x0030e970` | `undefined getBits()`                                                        |
| `sceMcEnd`                                        | :white_check_mark: |      1 |     76 | `0x0037f7c8` | `0x00313118` | `undefined ShutdownMemoryCardLibrary()`                                      |
| `sceUsbKbSetArrangement`                          | :white_check_mark: |      1 |     76 | `0x0037d1b0` | `0x00310c00` | `undefined sceUsbKbSetArrangement()`                                         |
| `sceUsbKbSetCodeType`                             | :white_check_mark: |      1 |     76 | `0x0037d160` | `0x00310bb0` | `undefined sceUsbKbSetCodeType()`                                            |
| `SetResult_GetLocation`                           | :white_check_mark: |      0 |     76 | `0x0037d8e8` | `0x00311338` | `undefined SetResult_GetLocation()`                                          |
| `sceMpegGetPicture`                               | :white_check_mark: |      1 |     72 | `0x00374688` | `0x003081f8` | `undefined sceMpegGetPicture()`                                              |
| `getBit`                                          | :white_check_mark: |     10 |     68 | `0x0037ae50` | `0x0030e9c0` | `undefined getBit()`                                                         |
| `SkipMpegExtraInformation`                        | :white_check_mark: |      2 |     68 | `0x00379410` | `0x0030cf80` | `undefined SkipMpegExtraInformation()`                                       |
| `sceUsbKbSetRepeat`                               | :white_check_mark: |      1 |     68 | `0x0037d118` | `0x00310b68` | `undefined sceUsbKbSetRepeat()`                                              |
| `mpeg___decodeOrSkip`                             | :white_check_mark: |      0 |     68 | `0x00374b98` | `0x00308708` | `undefined mpeg___decodeOrSkip()`                                            |
| `McDelayThread`                                   | :white_check_mark: |      1 |     56 | `0x0037fe38` | `0x00313798` | `undefined FUN_0037fe38()`                                                   |
| `_sceMpegErrorFormat`                             | :white_check_mark: |      7 |     52 | `0x00374f70` | `0x00308ae0` | `undefined ReportMpegSystemError()`                                          |
| `InitMpegBitReader`                               | :white_check_mark: |      1 |     52 | `0x0037ad10` | `0x0030e880` | `undefined InitMpegBitReader()`                                              |
| `memclr`                                          | :white_check_mark: |      1 |     52 | `0x0037df80` | `0x003119d0` | `undefined ZeroMemoryBytes()`                                                |
| `sceMcMkdir`                                      | :white_check_mark: |      1 |     52 | `0x0037f948` | `0x00313288` | `undefined SendMcMkdirRpc()`                                                 |
| `libusbkb__clear_repbuf`                          | :white_check_mark: |      2 |     48 | `0x0037da00` | `0x00311450` | `undefined libusbkb__clear_repbuf()`                                         |
| `IssueIpuCommand`                                 | :white_check_mark: |      8 |     44 | `0x003787f0` | `0x0030c360` | `undefined IssueIpuCommand()`                                                |
| `GetMpegBitReaderBytePointer`                     | :white_check_mark: |      4 |     44 | `0x0037aef0` | `0x0030ea60` | `undefined FUN_0037aef0()`                                                   |
| `_sceMpegCbNodata`                                | :white_check_mark: |      9 |     40 | `0x00374818` | `0x00308388` | `undefined FireMpegDecoderResetEvent()`                                      |
| `DPRINT`                                          | :white_check_mark: |      5 |     40 | `0x0037f1d8` | `0x00312b40` | `undefined MtapDebugLogStub()`                                               |
| `SetIpuControlEnableBit`                          | :white_check_mark: |      3 |     40 | `0x00377548` | `0x0030b0b8` | `undefined SetIpuControlEnableBit()`                                         |
| `SetMpegPictureReady`                             | :white_check_mark: |      2 |     40 | `0x00379f08` | `0x0030da78` | `void SetMpegPictureReady(MpegContext * w)`                                  |
| `sceMpegAddCallback`                              | :white_check_mark: |      3 |     36 | `0x003747a0` | `0x00308310` | `undefined sceMpegAddCallback()`                                             |
| `ResetMpegStreamSubField4c`                       | :white_check_mark: |      2 |     36 | `0x003757d8` | `0x00309348` | `undefined ResetMpegStreamSubField4c()`                                      |
| `SaveMpegPictureDataDmaChannel`                   | :white_check_mark: |      1 |     36 | `0x003757b0` | `0x00309320` | `undefined SaveMpegPictureDataDmaChannel()`                                  |
| `SetPlaneRectDimensions`                          | :white_check_mark: |      9 |     32 | `0x00375000` | `0x00308b70` | `undefined SetPlaneRectDimensions()`                                         |
| `sceMpegDemuxPss`                                 | :white_check_mark: |      1 |     32 | `0x00373b30` | `0x003076a0` | `undefined sceMpegDemuxPss()`                                                |
| `SetResult_SetLEDMode`                            | :white_check_mark: |      0 |     32 | `0x0037d958` | `0x003113a8` | `undefined SetResult_SetLEDMode()`                                           |
| `SetResult_SetLEDStatus`                          | :white_check_mark: |      0 |     32 | `0x0037d938` | `0x00311388` | `undefined SetResult_SetLEDStatus()`                                         |
| `PeekMpegBits`                                    | :white_check_mark: |      9 |     28 | `0x0037ad48` | `0x0030e8b8` | `undefined PeekMpegBits()`                                                   |
| `SetMpegStreamMode3`                              | :white_check_mark: |      2 |     28 | `0x00377810` | `0x0030b380` | `undefined SetMpegStreamMode3()`                                             |
| `_sceMpegAlalcInit`                               | :white_check_mark: |      1 |     20 | `0x00374868` | `0x003083d8` | `undefined _sceMpegAlalcInit()`                                              |
| `_sceMpegPrintError`                              | :white_check_mark: |      1 |     16 | `0x00374f60` | `0x00308ad0` | `undefined PrintMpegErrorMessage()`                                          |
| `graph003___GetGsDxDyOffset`                      | :white_check_mark: |      1 |     16 | `0x0037b9d8` | `0x0030f428` | `undefined graph003___GetGsDxDyOffset()`                                     |
| `HandleMpegPictureTemporalScalableExtUnsupported` | :white_check_mark: |     11 |     12 | `0x003757a0` | `0x00309310` | `undefined HandleMpegPictureTemporalScalableExtUnsupported()`                |
| `sceGsGetGParam`                                  | :white_check_mark: |      8 |     12 | `0x0037b928` | `0x0030f378` | `undefined sceGsGetGParam()`                                                 |
| `HandleMpegUnknownExtension`                      | :white_check_mark: |      6 |     12 | `0x00375780` | `0x003092f0` | `undefined HandleMpegUnknownExtension()`                                     |
| `libusbkb__rpccall_end`                           | :white_check_mark: |      4 |     12 | `0x0037d2a0` | `0x00310cf0` | `undefined libusbkb__rpccall_end()`                                          |
| `HandleMpegPictureSpatialScalableExtUnsupported`  | :white_check_mark: |      2 |     12 | `0x00375790` | `0x00309300` | `undefined HandleMpegPictureSpatialScalableExtUnsupported()`                 |
| `HandleMpegSequenceScalableExtUnsupported`        | :white_check_mark: |      2 |     12 | `0x00375770` | `0x003092e0` | `undefined HandleMpegSequenceScalableExtUnsupported()`                       |
| `ClearRetryFlagAndEnableIpu`                      | :white_check_mark: |      1 |     12 | `0x00374da8` | `0x00308918` | `undefined ClearRetryFlagAndEnableIpu()`                                     |
| `_sceMpegAlalcFree`                               | :white_check_mark: |      1 |     12 | `0x00374890` | `0x00308400` | `undefined _sceMpegAlalcFree()`                                              |
| `_sceMpegAlalcSetDynamic`                         | :white_check_mark: |      1 |     12 | `0x00374880` | `0x003083f0` | `undefined _sceMpegAlalcSetDynamic()`                                        |
| `sceMpegIsEnd`                                    | :white_check_mark: |      1 |     12 | `0x003746e0` | `0x00308250` | `undefined sceMpegIsEnd()`                                                   |
| `sceMpegDelete`                                   | :white_check_mark: |      1 |      8 | `0x00374678` | `0x003081e8` | `undefined sceMpegDelete()`                                                  |
