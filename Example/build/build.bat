@echo off

cd /d "%~dp0"
ninja build
mkdir Dist
python %STAPEL_SDK%\scripts\dist.py -o Dist
copy game.dll Dist\bin\game.dll
cd Dist