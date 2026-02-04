rem ConfocalCore
rd /s /q ConfocalCore\x64
rd /s /q ConfocalCore\Debug
rd /s /q ConfocalCore\Release

rem ConfocalScannerLib
rd /s /q ConfocalScannerLib\x64
rd /s /q ConfocalScannerLib\Debug
rd /s /q ConfocalScannerLib\Release

rem ConfocalUILib
rd /s /q ConfocalUILib\x64
rd /s /q ConfocalUILib\Debug
rd /s /q ConfocalUILib\Release

rem DMDConfocal
rd /s /q DMDConfocal\x64
rd /s /q DMDConfocal\Debug
rd /s /q DMDConfocal\Release

rem File
rd /s /q #Output
rd /s /q Debug
rd /s /q Release
rd /s /q x64
rd /s /q ipch
rd /s /q .vs
rd /s /q temp
del /s /q *.sdf
del /s /q *.VC.db

TIMEOUT /T 1
