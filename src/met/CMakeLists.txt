set(WAVELENGTH_MET_SRCS
    arenaunlockscreen.cpp
    autosavedonescreen.cpp
    avatarpanel.cpp
    bonuspicpanel.cpp
    bootscreen.cpp
    bossjourneyscreen.cpp
    bossunlockscreen.cpp
    cardsconnectedscreen.cpp
    chatpanel.cpp
    chattextentry.cpp
    choosememcardscreen.cpp
    controllerconfigscreen.cpp
    controllerpanel.cpp
    copyfreqscreen.cpp
    creditsscreen.cpp
    deletefreqscreen.cpp
    deleteremixscreen.cpp
    dialogpanel.cpp
    errorscreen.cpp
    focuschangepanel.cpp
    formatscreen.cpp
    freestylelaptipscreen.cpp
    freqconfirmscreen.cpp
    freqcopydelscreen.cpp
    freqlist.cpp
    freqloadlist.cpp
    freqmakercustomscreen.cpp
    freqmakeremblemscreen.cpp
    freqmakererrorscreen.cpp
    freqmakermainscreen.cpp
    freqmakerpartconfigscreen.cpp
    freqpanel.cpp
    freqscreen.cpp
    freqselpanel.cpp
    fullrankedpanel.cpp
    gameoptionsscreen.cpp
    gizmo.cpp
    helppanel.cpp
    introscreen.cpp
    jukeboxscreen.cpp
    keyboardkey.cpp
    keyboardpanel.cpp
    keyboardrequest.cpp
    keyboardscreen.cpp
    launchmtvpanel.cpp
    launchscreen.cpp
    listremixesscreen.cpp
    loadfreqscreen.cpp
    loadnetconfigsscreen.cpp
    loadremixscreen.cpp
    mainoptionsscreen.cpp
    marketingscreen.cpp
    mccheckscreen.cpp
    mcdialogpanel.cpp
    metaarenascreen.cpp
    metagame.cpp
    metagamearena.cpp
    metagameutil.cpp
    metamainscreen.cpp
    metamusicsong.cpp
    metaskillscreen.cpp
    metasongscreen.cpp
    metastartscreen.cpp
    mix.cpp
    mixmidibuilder.cpp
    mixtrack.cpp
    modescreen.cpp
    msgerrorscreen.cpp
    multiendgamescreen.cpp
    multifreqselectscreen.cpp
    multigamestatspanel.cpp
    multiloadfreqscreen.cpp
    multisetupremixsavescreen.cpp
    nameerrorscreen.cpp
    needsdialogscreen.cpp
    netbuttonpanel.cpp
    netchangepassword.cpp
    netchangepasswordscreen.cpp
    netchooseiconfigscreen.cpp
    netcreateaccount.cpp
    netcreateconfigscreen.cpp
    netcreateuserscreen.cpp
    netdodownloadscreen.cpp
    netdoplayersearchscreen.cpp
    netdouploadscreen.cpp
    netendgamescreen.cpp
    neteulascreen.cpp
    netflashupdate.cpp
    netfoundplayerpanel.cpp
    netgetdownloadsscreen.cpp
    nethostattemptscreen.cpp
    nethostingscreen.cpp
    nethostlpadbuttonpanel.cpp
    netinetconnect.cpp
    netinetdisconnect.cpp
    netjoinlpadscreen.cpp
    netlaunchpadquitscreen.cpp
    netlaunchscreen.cpp
    netlobbyconnect.cpp
    netlobbydisconnect.cpp
    netloginscreen.cpp
    netlpadgamepanel.cpp
    netlpadplayerpanel.cpp
    netlpadscreen.cpp
    netmainlaunchpadslist.cpp
    netmainlaunchpadspanel.cpp
    netmainlobbieslist.cpp
    netmainlobbiespanel.cpp
    netmainpanel.cpp
    netmainplayerslist.cpp
    netmainplayerspanel.cpp
    netmainscreen.cpp
    netnewchatroomscreen.cpp
    netparamsscreen.cpp
    netpasswordscreen.cpp
    netrankedplayerslist.cpp
    netrankedplayerspanel.cpp
    netrankingscreen.cpp
    netreadonlywarnscreen.cpp
    netsearchscreen.cpp
    netserverlogin.cpp
    netserverselscreen.cpp
    netsongpanel.cpp
    netsorteddatapanel.cpp
    netsortedlaunchpadslist.cpp
    netsortedlaunchpadspanel.cpp
    netsortedscreen.cpp
    netswitchlobbyscreen.cpp
    netwelcomescreen.cpp
    nocontrollerscreen.cpp
    numplayersscreen.cpp
    partunlockdonescreen.cpp
    partunlockscreen.cpp
    pausescreen.cpp
    playersearchresultsscreen.cpp
    playersearchsetupscreen.cpp
    playlist.cpp
    playlistpanel.cpp
    powerupscreen.cpp
    prelaunchscreen.cpp
    puptipsscreen.cpp
    readonlycheckscreen.cpp
    readonlysavescreen.cpp
    redbookplaybackscreen.cpp
    remixcontrollerscreen.cpp
    remixcopydelscreen.cpp
    remixcreatemodescreen.cpp
    remixdiscardscreen.cpp
    remixdownloadlist.cpp
    remixloadlist.cpp
    remixorfreqscreen.cpp
    remixselectscreen.cpp
    remixsongscreen.cpp
    remixtypescreen.cpp
    saveeditedfreqscreen.cpp
    saveerrorscreen.cpp
    savefreqscreen.cpp
    saveremixscreen.cpp
    savesettingsscreen.cpp
    selloadedfreqscreen.cpp
    selremixdownloadscreen.cpp
    selremixuploadscreen.cpp
    setupnamescreen.cpp
    setupnewchatroomscreen.cpp
    setupremixsavescreen.cpp
    shareremixscreen.cpp
    soloendgamescreen.cpp
    sologamestatspanel.cpp
    sololosestatspanel.cpp
    sololosetipspanel.cpp
    solowinstatspanel.cpp
    songdecryptscreen.cpp
    songpicpanel.cpp
    songpreview.cpp
    songsellist.cpp
    spaceerrorscreen.cpp
    tipsscreen.cpp
    transitionerrorscreen.cpp
    transitionmusic.cpp
    transitionscreen.cpp
    unlockavatarpanel.cpp
    unlockscreen.cpp
    uploadnotescreen.cpp
    viewanimplayer.cpp
    xproceedscreen.cpp)

