@rem Use this batch file to test network code on a single machine. For example:
@rem Example#1: net games\kjoust3d\kjoust3d.kc     <-- start 2 players
@rem Example#2: net 3 geeky\netsync.kc             <-- start 3 players
@rem NOTE:Make sure 'setactiveapp=1' in EVALDRAW.INI.

@echo off
@if "%1"=="2" goto doloop
@if "%1"=="3" goto doloop
@if "%1"=="4" goto doloop
@if "%1"=="5" goto doloop
@if "%1"=="6" goto doloop
@if "%1"=="7" goto doloop
@if "%1"=="8" goto doloop
@if "%1"=="9" goto doloop

start /i evaldraw /host /qme %*
start /i evaldraw /join /qme %*
@goto endit

:doloop
set tempcounter=%1
start /i evaldraw /host:%tempcounter% /qme %2 %3 %4 %5 %6 %7 %8 %9
:begloop
	start /i evaldraw /join /qme %2 %3 %4 %5 %6 %7 %8 %9
	set /a tempcounter-=1
	if %tempcounter% gtr 1 goto begloop
set tempcounter=

:endit
