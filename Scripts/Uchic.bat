REM This is an example script that performs a bunch of emmpm segmentations
REM

@echo off
set emmpm=C:\Users\mjackson\Workspace\EMMPMGui\x64\Bin\Release\emmpm.exe
REM *** These are the looping variables
set emiter=5,10,20,40
set mpmiter=1,2,3,4
set beta=0.001,0.01,0.1,1
set gamma=1.0,2.0,3.0,4.0

REM *** These are non-changing arguments to emmpm
set numclasses=3
set simanneal=--simanneal
set graytable=--graytable 0,128,200,255


SETLOCAL EnableDelayedExpansion

REM **** This is the first loop over the EM Iterations
for %%e IN (%emiter%) DO (
echo emLoop %%e
call :mpmLoops %%e
)
goto :eof

REM **** This is the next loop
:mpmLoops
for %%m in (%mpmiter%) DO (
echo -- mpmLoop %%m %1
call :betaLoops %%m %1
)
:break
goto :eof

REM **** 
:betaLoops
for %%b in (%beta%) DO (
echo --- betaLoop %%b %1 %2
call :gammaLoops %%b %1 %2
)
:break
goto :eof

REM ***
:gammaLoops
for %%g in (%gamma%) DO (
%emmpm% --gamma %%g -beta %1 --mpmIter %2 --emIter %3 --outputfile output_G%%g_B%1_E%3_M%2.tif --inputfile myinputimage.tif %graytable%
)
:break


goto :eof