add_library(wavelength_met STATIC ${WAVELENGTH_MET_SRCS})
target_link_libraries(wavelength_met PRIVATE wavelength_common)
# Patched builds show the commit and the build time on the title screen. The header is regenerated
# on every build.
if(ENABLE_PATCHES)
  set(_build_info "${CMAKE_BINARY_DIR}/generated/buildinfo.h")
  add_custom_target(
    wavelength_build_info
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" "-DOUTPUT=${_build_info}" -P
            "${CMAKE_SOURCE_DIR}/cmake/build-info.cmake"
    BYPRODUCTS "${_build_info}"
    VERBATIM)
  add_dependencies(wavelength_met wavelength_build_info)
  target_include_directories(wavelength_met PRIVATE "${CMAKE_BINARY_DIR}/generated")

  # The credits of patched builds open with this entry. The avatar is fetched once, when the
  # header is first generated. When it cannot be fetched, the entry shows the text alone.
  set(WAVELENGTH_CREDITS_GITHUB_USER
      "Tatsh"
      CACHE STRING "GitHub user whose avatar the first credit shows.")
  set(WAVELENGTH_CREDITS_TEXT
      "Reconstruction:\\nAndrew Udvare\\ngithub.com/Tatsh"
      CACHE STRING "Text of the first credit. \\n breaks a line.")
  find_program(WAVELENGTH_MAGICK magick)
  set(_credits_magick "${WAVELENGTH_MAGICK}")
  if(NOT WAVELENGTH_MAGICK)
    message(STATUS "ImageMagick was not found. The first credit has no avatar.")
    set(_credits_magick "")
  endif()
  set(_credits_avatar "${CMAKE_BINARY_DIR}/generated/creditsavatar.h")
  add_custom_command(
    OUTPUT "${_credits_avatar}"
    COMMAND
      "${WAVELENGTH_TOOL_CREDITS_AVATAR}" "${_credits_avatar}" --magick "${_credits_magick}" --user
      "${WAVELENGTH_CREDITS_GITHUB_USER}" --text "${WAVELENGTH_CREDITS_TEXT}"
    DEPENDS wavelength_tools "${WAVELENGTH_TOOL_CREDITS_AVATAR}"
    COMMENT "Writing ${_credits_avatar}"
    VERBATIM)
  add_custom_target(wavelength_credits_avatar DEPENDS "${_credits_avatar}")
  add_dependencies(wavelength_met wavelength_credits_avatar)
endif()
