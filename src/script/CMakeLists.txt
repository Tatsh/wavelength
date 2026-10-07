set(WAVELENGTH_SCRIPT_SRCS
    apptunnelscript.cpp
    cheatunlockstages.cpp
    cmdpostscript.cpp
    configquery.cpp
    cxx/extensionmodulebase.cpp
    cxx/int.cpp
    cxx/object.cpp
    cxx/pythonextensionbase.cpp
    cxx/pythontype.cpp
    cxx/seqbase.cpp
    cxx/string.cpp
    cxx/tuple.cpp
    gamecheats.cpp
    hudscript.cpp
    hxcommands.cpp
    hxmodule.cpp
    input.cpp
    playermode.cpp
    powerup.cpp
    pyinterpreter.cpp
    pyshell.cpp
    recinfo.cpp
    registercfunction.cpp
    sceneanim.cpp
    scripteval.cpp
    scriptcalltemplates.cpp
    scripthost.cpp
    scripttemplatemap.cpp
    testmap.cpp
    testregistry.cpp
    togglemode.cpp)

add_library(wavelength_script STATIC ${WAVELENGTH_SCRIPT_SRCS})
target_link_libraries(wavelength_script PRIVATE wavelength_common)