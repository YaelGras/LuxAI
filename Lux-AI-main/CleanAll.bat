
@echo off
if not "%1"=="" (
 set skip=1
 goto skip
)

@echo off
cls

echo You are about to clean up all temporary files in this project... Press enter to continue
pause

cls

:skip

echo Starting Project Cleanup

@echo off
del *.job>NUL
del jobfiles.txt>NUL
del jobfiles7z.txt>NUL

rd build-solution-x64 /Q /S

if exist build-solution-x64 ( 
		echo -- Failed to remove build-solution-x64
		goto fail
	)

	echo -- Cleaned Up build-solution-x64 successfully
	goto end

:success
	echo Project cleaned up successfully
	if %skip% == "" (
	    pause
	)
	goto end

:fail
	echo Failure in cleaning project
	if %skip% == "" (
	    pause
	)
	goto end

:end
