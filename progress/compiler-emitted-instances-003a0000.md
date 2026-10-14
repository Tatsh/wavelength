# Compiler-emitted instances, `0x003a0000` to `0x003b0000`

44 of 107 routines done.

Sorted by length, then reference count, then status (remaining first),
then name. Signatures are the current Ghidra prototypes and are
preliminary.

| Name                                              |       Status       | # xref | Length | NTSC-U/C     | PAL          | Signature                                                                          |
| ------------------------------------------------- | :----------------: | -----: | -----: | ------------ | ------------ | ---------------------------------------------------------------------------------- |
| `FxMidi__Destructor`                              | :white_check_mark: |      1 |   1200 | `0x003a5a80` | `0x00414760` | `void FxMidi__Destructor(void * pThis, uint nFlags)`                               |
| `FxMidi__FxMidi`                                  | :white_check_mark: |      1 |   1056 | `0x003a5f30` | `0x00414c10` | `void * FxMidi__FxMidi(void * pThis)`                                              |
| `CheatsManager__Destructor`                       | :white_check_mark: |      0 |    540 | `0x003a9268` | `0x00418028` | `void CheatsManager__Destructor(uint * pThis, ulong flags)`                        |
| `CopyConstructAsyncTaskList`                      |        :x:         |      2 |    340 | `0x003a8238` | `0x00416f18` | `void * CopyConstructAsyncTaskList(void * pThis, int * pSource)`                   |
| `ConstructChatMsgCopy`                            |        :x:         |      0 |    340 | `0x003a1a08` | `0x004106e8` | `undefined4 * ConstructChatMsgCopy(undefined4 * pThis, int pSource)`               |
| `ConstructGameOverPktCopy`                        |        :x:         |      0 |    272 | `0x003a1c70` | `0x00410950` | `undefined4 * ConstructGameOverPktCopy(undefined4 * pThis, int pSource)`           |
| `ConstructUpdateRanksPktCopy`                     |        :x:         |      0 |    272 | `0x003a1b60` | `0x00410840` | `undefined4 * ConstructUpdateRanksPktCopy(undefined4 * pThis, int pSource)`        |
| `operator_rshift_BinStream_Vector`                | :white_check_mark: |      1 |    248 | `0x003a4f20` | `0x00413c00` | `undefined8 operator_rshift_BinStream_Vector(undefined8 pStream, undefined8 pVec)` |
| `WorldBeat__WorldBeatCmd__Execute`                | :white_check_mark: |      0 |    204 | `0x003a64d0` | `0x004151b0` | `void WorldBeat__WorldBeatCmd__Execute(void * pThis)`                              |
| `CopyConstructIntVector`                          |        :x:         |      4 |    200 | `0x003a91a0` | `0x00417f60` | `void * CopyConstructIntVector(void * pThis, int * pSource)`                       |
| `GetTypeInfoForTableLinDerivedInterp`             |        :x:         |      0 |    164 | `0x003a8950` | `0x00417630` | `undefined GetTypeInfoForTableLinDerivedInterp()`                                  |
| `UpdateRanksPkt__Destructor`                      | :white_check_mark: |      0 |    156 | `0x003a15f8` | `0x004102d8` | `void UpdateRanksPkt__Destructor(int * pThis, ulong nFlags)`                       |
| `operator_lshift_BinStream_Vector`                | :white_check_mark: |      1 |    152 | `0x003a4a70` | `0x00413750` | `undefined8 operator_lshift_BinStream_Vector(undefined8 pStream, int * pVec)`      |
| `GetTypeInfoForCheatMapPairKeyedByUint`           |        :x:         |      2 |    136 | `0x003a9530` | `0x004182f0` | `undefined GetTypeInfoForCheatMapPairKeyedByUint()`                                |
| `MemFunCommand_Ramp__Execute`                     | :white_check_mark: |      0 |    124 | `0x003a6740` | `0x00415420` | `void MemFunCommand_Ramp__Execute(int pThis)`                                      |
| `ResolveDescriptorForSerialTasksObject`           |        :x:         |     23 |    120 | `0x003ab828` |              | `undefined ResolveDescriptorForSerialTasksObject()`                                |
| `GetTypeInfoForTrackBuilder`                      |        :x:         |     10 |    120 | `0x003a63e0` | `0x004150c0` | `undefined GetTypeInfoForTrackBuilder()`                                           |
| `ResolveDescriptorForStringClassObject`           |        :x:         |      3 |    120 | `0x003ab708` | `0x0041a468` | `undefined ResolveDescriptorForStringClassObject()`                                |
| `ResolvePointerDescriptorForTaskSinglePtr`        |        :x:         |      1 |    120 | `0x003ab8f0` |              | `undefined ResolvePointerDescriptorForTaskSinglePtr()`                             |
| `GetTypeInfoForATanInterpolator`                  |        :x:         |      0 |    120 | `0x003a8858` | `0x00417538` | `undefined GetTypeInfoForATanInterpolator()`                                       |
| `GetTypeInfoForArkFile`                           |        :x:         |      0 |    120 | `0x003a7e88` | `0x00416b68` | `undefined GetTypeInfoForArkFile()`                                                |
| `GetTypeInfoForArmPart`                           |        :x:         |      0 |    120 | `0x003a35a8` | `0x00412288` | `undefined GetTypeInfoForArmPart()`                                                |
| `GetTypeInfoForAsyncFile`                         |        :x:         |      0 |    120 | `0x003a7f08` | `0x00416be8` | `undefined GetTypeInfoForAsyncFile()`                                              |
| `GetTypeInfoForAsyncStream`                       |        :x:         |      0 |    120 | `0x003a89f8` |              | `undefined GetTypeInfoForAsyncStream()`                                            |
| `GetTypeInfoForChunkStream`                       |        :x:         |      0 |    120 | `0x003a95f8` |              | `undefined GetTypeInfoForChunkStream()`                                            |
| `GetTypeInfoForDebugStream`                       |        :x:         |      0 |    120 | `0x003a6c08` | `0x004158e8` | `undefined GetTypeInfoForDebugStream()`                                            |
| `GetTypeInfoForExpInterpolator`                   |        :x:         |      0 |    120 | `0x003a8700` | `0x004173e0` | `undefined GetTypeInfoForExpInterpolator()`                                        |
| `GetTypeInfoForFileStream`                        |        :x:         |      0 |    120 | `0x003a9d18` |              | `undefined GetTypeInfoForFileStream()`                                             |
| `GetTypeInfoForInstrPart`                         |        :x:         |      0 |    120 | `0x003a3498` | `0x00412178` | `undefined GetTypeInfoForInstrPart()`                                              |
| `GetTypeInfoForInverseExponentialInterpolator`    |        :x:         |      0 |    120 | `0x003a87a8` | `0x00417488` | `undefined GetTypeInfoForInverseExponentialInterpolator()`                         |
| `GetTypeInfoForLinearInterpolator`                |        :x:         |      0 |    120 | `0x003a8658` | `0x00417338` | `undefined GetTypeInfoForLinearInterpolator()`                                     |
| `GetTypeInfoForMemcardSyncHandler`                |        :x:         |      0 |    120 | `0x003a7050` | `0x00415d30` | `undefined GetTypeInfoForMemcardSyncHandler()`                                     |
| `GetTypeInfoForSFXBuilder`                        |        :x:         |      0 |    120 | `0x003a6350` | `0x00415030` | `undefined GetTypeInfoForSFXBuilder()`                                             |
| `GetTypeInfoForTableInterpolator`                 |        :x:         |      0 |    120 | `0x003a88d0` | `0x004175b0` | `undefined GetTypeInfoForTableInterpolator()`                                      |
| `GetTypeInfoForTorsoPart`                         |        :x:         |      0 |    120 | `0x003a3510` | `0x004121f0` | `undefined GetTypeInfoForTorsoPart()`                                              |
| `GetTypeInfoForTransportRT`                       |        :x:         |      0 |    120 | `0x003a1fc0` | `0x00410ca0` | `undefined GetTypeInfoForTransportRT()`                                            |
| `ResolveDescriptorForBoolOptionObject`            |        :x:         |      0 |    120 | `0x003ab580` | `0x0041a2e0` | `undefined ResolveDescriptorForBoolOptionObject()`                                 |
| `ResolveDescriptorForStringOptionObject`          |        :x:         |      0 |    120 | `0x003ab660` | `0x0041a3c0` | `undefined ResolveDescriptorForStringOptionObject()`                               |
| `JoypadInputMsg__Clone`                           |        :x:         |      1 |    116 | `0x003a7228` | `0x00415eb0` | `void JoypadInputMsg__Clone(int pSrcMsg)`                                          |
| `MemStream__Destructor`                           | :white_check_mark: |      0 |    112 | `0x003ab1a8` | `0x00419f18` | `undefined MemStream__Destructor()`                                                |
| `DestroyMsgSinkListObject`                        | :white_check_mark: |      0 |    108 | `0x003ab3d0` | `0x0041a130` | `undefined DestroyMsgSinkListObject()`                                             |
| `JoypadMsgSource__dtor`                           | :white_check_mark: |      0 |    108 | `0x003a6fb0` | `0x00415c90` | `void JoypadMsgSource__dtor(void * pThis, ulong nFlags)`                           |
| `NewMemFunCommand_Ramp`                           | :white_check_mark: |      1 |    104 | `0x003a6620` | `0x00415300` | `undefined4 * NewMemFunCommand_Ramp(undefined4 nValue, undefined8 param2)`         |
| `GetTypeInfoForAsyncTaskListNodePtr`              |        :x:         |      5 |     80 | `0x003a83e0` | `0x004170c0` | `undefined GetTypeInfoForAsyncTaskListNodePtr()`                                   |
| `GetTypeInfoForCharPtrListNodePtr`                |        :x:         |      4 |     80 | `0x003a6de0` | `0x00415ac0` | `undefined GetTypeInfoForCharPtrListNodePtr()`                                     |
| `GetTypeInfoForBlockRequestListNodePtr`           |        :x:         |      3 |     80 | `0x003a8390` | `0x00417070` | `undefined GetTypeInfoForBlockRequestListNodePtr()`                                |
| `GetTypeInfoForOptionPtrListNodePtr`              |        :x:         |      3 |     80 | `0x003a76b0` | `0x00416390` | `undefined GetTypeInfoForOptionPtrListNodePtr()`                                   |
| `GetTypeInfoForP9FileEntryPtr`                    |        :x:         |      3 |     80 | `0x003a7dc8` | `0x00416aa8` | `undefined GetTypeInfoForP9FileEntryPtr()`                                         |
| `GetTypeInfoForRbTreeNodeStrDataArrayPtr`         |        :x:         |      3 |     80 | `0x003a9be8` | `0x00418980` | `undefined GetTypeInfoForRbTreeNodeStrDataArrayPtr()`                              |
| `GetTypeInfoForSchedulerCommandNodePtr`           |        :x:         |      3 |     80 | `0x003a6a98` | `0x00415778` | `undefined GetTypeInfoForSchedulerCommandNodePtr()`                                |
| `GetTypeInfoForSongStatsPtr`                      |        :x:         |      3 |     80 | `0x003a5018` | `0x00413cf8` | `undefined GetTypeInfoForSongStatsPtr()`                                           |
| `GetTypeInfoForBtnInputEventIDQualified`          |        :x:         |      2 |     80 | `0x003a58e8` | `0x004145c8` | `undefined GetTypeInfoForBtnInputEventIDQualified()`                               |
| `GetTypeInfoForHandleToAvatar`                    |        :x:         |      2 |     80 | `0x003a2d48` | `0x00411a28` | `undefined GetTypeInfoForHandleToAvatar()`                                         |
| `GetTypeInfoForPlayerInfoPtrPtr`                  |        :x:         |      2 |     80 | `0x003a2960` | `0x00411640` | `undefined GetTypeInfoForPlayerInfoPtrPtr()`                                       |
| `GetTypeInfoForStickInputEventIDQualified`        |        :x:         |      2 |     80 | `0x003a5938` | `0x00414618` | `undefined GetTypeInfoForStickInputEventIDQualified()`                             |
| `GetTypeInfoForTaskQueueNodeHandle`               |        :x:         |      2 |     80 | `0x003a1e60` | `0x00410b40` | `undefined GetTypeInfoForTaskQueueNodeHandle()`                                    |
| `GetTypeInfoForVectorConstCharPtrPtr`             |        :x:         |      2 |     80 | `0x003a5068` | `0x00413d48` | `undefined GetTypeInfoForVectorConstCharPtrPtr()`                                  |
| `ResolvePointerDescriptorForMsgSinkListNode`      |        :x:         |      2 |     80 | `0x003ab440` | `0x0041a1a0` | `undefined ResolvePointerDescriptorForMsgSinkListNode()`                           |
| `GetQualifiedTypeInfoForSingleBlockPointer`       |        :x:         |      1 |     80 | `0x003a8520` | `0x00417200` | `undefined GetQualifiedTypeInfoForSingleBlockPointer()`                            |
| `GetQualifiedTypeInfoSingletonDataArrayPtrToPtr`  |        :x:         |      1 |     80 | `0x003a9e20` | `0x00418b90` | `undefined GetQualifiedTypeInfoSingletonDataArrayPtrToPtr()`                       |
| `GetQualifiedTypeInfoSingletonDataArraySinglePtr` |        :x:         |      1 |     80 | `0x003a9e70` | `0x00418be0` | `undefined GetQualifiedTypeInfoSingletonDataArraySinglePtr()`                      |
| `GetTypeInfoForAvatarPartPtr`                     |        :x:         |      1 |     80 | `0x003a2d98` | `0x00411a78` | `undefined GetTypeInfoForAvatarPartPtr()`                                          |
| `GetTypeInfoForAvatarPartPtrPtr`                  |        :x:         |      1 |     80 | `0x003a2cf8` | `0x004119d8` | `undefined GetTypeInfoForAvatarPartPtrPtr()`                                       |
| `GetTypeInfoForBlockDoublePointer`                |        :x:         |      1 |     80 | `0x003a8430` | `0x00417110` | `undefined GetTypeInfoForBlockDoublePointer()`                                     |
| `GetTypeInfoForFileClassDoublePtr`                |        :x:         |      1 |     80 | `0x003a6e30` | `0x00415b10` | `undefined GetTypeInfoForFileClassDoublePtr()`                                     |
| `GetTypeInfoForFileClassPointer`                  |        :x:         |      1 |     80 | `0x003a6ed0` | `0x00415bb0` | `undefined GetTypeInfoForFileClassPointer()`                                       |
| `GetTypeInfoForLongCheatPointer`                  |        :x:         |      1 |     80 | `0x003a94d8` | `0x00418298` | `undefined GetTypeInfoForLongCheatPointer()`                                       |
| `GetTypeInfoForPlayerInfoPtr`                     |        :x:         |      1 |     80 | `0x003a29b0` | `0x00411690` | `undefined GetTypeInfoForPlayerInfoPtr()`                                          |
| `GetTypeInfoForPointerToAvatar`                   |        :x:         |      1 |     80 | `0x003a2de8` | `0x00411ac8` | `undefined GetTypeInfoForPointerToAvatar()`                                        |
| `GetTypeInfoForRndTransAnimPtr`                   |        :x:         |      1 |     80 | `0x003a3678` | `0x00412358` | `undefined GetTypeInfoForRndTransAnimPtr()`                                        |
| `GetTypeInfoForRndTransAnimPtrPtr`                |        :x:         |      1 |     80 | `0x003a3628` | `0x00412308` | `undefined GetTypeInfoForRndTransAnimPtrPtr()`                                     |
| `ResolvePointerDescriptorForFuncDescObject`       |        :x:         |      1 |     80 | `0x003a9c88` | `0x00418a20` | `undefined ResolvePointerDescriptorForFuncDescObject()`                            |
| `ResolvePointerDescriptorForMsgFactoryEntry`      |        :x:         |      1 |     80 | `0x003ab2a0` | `0x0041a000` | `undefined ResolvePointerDescriptorForMsgFactoryEntry()`                           |
| `ResolvePointerDescriptorForTaskPtrToPtr`         |        :x:         |      1 |     80 | `0x003ab8a0` |              | `undefined ResolvePointerDescriptorForTaskPtrToPtr()`                              |
| `ATanInterpolator__Destructor`                    | :white_check_mark: |      0 |     52 | `0x003a8820` | `0x00417500` | `void ATanInterpolator__Destructor(void * pThis, ulong flags)`                     |
| `Scheduler__ByCommand__operator_call`             | :white_check_mark: |      0 |     52 | `0x003a6b38` | `0x00415818` | `bool Scheduler__ByCommand__operator_call(int pPred, undefined4 * pNode)`          |
| `ChatMsg__Destructor`                             |        :x:         |      0 |     48 | `0x003a1768` | `0x00410448` | `void ChatMsg__Destructor(undefined4 * pThis, ulong nFlags)`                       |
| `BoolOption__Destructor`                          | :white_check_mark: |      0 |     48 | `0x003ab550` | `0x0041a2b0` | `undefined BoolOption__Destructor()`                                               |
| `BootPkt__Destructor`                             | :white_check_mark: |      0 |     48 | `0x003a0f18` | `0x0040fbf8` | `void BootPkt__Destructor(undefined4 * pThis, ulong nFlags)`                       |
| `BufStream__Destructor`                           | :white_check_mark: |      0 |     48 | `0x003a8ae0` |              | `void BufStream__Destructor(void * pThis, ulong flags)`                            |
| `ClientStatusPkt__Destructor`                     | :white_check_mark: |      0 |     48 | `0x003a0820` | `0x0040f500` | `void ClientStatusPkt__Destructor(undefined4 * pThis, ulong nFlags)`               |
| `DestroyArkFileVtableThunk`                       | :white_check_mark: |      0 |     48 | `0x003a7e58` | `0x00416b38` | `void DestroyArkFileVtableThunk(void * pThis, ulong flags)`                        |
| `ExpInterpolator__Destructor`                     | :white_check_mark: |      0 |     48 | `0x003a86d0` | `0x004173b0` | `void ExpInterpolator__Destructor(void * pThis, ulong flags)`                      |
| `Interpolator__Destructor`                        | :white_check_mark: |      0 |     48 | `0x003a85f0` | `0x004172d0` | `void Interpolator__Destructor(void * pThis, ulong flags)`                         |
| `InvExpInterpolator__Destructor`                  | :white_check_mark: |      0 |     48 | `0x003a8778` | `0x00417458` | `void InvExpInterpolator__Destructor(void * pThis, ulong flags)`                   |
| `JoinFailedPkt__Destructor`                       | :white_check_mark: |      0 |     48 | `0x003a0448` | `0x0040f128` | `undefined JoinFailedPkt__Destructor()`                                            |
| `JoypadInputMsg__Destructor`                      | :white_check_mark: |      0 |     48 | `0x003a71a0` | `0x00415e80` | `void JoypadInputMsg__Destructor(undefined4 * pThis, ulong nFlags)`                |
| `MemcardSyncHandler__dtor`                        | :white_check_mark: |      0 |     48 | `0x003a7020` | `0x00415d00` | `void MemcardSyncHandler__dtor(undefined4 * pThis, ulong nFlags)`                  |
| `NetTransport__Destructor`                        | :white_check_mark: |      0 |     48 | `0x003a1f90` | `0x00410c70` | `void NetTransport__Destructor(undefined4 * pThis, ulong nFlags)`                  |
| `Option__Destructor`                              | :white_check_mark: |      0 |     48 | `0x003ab520` | `0x0041a280` | `undefined Option__Destructor()`                                                   |
| `ShareRemixPkt__Destructor`                       | :white_check_mark: |      0 |     48 | `0x003a1048` | `0x0040fd28` | `void ShareRemixPkt__Destructor(undefined4 * pThis, ulong nFlags)`                 |
| `TimeRequestPkt__Destructor`                      | :white_check_mark: |      0 |     48 | `0x003a0ab8` | `0x0040f798` | `void TimeRequestPkt__Destructor(undefined4 * pThis, ulong nFlags)`                |
| `MemFunCommand_Ramp__Destructor`                  | :white_check_mark: |      0 |     40 | `0x003a66c8` | `0x004153a8` | `void MemFunCommand_Ramp__Destructor(int pThis)`                                   |
| `WorldBeat__WorldBeatCmd__Destructor`             | :white_check_mark: |      0 |     40 | `0x003a6458` | `0x00415138` | `void WorldBeat__WorldBeatCmd__Destructor(int pThis)`                              |
| `RegisterBoolOptionParser`                        | :white_check_mark: |      0 |     36 | `0x003ab5f8` | `0x0041a358` | `undefined RegisterBoolOptionParser()`                                             |
| `RegisterStringOptionParser`                      | :white_check_mark: |      0 |     36 | `0x003ab6d8` | `0x0041a438` | `undefined RegisterStringOptionParser()`                                           |
| `ArmPart__Destructor`                             | :white_check_mark: |      0 |     28 | `0x003a3588` | `0x00412268` | `undefined ArmPart__Destructor()`                                                  |
| `InstrPart__Destructor`                           | :white_check_mark: |      0 |     28 | `0x003a3478` | `0x00412158` | `undefined InstrPart__Destructor()`                                                |
| `AsyncStream__Eof`                                | :white_check_mark: |      0 |     20 | `0x003a8a80` |              | `bool AsyncStream__Eof(AsyncStream * this)`                                        |
| `Ptr__Get`                                        | :white_check_mark: |      3 |      8 | `0x003a67c0` | `0x004154a0` | `undefined4 Ptr__Get(undefined4 * pObj)`                                           |
| `Ptr__operator_arrow`                             | :white_check_mark: |      1 |      8 | `0x003a6a90` | `0x00415770` | `undefined4 Ptr__operator_arrow(undefined4 * pNode)`                               |
| `AsyncStream__Fail`                               | :white_check_mark: |      0 |      8 | `0x003a8a98` |              | `bool AsyncStream__Fail(AsyncStream * this)`                                       |
| `AsyncStream__Flush`                              | :white_check_mark: |      0 |      8 | `0x003a8a70` |              | `void AsyncStream__Flush(AsyncStream * this)`                                      |
| `AsyncStream__Tell`                               | :white_check_mark: |      0 |      8 | `0x003a8a78` |              | `int AsyncStream__Tell(AsyncStream * this)`                                        |
| `Task__OnPoll`                                    | :white_check_mark: |      0 |      8 | `0x003ab820` |              | `void Task__OnPoll(void)`                                                          |
| `Task__OnStart`                                   | :white_check_mark: |      0 |      8 | `0x003ab810` |              | `void Task__OnStart(void)`                                                         |
| `Task__OnStop`                                    | :white_check_mark: |      0 |      8 | `0x003ab818` |              | `void Task__OnStop(void)`                                                          |
