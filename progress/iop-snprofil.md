# IOP module SNProfil

3 of 3 routines done.

Addresses are module-relative. Import stubs are left out because the
build generates them from the module's import tables.

| Name         |       Status       | # xref | Length | Address      | Signature                                                                           |
| ------------ | :----------------: | -----: | -----: | ------------ | ----------------------------------------------------------------------------------- |
| `rpc_server` | :white_check_mark: |      1 |    196 | `0x00000148` | `ProfileBufferInfo * rpc_server(uint function, ProfileBufferInfo * data, int size)` |
| `test_th`    | :white_check_mark: |      1 |    176 | `0x00000098` | `void test_th(void)`                                                                |
| `start`      | :white_check_mark: |      4 |    152 | `0x00000000` | `int start(int argc, char * * argv)`                                                |
