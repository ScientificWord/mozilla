#!perl
#
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is mozilla.org code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1999
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Samir Gehani <sgehani@netscape.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either of the GNU General Public License Version 2 or later (the "GPL"),
# or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

#==============================================================================
# usage: perl deliver.pl version URLPath stubName blobName buildWizard appName appDisplayName
# e.g.   perl deliver.pl 5.0.0.1 ftp://foo/ mozilla-installer mozilla-installer mozilla Mozilla
#
# Delivers the stub and blob installers to mozilla/installer/stub 
# and mozilla/installer/sea, respectively.  Also, delivers the .xpis
# to mozilla/installer/raw/xpi.
#
# NOTE:
# -----
#   * all args are optional 
#   * version is used by xpinstall and needs to bumped every build
#   * URLPath must have a trailing slash
#   * if you are not building a release version no need to pass any args
#   * pass in "buildwizard" as the last arg to build the wizard too
#   * you must be in deliver.pl's dir when calling it since it assumes DEPTH
#==============================================================================

use Cwd;

#// constants
$_DEPTH  = "../../..";
$_orig = cwd();
chdir($_DEPTH); # resolve absolute path
$TREETOP = cwd();
chdir($_orig);

$WIZARD = $TREETOP."/xpinstall/wizard/unix/src2";
$ROOT   = $TREETOP."/installer";
$STAGE  = $ROOT."/stage";
$RAW    = $ROOT."/raw";
$XPI    = $RAW."/xpi";
$BLOB   = $ROOT."/sea";
$STUB   = $ROOT."/stub";

#// default args
$aVersion = "5.0.0.0";
$aURLPath = "ftp://ftp.mozilla.org/";
$aStubName = "mozilla-installer";
$aBlobName = "mozilla-installer";
$aBuildWizard = "NO";
$aMozAppName = "mozilla";
$aMozAppDisplayName = "Mozilla";

#// parse args
# all optional args: version, URLPath, stubName, blobName, mozAppName,
# mozAppDisplayName
if ($#ARGV >= 6) { $aMozAppDisplayName = $ARGV[6]; }
if ($#ARGV >= 5) { $aMozAppName        = $ARGV[5]; }
if ($#ARGV >= 4) { $aBuildWizard       = $ARGV[4]; }
if ($#ARGV >= 3) { $aBlobName          = $ARGV[3]; }
if ($#ARGV >= 2) { $aStubName          = $ARGV[2]; }
if ($#ARGV >= 1) { $aURLPath           = $ARGV[1]; }
if ($#ARGV >= 0) { $aVersion           = $ARGV[0]; }

$SUBDIR = "$aMozAppName-installer";

#// create dist structure ($ROOT/{stage,raw,stub,sea})
if (-e $ROOT)
{
    if (-w $ROOT) 
        { system("rm -rf $ROOT"); }
    else 
        { die "--- deliver.pl: check perms on $ROOT: $!"; }
}

mkdir($ROOT, 0777)  || die "--- deliver.pl: couldn't mkdir root: $!";
mkdir($STAGE, 0777) || die "--- deliver.pl: couldn't mkdir stage: $!";
mkdir($RAW, 0777)   || die "--- deliver.pl: couldn't mkdir raw: $!";
mkdir($XPI, 0777)   || die "--- deliver.pl: couldn't mkdir xpi: $!";
mkdir($BLOB, 0777)  || die "--- deliver.pl: couldn't mkdir sea: $!";
mkdir($STUB, 0777)  || die "--- deliver.pl: couldn't mkdir stub: $!";


#-------------------------------------------------------------------------
#   Deliver wizard
#-------------------------------------------------------------------------
#// build the wizard 
if ($aBuildWizard eq "buildwizard")
{
    chdir($WIZARD);
    system($TREETOP."/build/autoconf/update-makefile.sh");

    #// make unix wizard
    system("make");
    chdir($_orig);
}

#// deliver wizard to staging area (mozilla/installer/stage)
copy("$WIZARD/mozilla-installer", "$RAW/$aMozAppName-installer");
copy("$WIZARD/mozilla-installer-bin", "$RAW/$aMozAppName-installer-bin");
copy("$WIZARD/installer.ini", $RAW);
copy("$WIZARD/README", $RAW);
copy("$WIZARD/MPL-1.1.txt", $RAW);
chmod(0755, "$RAW/$aMozAppName-installer"); #// ensure shell script is executable

spew("Completed delivering wizard");


#-------------------------------------------------------------------------
#   Make .xpis
#-------------------------------------------------------------------------
#// call pkgcp.pl
chdir("$TREETOP/xpinstall/packager");
system("perl pkgcp.pl -o unix -s $TREETOP/dist -d $STAGE -f $TREETOP/xpinstall/packager/packages-static-unix -v");
spew("Completed copying build files for STATIC BUILD");

#// call xptlink.pl to make big .xpt files/component
system("perl xptlink.pl -s $TREETOP/dist -d $STAGE -v");
spew("Completed xptlinking"); 

#// call makeall.pl tunneling args (delivers .xpis to mozilla/installer/stage)
chdir("$TREETOP/xpinstall/packager/unix");
system("perl makeall.pl $aVersion $aURLPath $STAGE $XPI $aMozAppName $aMozAppDisplayName");
system("mv $TREETOP/xpinstall/packager/unix/config.ini $RAW");
spew("Completed making .xpis");


#-------------------------------------------------------------------------
#   Package stub and sea
#-------------------------------------------------------------------------
#// tar and gzip mozilla-installer, mozilla-installer-bin, README, license, 
#// config.ini, installer.ini into stub
spew("Creating stub installer tarball...");
chdir("$RAW/..");
system("mv $RAW $ROOT/$SUBDIR");
system("tar cvf $STUB/$aStubName.tar ./$SUBDIR/$aMozAppName-installer ./$SUBDIR/$aMozAppName-installer-bin ./$SUBDIR/installer.ini ./$SUBDIR/README ./$SUBDIR/config.ini ./$SUBDIR/MPL-1.1.txt"); 
system("mv $ROOT/$SUBDIR $RAW");
system("bzip2 $STUB/$aStubName.tar");
spew("Completed creating stub installer tarball");

#// tar and gzip mozilla-installer, mozilla-installer-bin, README, license, 
#// config.ini, installer.ini and .xpis into sea
spew("Creating blob (aka full or sea) installer tarball...");
system("mv $RAW $ROOT/$SUBDIR");
system("tar cvf $BLOB/$aBlobName.tar ./$SUBDIR/"); 
system("mv $ROOT/$SUBDIR $RAW");
system("bzip2 $BLOB/$aBlobName.tar");
spew("Completed creating blob (aka full or sea) installer tarball");
chdir($_orig);

spew("Completed packaging stub and sea");
spew("Installers built (see $ROOT/{stub,sea})");


#-------------------------------------------------------------------------
#   Utilities
#-------------------------------------------------------------------------
sub spew 
{
    print "+++ deliver.pl: ".$_[0]."\n";
}

sub copy
{
    if (! -e $_[0])
    {
        die "--- deliver.pl: couldn't cp cause ".$_[0]." doesn't exist: $!";
    }
    system ("cp ".$_[0]." ".$_[1]);
}
