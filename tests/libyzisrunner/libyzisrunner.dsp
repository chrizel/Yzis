# Microsoft Developer Studio Project File - Name="libyzisrunner" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=libyzisrunner - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libyzisrunner.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libyzisrunner.mak" CFG="libyzisrunner - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libyzisrunner - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "libyzisrunner - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libyzisrunner - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD CPP -MD /W3 /I "." /I "..\tests\cpp" /I ".." /I "..\libyzis" /I "d:\program\Lua" /I "$(QTDIR)\include" /I "D:\work\work\yzis\libyzisrunner" /I "d:\program\qt321nc\mkspecs\win32-msvc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D UNICODE /D YZIS_WIN32_MSVC /D QT_DLL /D QT_THREAD_SUPPORT /D "QT_NO_DEBUG" /FD /c -nologo -Zm200 -GX -GR -O1
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32  "qt-mtnc321.lib"  "qtmain.lib"  "kernel32.lib"  "user32.lib"  "gdi32.lib"  "comdlg32.lib"  "advapi32.lib"  "shell32.lib"  "ole32.lib"  "oleaut32.lib"  "uuid.lib"  "imm32.lib"  "winmm.lib"  "wsock32.lib"  "winspool.lib"  "d:\program\Lua\Lua.lib"  "d:\program\Lua\LuaLib.lib"  "kernel32.lib"  "user32.lib"  "gdi32.lib"  "comdlg32.lib"  "advapi32.lib"  "shell32.lib"  "ole32.lib"  "oleaut32.lib"  "uuid.lib"  "imm32.lib"  "winmm.lib"  "wsock32.lib"  "winspool.lib"  "opengl32.lib"  "glu32.lib"  "delayimp.lib"   /NOLOGO /SUBSYSTEM:console /LIBPATH:"$(QTDIR)\lib" delayimp.lib /DELAYLOAD:comdlg32.dll /DELAYLOAD:oleaut32.dll /DELAYLOAD:winmm.dll /DELAYLOAD:wsock32.dll /DELAYLOAD:winspool.dll


!ELSEIF  "$(CFG)" == "libyzisrunner - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "./"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD CPP -MDd /W3 /Gm /GZ /ZI /Od /I "." /I "..\tests\cpp" /I ".." /I "..\libyzis" /I "d:\program\Lua" /I "$(QTDIR)\include" /I "D:\work\work\yzis\libyzisrunner" /I "d:\program\qt321nc\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D UNICODE /D YZIS_WIN32_MSVC /D QT_DLL /D QT_THREAD_SUPPORT /FD /c -nologo -Zm200 -GX -GR -Zi
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32  "qt-mtnc321.lib"  "qtmain.lib"  "kernel32.lib"  "user32.lib"  "gdi32.lib"  "comdlg32.lib"  "advapi32.lib"  "shell32.lib"  "ole32.lib"  "oleaut32.lib"  "uuid.lib"  "imm32.lib"  "winmm.lib"  "wsock32.lib"  "winspool.lib"  "d:\program\Lua\Lua.lib"  "d:\program\Lua\LuaLib.lib"  "kernel32.lib"  "user32.lib"  "gdi32.lib"  "comdlg32.lib"  "advapi32.lib"  "shell32.lib"  "ole32.lib"  "oleaut32.lib"  "uuid.lib"  "imm32.lib"  "winmm.lib"  "wsock32.lib"  "winspool.lib"  "opengl32.lib"  "glu32.lib"  "delayimp.lib"   /NOLOGO /SUBSYSTEM:console /LIBPATH:"$(QTDIR)\lib" /pdbtype:sept /DEBUG


!ENDIF 

# Begin Target

# Name "libyzisrunner - Win32 Release"
# Name "libyzisrunner - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\tests\cpp\TView.cpp
# End Source File
# Begin Source File

SOURCE=main.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\action.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\attribute.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\events.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\ex_lua.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\internal_options.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\line.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\linesearch.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mapping.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mark.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\option.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\printer.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\qtprinter.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\registers.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\schema.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\search.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\selection.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\session.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\swapfile.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\syntaxdocument.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\syntaxhighlight.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\undo.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\view.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_command.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_ex.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_insert.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_search.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_visual.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecordsearchhistory.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecordstartposition.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecord.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\viewcursor.cpp
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\tests\cpp\TBuffer.h

# End Source File
# Begin Source File

SOURCE=..\tests\cpp\TSession.h

# End Source File
# Begin Source File

SOURCE=..\tests\cpp\TView.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\action.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\attribute.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\buffer.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\cursor.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\debug.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\events.h

USERDEP_="$(QTDIR)\bin\moc.exe"

!IF  "$(CFG)" == "libyzisrunner - Win32 Release"

# Begin Custom Build - Moc'ing ..\libyzis\events.h...
InputPath=.\..\libyzis\events.h

"..\libyzis\moc_events.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\events.h -o ..\libyzis\moc_events.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "libyzisrunner - Win32 Debug"

# Begin Custom Build - Moc'ing ..\libyzis\events.h...
InputPath=.\..\libyzis\events.h

"..\libyzis\moc_events.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\events.h -o ..\libyzis\moc_events.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libyzis\ex_lua.h

USERDEP_="$(QTDIR)\bin\moc.exe"

!IF  "$(CFG)" == "libyzisrunner - Win32 Release"

# Begin Custom Build - Moc'ing ..\libyzis\ex_lua.h...
InputPath=.\..\libyzis\ex_lua.h

"..\libyzis\moc_ex_lua.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\ex_lua.h -o ..\libyzis\moc_ex_lua.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "libyzisrunner - Win32 Debug"

# Begin Custom Build - Moc'ing ..\libyzis\ex_lua.h...
InputPath=.\..\libyzis\ex_lua.h

"..\libyzis\moc_ex_lua.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\ex_lua.h -o ..\libyzis\moc_ex_lua.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libyzis\internal_options.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\line.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\linesearch.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mapping.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mark.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\option.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\printer.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\qtprinter.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\registers.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\schema.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\search.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\selection.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\session.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\swapfile.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\syntaxdocument.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\syntaxhighlight.h

USERDEP_="$(QTDIR)\bin\moc.exe"

!IF  "$(CFG)" == "libyzisrunner - Win32 Release"

# Begin Custom Build - Moc'ing ..\libyzis\syntaxhighlight.h...
InputPath=.\..\libyzis\syntaxhighlight.h

"..\libyzis\moc_syntaxhighlight.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\syntaxhighlight.h -o ..\libyzis\moc_syntaxhighlight.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "libyzisrunner - Win32 Debug"

# Begin Custom Build - Moc'ing ..\libyzis\syntaxhighlight.h...
InputPath=.\..\libyzis\syntaxhighlight.h

"..\libyzis\moc_syntaxhighlight.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc ..\libyzis\syntaxhighlight.h -o ..\libyzis\moc_syntaxhighlight.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libyzis\undo.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_command.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_ex.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_insert.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_search.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\mode_visual.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\view.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\viewcursor.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\portability.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinfo.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecordsearchhistory.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecordstartposition.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\yzisinforecord.h

# End Source File
# Begin Source File

SOURCE=..\libyzis\yzis.h

# End Source File

# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group







# Begin Group "Generated"
# Begin Source File

SOURCE=..\libyzis\moc_events.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\moc_ex_lua.cpp
# End Source File
# Begin Source File

SOURCE=..\libyzis\moc_syntaxhighlight.cpp
# End Source File






# Prop Default_Filter "moc"
# End Group
# End Target
# End Project

