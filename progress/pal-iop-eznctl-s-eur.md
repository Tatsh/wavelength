# PAL IOP module eznctl_s-eur.irx

27 of 27 routines done.

Addresses are module-relative. Import stubs are left out because the
build generates them from the module's import tables.

| Name                            |       Status       | # xref | Length | Address      | Signature                                                                                   |
| ------------------------------- | :----------------: | -----: | -----: | ------------ | ------------------------------------------------------------------------------------------- |
| `NetcnfifEnv__WriteInterface`   | :white_check_mark: |      2 |   1228 | `0x00001524` | `int NetcnfifEnv__WriteInterface(sceNetCnfEnv * this, NetcnfifData * data, int kind)`       |
| `NetcnfifEnv__MergeDialNumbers` | :white_check_mark: |      1 |    856 | `0x000006f4` | `void NetcnfifEnv__MergeDialNumbers(sceNetCnfEnv * this)`                                   |
| `NetCtlStatus__Update`          | :white_check_mark: |      1 |    776 | `0x0000033c` | `int NetCtlStatus__Update(NetCtlStatus * this)`                                             |
| `NetcnfifEnv__WritePairs`       | :white_check_mark: |      1 |    496 | `0x000019f0` | `int NetcnfifEnv__WritePairs(sceNetCnfEnv * this, NetcnfifData * data)`                     |
| `EzNetCtl__RpcHandler`          | :white_check_mark: |      0 |    408 | `0x00000c30` | `void * EzNetCtl__RpcHandler(uint function, void * buffer, int size)`                       |
| `NetcnfifEnv__Attach`           | :white_check_mark: |      2 |    368 | `0x000013b4` | `int NetcnfifEnv__Attach(sceNetCnfEnv * this, int kind)`                                    |
| `NetcnfifEnv__AddRoute`         | :white_check_mark: |      1 |    320 | `0x00001098` | `int NetcnfifEnv__AddRoute(sceNetCnfEnv * this, char * gateway)`                            |
| `EzNetCtl__LookUpName`          | :white_check_mark: |      1 |    296 | `0x00000b08` | `char * EzNetCtl__LookUpName(char * name, int size)`                                        |
| `NetcnfifEnv__WriteAddresses`   | :white_check_mark: |      1 |    264 | `0x000012ac` | `int NetcnfifEnv__WriteAddresses(sceNetCnfEnv * this, NetcnfifData * data)`                 |
| `NetcnfifEnv__AddNameServer`    | :white_check_mark: |      2 |    212 | `0x000011d8` | `int NetcnfifEnv__AddNameServer(sceNetCnfEnv * this, char * address, int index)`            |
| `NetcnfifEnv__WriteEnv`         | :white_check_mark: |      1 |    204 | `0x00001be0` | `int NetcnfifEnv__WriteEnv(sceNetCnfEnv * this, NetcnfifData * data, int kind)`             |
| `EzNetCtl__Connect`             | :white_check_mark: |      1 |    188 | `0x00000a4c` | `int EzNetCtl__Connect(sceNetCnfEnv * env)`                                                 |
| `EzNetCtl__ModuleStart`         | :white_check_mark: |      1 |    164 | `0x00000034` | `int EzNetCtl__ModuleStart(void)`                                                           |
| `NetcnfifEnv__ResetInterface`   | :white_check_mark: |      1 |    164 | `0x00000fa0` | `void NetcnfifEnv__ResetInterface(sceNetCnfInterface * interface)`                          |
| `EzNetCtl__ModuleStop`          | :white_check_mark: |      1 |    156 | `0x000000d8` | `int EzNetCtl__ModuleStop(int argc, char * * argv)`                                         |
| `EzNetCtl__Initialize`          | :white_check_mark: |      1 |    152 | `0x00000230` | `int EzNetCtl__Initialize(void)`                                                            |
| `NetcnfifEnv__LoadEntry`        | :white_check_mark: |      1 |    148 | `0x00000dd0` | `int NetcnfifEnv__LoadEntry(sceNetCnfEnv * this, char * fileName, char * userName)`         |
| `EzNetCtl__EventHandler`        | :white_check_mark: |      0 |    140 | `0x00000668` | `void EzNetCtl__EventHandler(int interfaceId, int event)`                                   |
| `NetcnfifEnv__SelectByDevice`   | :white_check_mark: |      1 |    136 | `0x00000e64` | `int NetcnfifEnv__SelectByDevice(char * fileName, int memoryCard, int hardDisk, int other)` |
| `EzNetCtl__RpcServerThread`     | :white_check_mark: |      1 |    120 | `0x00000174` | `void EzNetCtl__RpcServerThread(void)`                                                      |
| `EzNetCtl__Finalize`            | :white_check_mark: |      2 |    116 | `0x000002c8` | `int EzNetCtl__Finalize(void)`                                                              |
| `NetcnfifData__Reset`           | :white_check_mark: |      0 |    112 | `0x00000f2c` | `void NetcnfifData__Reset(NetcnfifData * this)`                                             |
| `NetcnfifEnv__IsNonzeroAddress` | :white_check_mark: |      3 |     84 | `0x00001044` | `bool NetcnfifEnv__IsNonzeroAddress(char * text)`                                           |
| `NetcnfifEnv__Init`             | :white_check_mark: |      2 |     64 | `0x00000eec` | `void NetcnfifEnv__Init(sceNetCnfEnv * this)`                                               |
| `EzNetCtl__RemoveRpcServer`     | :white_check_mark: |      1 |     56 | `0x000001ec` | `void EzNetCtl__RemoveRpcServer(void)`                                                      |
| `start`                         | :white_check_mark: |      3 |     52 | `0x00000000` | `int start(int argc, char * * argv)`                                                        |
| `EzNetCtl__AlarmHandler`        | :white_check_mark: |      0 |     36 | `0x00000644` | `uint EzNetCtl__AlarmHandler(int thread)`                                                   |
