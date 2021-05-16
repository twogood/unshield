/*
MIT License
Copyright (c) 2019 win32ports
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef __UNISTD_H_17CD2BD1_839A_4E25_97C7_DE9544B8B59C__
#define __UNISTD_H_17CD2BD1_839A_4E25_97C7_DE9544B8B59C__

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <io.h>       /* _access() */
#include <direct.h>   /* _chdir() */
#include <process.h>  /* _execl() */
#include <sys/stat.h>

#define srandom srand
#define random rand

//#define close _close
/* read, write, and close are NOT being #defined here, because while there are file 
 * handle specific versions for Windows, they probably don't work for sockets.
 * You need to look at your app and consider whether to call e.g. closesocket(). */
#define execve _execve
#define ftruncate _chsize
#define fileno _fileno
#define getcwd _getcwd
#define isatty _isatty
#define lseek _lseek

#define access _access
#define chdir _chdir
#define dup _dup
#define dup2 _dup2
#define execl _execl
#define execle _execle
#define execlp _execlp
#define execp _execp
#define execpe _execpe
#define execpp _execpp
#define rmdir _rmdir
#define unlink _unlink

#ifndef R_OK
#define R_OK 04
#define W_OK 02
#define X_OK R_OK
#define F_OK 00
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

/* permission bits below must be defined in sys/stat.h, but MSVC lacks them */

#ifndef S_IRWXU
#define S_IRWXU 0700
#endif

#ifndef S_IRUSR
#define S_IRUSR 0400
#endif

#ifndef S_IWUSR
#define S_IWUSR 0200
#endif

#ifndef S_IXUSR
#define S_IXUSR 0100
#endif

#ifndef S_IRWXG
#define S_IRWXG 070
#endif

#ifndef S_IRGRP
#define S_IRGRP 040
#endif

#ifndef S_IWGRP
#define S_IWGRP 020
#endif

#ifndef S_IXGRP
#define S_IXGRP 010
#endif

#ifndef S_IRWXO
#define S_IRWXO 07
#endif

#ifndef S_IROTH
#define S_IROTH 04
#endif

#ifndef S_IWOTH
#define S_IWOTH 02
#endif

#ifndef S_IXOTH
#define S_IXOTH 01
#endif

#ifndef S_ISUID
#define S_ISUID 04000
#endif

#ifndef S_ISGID
#define S_ISGID 02000
#endif

#ifndef S_ISVTX
#define S_ISVTX 01000
#endif

#ifndef S_IRWXUGO
#define S_IRWXUGO 0777
#endif

#ifndef S_IALLUGO
#define S_IALLUGO 0777
#endif

#ifndef S_IRUGO
#define S_IRUGO 0444
#endif

#ifndef S_IWUGO
#define S_IWUGO 0222
#endif

#ifndef S_IXUGO
#define S_IXUGO 0111
#endif

#ifndef _S_IFMT
#define _S_IFMT 0xF000
#endif

#ifndef _S_IFIFO
#define _S_IFIFO 0x1000
#endif

#ifndef _S_IFCHR
#define _S_IFCHR 0x2000
#endif

#ifndef _S_IFDIR
#define _S_IFDIR 0x4000
#endif

#ifndef _S_IFBLK
#define _S_IFBLK 0x6000
#endif

#ifndef _S_IFREG
#define _S_IFREG 0x8000
#endif

#ifndef _S_IFLNK
#define _S_IFLNK 0xA000
#endif

#ifndef _S_IFSOCK
#define _S_IFSOCK 0xC000
#endif

#ifndef S_IFMT
#define S_IFMT _S_IFMT
#endif

#ifndef S_IFIFO
#define S_IFIFO _S_IFIFO
#endif

#ifndef S_IFCHR
#define S_IFCHR _S_IFCHR
#endif

#ifndef S_IFDIR
#define S_IFDIR _S_IFDIR
#endif

#ifndef S_IFBLK
#define S_IFBLK _S_IFBLK
#endif

#ifndef S_IFREG
#define S_IFREG _S_IFREG
#endif

#ifndef S_IFLNK
#define S_IFLNK _S_IFLNK
#endif

#ifndef S_IFSOCK
#define S_IFSOCK _S_IFSOCK
#endif

#ifndef S_ISTYPE
#define S_ISTYPE(mode, mask) (((mode) & S_IFMT) == (mask))
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(mode) S_ISTYPE(mode, S_IFIFO)
#endif

#ifndef S_ISCHR
#define S_ISCHR(mode) S_ISTYPE(mode, S_IFCHR)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) S_ISTYPE(mode, S_IFDIR)
#endif

#ifndef S_ISBLK
#define S_ISBLK(mode) S_ISTYPE(mode, S_IFBLK)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) S_ISTYPE(mode, S_IFREG)
#endif

#ifndef S_ISLNK
#define S_ISLNK(mode) S_ISTYPE(mode, S_IFLNK)
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(mode) S_ISTYPE(mode, S_IFSOCK)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* __UNISTD_H_17CD2BD1_839A_4E25_97C7_DE9544B8B59C__ */
