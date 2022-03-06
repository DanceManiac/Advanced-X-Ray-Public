# Microsoft Developer Studio Project File - Name="peerps2prodg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=peerps2prodg - Win32 Release_Insock
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "peerps2prodg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "peerps2prodg.mak" CFG="peerps2prodg - Win32 Release_Insock"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "peerps2prodg - Win32 Debug_EENet" (based on "Win32 (x86) Console Application")
!MESSAGE "peerps2prodg - Win32 Debug_Insock" (based on "Win32 (x86) Console Application")
!MESSAGE "peerps2prodg - Win32 Debug_SNSystems" (based on "Win32 (x86) Console Application")
!MESSAGE "peerps2prodg - Win32 Release_EENet" (based on "Win32 (x86) Console Application")
!MESSAGE "peerps2prodg - Win32 Release_Insock" (based on "Win32 (x86) Console Application")
!MESSAGE "peerps2prodg - Win32 Release_SNSystems" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Gamespy/GOA/Peer/peerc/peerps2prodg", NTEDAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "peerps2prodg - Win32 Debug_EENet"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_EENet"
# PROP BASE Intermediate_Dir "Debug_EENet"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_EENet"
# PROP Intermediate_Dir "Debug_EENet"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD /debug /c
# ADD CPP /nologo /W4 /WX /Od /I "c:\usr\local\sce\ee\include\libeenet" /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "EENET" /D "_DEBUG" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /debug /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /debug /machine:IX86 /out:"PS2_EE_Debug\peerps2prodg.elf" /D:SN_TARGET_PS2
# ADD LINK32 libc.a eenetctl.a ent_smap.a ent_eth.a ent_ppp.a libeenet.a libscf.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /debug /machine:IX86 /nodefaultlib /out:"Debug_EENet\peerps2prodg.elf" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "peerps2prodg - Win32 Debug_Insock"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "peerps2prodg___Win32_Debug_Insock"
# PROP BASE Intermediate_Dir "peerps2prodg___Win32_Debug_Insock"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Insock"
# PROP Intermediate_Dir "Debug_Insock"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "_DEBUG" /D "SN_SYSTEMS" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /debug /c
# ADD CPP /nologo /W4 /WX /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "_DEBUG" /D "SN_TARGET_PS2" /D "INSOCK" /D "UNIQUEID" /FD /debug /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 libc.a sneetcp.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /debug /machine:IX86 /nodefaultlib /out:"Debug_SNSystems\peerps2prodg.elf" /D:SN_TARGET_PS2 -fshort-wchar
# ADD LINK32 libinsck.a libnet.a libmrpc.a libkernl.a libcdvd.a libnetif.a netcnfif.a /nologo /pdb:none /debug /machine:IX86 /nodefaultlib /out:"Debug_Insock\peerps2prodg.elf" /D:SN_TARGET_PS2 -fshort-wchar

!ELSEIF  "$(CFG)" == "peerps2prodg - Win32 Debug_SNSystems"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_SNSystems"
# PROP BASE Intermediate_Dir "Debug_SNSystems"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_SNSystems"
# PROP Intermediate_Dir "Debug_SNSystems"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD /debug /c
# ADD CPP /nologo /W4 /WX /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "_DEBUG" /D "SN_SYSTEMS" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /debug /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /debug /machine:IX86 /out:"PS2_EE_Debug\peerps2prodg.elf" /D:SN_TARGET_PS2
# ADD LINK32 libc.a sneetcp.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /debug /machine:IX86 /nodefaultlib /out:"Debug_SNSystems\peerps2prodg.elf" /D:SN_TARGET_PS2 -fshort-wchar

!ELSEIF  "$(CFG)" == "peerps2prodg - Win32 Release_EENet"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Release_EENet"
# PROP BASE Intermediate_Dir "Release_EENet"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release_EENet"
# PROP Intermediate_Dir "Release_EENet"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /O2 /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_TARGET_PS2" /Fo"PS2_EE_Release/" /FD /c
# ADD CPP /nologo /W4 /WX /Od /I "c:\usr\local\sce\ee\include\libeenet" /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "EENET" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /machine:IX86 /out:"PS2_EE_Release\peerps2prodg.elf" /D:SN_TARGET_PS2
# ADD LINK32 eenetctl.a ent_smap.a ent_eth.a ent_ppp.a libeenet.a libscf.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /machine:IX86 /out:"Release_EENet\peerps2prodg.elf" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "peerps2prodg - Win32 Release_Insock"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "peerps2prodg___Win32_Release_Insock"
# PROP BASE Intermediate_Dir "peerps2prodg___Win32_Release_Insock"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release_Insock"
# PROP Intermediate_Dir "Release_Insock"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /WX /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_SYSTEMS" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /c
# ADD CPP /nologo /W4 /WX /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_TARGET_PS2" /D "INSOCK" /D "UNIQUEID" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 sneetcp.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /machine:IX86 /out:"Release_SNSystems\peerps2prodg.elf" /D:SN_TARGET_PS2
# ADD LINK32 libinsck.a libnet.a libmrpc.a libkernl.a libcdvd.a libnetif.a netcnfif.a /nologo /pdb:none /machine:IX86 /out:"Release_Insock\peerps2prodg.elf" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "peerps2prodg - Win32 Release_SNSystems"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Release_SNSystems"
# PROP BASE Intermediate_Dir "Release_SNSystems"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release_SNSystems"
# PROP Intermediate_Dir "Release_SNSystems"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /O2 /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_TARGET_PS2" /Fo"PS2_EE_Release/" /FD /c
# ADD CPP /nologo /W4 /WX /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /D "SN_SYSTEMS" /D "SN_TARGET_PS2" /D "UNIQUEID" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=snBsc.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /machine:IX86 /out:"PS2_EE_Release\peerps2prodg.elf" /D:SN_TARGET_PS2
# ADD LINK32 sneetcp.a libcdvd.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a /nologo /pdb:none /machine:IX86 /out:"Release_SNSystems\peerps2prodg.elf" /D:SN_TARGET_PS2

