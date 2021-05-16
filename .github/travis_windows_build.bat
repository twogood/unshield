
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
cmake -G "NMake Makefiles" -DBUILD_STATIC=ON ..
nmake
if %ERRORLEVEL% GEQ 1 EXIT /B 1
