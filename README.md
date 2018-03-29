Unshield
========

[![Build Status](https://travis-ci.org/twogood/unshield.png?branch=master)](https://travis-ci.org/twogood/unshield)


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


License
-------

Unshield uses the MIT license. The short version is "do as you
like, but don't blame me if anything goes wrong".

See the file LICENSE for details.



