Some notes on how this repository was created.

Create new mozilla directory.
(with XULRunner env)
cvs checkout -D 2006/07/13 mozilla/client.mk
set MOZ_CO_DATE=2006/07/13
cd mozilla
make -f client.mk checkout
(with SeaMonkey env)
make -f client.mk checkout

(now, test tree)
make -f client.mk build
(assuming all went OK)
del obj-SeaMonkey


Create new repository /Prince2.

Move over and check in CVSMailer files in CVSROOT.
CVSMailer user names are case sensitive!
Fix CVSROOT/cvswrappers.


D:\Prince\moztools>cvs import -I ! -W "*.exe -kb" -W "*.dll -kb" -W "*.lib -kb"
-m "Load initial version." moztools MACKICHAN PRINCE_01

D:\Prince\buildtools>cvs import -I ! -W "*.exe -kb" -W "*.dll -kb" -W "*.lib -kb
" -m "Load initial version." buildtools MACKICHAN PRINCE_01

D:\Mozilla\mozilla>cvs import -I ! -W "*.exe -kb" -W "*.dll -kb" -m "Load Mozill
a 2006/07/13." mozilla MOZILLA MOZ_19a0


D:\Prince>cvs checkout mozilla

windiff

Some files were lost or were different.  A few were changed because of CVS
keyword expansion.


D:\Prince\mozilla>make -f client.mk checkout
(OK, that didn't work because of Mozilla tags in CVS. When I tag our CVS, it fails.)

Edit client.mk to not define NSPR_CO_TAG, NSS_CO_TAG, LDAPCSDK_CO_TAG.

del mozilla
cd D:\Prince
cvs co mozilla/client.mk
cd mozilla
make -f client.mk

(With the SeaMonkey env, this builds SeaMonkey.  With the Prince env, this builds XULRunner.)

Import stub Prince directory.

Modify client.mk, Makefile.in, allmakefiles.sh, configure, configure.in, config/autoconf.mk.in
to deal with Prince directory.


Fix bug 342533 in nsMathMLChar.cpp.

Allow editor to load XHTML files in nsEditingSession.cpp.

Copy Prince/samples directory from old project.


Copy Prince/compute directory from old project.  Get it to compile.



======================================================================

Now, update from Mozilla trunk.

Pull clean tree, combining SeaMonkey and XULRunner.

Import with

D:\Mozilla\mozilla>cvs import -ko -m "update to Mozilla 20060730" mozilla MOZILL
A MOZ_19a0+

(Might want to capture output of this command.  There's also a strange 10 minute
wait for the command to complete.)

This reports:  

7 conflicts created by this import.
Use the following command to help the merge:

        cvs checkout -jMOZILLA:yesterday -jMOZILLA mozilla



D:\Prince>cvs checkout -jMOZILLA:yesterday -jMOZILLA mozilla

(Might want to capture the output of this command.)


Grep for >>> in files.
Sync nsMathMLChar.cpp with trunk.

cvs commit these changes.

d:\Prince\mozilla>cvs update


Should build.  Might want to do a clean build just in case.

FAILED.  nsDocument.cpp won't compile.  argh.



