@ECHO OFF
SET OUTDIR=%~dp0build
SET CFLAGS= -Wall -Werror -ggdb3
cmake -G "MinGW Makefiles" -DUSE_OUR_OWN_MD5=ON -DCMAKE_INSTALL_PREFIX:PATH=%OUTDIR% . && mingw32-make && mingw32-make install
