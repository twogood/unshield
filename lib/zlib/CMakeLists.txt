set(ZLIB_HEADERS
	"crc32.h"
#	"deflate.h"
	"inffast.h"
	"inflate.h"
	"inffixed.h"
	"inftrees.h"
#	"trees.h"
	"zconf.h"
	"zlib.h"
	"zutil.h"
)

set(ZLIB_SOURCES
	"adler32.c"
	"crc32.c"
#	"deflate.c"
	"infback.c"
	"inffast.c"
	"inflate.c"
	"inftrees.c"
#	"trees.c"
	"zutil.c"
)

if(BUILD_STATIC AND MSVC)
    set_msvc_runtime_static()
endif()

add_library(zlib STATIC ${ZLIB_HEADERS} ${ZLIB_SOURCES})

# Linux/Clang-7
# relocation R_X86_64_32S against `.rodata' can not be used when making a shared object;
# recompile with -fPIC
include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-fPIC" COMPILER_HAS_FPIC)
if (COMPILER_HAS_FPIC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()