!ENDIF 

# Begin Target

# Name "peerps2prodg - Win32 Debug_EENet"
# Name "peerps2prodg - Win32 Debug_Insock"
# Name "peerps2prodg - Win32 Debug_SNSystems"
# Name "peerps2prodg - Win32 Release_EENet"
# Name "peerps2prodg - Win32 Release_Insock"
# Name "peerps2prodg - Win32 Release_SNSystems"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\peerc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ps2\ps2common.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\common\ps2\prodg\PS2_in_VC.h
# End Source File
# End Group
# Begin Group "Peer"

# PROP Default_Filter ""
# Begin Group "ServerBrowsingSDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_crypt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_internal.h
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_queryengine.c
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_server.c
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_serverbrowsing.c
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_serverbrowsing.h
# End Source File
# Begin Source File

SOURCE=..\..\..\serverbrowsing\sb_serverlist.c
# End Source File
# End Group
# Begin Group "QueryReportingSDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\qr2\qr2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\qr2\qr2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\qr2\qr2regkeys.c
# End Source File
# Begin Source File

SOURCE=..\..\..\qr2\qr2regkeys.h
# End Source File
# End Group
# Begin Group "ChatSDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Chat\chat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatCallbacks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatChannel.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatChannel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatCrypt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatCrypt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatHandlers.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatHandlers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatMain.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatMain.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatSocket.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Chat\chatSocket.h
# End Source File
# End Group
# Begin Group "Pinger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\pinger\pinger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pinger\pingerMain.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\peer.h
# End Source File
# Begin Source File

SOURCE=..\..\peerAutoMatch.c
# End Source File
# Begin Source File

SOURCE=..\..\peerAutoMatch.h
# End Source File
# Begin Source File

SOURCE=..\..\peerCallbacks.c
# End Source File
# Begin Source File

SOURCE=..\..\peerCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\..\peerGlobalCallbacks.c
# End Source File
# Begin Source File

SOURCE=..\..\peerGlobalCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\..\peerHost.c
# End Source File
# Begin Source File

SOURCE=..\..\peerHost.h
# End Source File
# Begin Source File

SOURCE=..\..\peerKeys.c
# End Source File
# Begin Source File

SOURCE=..\..\peerKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\peerMain.c
# End Source File
# Begin Source File

SOURCE=..\..\peerMain.h
# End Source File
# Begin Source File

SOURCE=..\..\peerMangle.c
# End Source File
# Begin Source File

SOURCE=..\..\peerMangle.h
# End Source File
# Begin Source File

SOURCE=..\..\peerOperations.c
# End Source File
# Begin Source File

SOURCE=..\..\peerOperations.h
# End Source File
# Begin Source File

SOURCE=..\..\peerPing.c
# End Source File
# Begin Source File

SOURCE=..\..\peerPing.h
# End Source File
# Begin Source File

SOURCE=..\..\peerPlayers.c
# End Source File
# Begin Source File

SOURCE=..\..\peerPlayers.h
# End Source File
# Begin Source File

SOURCE=..\..\peerQR.c
# End Source File
# Begin Source File

SOURCE=..\..\peerQR.h
# End Source File
# Begin Source File

SOURCE=..\..\peerRooms.c
# End Source File
# Begin Source File

SOURCE=..\..\peerRooms.h
# End Source File
# Begin Source File

SOURCE=..\..\peerSB.c
# End Source File
# Begin Source File

SOURCE=..\..\peerSB.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\darray.c
# End Source File
# Begin Source File

SOURCE=..\..\..\darray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsAssert.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsAssert.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsAvailable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsAvailable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsMemory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsMemory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatform.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatform.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformSocket.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformThread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsPlatformUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsStringUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\gsStringUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\hashtable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\md5c.c
# End Source File
# End Group
# Begin Source File

SOURCE=c:\usr\local\sce\ee\lib\app.cmd
# End Source File
# Begin Source File

SOURCE=c:\usr\local\sce\ee\lib\crt0.s
# End Source File
# Begin Source File

SOURCE=..\..\..\ps2common\prodg\ps2.lk
# End Source File
# End Target
# End Project
