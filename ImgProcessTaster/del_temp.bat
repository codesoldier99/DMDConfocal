rem ImgProcessPlug
rd /s /q ImgProcessPlug\x64
rd /s /q ImgProcessPlug\Debug
rd /s /q ImgProcessPlug\Release

rem ImgProcessTaster
rd /s /q ImgProcessTaster\x64
rd /s /q ImgProcessTaster\Debug
rd /s /q ImgProcessTaster\Release

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
