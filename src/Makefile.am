AM_CPPFLAGS = -I../lib
LDADD = ../lib/libunshield.la $(ZLIB_LIBS) $(SSL_LIBS)

bin_PROGRAMS = unshield
noinst_PROGRAMS = unshield-deobfuscate

unshield_SOURCES = unshield.c

unshield_deobfuscate_SOURCES = unshield-deobfuscate.c

