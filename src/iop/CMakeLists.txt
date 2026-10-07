# One line per IOP module. The sources are listed relative to the module's directory in link order.
wavelength_add_irx(softfx_s softfx.cpp lock.cpp periodictimer.cpp effects.cpp ringbuffer.cpp
                   streambuffer.cpp imports.S)
wavelength_add_irx(
  synth_s
  bankheader.cpp
  synthmodule.cpp
  synthserver.cpp
  synth.cpp
  program.cpp
  midieventqueue.cpp
  poolallocator.cpp
  sample.cpp
  sampledesc.cpp
  banksampledesc.cpp
  bankprogram.cpp
  bank.cpp
  channel.cpp
  voice.cpp
  synthlock.cpp
  ticktimer.cpp
  ioputil.cpp
  vibrato.cpp
  random.cpp
  imports.S)
wavelength_add_irx(
  ezncnf_s
  NO_CXX_RUNTIME
  eznetcnf.cpp
  netcnfrequest.cpp
  netcnfifenv.cpp
  netcnfifdata.cpp
  imports.S)
wavelength_add_irx(
  eznctl_s
  NO_CXX_RUNTIME
  eznetctl.cpp
  netctlstatus.cpp
  netcnfifenv.cpp
  netcnfifdata.cpp
  imports.S)
# The modem driver ships only on the NTSC-U/C disc.
if(VIDEO_STANDARD STREQUAL "NTSC")
  wavelength_add_irx(cxtmdm NO_CXX_RUNTIME cxtmodem.cpp cxtmdm.cpp imports.S)
endif()
