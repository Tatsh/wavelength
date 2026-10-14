# Game code, `0x002a0000` to `0x002b0000`

67 of 67 routines done.

Sorted by length, then reference count, then status (remaining first),
then name. Signatures are the current Ghidra prototypes and are
preliminary.

| Name                                 |       Status       | # xref | Length | NTSC-U/C     | PAL          | Signature                                                                             |
| ------------------------------------ | :----------------: | -----: | -----: | ------------ | ------------ | ------------------------------------------------------------------------------------- |
| `GIF_describeRegister`               | :white_check_mark: |      4 |   9068 | `0x002a2ac0` | `0x002aca30` | `int GIF_describeRegister(int id, uint * value, char * line)`                         |
| `VIF_decodeCode`                     | :white_check_mark: |      1 |   2168 | `0x002a6068` | `0x002affd8` | `int VIF_decodeCode(uint * code, char * line)`                                        |
| `VIF_disassemble`                    | :white_check_mark: |      0 |   1892 | `0x002a7488` | `0x002b13f8` | `int VIF_disassemble(void)`                                                           |
| `DMA_readTag`                        | :white_check_mark: |      1 |   1552 | `0x002a1c10` | `0x002abb80` | `int DMA_readTag(void)`                                                               |
| `GIF_describeTag`                    | :white_check_mark: |      1 |   1268 | `0x002a5420` | `0x002af390` | `int GIF_describeTag(char * line)`                                                    |
| `VIF_writeVector`                    | :white_check_mark: |      1 |   1024 | `0x002a6cb0` | `0x002b0c20` | `int VIF_writeVector(uint address, uint maskBits, int useTops, int column)`           |
| `DEBUG_init`                         | :white_check_mark: |      1 |    992 | `0x002a0920` | `0x002aa890` | `void DEBUG_init(char * options)`                                                     |
| `DEBUG_addField`                     | :white_check_mark: |    192 |    884 | `0x002a1200` | `0x002ab170` | `void DEBUG_addField(int placement, char * line, char * format, ...)`                 |
| `VIF_simulateUnpack`                 | :white_check_mark: |      1 |    864 | `0x002a70b0` | `0x002b1020` | `int VIF_simulateUnpack(char * line)`                                                 |
| `GIF_describeData`                   | :white_check_mark: |      1 |    660 | `0x002a5188` | `0x002af0f8` | `int GIF_describeData(char * line)`                                                   |
| `GIF_describePackedRegister`         | :white_check_mark: |      1 |    532 | `0x002a4f70` | `0x002aeee0` | `int GIF_describePackedRegister(int reg, uint * data, char * line)`                   |
| `Base64Decode`                       | :white_check_mark: |      1 |    512 | `0x002a0700` | `0x002aa670` | `void Base64Decode(uchar * pOut, int * pnOutLength, char * pszIn, int nInLength)`     |
| `VIF_grabElement`                    | :white_check_mark: |      1 |    512 | `0x002a68e0` | `0x002b0850` | `int VIF_grabElement(int sizeShift, int isUnsigned, uint * element)`                  |
| `GIF_describePrimitive`              | :white_check_mark: |      2 |    500 | `0x002a2770` | `0x002ac6e0` | `char * GIF_describePrimitive(uint prim, int withType, int withMeanings)`             |
| `VIF_dumpRegisters`                  | :white_check_mark: |      1 |    496 | `0x002a5b10` | `0x002afa80` | `int VIF_dumpRegisters(void)`                                                         |
| `DEBUG_message`                      | :white_check_mark: |      3 |    484 | `0x002a0f48` | `0x002aaeb8` | `int DEBUG_message(int number, int severity, char * text)`                            |
| `DMA_stepChain`                      | :white_check_mark: |      1 |    472 | `0x002a2410` | `0x002ac380` | `int DMA_stepChain(void)`                                                             |
| `VIF_grabVector`                     | :white_check_mark: |      1 |    460 | `0x002a6ae0` | `0x002b0a50` | `int VIF_grabVector(void)`                                                            |
| `DEBUG_print`                        | :white_check_mark: |     32 |    444 | `0x002a0d88` | `0x002aacf8` | `int DEBUG_print(char * text)`                                                        |
| `VIF_computeLength`                  | :white_check_mark: |      1 |    436 | `0x002a5eb0` | `0x002afe20` | `int VIF_computeLength(vif_registers * registers)`                                    |
| `SerialTasks__Add`                   | :white_check_mark: |     60 |    408 | `0x002a0340` | `0x002a9ff8` | `void SerialTasks__Add(SerialTasks * this, Task * pTask)`                             |
| `DMA_dumpStatistics`                 | :white_check_mark: |      1 |    336 | `0x002a15a0` | `0x002ab510` | `int DMA_dumpStatistics(void)`                                                        |
| `DMA_report`                         | :white_check_mark: |     10 |    332 | `0x002a16f0` | `0x002ab660` | `int DMA_report(int number, int severity, uint tagAddress, char * text)`              |
| `GIF_processQword`                   | :white_check_mark: |      1 |    332 | `0x002a5950` | `0x002af8c0` | `int GIF_processQword(void)`                                                          |
| `VIF_dumpStatistics`                 | :white_check_mark: |      1 |    292 | `0x002a5d00` | `0x002afc70` | `int VIF_dumpStatistics(void)`                                                        |
| `Base64Encode`                       | :white_check_mark: |      1 |    264 | `0x002a05f8` | `0x002aa568` | `void Base64Encode(char * pszOut, int * pnOutLength, uchar * pIn, int nInLength)`     |
| `DMA_isValidAddress`                 | :white_check_mark: |      1 |    224 | `0x002a1840` | `0x002ab7b0` | `int DMA_isValidAddress(uint address)`                                                |
| `GIF_pixelFormatName`                | :white_check_mark: |     14 |    212 | `0x002a2968` | `0x002ac8d8` | `char * GIF_pixelFormatName(uint psm)`                                                |
| `DMA_validateAddress`                | :white_check_mark: |      5 |    212 | `0x002a19a0` | `0x002ab910` | `int DMA_validateAddress(uint address, char * name)`                                  |
| `SerialTasks__Destructor`            | :white_check_mark: |     49 |    192 | `0x002a0280` | `0x002a9f38` | `void SerialTasks__Destructor(void * pTask, ulong dwFlags)`                           |
| `GIF_describeListRegister`           | :white_check_mark: |      1 |    184 | `0x002a4eb8` | `0x002aee28` | `int GIF_describeListRegister(int reg, uint * data, char * line)`                     |
| `SerialTasks__OnPoll`                | :white_check_mark: |      3 |    148 | `0x002a0560` | `0x002aa218` | `void SerialTasks__OnPoll(int * pTask)`                                               |
| `DEBUG_visibleLength`                | :white_check_mark: |      4 |    132 | `0x002a1130` | `0x002ab0a0` | `int DEBUG_visibleLength(char * text)`                                                |
| `GIF_resolveRegister`                | :white_check_mark: |      2 |    132 | `0x002a4e30` | `0x002aeda0` | `int GIF_resolveRegister(int reg, uint * data, int packed)`                           |
| `GIF_fixedToFloat`                   | :white_check_mark: |      9 |    128 | `0x002a26b8` | `0x002ac628` | `float GIF_fixedToFloat(uint value, int isSigned, int integerBits, int fractionBits)` |
| `GIF_extractBits`                    | :white_check_mark: |      1 |    128 | `0x002a2a40` | `0x002ac9b0` | `uint GIF_extractBits(uint * value, int shift, int bits)`                             |
| `VIF_refill`                         | :white_check_mark: |      6 |    120 | `0x002a7410` | `0x002b1380` | `int VIF_refill(void)`                                                                |
| `DMA_transferData`                   | :white_check_mark: |      2 |    120 | `0x002a1b98` | `0x002abb08` | `int DMA_transferData(void)`                                                          |
| `StreamBuffer__BytesAvailable`       | :white_check_mark: |      1 |    116 | `0x002a00a8` | `0x002a9d60` | `int StreamBuffer__BytesAvailable(StreamBuffer * pThis, int * pbInputDone)`           |
| `DMA_stop`                           | :white_check_mark: |      2 |    100 | `0x002a1b30` | `0x002abaa0` | `void DMA_stop(void)`                                                                 |
| `VIF_report`                         | :white_check_mark: |     17 |     96 | `0x002a5ab0` | `0x002afa20` | `int VIF_report(int number, int severity, char * text)`                               |
| `DMA_step`                           | :white_check_mark: |      2 |     84 | `0x002a2638` | `0x002ac5a8` | `int DMA_step(void)`                                                                  |
| `DMA_stepNormal`                     | :white_check_mark: |      1 |     80 | `0x002a25e8` | `0x002ac558` | `int DMA_stepNormal(void)`                                                            |
| `DMA_resolveAddress`                 | :white_check_mark: |      8 |     76 | `0x002a1920` | `0x002ab890` | `uint DMA_resolveAddress(uint address)`                                               |
| `Task__Stop`                         | :white_check_mark: |     69 |     72 | `0x002a0238` | `0x002a9ef0` | `void Task__Stop(Task * this)`                                                        |
| `Task__Finish`                       | :white_check_mark: |     59 |     72 | `0x002a0160` | `0x002a9e18` | `void Task__Finish(int * pThis, long bCancel)`                                        |
| `Task__Poll`                         | :white_check_mark: |     39 |     72 | `0x002a01a8` | `0x002a9e60` | `int Task__Poll(Task * this)`                                                         |
| `SerialTasks__OnStart`               | :white_check_mark: |     14 |     72 | `0x002a04d8` | `0x002aa190` | `void SerialTasks__OnStart(int * pTask)`                                              |
| `DEBUG_nextConversion`               | :white_check_mark: |      2 |     72 | `0x002a11b8` | `0x002ab128` | `char * DEBUG_nextConversion(char * format)`                                          |
| `DEBUG_reset`                        | :white_check_mark: |      1 |     72 | `0x002a0d40` | `0x002aacb0` | `void DEBUG_reset(void)`                                                              |
| `VIF_lookupCommand`                  | :white_check_mark: |      1 |     72 | `0x002a5e68` | `0x002afdd8` | `int VIF_lookupCommand(uint cmd)`                                                     |
| `Task__Start`                        | :white_check_mark: |     23 |     64 | `0x002a01f8` | `0x002a9eb0` | `void Task__Start(Task * this)`                                                       |
| `DEBUG_initUninitialised`            | :white_check_mark: |      3 |     64 | `0x002a0d00` | `0x002aac70` | `void DEBUG_initUninitialised(void)`                                                  |
| `SerialTasks__OnStop`                | :white_check_mark: |      1 |     64 | `0x002a0520` | `0x002aa1d8` | `void SerialTasks__OnStop(NetMultiTask * pTask)`                                      |
| `DMA_getDataAddress`                 | :white_check_mark: |      2 |     60 | `0x002a1ab0` | `0x002aba20` | `uint DMA_getDataAddress(void)`                                                       |
| `VIF_lookupUnpackFormat`             | :white_check_mark: |      1 |     60 | `0x002a5e28` | `0x002afd98` | `int VIF_lookupUnpackFormat(uint cmd)`                                                |
| `StreamBuffer__EmptySpaceForFilling` | :white_check_mark: |      4 |     56 | `0x002a0070` | `0x002a9d28` | `int StreamBuffer__EmptySpaceForFilling(StreamBuffer * pThis)`                        |
| `GIF_resetProgress`                  | :white_check_mark: |      1 |     56 | `0x002a5918` | `0x002af888` | `void GIF_resetProgress(void)`                                                        |
| `DMA_setDataAddress`                 | :white_check_mark: |      1 |     52 | `0x002a1a78` | `0x002ab9e8` | `void DMA_setDataAddress(uint address)`                                               |
| `Task__Destructor`                   | :white_check_mark: |    101 |     48 | `0x002a0130` | `0x002a9de8` | `void Task__Destructor(void * pThis, ulong dwFlags)`                                  |
| `DMA_toTagAddress`                   | :white_check_mark: |      9 |     44 | `0x002a1970` | `0x002ab8e0` | `uint DMA_toTagAddress(uint address)`                                                 |
| `DMA_advanceDataAddress`             | :white_check_mark: |      1 |     36 | `0x002a1af0` | `0x002aba60` | `void DMA_advanceDataAddress(void)`                                                   |
| `GIF_report`                         | :white_check_mark: |     21 |     28 | `0x002a2750` | `0x002ac6c0` | `int GIF_report(int number, int severity, char * text)`                               |
| `DMA_setQword`                       | :white_check_mark: |      2 |     20 | `0x002a1b18` | `0x002aba88` | `void DMA_setQword(uint address, int isTag)`                                          |
| `GIF_isTagDone`                      | :white_check_mark: |      1 |     20 | `0x002a2738` | `0x002ac6a8` | `int GIF_isTagDone(void)`                                                             |
| `StreamBuffer__Rewind`               | :white_check_mark: |      0 |     16 | `0x002a0120` |              | `void StreamBuffer__Rewind(StreamBuffer * pThis)`                                     |
| `Task__GetState`                     | :white_check_mark: |     13 |      8 | `0x002a01f0` | `0x002a9ea8` | `int Task__GetState(Task * this)`                                                     |
