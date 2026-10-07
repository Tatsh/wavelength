# Builds the IOP modules (IRX files) under src/iop.
#
# A module is R3000 code the IOP loader relocates. wavelength_add_irx() drives the IOP toolchain
# directly with custom commands: compile, relocatable link, fix up into an IRX, then write the
# module name into the .iopmod record. The modules build against the reconstructed IOP headers
# under sce/iop and import through the stub tables each module lists in an assembler source.
# Without the IOP toolchain, configuration records the gap and defines no module target.
#
# Each module is registered for the image target, which replaces the file of the same name in the
# iop directory of the disc.

find_program(WAVELENGTH_IOP_CC mipsel-none-elf-gcc PATHS "$ENV{PS2DEV}/iop/bin" "$ENV{PS2DEV}/bin")
find_program(WAVELENGTH_IOP_CXX mipsel-none-elf-g++ PATHS "$ENV{PS2DEV}/iop/bin"
                                                          "$ENV{PS2DEV}/bin")
find_program(
  WAVELENGTH_IOP_FIXUP
  NAMES srxfixup iopfixup
  PATHS "$ENV{PS2DEV}/ps2sdk/bin" "$ENV{PS2SDK}/bin" "$ENV{PS2DEV}/bin")

set(_wavelength_iop_missing "")
foreach(_probe WAVELENGTH_IOP_CC WAVELENGTH_IOP_CXX WAVELENGTH_IOP_FIXUP)
  if(NOT ${_probe})
    list(APPEND _wavelength_iop_missing ${_probe})
  endif()
endforeach()
if(_wavelength_iop_missing)
  message(STATUS "The IOP modules are skipped, missing: ${_wavelength_iop_missing}")
endif()

# Every IOP module, for a target that builds all of them at once.
add_custom_target(iop_modules)

set(WAVELENGTH_IOP_LINKFILE "${CMAKE_SOURCE_DIR}/sce/iop/iop.ld")
set(WAVELENGTH_IOP_FLAGS
    -G0
    -Os
    -Wall
    -Wextra
    -fno-builtin
    -fno-tree-loop-distribute-patterns
    -msoft-float
    -mno-explicit-relocs
    -I${CMAKE_SOURCE_DIR}/sce/iop/include
    -I${CMAKE_SOURCE_DIR}/src/iop)
# A custom command does not receive the directory definitions.
if(VIDEO_STANDARD STREQUAL "PAL")
  list(APPEND WAVELENGTH_IOP_FLAGS -DVIDEO_STANDARD_PAL)
endif()
set(WAVELENGTH_IOP_CXX_FLAGS
    -std=gnu++23
    -fno-exceptions
    -fno-rtti
    -fno-sized-deallocation
    -fno-threadsafe-statics
    -fno-asynchronous-unwind-tables)
# The C++ runtime every C++ module links.
set(WAVELENGTH_IOP_CXX_RUNTIME "${CMAKE_SOURCE_DIR}/src/iop/runtime/runtime.cpp")

# wavelength_add_irx(<module> [NO_CXX_RUNTIME] <source>...)
#
# Builds <module>.irx from the sources, given relative to src/iop/<module> in link order. C, C++,
# and preprocessed assembler (.S) sources are accepted. A module with a C++ source also links the
# C++ runtime under src/iop/runtime, unless NO_CXX_RUNTIME is given for C++ code that needs none
# of it.
function(wavelength_add_irx module)
  if(_wavelength_iop_missing)
    return()
  endif()
  cmake_parse_arguments(PARSE_ARGV 1 _irx "NO_CXX_RUNTIME" "" "")
  set(_src_dir "${CMAKE_SOURCE_DIR}/src/iop/${module}")
  set(_build_dir "${CMAKE_BINARY_DIR}/iop/${module}")
  file(MAKE_DIRECTORY "${_build_dir}")

  set(_sources "")
  set(_needs_runtime FALSE)
  foreach(_src ${_irx_UNPARSED_ARGUMENTS})
    list(APPEND _sources "${_src_dir}/${_src}")
    if(_src MATCHES "\\.cpp$" AND NOT _irx_NO_CXX_RUNTIME)
      set(_needs_runtime TRUE)
    endif()
  endforeach()
  if(_needs_runtime)
    list(APPEND _sources "${WAVELENGTH_IOP_CXX_RUNTIME}")
  endif()

  set(_objects "")
  foreach(_source ${_sources})
    get_filename_component(_name "${_source}" NAME)
    set(_object "${_build_dir}/${_name}.o")
    set(_compiler "${WAVELENGTH_IOP_CC}")
    set(_flags ${WAVELENGTH_IOP_FLAGS})
    if(_source MATCHES "\\.cpp$")
      set(_compiler "${WAVELENGTH_IOP_CXX}")
      list(APPEND _flags ${WAVELENGTH_IOP_CXX_FLAGS})
    endif()
    add_custom_command(
      OUTPUT "${_object}"
      COMMAND "${_compiler}" ${_flags} -MD -MF "${_object}.d" -c "${_source}" -o "${_object}"
      DEPENDS "${_source}"
      DEPFILE "${_object}.d"
      COMMENT "Compiling ${_name} for the IOP"
      VERBATIM)
    list(APPEND _objects "${_object}")
  endforeach()

  set(_elf "${_build_dir}/${module}.elf")
  set(_irx "${_build_dir}/${module}.irx")
  add_custom_command(
    OUTPUT "${_irx}"
    COMMAND "${WAVELENGTH_IOP_CC}" -T${WAVELENGTH_IOP_LINKFILE} -nostdlib -o "${_elf}" ${_objects}
            -Wl,-r -Wl,-dc -Wl,--force-group-allocation
    # The shipped modules have no symbol table, and srxfixup writes none through -o.
    COMMAND "${WAVELENGTH_IOP_FIXUP}" --rb --irx1 --allow-zero-text -o "${_irx}" "${_elf}"
    # srxfixup leaves the name in the .iopmod record empty. The shipped modules have it there.
    COMMAND "${WAVELENGTH_TOOL_NAME_IRX}" "${_irx}"
    DEPENDS ${_objects} "${WAVELENGTH_IOP_LINKFILE}" wavelength_tools "${WAVELENGTH_TOOL_NAME_IRX}"
    COMMENT "Linking ${module}.irx"
    VERBATIM)

  add_custom_target(${module}_irx ALL DEPENDS "${_irx}")
  add_dependencies(iop_modules ${module}_irx)
  set_property(GLOBAL APPEND PROPERTY WAVELENGTH_IRX_TARGETS ${module}_irx)
  set_property(GLOBAL APPEND PROPERTY WAVELENGTH_IRX_FILES "${_irx}")
endfunction()
