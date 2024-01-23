
@echo off

cls

set pauseOpt=nopause

SET bDirVar=build-solution-x64

echo Removing Old Build Directory

rd %bDirVar% /Q /S

mkdir %bDirVar%

cd %bDirVar%

if exist "%CMAKE_PATH%/cmake.exe" (
"%CMAKE_PATH%/cmake" .. -G "Visual Studio 17 2022" -A x64"
goto generated
)

cmake .. -G "Visual Studio 17 2022" -A x64"

:generated

cd ..

if NOT %pauseOpt% == nopause (
    pause
)
