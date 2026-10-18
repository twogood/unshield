Unshield
========

[![Packaging status](https://repology.org/badge/tiny-repos/unshield.svg)](https://repology.org/project/unshield/versions)
[![Homebrew package](https://repology.org/badge/version-for-repo/homebrew/unshield.svg)](https://repology.org/project/unshield/versions)


Support Unshield development
----------------------------

- [PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=SQ7PEFMJK36AU)


Dictionary
----------

InstallShield (IS): see www.installshield.com

InstallShield Cabinet File (ISCF): A .cab file used by IS.

Microsoft Cabinet File (MSCF): A .cab file used by Microsoft.


About Unshield
--------------

To install a Pocket PC application remotely, an installable
Microsoft Cabinet File is copied to the /Windows/AppMgr/Install
directory on the PDA and then the wceload.exe is executed to
perform the actual install. That is a very simple procedure.

Unfortunately, many applications for Pocket PC are distributed as
InstallShield installers for Microsoft Windows, and not as
individual Microsoft Cabinet Files. That is very impractical for
users of other operating systems, such as Linux or FreeBSD.

An installer created by the InstallShield software stores the
files it will install inside of InstallShield Cabinet Files. It
would thus be desirable to be able to extract the Microsoft
Cabinet Files from the InstallShield Cabinet Files in order to be
able to install the applications without access to Microsoft
Windows.

The format of InstallShield Cabinet Files is not officially
documented but there are two tools available for Microsoft
Windows that extracts files from InstallShield installers, and
they are distributed with source code included. These tools are
named "i5comp" and "i6comp" and can be downloaded from the
Internet.

One major drawback with these tools are that for the actual
decompression of the files stored in the InstallShield Cabinet
Files they require the use of code written by InstallShield that
is not available as source code. Luckily, by examining this code
with the 'strings' tool, I discovered that they were using the
open source zlib library (www.gzip.org/zlib) for decompression.

I could have modified i5comp and i6comp to run on other operating
systems than Microsoft Windows, but I preferred to use them as a
reference for this implementation. The goals of this
implementation are:

- Use a well known open source license (MIT)

- Work on both little-endian and big-endian systems

- Separate the implementation in a tool and a library

- Support InstallShield versions 5 and later

- Be able to list contents of InstallShield Cabinet Files

- Be able to extract files from InstallShield Cabinet Files


Usage
-----

List the files in an InstallShield cabinet, or extract them to a directory:

``` sh
unshield l data1.cab
unshield -d extracted x data1.cab
```

See `unshield -h` or the [manual page](man/unshield.1) for more options.

### Troubleshooting extraction failures

If extraction fails, enable debug logging with `-D 3`:

``` sh
unshield -D 3 -d extracted-debug x data1.cab
```

Some older InstallShield cabinets require the old-compression option,
`-O`. If the cabinet uses that format, retry with:

``` sh
unshield -O -d extracted-old x data1.cab
```

Use a new output directory for each attempt. An unsuccessful extraction
can still leave successfully extracted files behind; their presence alone
does not mean that the whole cabinet was extracted. Check the exit status
and any reported errors.

`-O` selects a compression method, not a general repair mode for damaged
or unsupported cabinets. If extraction still fails, include the Unshield
version (`unshield -V`), command used and relevant `-D 3` diagnostics when
reporting the problem. Review logs for private paths or other sensitive
information before sharing them.


License
-------

Unshield uses the MIT license. The short version is "do as you
like, but don't blame me if anything goes wrong".

See the file LICENSE for details.


Build From Source
-----------------

Just use the standard CMake build process:

``` sh
cmake .
make
make install
```
