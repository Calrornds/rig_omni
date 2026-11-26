@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
if not exist tests\host-build mkdir tests\host-build
copy /Y main\boards\common\esp32_camera.cc tests\host-build\esp32_camera.cc
if errorlevel 1 exit /b 1
cl /nologo /std:c++20 /EHsc /DPUPPY_CAMERA_RELEASE_TEST /I tests\cam_stubs /I main\boards\common /Fotests\host-build\ /Fetests\host-build\puppy_camera_release_test.exe tests\puppy_camera_release_test.cc tests\host-build\esp32_camera.cc
if errorlevel 1 exit /b 1
tests\host-build\puppy_camera_release_test.exe
exit /b %ERRORLEVEL%
