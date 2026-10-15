# Sony online library (SCE-RT), PAL `0x002e0000` to `0x002f0000`

28 of 28 routines done.

Sorted by status (remaining first), then length, then reference count, then
name. Names and signatures are the current Ghidra prototypes of the PAL
program and are preliminary. The NTSC-U/C column is empty for a routine only
the PAL executable has.

| Name                              |       Status       | # xref | Length | PAL          | NTSC-U/C     | Signature                                     |
| --------------------------------- | :----------------: | -----: | -----: | ------------ | ------------ | --------------------------------------------- |
| `rtDmeDeviceDecodeAudio`          | :white_check_mark: |      1 |   1232 | `0x002efe2c` | `0x002e5ebc` | `undefined DecodeAndDispatchAudioFrames()`    |
| `rtDmeDeviceDispatchRecord`       | :white_check_mark: |      1 |    920 | `0x002ef534` | `0x002e55c4` | `undefined DispatchDeviceWireRecord()`        |
| `rtDmeDeviceInit`                 | :white_check_mark: |      1 |    764 | `0x002ee8a4` | `0x002e4934` | `undefined InitializeDecoderDevice()`         |
| `rtDmeDeviceUpdate`               | :white_check_mark: |      2 |    620 | `0x002eef8c` | `0x002e501c` | `undefined PumpDecoderDeviceTick()`           |
| `rtDmeDeviceFree`                 | :white_check_mark: |      1 |    516 | `0x002eeba0` | `0x002e4c30` | `undefined FreeDecoderDevice()`               |
| `rtDmeDeviceOpenConnection`       | :white_check_mark: |      1 |    424 | `0x002ef38c` | `0x002e541c` | `undefined OpenNamedDeviceConnection()`       |
| `rtDmeDeviceServicePeers`         | :white_check_mark: |      1 |    404 | `0x002ef1f8` | `0x002e5288` | `undefined ProcessDeviceConnectionTimeouts()` |
| `rtDmeDeviceMapPort`              | :white_check_mark: |      1 |    392 | `0x002eeda4` | `0x002e4e34` | `undefined ConnectDeviceViaUpnp()`            |
| `rtDmeDeviceDisplayPeerVideo`     | :white_check_mark: |      1 |    380 | `0x002ee634` | `0x002e46c4` | `undefined ProcessXmlParserBuffer()`          |
| `rtDmeDeviceSendRecord`           | :white_check_mark: |      9 |    352 | `0x002ef8cc` | `0x002e595c` | `undefined SendConnectionWireRecord()`        |
| `rtDmeDeviceCaptureVideo`         | :white_check_mark: |      1 |    244 | `0x002ee540` | `0x002e45d0` | `undefined RunTimedParseCallback()`           |
| `rtDmeDeviceSendToPeer`           | :white_check_mark: |      5 |    240 | `0x002efaf4` | `0x002e5b84` | `undefined SendToConnectionEndpoints()`       |
| `rtDmeDeviceReceive`              | :white_check_mark: |      1 |    220 | `0x002efbe4` | `0x002e5c74` | `undefined ReceiveDeviceDatagram()`           |
| `rtDmeDeviceSendDatagram`         | :white_check_mark: |      4 |    200 | `0x002efa2c` | `0x002e5abc` | `undefined SendDeviceDatagram()`              |
| `rtDmeDeviceCreate`               | :white_check_mark: |      1 |    132 | `0x002ee7b0` | `0x002e4840` | `undefined AllocateXmlParserContext()`        |
| `rtDmeDeviceAddConnection`        | :white_check_mark: |      5 |     96 | `0x002efcc0` | `0x002e5d50` | `undefined AllocateConnectionNode()`          |
| `rtDmeDeviceStop`                 | :white_check_mark: |      1 |     96 | `0x002eef2c` | `0x002e4fbc` | `undefined UpdateDeviceRunFlag()`             |
| `rtDmeDeviceDestroySpeechCodecs`  | :white_check_mark: |      1 |     80 | `0x002efd8c` | `0x002e5e1c` | `undefined DestroyDeviceSyncPairA()`          |
| `rtDmeDeviceDestroySubbandCodecs` | :white_check_mark: |      1 |     80 | `0x002efddc` | `0x002e5e6c` | `undefined DestroyDeviceSyncPairB()`          |
| `rtDmeDeviceRemoveConnection`     | :white_check_mark: |      1 |     56 | `0x002efd54` | `0x002e5de4` | `undefined UnlinkConnectionNode()`            |
| `rtDmeDeviceFindConnection`       | :white_check_mark: |     21 |     52 | `0x002efd20` | `0x002e5db0` | `undefined FindConnectionById()`              |
| `rtDmeDeviceDestroy`              | :white_check_mark: |      1 |     52 | `0x002ee834` | `0x002e48c4` | `undefined ShutdownDecoderDevice()`           |
| `rtDmeGetProfileTimerElapsed`     | :white_check_mark: |     14 |     40 | `0x002ee518` | `0x002e45a8` | `undefined GetProfileTimerElapsed()`          |
| `rtDmeStartProfileTimer`          | :white_check_mark: |     14 |     36 | `0x002ee4f4` | `0x002e4584` | `undefined StartProfileTimer()`               |
| `rtDmeDeviceNextVideoSequence`    | :white_check_mark: |      1 |     24 | `0x002ee88c` | `0x002e491c` | `undefined PostIncrementObjectCounterField()` |
| `rtDmeDeviceNextAudioSequence`    | :white_check_mark: |      1 |     20 | `0x002ee878` | `0x002e4908` | `undefined GetAndIncrementCounter()`          |
| `rtDmeDeviceResetAudioSequence`   | :white_check_mark: |      2 |      8 | `0x002ee868` | `0x002e48f8` | `undefined ResetFieldZero()`                  |
| `rtDmeDeviceResetVideoSequence`   | :white_check_mark: |      1 |      8 | `0x002ee870` | `0x002e4900` | `undefined ResetFieldAt4()`                   |
