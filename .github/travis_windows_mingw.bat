
mkdir build
cd build
cmake -G "MinGW Makefiles" -DBUILD_STATIC=ON ..
mingw32-make
if %ERRORLEVEL% GEQ 1 EXIT /B 1
