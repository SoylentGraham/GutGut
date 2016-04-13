# Microsoft Developer Studio Project File - Name="GutGut" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GutGut - Win32 MaxHybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GutGut.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GutGut.mak" CFG="GutGut - Win32 MaxHybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GutGut - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GutGut - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GutGut - Win32 MaxHybrid" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "GutGut"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GutGut - Win32 Release"

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
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GutGut.lib"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GutGut - Win32 Debug"

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
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GutGut.lib" /pdbtype:sept
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GutGut - Win32 MaxHybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GutGut___Win32_MaxHybrid"
# PROP BASE Intermediate_Dir "GutGut___Win32_MaxHybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MaxHybrid"
# PROP Intermediate_Dir "MaxHybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GutGut.lib" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GutGut.lib" /pdbtype:sept
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "d:\3dsmax6\maxsdk\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "GUT_MAXSDK_SUPPORT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"GutGut.lib"
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GutGut - Win32 Release"
# Name "GutGut - Win32 Debug"
# Name "GutGut - Win32 MaxHybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GApp.cpp
# End Source File
# Begin Source File

SOURCE=.\GAsset.cpp
# End Source File
# Begin Source File

SOURCE=.\GAssetList.cpp
# End Source File
# Begin Source File

SOURCE=.\GBinaryData.cpp
# End Source File
# Begin Source File

SOURCE=.\GCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\GCollisionObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\GDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\GDisplayExt.cpp
# End Source File
# Begin Source File

SOURCE=.\GFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GGutFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GInput.cpp
# End Source File
# Begin Source File

SOURCE=.\GKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\GList.cpp
# End Source File
# Begin Source File

SOURCE=.\GListFixed.cpp
# End Source File
# Begin Source File

SOURCE=.\GListStack.cpp
# End Source File
# Begin Source File

SOURCE=.\GListTemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\GMain.cpp
# End Source File
# Begin Source File

SOURCE=.\GMap.cpp
# End Source File
# Begin Source File

SOURCE=.\GMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\GMax.cpp
# End Source File
# Begin Source File

SOURCE=.\GMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\GMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\GMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\GObject.cpp
# End Source File
# Begin Source File

SOURCE=.\GPad.cpp
# End Source File
# Begin Source File

SOURCE=.\GPhysics.cpp
# End Source File
# Begin Source File

SOURCE=.\GQuaternion.cpp
# End Source File
# Begin Source File

SOURCE=.\GShader.cpp
# End Source File
# Begin Source File

SOURCE=.\GSkeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\GSkin.cpp
# End Source File
# Begin Source File

SOURCE=.\GSkyBox.cpp
# End Source File
# Begin Source File

SOURCE=.\GSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GStats.cpp
# End Source File
# Begin Source File

SOURCE=.\GString.cpp
# End Source File
# Begin Source File

SOURCE=..\GuttEd\GText.cpp
# End Source File
# Begin Source File

SOURCE=.\GTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\GTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\GWin32.cpp
# End Source File
# Begin Source File

SOURCE=.\GWinControl.cpp
# End Source File
# Begin Source File

SOURCE=.\GWinDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\GWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\GWinImageList.cpp
# End Source File
# Begin Source File

SOURCE=.\GWinTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\GWorld.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GApp.h
# End Source File
# Begin Source File

SOURCE=.\GAsset.h
# End Source File
# Begin Source File

SOURCE=.\GAssetList.h
# End Source File
# Begin Source File

SOURCE=.\GBinaryData.h
# End Source File
# Begin Source File

SOURCE=.\GCamera.h
# End Source File
# Begin Source File

SOURCE=.\GCollisionObject.h
# End Source File
# Begin Source File

SOURCE=.\GDebug.h
# End Source File
# Begin Source File

SOURCE=.\GDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GDisplayExt.h
# End Source File
# Begin Source File

SOURCE=.\GFile.h
# End Source File
# Begin Source File

SOURCE=.\GFloat3.h
# End Source File
# Begin Source File

SOURCE=.\GGutFile.h
# End Source File
# Begin Source File

SOURCE=.\GInput.h
# End Source File
# Begin Source File

SOURCE=.\GKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\GList.h
# End Source File
# Begin Source File

SOURCE=.\GListFixed.h
# End Source File
# Begin Source File

SOURCE=.\GListStack.h
# End Source File
# Begin Source File

SOURCE=.\GListTemplate.h
# End Source File
# Begin Source File

SOURCE=.\GMain.h
# End Source File
# Begin Source File

SOURCE=.\GMap.h
# End Source File
# Begin Source File

SOURCE=.\GMatrix.h
# End Source File
# Begin Source File

SOURCE=.\GMax.h
# End Source File
# Begin Source File

SOURCE=.\GMenu.h
# End Source File
# Begin Source File

SOURCE=.\GMesh.h
# End Source File
# Begin Source File

SOURCE=.\GMouse.h
# End Source File
# Begin Source File

SOURCE=.\GObject.h
# End Source File
# Begin Source File

SOURCE=.\GPad.h
# End Source File
# Begin Source File

SOURCE=.\GPhysics.h
# End Source File
# Begin Source File

SOURCE=.\GQuaternion.h
# End Source File
# Begin Source File

SOURCE=.\GShader.h
# End Source File
# Begin Source File

SOURCE=.\GSkeleton.h
# End Source File
# Begin Source File

SOURCE=.\GSkin.h
# End Source File
# Begin Source File

SOURCE=.\GSkyBox.h
# End Source File
# Begin Source File

SOURCE=.\GSocket.h
# End Source File
# Begin Source File

SOURCE=.\GStats.h
# End Source File
# Begin Source File

SOURCE=.\GString.h
# End Source File
# Begin Source File

SOURCE=..\GuttEd\GText.h
# End Source File
# Begin Source File

SOURCE=.\GTexture.h
# End Source File
# Begin Source File

SOURCE=.\GTypes.h
# End Source File
# Begin Source File

SOURCE=.\GWin32.h
# End Source File
# Begin Source File

SOURCE=.\GWinControl.h
# End Source File
# Begin Source File

SOURCE=.\GWinDialog.h
# End Source File
# Begin Source File

SOURCE=.\GWindow.h
# End Source File
# Begin Source File

SOURCE=.\GWinImageList.h
# End Source File
# Begin Source File

SOURCE=.\GWinTreeView.h
# End Source File
# Begin Source File

SOURCE=.\GWorld.h
# End Source File
# End Group
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GutSkinningNV.vp
# End Source File
# End Group
# End Target
# End Project
