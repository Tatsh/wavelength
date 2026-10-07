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
