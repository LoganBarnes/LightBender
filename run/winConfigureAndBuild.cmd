@ECHO OFF

rem run this from the Developer Command Prompt for VS since
rem the MSBuild.exe path will already be set

SET RUN_DIR="%CD%"

IF EXIST _build GOTO SKIPBUILDDIR
mkdir _build
:SKIPBUILDDIR

cd _build
cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_INSTALL_PREFIX=%RUN_DIR% ..\..
cmake --build . --target INSTALL --config Release -- /m
cd ..
