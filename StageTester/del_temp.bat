rem IHdsStage
rd /s /q HdsStage\x64
rd /s /q HdsStage\Debug
rd /s /q HdsStage\Release

rem MoticStage
rd /s /q MoticStage\x64
rd /s /q MoticStage\Debug
rd /s /q MoticStage\Release

rem StageTester
rd /s /q StageTester\x64
rd /s /q StageTester\Debug
rd /s /q StageTester\Release

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