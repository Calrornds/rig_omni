@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
if not exist tests\host-build mkdir tests\host-build
cl /nologo /std:c++17 /EHsc /I main\protocols /Fotests\host-build\ /Fetests\host-build\tunnel_access_test.exe tests\tunnel_access_test.cc main\protocols\tunnel_access.cc
if errorlevel 1 exit /b 1
tests\host-build\tunnel_access_test.exe
exit /b %ERRORLEVEL%
