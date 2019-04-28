@echo off
set CUR_PATH=%~dp0
echo %CUR_PATH%

set exeFile=%CUR_PATH%run-epanet3.exe
set inpFile="C:\Users\xuxi\Documents\EPANET Projects\pudong\test_good_2019.inp"
set pressureNodesFile="C:\Users\xuxi\Documents\EPANET Projects\pudong\CYD.txt"
set demandDelta=50
set pressureDelta=0.5
set resultDir=%CUR_PATH%\
echo %resultDir%

echo call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%
call %exeFile% -inpFile %inpFile% -pressureNodesFile %pressureNodesFile% -resultDir "%resultDir%" -demandDelta %demandDelta% -pressureDelta %pressureDelta%

pause
