# Compiler-emitted instances, `0x003a0000` to `0x003b0000`

9 of 107 routines done.

Sorted by length, then reference count, then status (remaining first),
then name. Signatures are the current Ghidra prototypes and are
preliminary.

| Name                                               |       Status       | # xref | Length | NTSC-U/C     | PAL          | Signature                                                                          |
| -------------------------------------------------- | :----------------: | -----: | -----: | ------------ | ------------ | ---------------------------------------------------------------------------------- |
| `DestroyMuseBuilder`                               |        :x:         |      1 |   1200 | `0x003a5a80` | `0x00414760` | `void DestroyMuseBuilder(void * pThis, uint nFlags)`                               |
| `ConstructMuseBuilder`                             |        :x:         |      1 |   1056 | `0x003a5f30` | `0x00414c10` | `void * ConstructMuseBuilder(void * pThis)`                                        |
| `DestroyCheatsManagerContainers`                   |        :x:         |      0 |    540 | `0x003a9268` | `0x00418028` | `void DestroyCheatsManagerContainers(uint * pThis, ulong flags)`                   |
| `CopyConstructAsyncTaskList`                       |        :x:         |      2 |    340 | `0x003a8238` | `0x00416f18` | `void * CopyConstructAsyncTaskList(void * pThis, int * pSource)`                   |
| `ConstructChatMsgCopy`                             |        :x:         |      0 |    340 | `0x003a1a08` | `0x004106e8` | `undefined4 * ConstructChatMsgCopy(undefined4 * pThis, int pSource)`               |
| `ConstructGameOverPktCopy`                         |        :x:         |      0 |    272 | `0x003a1c70` | `0x00410950` | `undefined4 * ConstructGameOverPktCopy(undefined4 * pThis, int pSource)`           |
| `ConstructUpdateRanksPktCopy`                      |        :x:         |      0 |    272 | `0x003a1b60` | `0x00410840` | `undefined4 * ConstructUpdateRanksPktCopy(undefined4 * pThis, int pSource)`        |
| `DeserializeTriWordStructVector`                   |        :x:         |      1 |    248 | `0x003a4f20` | `0x00413c00` | `undefined8 DeserializeTriWordStructVector(undefined8 pStream, undefined8 pVec)`   |
| `AdvanceWorldBeatEventCursor`                      | :white_check_mark: |      0 |    204 | `0x003a64d0` | `0x004151b0` | `void AdvanceWorldBeatEventCursor(void * pThis)`                                   |
| `CopyConstructIntVector`                           |        :x:         |      4 |    200 | `0x003a91a0` | `0x00417f60` | `void * CopyConstructIntVector(void * pThis, int * pSource)`                       |
| `GetTypeInfoForTableLinDerivedInterp`              |        :x:         |      0 |    164 | `0x003a8950` | `0x00417630` | `undefined GetTypeInfoForTableLinDerivedInterp()`                                  |
| `DestroyGameOverPktVector`                         |        :x:         |      0 |    156 | `0x003a15f8` | `0x004102d8` | `void DestroyGameOverPktVector(int * pThis, ulong nFlags)`                         |
| `SerializeTriWordStructVector`                     |        :x:         |      1 |    152 | `0x003a4a70` | `0x00413750` | `undefined8 SerializeTriWordStructVector(undefined8 pStream, int * pVec)`          |
| `GetTypeInfoForCheatMapPairKeyedByUint`            |        :x:         |      2 |    136 | `0x003a9530` | `0x004182f0` | `undefined GetTypeInfoForCheatMapPairKeyedByUint()`                                |
| `InvokeMemberFunctionPointer`                      |        :x:         |      0 |    124 | `0x003a6740` | `0x00415420` | `void InvokeMemberFunctionPointer(int pThis)`                                      |
| `ResolveDescriptorForSerialTasksObject`            |        :x:         |     23 |    120 | `0x003ab828` |              | `undefined ResolveDescriptorForSerialTasksObject()`                                |
| `GetTypeInfoForTrackBuilder`                       |        :x:         |     10 |    120 | `0x003a63e0` | `0x004150c0` | `undefined GetTypeInfoForTrackBuilder()`                                           |
| `ResolveDescriptorForStringClassObject`            |        :x:         |      3 |    120 | `0x003ab708` | `0x0041a468` | `undefined ResolveDescriptorForStringClassObject()`                                |
| `ResolvePointerDescriptorForTaskSinglePtr`         |        :x:         |      1 |    120 | `0x003ab8f0` |              | `undefined ResolvePointerDescriptorForTaskSinglePtr()`                             |
| `GetTypeInfoForATanInterpolator`                   |        :x:         |      0 |    120 | `0x003a8858` | `0x00417538` | `undefined GetTypeInfoForATanInterpolator()`                                       |
| `GetTypeInfoForArkFile`                            |        :x:         |      0 |    120 | `0x003a7e88` | `0x00416b68` | `undefined GetTypeInfoForArkFile()`                                                |
| `GetTypeInfoForArmPart`                            |        :x:         |      0 |    120 | `0x003a35a8` | `0x00412288` | `undefined GetTypeInfoForArmPart()`                                                |
| `GetTypeInfoForAsyncFile`                          |        :x:         |      0 |    120 | `0x003a7f08` | `0x00416be8` | `undefined GetTypeInfoForAsyncFile()`                                              |
| `GetTypeInfoForAsyncStream`                        |        :x:         |      0 |    120 | `0x003a89f8` |              | `undefined GetTypeInfoForAsyncStream()`                                            |
| `GetTypeInfoForChunkStream`                        |        :x:         |      0 |    120 | `0x003a95f8` |              | `undefined GetTypeInfoForChunkStream()`                                            |
| `GetTypeInfoForDebugStream`                        |        :x:         |      0 |    120 | `0x003a6c08` | `0x004158e8` | `undefined GetTypeInfoForDebugStream()`                                            |
| `GetTypeInfoForExpInterpolator`                    |        :x:         |      0 |    120 | `0x003a8700` | `0x004173e0` | `undefined GetTypeInfoForExpInterpolator()`                                        |
| `GetTypeInfoForFileStream`                         |        :x:         |      0 |    120 | `0x003a9d18` |              | `undefined GetTypeInfoForFileStream()`                                             |
| `GetTypeInfoForInstrPart`                          |        :x:         |      0 |    120 | `0x003a3498` | `0x00412178` | `undefined GetTypeInfoForInstrPart()`                                              |
| `GetTypeInfoForInverseExponentialInterpolator`     |        :x:         |      0 |    120 | `0x003a87a8` | `0x00417488` | `undefined GetTypeInfoForInverseExponentialInterpolator()`                         |
| `GetTypeInfoForLinearInterpolator`                 |        :x:         |      0 |    120 | `0x003a8658` | `0x00417338` | `undefined GetTypeInfoForLinearInterpolator()`                                     |
| `GetTypeInfoForMemcardSyncHandler`                 |        :x:         |      0 |    120 | `0x003a7050` | `0x00415d30` | `undefined GetTypeInfoForMemcardSyncHandler()`                                     |
| `GetTypeInfoForSFXBuilder`                         |        :x:         |      0 |    120 | `0x003a6350` | `0x00415030` | `undefined GetTypeInfoForSFXBuilder()`                                             |
| `GetTypeInfoForTableInterpolator`                  |        :x:         |      0 |    120 | `0x003a88d0` | `0x004175b0` | `undefined GetTypeInfoForTableInterpolator()`                                      |
| `GetTypeInfoForTorsoPart`                          |        :x:         |      0 |    120 | `0x003a3510` | `0x004121f0` | `undefined GetTypeInfoForTorsoPart()`                                              |
| `GetTypeInfoForTransportRT`                        |        :x:         |      0 |    120 | `0x003a1fc0` | `0x00410ca0` | `undefined GetTypeInfoForTransportRT()`                                            |
| `ResolveDescriptorForBoolOptionObject`             |        :x:         |      0 |    120 | `0x003ab580` | `0x0041a2e0` | `undefined ResolveDescriptorForBoolOptionObject()`                                 |
| `ResolveDescriptorForStringOptionObject`           |        :x:         |      0 |    120 | `0x003ab660` | `0x0041a3c0` | `undefined ResolveDescriptorForStringOptionObject()`                               |
| `JoypadInputMsg__Clone`                            |        :x:         |      1 |    116 | `0x003a7228` | `0x00415eb0` | `void JoypadInputMsg__Clone(int pSrcMsg)`                                          |
| `DestroyMemStreamObject`                           |        :x:         |      0 |    112 | `0x003ab1a8` | `0x00419f18` | `undefined DestroyMemStreamObject()`                                               |
| `DestroyMsgSinkListObject`                         |        :x:         |      0 |    108 | `0x003ab3d0` | `0x0041a130` | `undefined DestroyMsgSinkListObject()`                                             |
| `DestroyJoypadMsgSource`                           | :white_check_mark: |      0 |    108 | `0x003a6fb0` | `0x00415c90` | `void DestroyJoypadMsgSource(void * pThis, ulong nFlags)`                          |
| `CreateRampMidiObject`                             |        :x:         |      1 |    104 | `0x003a6620` | `0x00415300` | `undefined4 * CreateRampMidiObject(undefined4 nValue, undefined8 param2)`          |
| `GetTypeInfoForAsyncTaskListNodePtr`               |        :x:         |      5 |     80 | `0x003a83e0` | `0x004170c0` | `undefined GetTypeInfoForAsyncTaskListNodePtr()`                                   |
| `GetTypeInfoForCharPtrListNodePtr`                 |        :x:         |      4 |     80 | `0x003a6de0` | `0x00415ac0` | `undefined GetTypeInfoForCharPtrListNodePtr()`                                     |
| `GetTypeInfoForBlockRequestListNodePtr`            |        :x:         |      3 |     80 | `0x003a8390` | `0x00417070` | `undefined GetTypeInfoForBlockRequestListNodePtr()`                                |
| `GetTypeInfoForOptionPtrListNodePtr`               |        :x:         |      3 |     80 | `0x003a76b0` | `0x00416390` | `undefined GetTypeInfoForOptionPtrListNodePtr()`                                   |
| `GetTypeInfoForP9FileEntryPtr`                     |        :x:         |      3 |     80 | `0x003a7dc8` | `0x00416aa8` | `undefined GetTypeInfoForP9FileEntryPtr()`                                         |
| `GetTypeInfoForRbTreeNodeStrDataArrayPtr`          |        :x:         |      3 |     80 | `0x003a9be8` | `0x00418980` | `undefined GetTypeInfoForRbTreeNodeStrDataArrayPtr()`                              |
| `GetTypeInfoForSchedulerCommandNodePtr`            |        :x:         |      3 |     80 | `0x003a6a98` | `0x00415778` | `undefined GetTypeInfoForSchedulerCommandNodePtr()`                                |
| `GetTypeInfoForSongStatsPtr`                       |        :x:         |      3 |     80 | `0x003a5018` | `0x00413cf8` | `undefined GetTypeInfoForSongStatsPtr()`                                           |
| `GetTypeInfoForBtnInputEventIDQualified`           |        :x:         |      2 |     80 | `0x003a58e8` | `0x004145c8` | `undefined GetTypeInfoForBtnInputEventIDQualified()`                               |
| `GetTypeInfoForHandleToAvatar`                     |        :x:         |      2 |     80 | `0x003a2d48` | `0x00411a28` | `undefined GetTypeInfoForHandleToAvatar()`                                         |
| `GetTypeInfoForPlayerInfoPtrPtr`                   |        :x:         |      2 |     80 | `0x003a2960` | `0x00411640` | `undefined GetTypeInfoForPlayerInfoPtrPtr()`                                       |
| `GetTypeInfoForStickInputEventIDQualified`         |        :x:         |      2 |     80 | `0x003a5938` | `0x00414618` | `undefined GetTypeInfoForStickInputEventIDQualified()`                             |
| `GetTypeInfoForTaskQueueNodeHandle`                |        :x:         |      2 |     80 | `0x003a1e60` | `0x00410b40` | `undefined GetTypeInfoForTaskQueueNodeHandle()`                                    |
| `GetTypeInfoForVectorConstCharPtrPtr`              |        :x:         |      2 |     80 | `0x003a5068` | `0x00413d48` | `undefined GetTypeInfoForVectorConstCharPtrPtr()`                                  |
| `ResolvePointerDescriptorForMsgSinkListNode`       |        :x:         |      2 |     80 | `0x003ab440` | `0x0041a1a0` | `undefined ResolvePointerDescriptorForMsgSinkListNode()`                           |
| `GetQualifiedTypeInfoForSingleBlockPointer`        |        :x:         |      1 |     80 | `0x003a8520` | `0x00417200` | `undefined GetQualifiedTypeInfoForSingleBlockPointer()`                            |
| `GetQualifiedTypeInfoSingletonDataArrayPtrToPtr`   |        :x:         |      1 |     80 | `0x003a9e20` | `0x00418b90` | `undefined GetQualifiedTypeInfoSingletonDataArrayPtrToPtr()`                       |
| `GetQualifiedTypeInfoSingletonDataArraySinglePtr`  |        :x:         |      1 |     80 | `0x003a9e70` | `0x00418be0` | `undefined GetQualifiedTypeInfoSingletonDataArraySinglePtr()`                      |
| `GetTypeInfoForAvatarPartPtr`                      |        :x:         |      1 |     80 | `0x003a2d98` | `0x00411a78` | `undefined GetTypeInfoForAvatarPartPtr()`                                          |
| `GetTypeInfoForAvatarPartPtrPtr`                   |        :x:         |      1 |     80 | `0x003a2cf8` | `0x004119d8` | `undefined GetTypeInfoForAvatarPartPtrPtr()`                                       |
| `GetTypeInfoForBlockDoublePointer`                 |        :x:         |      1 |     80 | `0x003a8430` | `0x00417110` | `undefined GetTypeInfoForBlockDoublePointer()`                                     |
| `GetTypeInfoForFileClassDoublePtr`                 |        :x:         |      1 |     80 | `0x003a6e30` | `0x00415b10` | `undefined GetTypeInfoForFileClassDoublePtr()`                                     |
| `GetTypeInfoForFileClassPointer`                   |        :x:         |      1 |     80 | `0x003a6ed0` | `0x00415bb0` | `undefined GetTypeInfoForFileClassPointer()`                                       |
| `GetTypeInfoForLongCheatPointer`                   |        :x:         |      1 |     80 | `0x003a94d8` | `0x00418298` | `undefined GetTypeInfoForLongCheatPointer()`                                       |
| `GetTypeInfoForPlayerInfoPtr`                      |        :x:         |      1 |     80 | `0x003a29b0` | `0x00411690` | `undefined GetTypeInfoForPlayerInfoPtr()`                                          |
| `GetTypeInfoForPointerToAvatar`                    |        :x:         |      1 |     80 | `0x003a2de8` | `0x00411ac8` | `undefined GetTypeInfoForPointerToAvatar()`                                        |
| `GetTypeInfoForRndTransAnimPtr`                    |        :x:         |      1 |     80 | `0x003a3678` | `0x00412358` | `undefined GetTypeInfoForRndTransAnimPtr()`                                        |
| `GetTypeInfoForRndTransAnimPtrPtr`                 |        :x:         |      1 |     80 | `0x003a3628` | `0x00412308` | `undefined GetTypeInfoForRndTransAnimPtrPtr()`                                     |
| `ResolvePointerDescriptorForFuncDescObject`        |        :x:         |      1 |     80 | `0x003a9c88` | `0x00418a20` | `undefined ResolvePointerDescriptorForFuncDescObject()`                            |
| `ResolvePointerDescriptorForMsgFactoryEntry`       |        :x:         |      1 |     80 | `0x003ab2a0` | `0x0041a000` | `undefined ResolvePointerDescriptorForMsgFactoryEntry()`                           |
| `ResolvePointerDescriptorForTaskPtrToPtr`          |        :x:         |      1 |     80 | `0x003ab8a0` |              | `undefined ResolvePointerDescriptorForTaskPtrToPtr()`                              |
| `DestroyATanInterpolatorVtableThunk`               |        :x:         |      0 |     52 | `0x003a8820` | `0x00417500` | `void DestroyATanInterpolatorVtableThunk(void * pThis, ulong flags)`               |
| `MatchSchedulerCommandId`                          | :white_check_mark: |      0 |     52 | `0x003a6b38` | `0x00415818` | `bool MatchSchedulerCommandId(int pPred, undefined4 * pNode)`                      |
| `DestroyBoolOptionObject`                          |        :x:         |      0 |     48 | `0x003ab520` | `0x0041a280` | `undefined DestroyBoolOptionObject()`                                              |
| `DestroyBootPkt`                                   |        :x:         |      0 |     48 | `0x003a0f18` | `0x0040fbf8` | `void DestroyBootPkt(undefined4 * pThis, ulong nFlags)`                            |
| `DestroyClientStatusPkt`                           |        :x:         |      0 |     48 | `0x003a0820` | `0x0040f500` | `void DestroyClientStatusPkt(undefined4 * pThis, ulong nFlags)`                    |
| `DestroyExpInterpolatorVtableThunk`                |        :x:         |      0 |     48 | `0x003a86d0` | `0x004173b0` | `void DestroyExpInterpolatorVtableThunk(void * pThis, ulong flags)`                |
| `DestroyInverseExponentialInterpolatorVtableThunk` |        :x:         |      0 |     48 | `0x003a8778` | `0x00417458` | `void DestroyInverseExponentialInterpolatorVtableThunk(void * pThis, ulong flags)` |
| `DestroyLinearInterpolatorVtableThunk`             |        :x:         |      0 |     48 | `0x003a85f0` | `0x004172d0` | `void DestroyLinearInterpolatorVtableThunk(void * pThis, ulong flags)`             |
| `DestroyLoadMilestoneMsg`                          |        :x:         |      0 |     48 | `0x003a71a0` | `0x00415e80` | `void DestroyLoadMilestoneMsg(undefined4 * pThis, ulong nFlags)`                   |
| `DestroyNetTransportObject`                        |        :x:         |      0 |     48 | `0x003a1f90` | `0x00410c70` | `void DestroyNetTransportObject(undefined4 * pThis, ulong nFlags)`                 |
| `DestroyShareRemixPkt`                             |        :x:         |      0 |     48 | `0x003a1048` | `0x0040fd28` | `void DestroyShareRemixPkt(undefined4 * pThis, ulong nFlags)`                      |
| `DestroyStringOptionObject`                        |        :x:         |      0 |     48 | `0x003ab550` | `0x0041a2b0` | `undefined DestroyStringOptionObject()`                                            |
| `DestroyTimeRequestPkt`                            |        :x:         |      0 |     48 | `0x003a0ab8` | `0x0040f798` | `void DestroyTimeRequestPkt(undefined4 * pThis, ulong nFlags)`                     |
| `DestroyUpdateRanksPkt`                            |        :x:         |      0 |     48 | `0x003a1768` | `0x00410448` | `void DestroyUpdateRanksPkt(undefined4 * pThis, ulong nFlags)`                     |
| `DestructJoinFailedPkt`                            |        :x:         |      0 |     48 | `0x003a0448` | `0x0040f128` | `undefined DestructJoinFailedPkt()`                                                |
| `DestroyBufStreamVtableThunk`                      |       :memo:       |      0 |     48 | `0x003a8ae0` |              | `void DestroyBufStreamVtableThunk(void * pThis, ulong flags)`                      |
| `DestroyArkFileVtableThunk`                        | :white_check_mark: |      0 |     48 | `0x003a7e58` | `0x00416b38` | `void DestroyArkFileVtableThunk(void * pThis, ulong flags)`                        |
| `DestroyMemcardSyncHandler`                        | :white_check_mark: |      0 |     48 | `0x003a7020` | `0x00415d00` | `void DestroyMemcardSyncHandler(undefined4 * pThis, ulong nFlags)`                 |
| `DestroyDerivedMidiReceiverObject`                 |        :x:         |      0 |     40 | `0x003a6458` | `0x00415138` | `void DestroyDerivedMidiReceiverObject(int pThis)`                                 |
| `DestroyMidiReceiverSubobjectVariantB`             |        :x:         |      0 |     40 | `0x003a66c8` | `0x004153a8` | `void DestroyMidiReceiverSubobjectVariantB(int pThis)`                             |
| `RegisterBoolOptionParser`                         |        :x:         |      0 |     36 | `0x003ab5f8` | `0x0041a358` | `undefined RegisterBoolOptionParser()`                                             |
| `RegisterStringOptionParser`                       |        :x:         |      0 |     36 | `0x003ab6d8` | `0x0041a438` | `undefined RegisterStringOptionParser()`                                           |
| `DestructAvatarPartThunkAlpha`                     |        :x:         |      0 |     28 | `0x003a3478` | `0x00412158` | `undefined DestructAvatarPartThunkAlpha()`                                         |
| `DestructAvatarPartThunkBeta`                      |        :x:         |      0 |     28 | `0x003a3588` | `0x00412268` | `undefined DestructAvatarPartThunkBeta()`                                          |
| `AsyncStream__Eof`                                 | :white_check_mark: |      0 |     20 | `0x003a8a80` |              | `bool AsyncStream__Eof(AsyncStream * this)`                                        |
| `GetFirstWordField`                                |        :x:         |      3 |      8 | `0x003a67c0` | `0x004154a0` | `undefined4 GetFirstWordField(undefined4 * pObj)`                                  |
| `ReadFirstWordOfRampNode`                          |        :x:         |      1 |      8 | `0x003a6a90` | `0x00415770` | `undefined4 ReadFirstWordOfRampNode(undefined4 * pNode)`                           |
| `AsyncStream__Fail`                                | :white_check_mark: |      0 |      8 | `0x003a8a98` |              | `bool AsyncStream__Fail(AsyncStream * this)`                                       |
| `AsyncStream__Flush`                               | :white_check_mark: |      0 |      8 | `0x003a8a70` |              | `void AsyncStream__Flush(AsyncStream * this)`                                      |
| `AsyncStream__Tell`                                | :white_check_mark: |      0 |      8 | `0x003a8a78` |              | `int AsyncStream__Tell(AsyncStream * this)`                                        |
| `NopVirtual0x3ab810`                               |        :x:         |      0 |      1 | `0x003ab810` |              | `void NopVirtual0x3ab810(void)`                                                    |
| `NopVirtual0x3ab818`                               |        :x:         |      0 |      1 | `0x003ab818` |              | `void NopVirtual0x3ab818(void)`                                                    |
| `NopVirtual0x3ab820`                               |        :x:         |      0 |      1 | `0x003ab820` |              | `void NopVirtual0x3ab820(void)`                                                    |
