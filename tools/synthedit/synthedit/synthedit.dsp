# Microsoft Developer Studio Project File - Name="synthedit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=synthedit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE
!MESSAGE NMAKE /f "synthedit.mak".
!MESSAGE
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "synthedit.mak" CFG="synthedit - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "synthedit - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "synthedit - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "synthedit - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /I "..\encvag" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 encvag.lib dinput.lib dxguid.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"Release"
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Release
TargetPath=.\Release\synthedit.ocx
InputPath=.\Release\synthedit.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)"
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg"

# End Custom Build

!ELSEIF  "$(CFG)" == "synthedit - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I "..\encvag" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "$(OUTDIR)" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 encvag.lib dinput.lib dxguid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"Debug"
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Debug
TargetPath=.\Debug\synthedit.ocx
InputPath=.\Debug\synthedit.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)"
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg"

# End Custom Build

!ENDIF

# Begin Target

# Name "synthedit - Win32 Release"
# Name "synthedit - Win32 Debug"
# Begin Group "synthedit"

# PROP Default_Filter "cpp;h;odl;def;rc"
# Begin Source File

SOURCE=.\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\HxLongSafeArray.cpp
# End Source File
# Begin Source File

SOURCE=.\HxSafeArray.cpp
# End Source File
# Begin Source File

SOURCE=.\HxStringSafeArray.cpp
# End Source File
# Begin Source File

SOURCE=.\IntDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\synthedit.cpp
# End Source File
# Begin Source File

SOURCE=.\synthedit.def
# End Source File
# Begin Source File

SOURCE=.\synthedit.odl
# End Source File
# Begin Source File

SOURCE=.\synthedit.rc
# End Source File
# Begin Source File

SOURCE=.\SyntheditCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\SyntheditPpg.cpp
# End Source File
# End Group
# Begin Group "os"

# PROP Default_Filter "cpp;h"
# Begin Source File

SOURCE=..\os\Archive.cpp
# End Source File
# Begin Source File

SOURCE=..\os\ArkFile.cpp
# End Source File
# Begin Source File

SOURCE=..\os\ArkHash.cpp
# End Source File
# Begin Source File

SOURCE=..\os\AsyncFile.cpp
# End Source File
# Begin Source File

SOURCE=..\os\AsyncFileWin.cpp
# End Source File
# Begin Source File

SOURCE=..\os\AsyncTask.cpp
# End Source File
# Begin Source File

SOURCE=..\os\Block.cpp
# End Source File
# Begin Source File

SOURCE=..\os\BlockMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\os\BlockRequest.cpp
# End Source File
# Begin Source File

SOURCE=..\os\CDReader.cpp
# End Source File
# Begin Source File

SOURCE=..\os\DateTime.cpp
# End Source File
# Begin Source File

SOURCE=..\os\Debug.cpp
# End Source File
# Begin Source File

SOURCE=..\os\DIJoypad.cpp
# End Source File
# Begin Source File

SOURCE=..\os\File.cpp
# End Source File
# Begin Source File

SOURCE=..\os\Joypad.cpp
# End Source File
# Begin Source File

SOURCE=..\os\JoypadData.cpp
# End Source File
# Begin Source File

SOURCE=..\os\JoypadMsgSource.cpp
# End Source File
# Begin Source File

SOURCE=..\os\JoypadWin.cpp
# End Source File
# Begin Source File

SOURCE=..\os\PadMap.cpp
# End Source File
# Begin Source File

SOURCE=..\os\System.cpp
# End Source File
# Begin Source File

SOURCE=..\os\SystemWin.cpp
# End Source File
# Begin Source File

SOURCE=..\os\Timer.cpp
# End Source File
# End Group
# Begin Group "utl"

# PROP Default_Filter "cpp;h;l"
# Begin Source File

SOURCE=..\utl\AllocInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\BinStream.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\BlockStat.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\BoolOption.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\BufStream.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Cheats.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\ChunkAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\common\Pool.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Data.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\DataFile.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\DataFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\DataLex.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\DataLex.l
# Begin Custom Build - Running flex on $(InputPath)
InputPath=..\utl\DataLex.l

"..\utl\DataLex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -o..\utl\DataLex.cpp $(InputPath)

# End Custom Build
# End Source File
# Begin Source File

SOURCE=..\utl\DataString.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\EmbeddedFile.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\EmbeddedFileTable.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\FileStream.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\FixedSizeAlloc.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\FreeBlock.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Hash.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Heap.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Locale.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\MemMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\MemStats.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\MemTrack.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\MsgSink.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\MsgSource.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Option.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\OptionProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\PoolAlloc.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\PrnStream.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Rand.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Str.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\StringOption.cpp
# End Source File
# Begin Source File

SOURCE=..\utl\Trig.cpp
# End Source File
# End Group
# Begin Group "synth"

# PROP Default_Filter "cpp;h"
# Begin Source File

SOURCE=..\synth\AudioIO.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\BankFileIO.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\BankManager.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\common\Bank.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\common\Instrument.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\LazyFileStream.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\common\Sample.cpp
# End Source File
# Begin Source File

SOURCE=..\synth\common\SampleDescription.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;bmp"
# Begin Source File

SOURCE=.\res\synthedit.ico
# End Source File
# Begin Source File

SOURCE=.\res\SyntheditCtl.bmp
# End Source File
# End Group
# End Target
# End Project
