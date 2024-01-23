@echo off

rem =================================================>
rem Update Configs Here

rem example:
rem set TEAM=LesOutardes

set TEAM=Unan
rem <=================================================

python GenSubmit.py %TEAM%
