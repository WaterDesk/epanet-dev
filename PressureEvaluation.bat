@echo off
set CUR_PATH=%~dp0

set inpFile="C:\Users\xuxi\Documents\EPANET Projects\pudong\test_good_2019.inp"
set pressureNodesFile="C:\Users\xuxi\Documents\EPANET Projects\pudong\pressureNodes.txt"

set demandDelta=50
set pressureDelta=0.5

if not exist %inpFile% (
    echo %inpFile% doesn't not exist
    pause
    exit
)

if not exist %pressureNodesFile% (
    echo %pressureNodesFile% doesn't not exist
    pause
    exit
)

REM testNodesFile is optional. If null, all nodes will be considered to evaluate the pressure nodes
REM set testNodesFile="C:\Users\xuxi\Documents\EPANET Projects\pudong\testNodes.txt"
REM if not exist %testNodesFile% (
REM     echo %testNodesFile% doesn't not exist
REM     pause
REM     exit
REM )


set exeFile=%CUR_PATH%run-epanet3_evaluation.exe
set resultDir=%CUR_PATH%\

REM no testNodesFile
echo call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%
call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%

REM have testNodesFile
REM echo call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -testNodesFile %testNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%
REM call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -testNodesFile %testNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%

pause
