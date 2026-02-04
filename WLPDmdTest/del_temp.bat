rem WlpDMDLib
rd /s /q WlpDMDLib\x64
rd /s /q WlpDMDLib\Debug
rd /s /q WlpDMDLib\Release

rem WLPDMDTester
rd /s /q WLPDMDTester\x64
rd /s /q WLPDMDTester\Debug
rd /s /q WLPDMDTester\Release

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