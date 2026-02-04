rem DahuaMvCamera
rd /s /q DahuaMvCamera\x64
rd /s /q DahuaMvCamera\Debug
rd /s /q DahuaMvCamera\Release

rem HamamatsuCamera
rd /s /q HamamatsuCamera\x64
rd /s /q HamamatsuCamera\Debug
rd /s /q HamamatsuCamera\Release

rem MoticCamera
rd /s /q MoticCamera\x64
rd /s /q MoticCamera\Debug
rd /s /q MoticCamera\Release

rem TucsenCamera
rd /s /q TucsenCamera\x64
rd /s /q TucsenCamera\Debug
rd /s /q TucsenCamera\Release

rem VideoPlugin
rd /s /q VideoPlugin\x64
rd /s /q VideoPlugin\Debug
rd /s /q VideoPlugin\Release

rem VideoTester
rd /s /q VideoTester\x64
rd /s /q VideoTester\Debug
rd /s /q VideoTester\Release

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