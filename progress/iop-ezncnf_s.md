# IOP module ezncnf_s

22 of 22 routines done.

Addresses are module-relative. Import stubs are left out because the
build generates them from the module's import tables.

| Name                          |       Status       | # xref | Length | Address      | Signature                                                                                               |
| ----------------------------- | :----------------: | -----: | -----: | ------------ | ------------------------------------------------------------------------------------------------------- |
| `NetCnfRequest__GetList`      | :white_check_mark: |      1 |   1180 | `0x000003ec` | `int NetCnfRequest__GetList(NetCnfRequest * this)`                                                      |
| `NetcnfifData__ReadInterface` | :white_check_mark: |      3 |    800 | `0x00000dc8` | `int NetcnfifData__ReadInterface(NetcnfifData * this, sceNetCnfInterface * interface, int kind)`        |
| `NetCnfRequest__LoadEntry`    | :white_check_mark: |      1 |    272 | `0x00000888` | `int NetCnfRequest__LoadEntry(NetCnfRequest * this)`                                                    |
| `EzNetCnf__SendToEe`          | :white_check_mark: |      2 |    212 | `0x00000a68` | `uint EzNetCnf__SendToEe(void * data, void * destination, int size, int noWait)`                        |
| `EzNetCnf__RpcHandler`        | :white_check_mark: |      0 |    208 | `0x00000998` | `void * EzNetCnf__RpcHandler(uint function, void * buffer, int size)`                                   |
| `NetcnfifData__ReadPairs`     | :white_check_mark: |      1 |    192 | `0x000010e8` | `int NetcnfifData__ReadPairs(NetcnfifData * this, sceNetCnfRoot * root)`                                |
| `NetcnfifData__ReadCommand`   | :white_check_mark: |      1 |    184 | `0x00000d10` | `int NetcnfifData__ReadCommand(NetcnfifData * this, sceNetCnfCommand * command, int * nameServerCount)` |
| `EzNetCnf__ModuleStart`       | :white_check_mark: |      1 |    164 | `0x00000034` | `int EzNetCnf__ModuleStart(void)`                                                                       |
| `NetcnfifEnv__GetDeviceType`  | :white_check_mark: |      1 |    164 | `0x00000240` | `int NetcnfifEnv__GetDeviceType(sceNetCnfEnv * this)`                                                   |
| `EzNetCnf__ModuleStop`        | :white_check_mark: |      1 |    156 | `0x000000d8` | `int EzNetCnf__ModuleStop(int argc, char * * argv)`                                                     |
| `NetcnfifEnv__LoadEntry`      | :white_check_mark: |      2 |    148 | `0x00000b40` | `int NetcnfifEnv__LoadEntry(sceNetCnfEnv * this, char * fileName, char * userName)`                     |
| `NetCnfRequest__FindUserName` | :white_check_mark: |      2 |    144 | `0x000002e4` | `char * NetCnfRequest__FindUserName(char * systemName, sceNetCnfList * list, int count)`                |
| `NetcnfifEnv__SelectByDevice` | :white_check_mark: |      2 |    136 | `0x00000bd4` | `int NetcnfifEnv__SelectByDevice(char * fileName, int memoryCard, int hardDisk, int other)`             |
| `EzNetCnf__RpcServerThread`   | :white_check_mark: |      1 |    120 | `0x00000174` | `void EzNetCnf__RpcServerThread(void)`                                                                  |
| `NetCnfRequest__ParseNumber`  | :white_check_mark: |      1 |    120 | `0x00000374` | `int NetCnfRequest__ParseNumber(char * name)`                                                           |
| `NetcnfifData__ReadEnv`       | :white_check_mark: |      1 |    120 | `0x000011a8` | `int NetcnfifData__ReadEnv(NetcnfifData * this, sceNetCnfEnv * env, int kind)`                          |
| `NetcnfifData__Reset`         | :white_check_mark: |      1 |    112 | `0x00000c9c` | `void NetcnfifData__Reset(NetcnfifData * this)`                                                         |
| `NetcnfifEnv__Init`           | :white_check_mark: |      1 |     64 | `0x00000c5c` | `void NetcnfifEnv__Init(sceNetCnfEnv * this)`                                                           |
| `EzNetCnf__RemoveRpcServer`   | :white_check_mark: |      1 |     56 | `0x000001ec` | `void EzNetCnf__RemoveRpcServer(void)`                                                                  |
| `start`                       | :white_check_mark: |      3 |     52 | `0x00000000` | `int start(int argc, char * * argv)`                                                                    |
| `EzNetCnf__Finalize`          | :white_check_mark: |      2 |      8 | `0x00000238` | `int EzNetCnf__Finalize(void)`                                                                          |
| `EzNetCnf__Initialize`        | :white_check_mark: |      1 |      8 | `0x00000230` | `int EzNetCnf__Initialize(void)`                                                                        |
