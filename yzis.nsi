;--------------------------------

Caption "Yzis Installer"
Name "Yzis"
Icon "${NSISDIR}\Contrib\Graphics\Icons\nsis1-install.ico"

; The file to write
OutFile "YzisWindowsInstaller-1.0-alpha1.2.exe"

LicenseText "You must agree to the license before installing."
LicenseData "COPYING"

; The default installation directory
InstallDir $PROGRAMFILES\Yzis\

XPStyle on

;--------------------------------

; install Pages
Page license
Page directory
Page instfiles

; uninstall Pages
UninstPage uninstConfirm
UninstPage instfiles



;--------------------------------
; installer

Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File build\libyzis\libyzis.dll
  File build\qyzis\qyzis.exe
  File c:\qt\4.4.0\bin\mingwm10.DLL
  File c:\qt\4.4.0\bin\QtCore4.dll
  File c:\qt\4.4.0\bin\QtGui4.dll
  File c:\qt\4.4.0\bin\QtXml4.dll
  File C:\usr\local\gnuwin32\bin\libintl3.dll
  File C:\usr\local\gnuwin32\bin\magic1.dll
  File C:\usr\local\gnuwin32\bin\zlib1.dll
  File c:\usr\local\gnuwin32\bin\REGEX2.DLL
  File C:\usr\local\gnuwin32\bin\libiconv2.dll
  File c:\usr\local\lua5.1.2\lua5.1.dll
  WriteUninstaller "yzis-uninst.exe"

  CreateDirectory $SMPROGRAMS\Yzis
  CreateShortCut "$SMPROGRAMS\Yzis\Yzis.lnk" $INSTDIR\qyzis.exe
  CreateShortCut "$SMPROGRAMS\Yzis\Uninstall Yzis.lnk" $INSTDIR\yzis-uninst.exe
#  SetShellVarContext All
#  CreateShortCut "$SMPROGRAMS\Yzis\Yzis.lnk" $INSTDIR\qyzis.exe
#  CreateShortCut "$SMPROGRAMS\Yzis\Uninstall Yzis.lnk" $INSTDIR\yzis-uninst.exe
#  skip:
SectionEnd ; end the section


;--------------------------------
; Uninstaller

UninstallText "This will uninstall Yzis. Hit next to continue."
UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\nsis1-uninstall.ico"
Section "Uninstall"

  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\Yzis\*.*"
  RMDir "$SMPROGRAMS\Yzis"
#  SetShellVarContext All
#  Delete "$SMPROGRAMS\Yzis\*.*"
#  RMDir "$SMPROGRAMS\Yzis"
#  skip2:
SectionEnd
