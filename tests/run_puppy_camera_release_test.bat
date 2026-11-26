@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
if not exist tests\host-build mkdir tests\host-build
copy /Y main\boards\common\esp32_camera.cc tests\host-build\esp32_camera.cc
if errorlevel 1 exit /b 1
set RC=0
cl /nologo /std:c++20 /EHsc /I tests\cam_stubs /I main\boards\common /Fotests\host-build\ /Fetests\host-build\puppy_camera_release_test.exe tests\puppy_camera_release_test.cc tests\host-build\esp32_camera.cc
if errorlevel 1 exit /b 1
echo --- puppy ---
call :run
if errorlevel 1 set RC=1
cl /nologo /std:c++20 /EHsc /DCAM_HOST_BOARD_NON_PUPPY /I tests\cam_stubs /I main\boards\common /Fotests\host-build\ /Fetests\host-build\puppy_camera_release_test.exe tests\puppy_camera_release_test.cc tests\host-build\esp32_camera.cc
if errorlevel 1 exit /b 1
echo --- non-puppy ---
call :run
if errorlevel 1 set RC=1
exit /b %RC%

:run
tests\host-build\puppy_camera_release_test.exe
if not errorlevel 1 exit /b 0
tests\host-build\puppy_camera_release_test.exe
if not errorlevel 1 exit /b 0
tests\host-build\puppy_camera_release_test.exe
exit /b %ERRORLEVEL%
