# IOP module libnetb

46 of 46 routines done.

Addresses are module-relative. Import stubs are left out because the
build generates them from the module's import tables.

| Name                                 |       Status       | # xref | Length | Address      | Signature                                                                                                                     |
| ------------------------------------ | :----------------: | -----: | -----: | ------------ | ----------------------------------------------------------------------------------------------------------------------------- |
| `Libnet__RpcHandler`                 | :white_check_mark: |      0 |   1404 | `0x000003f8` | `void * Libnet__RpcHandler(uint function, void * buffer, int size)`                                                           |
| `AsyncInfo__DumpList`                | :white_check_mark: |      2 |   1280 | `0x0000201c` | `void AsyncInfo__DumpList(int list)`                                                                                          |
| `Libnet__HandleAsync`                | :white_check_mark: |      1 |   1084 | `0x00000d18` | `void * Libnet__HandleAsync(uint function, void * buffer)`                                                                    |
| `AsyncInfo__TcpReceiveThread`        | :white_check_mark: |      0 |    888 | `0x000012e4` | `int AsyncInfo__TcpReceiveThread(AsyncInfo * this)`                                                                           |
| `Libnet__DumpConnections`            | :white_check_mark: |      1 |    816 | `0x0000251c` | `void Libnet__DumpConnections(void)`                                                                                          |
| `RawIp__Receive`                     | :white_check_mark: |      1 |    808 | `0x00002dd8` | `int RawIp__Receive(int cid, ushort * port, sceInetAddress * address, bool * received, char * name)`                          |
| `AsyncInfo__ReceiveBlock`            | :white_check_mark: |      1 |    736 | `0x00002af8` | `int AsyncInfo__ReceiveBlock(AsyncInfo * this, int kind, char * name)`                                                        |
| `RawIp__Send`                        | :white_check_mark: |      1 |    736 | `0x00003100` | `int RawIp__Send(int cid, ushort port, sceInetAddress * address, bool * sent, char * name)`                                   |
| `AsyncInfo__ReserveBlock`            | :white_check_mark: |      1 |    572 | `0x0000372c` | `int AsyncInfo__ReserveBlock(AsyncInfo * this, uint size, uint * reserved, bool * ready)`                                     |
| `AsyncInfo__SendBlock`               | :white_check_mark: |      1 |    572 | `0x000028bc` | `int AsyncInfo__SendBlock(AsyncInfo * this, int kind, char * name)`                                                           |
| `AsyncInfo__TcpSendThread`           | :white_check_mark: |      0 |    452 | `0x00001738` | `void AsyncInfo__TcpSendThread(AsyncInfo * this)`                                                                             |
| `Libnet__FormatBuildDate`            | :white_check_mark: |      2 |    444 | `0x00000b5c` | `bool Libnet__FormatBuildDate(char * text, int size)`                                                                         |
| `AsyncInfo__BlockSendThread`         | :white_check_mark: |      0 |    400 | `0x000018fc` | `void AsyncInfo__BlockSendThread(AsyncInfo * this)`                                                                           |
| `Libnet__PrintInetError`             | :white_check_mark: |      7 |    364 | `0x0000343c` | `void Libnet__PrintInetError(int error)`                                                                                      |
| `InetEventQueue__GetEvent`           | :white_check_mark: |      3 |    364 | `0x000001ec` | `int InetEventQueue__GetEvent(int * interfaceId, int * event)`                                                                |
| `AsyncInfo__StopSendThread`          | :white_check_mark: |      1 |    300 | `0x00001cf8` | `bool AsyncInfo__StopSendThread(int cid, int timeout)`                                                                        |
| `InetEventQueue__EventHandler`       | :white_check_mark: |      3 |    272 | `0x00000000` | `void InetEventQueue__EventHandler(int interfaceId, int event)`                                                               |
| `AsyncInfo__StopReadThread`          | :white_check_mark: |      1 |    260 | `0x00001bf4` | `bool AsyncInfo__StopReadThread(int cid, int timeout)`                                                                        |
| `AsyncInfo__BlockReceiveThread`      | :white_check_mark: |      0 |    220 | `0x0000165c` | `void AsyncInfo__BlockReceiveThread(AsyncInfo * this)`                                                                        |
| `AsyncInfo__FindFilledBlock`         | :white_check_mark: |      1 |    216 | `0x00003968` | `int AsyncInfo__FindFilledBlock(AsyncInfo * this, AsyncBlock * * block, bool * found)`                                        |
| `AsyncInfo__PollBlocks`              | :white_check_mark: |      4 |    204 | `0x000035b0` | `int AsyncInfo__PollBlocks(AsyncInfo * this, bool blocks, bool sending, bool * available)`                                    |
| `Libnet__ParseArguments`             | :white_check_mark: |      1 |    200 | `0x00000a38` | `void Libnet__ParseArguments(int argc, char * * argv)`                                                                        |
| `AsyncInfo__StartReadThread`         | :white_check_mark: |      1 |    180 | `0x00001a8c` | `bool AsyncInfo__StartReadThread(AsyncInfo * this)`                                                                           |
| `AsyncInfo__StartSendThread`         | :white_check_mark: |      1 |    180 | `0x00001b40` | `bool AsyncInfo__StartSendThread(AsyncInfo * this)`                                                                           |
| `Libnet__SendToEe`                   | :white_check_mark: |      7 |    164 | `0x00001240` | `int Libnet__SendToEe(void * data, void * destination, int size)`                                                             |
| `InetEventQueue__MapEvent`           | :white_check_mark: |      2 |    160 | `0x00000358` | `int InetEventQueue__MapEvent(int interfaceId, int event)`                                                                    |
| `LibnetConfig__Apply`                | :white_check_mark: |      1 |    144 | `0x00001154` | `void LibnetConfig__Apply(LibnetConfig * this)`                                                                               |
| `Libnet__StartRpcThread`             | :white_check_mark: |      1 |    136 | `0x000009b0` | `int Libnet__StartRpcThread(void)`                                                                                            |
| `InetEventQueue__RecordInterface`    | :white_check_mark: |      1 |    132 | `0x00000110` | `void InetEventQueue__RecordInterface(int interfaceId)`                                                                       |
| `AsyncInfo__RemoveRead`              | :white_check_mark: |      1 |    116 | `0x00001ec4` | `void AsyncInfo__RemoveRead(AsyncInfo * this)`                                                                                |
| `AsyncInfo__RemoveSend`              | :white_check_mark: |      1 |    116 | `0x00001f38` | `void AsyncInfo__RemoveSend(AsyncInfo * this)`                                                                                |
| `Libnet__CreateNamedThread`          | :white_check_mark: |      2 |    112 | `0x0000284c` | `int Libnet__CreateNamedThread(ThreadParam * param, char * name, void * entry, int stackSize, int priority, void * argument)` |
| `AsyncBlock__MeasureFree`            | :white_check_mark: |      1 |    104 | `0x000036c4` | `int AsyncBlock__MeasureFree(AsyncBlock * this, uint size, uint * free)`                                                      |
| `RawIp__Checksum`                    | :white_check_mark: |      1 |     92 | `0x000033e0` | `ushort RawIp__Checksum(ushort * data, int size)`                                                                             |
| `InetEventQueue__IsPrimaryInterface` | :white_check_mark: |      1 |     88 | `0x00000194` | `int InetEventQueue__IsPrimaryInterface(int interfaceId)`                                                                     |
| `AsyncInfo__AppendRead`              | :white_check_mark: |      1 |     80 | `0x00001e24` | `void AsyncInfo__AppendRead(AsyncInfo * this)`                                                                                |
| `AsyncInfo__AppendSend`              | :white_check_mark: |      1 |     80 | `0x00001e74` | `void AsyncInfo__AppendSend(AsyncInfo * this)`                                                                                |
| `AsyncBlock__InitRing`               | :white_check_mark: |      2 |     72 | `0x0000367c` | `int AsyncBlock__InitRing(AsyncBlock * this, uint size)`                                                                      |
| `Libnet__PrintSendDelay`             | :white_check_mark: |      1 |     68 | `0x000011fc` | `void Libnet__PrintSendDelay(void)`                                                                                           |
| `start`                              | :white_check_mark: |      0 |     68 | `0x00000b00` | `int start(int argc, char * * argv)`                                                                                          |
| `Libnet__RpcThread`                  | :white_check_mark: |      0 |     60 | `0x00000974` | `int Libnet__RpcThread(void)`                                                                                                 |
| `AsyncInfo__FindRead`                | :white_check_mark: |      2 |     56 | `0x00001fac` | `AsyncInfo * AsyncInfo__FindRead(int cid)`                                                                                    |
| `AsyncInfo__FindSend`                | :white_check_mark: |      2 |     56 | `0x00001fe4` | `AsyncInfo * AsyncInfo__FindSend(int cid)`                                                                                    |
| `LibnetConfig__Init`                 | :white_check_mark: |      1 |     24 | `0x000011e4` | `void LibnetConfig__Init(LibnetConfig * this)`                                                                                |
| `Libnet__GetVersion`                 | :white_check_mark: |      2 |     12 | `0x00000b50` | `char * Libnet__GetVersion(void)`                                                                                             |
| `LibnetExportStub`                   | :white_check_mark: |      0 |      8 | `0x00003a68` | `void LibnetExportStub(void)`                                                                                                 |
