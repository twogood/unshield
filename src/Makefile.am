
AM_CPPFLAGS = -I../lib $(ICONV_CFLAGS)
LDADD = ../lib/libunshield.la $(ICONV_LIBS)

bin_PROGRAMS = unshield
noinst_PROGRAMS = unshield-deobfuscate

unshield_SOURCES = unshield.c
unshield_deobfuscate_SOURCES = unshield-deobfuscate.c
