# Microsoft Developer Studio Project File - Name="MultiDesktop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MultiDesktop - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MultiDesktop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MultiDesktop.mak" CFG="MultiDesktop - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MultiDesktop - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MultiDesktop - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MultiDesktop - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FAs /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Ole32.lib kernel32.lib user32.lib advapi32.lib Shell32.lib Netapi32.lib comdlg32.lib ComCtl32.Lib ws2_32.lib Rpcrt4.lib oleaut32.lib Shlwapi.lib /nologo /entry:"main" /subsystem:console /machine:I386 /def:"Multidesktop.def" /OPT:NOWIN98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MultiDesktop - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ole32.lib kernel32.lib user32.lib advapi32.lib Shell32.lib Netapi32.lib comdlg32.lib ComCtl32.Lib ws2_32.lib Rpcrt4.lib oleaut32.lib Shlwapi.lib /nologo /entry:"main" /subsystem:console /debug /machine:I386 /def:"Multidesktop.def" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "MultiDesktop - Win32 Release"
# Name "MultiDesktop - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Bace64.c
# End Source File
# Begin Source File

SOURCE=.\Clipboard.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\crc16.c
# End Source File
# Begin Source File

SOURCE=.\crc32.c
# End Source File
# Begin Source File

SOURCE=.\DesktopList.c
# End Source File
# Begin Source File

SOURCE=.\Edit.c
# End Source File
# Begin Source File

SOURCE=.\Encryption.c
# End Source File
# Begin Source File

SOURCE=.\Files.c
# End Source File
# Begin Source File

SOURCE=.\Format.c
# End Source File
# Begin Source File

SOURCE=.\HashMD5.c
# End Source File
# Begin Source File

SOURCE=.\Html.c
# End Source File
# Begin Source File

SOURCE=.\MainWnd.c
# End Source File
# Begin Source File

SOURCE=.\Menu.c
# End Source File
# Begin Source File

SOURCE=.\MessageLoop.c
# End Source File
# Begin Source File

SOURCE=.\MultiDesktop.c
# End Source File
# Begin Source File

SOURCE=.\MultiDesktop.rc
# End Source File
# Begin Source File

SOURCE=.\NetSend.c
# End Source File
# Begin Source File

SOURCE=.\Network.c
# End Source File
# Begin Source File

SOURCE=.\Path.c
# End Source File
# Begin Source File

SOURCE=.\Rc4EncryptDecrypt.c
# End Source File
# Begin Source File

SOURCE=.\ScanFiles.c
# End Source File
# Begin Source File

SOURCE=.\Shoutcast.c
# End Source File
# Begin Source File

SOURCE=.\Text.c
# End Source File
# Begin Source File

SOURCE=.\Tools.c
# End Source File
# Begin Source File

SOURCE=.\TreeCtrl.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Base64.h
# End Source File
# Begin Source File

SOURCE=.\Clipboard.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\crc16.h
# End Source File
# Begin Source File

SOURCE=.\crc32.h
# End Source File
# Begin Source File

SOURCE=.\DesktopList.h
# End Source File
# Begin Source File

SOURCE=.\Edit.h
# End Source File
# Begin Source File

SOURCE=.\Encryption.h
# End Source File
# Begin Source File

SOURCE=.\Files.h
# End Source File
# Begin Source File

SOURCE=.\Format.h
# End Source File
# Begin Source File

SOURCE=.\HashMD5.h
# End Source File
# Begin Source File

SOURCE=.\Html.h
# End Source File
# Begin Source File

SOURCE=.\MainWnd.h
# End Source File
# Begin Source File

SOURCE=.\Menu.h
# End Source File
# Begin Source File

SOURCE=.\MessageLoop.h
# End Source File
# Begin Source File

SOURCE=.\NetSend.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
# End Source File
# Begin Source File

SOURCE=.\Path.h
# End Source File
# Begin Source File

SOURCE=.\Rc4EncryptDecrypt.h
# End Source File
# Begin Source File

SOURCE=.\ScanFiles.h
# End Source File
# Begin Source File

SOURCE=.\Shoutcast.h
# End Source File
# Begin Source File

SOURCE=.\Text.h
# End Source File
# Begin Source File

SOURCE=.\Tools.h
# End Source File
# Begin Source File

SOURCE=.\TreeCtrl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DESKTOP.ICO
# End Source File
# End Group
# End Target
# End Project
