# Sony online library (SCE-RT), PAL `0x00310000` to `0x00320000`

28 of 28 routines done.

Sorted by status (remaining first), then length, then reference count, then
name. Names and signatures are the current Ghidra prototypes of the PAL
program and are preliminary. The NTSC-U/C column is empty for a routine only
the PAL executable has.

| Name                           |       Status       | # xref | Length | PAL          | NTSC-U/C     | Signature                                     |
| ------------------------------ | :----------------: | -----: | -----: | ------------ | ------------ | --------------------------------------------- |
| `LibnetAsync__Dequeue`         | :white_check_mark: |      3 |    548 | `0x003109e8` | `0x00306760` | `undefined DequeueUploadChannelPacket()`      |
| `LibnetAsync__ReserveBlock`    | :white_check_mark: |      2 |    548 | `0x003112b8` | `0x00307030` | `undefined ReserveUploadRingSlot()`           |
| `LibnetAsync__Enqueue`         | :white_check_mark: |      2 |    524 | `0x00310ca0` | `0x00306a18` | `undefined EnqueueUploadChannelDmaPacket()`   |
| `LibnetAsyncWriteStream`       | :white_check_mark: |      1 |    308 | `0x00310600` | `0x00306378` | `undefined FillUploadStreamFromBuffer()`      |
| `LibnetAsyncOpen`              | :white_check_mark: |      1 |    276 | `0x00310250` | `0x00305fb8` | `undefined OpenUploadChannelStreams()`        |
| `LibnetAsyncCreate`            | :white_check_mark: |      1 |    272 | `0x00310130` | `0x00305e98` | `undefined CreateUploadChannelHandle()`       |
| `LibnetAsyncReadStream`        | :white_check_mark: |      1 |    264 | `0x00310400` | `0x00306168` | `undefined DrainUploadStreamToBuffer()`       |
| `LibnetAsync__FindFilledBlock` | :white_check_mark: |      1 |    188 | `0x003114e0` | `0x00307258` | `undefined PeekUploadRingReadyRecord()`       |
| `LibnetAsync__Poll`            | :white_check_mark: |      1 |    180 | `0x00311158` | `0x00306ed0` | `undefined CheckUploadChannelPendingFlag()`   |
| `LibnetAsync__StartRead`       | :white_check_mark: |      1 |    164 | `0x00311008` | `0x00306d80` | `undefined OpenUploadChannelReceiveRing()`    |
| `LibnetAsync__StartSend`       | :white_check_mark: |      1 |    164 | `0x003110b0` | `0x00306e28` | `undefined OpenUploadChannelTransmitRing()`   |
| `LibnetAsyncClose`             | :white_check_mark: |      2 |    152 | `0x00310368` | `0x003060d0` | `undefined CloseUploadChannelHandle()`        |
| `LibnetStringToAddress`        | :white_check_mark: |      2 |    152 | `0x00310868` | `0x003065e0` | `undefined DecodeIpv4StringToOctets()`        |
| `LibnetAsync__CopyToIop`       | :white_check_mark: |      6 |    148 | `0x00310950` | `0x003066c8` | `undefined SubmitSifDmaAndWait()`             |
| `LibnetAsyncSendTo`            | :white_check_mark: |      1 |    144 | `0x00310738` | `0x003064b0` | `undefined ReceiveUploadChannelMessage()`     |
| `LibnetAsync__DequeueMarker`   | :white_check_mark: |      1 |    140 | `0x00310c10` | `0x00306988` | `undefined CheckUploadStopMarkerDequeued()`   |
| `LibnetAsyncRecvFrom`          | :white_check_mark: |      1 |    136 | `0x00310508` | `0x00306280` | `undefined ReadUploadChannelData()`           |
| `LibnetAsync__EnqueueMarker`   | :white_check_mark: |      1 |    128 | `0x00310eb0` | `0x00306c28` | `undefined EnqueueUploadStopMarkerPacket()`   |
| `LibnetAsync__AllocBuffers`    | :white_check_mark: |      1 |    124 | `0x00310f30` | `0x00306ca8` | `undefined AllocateUploadChannelDmaBuffers()` |
| `LibnetAsync__MeasureFree`     | :white_check_mark: |      1 |    104 | `0x00311250` | `0x00306fc8` | `undefined ScanUploadRingFreeSpan()`          |
| `Libnet__WaitGetAddress`       | :white_check_mark: |      1 |    104 | `0x00310078` | `0x00305de0` | `undefined SendUploadChannelDataDirect()`     |
| `LibnetAsyncRecvMarker`        | :white_check_mark: |      1 |     96 | `0x00310598` | `0x00306310` | `undefined WriteUploadChannelData()`          |
| `LibnetAsyncSendMarker`        | :white_check_mark: |      1 |     96 | `0x003107d0` | `0x00306548` | `undefined ControlUploadChannel()`            |
| `LibnetAsync__FreeBuffers`     | :white_check_mark: |      2 |     80 | `0x00310fb0` | `0x00306d28` | `undefined FreeUploadChannelDmaBuffers()`     |
| `LibnetAddressToString`        | :white_check_mark: |      4 |     76 | `0x00310900` | `0x00306678` | `undefined EncodeIpv4OctetsAsDottedQuad()`    |
| `LibnetAsync__InitRing`        | :white_check_mark: |      2 |     64 | `0x00311210` | `0x00306f88` | `undefined InitUploadRingHeader()`            |
| `sceLibnetWaitGetAddress`      | :white_check_mark: |      1 |     52 | `0x003100e8` | `0x00305e50` | `undefined SendUploadChannelDataByMode()`     |
| `LibnetAsyncControl`           | :white_check_mark: |      1 |     44 | `0x00310838` | `0x003065b0` | `undefined EncodeUploadHandleVarRecord()`     |
