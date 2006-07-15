#!c:\perl\bin\perl
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
# The Original Code is Mozilla Communicator client code, released
# March 31, 1998.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1999
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Sean Su <ssu@netscape.com>
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

#
# Purpose:
#    To build the mozilla self-extracting installer and its corresponding .xpi files
#    given a mozilla build on a local system.
#
# Requirements:
# 1. perl needs to be installed correctly on the build system because cwd.pm is used.
# 2. mozilla\xpinstall\wizard\windows needs to be built.
#    a. to build it, MFC must be installed with VC
#    b. set MOZ_MFC=1 in the build environment
#    c. run nmake -f makefile.win from the above directory
#

use Cwd;
use File::Copy;
use File::Path;
use File::Basename;

$DEPTH         = "../../../..";
$topsrcdir     = GetTopSrcDir();
# ensure that Packager.pm is in @INC, since we might not be called from
# mozilla/xpinstall/packager
push(@INC, "$topsrcdir/../mozilla/xpinstall/packager");
require StageUtils;

ParseArgv(@ARGV);

$topobjdir        = $topsrcdir                   if !defined($topobjdir);
$inStagePath      = "$topobjdir/stage"           if !defined($inStagePath);
$inDistPath       = "$topobjdir/dist"            if !defined($inDistPath);
$inXpiURL         = "ftp://not.supplied.invalid" if !defined($inXpiURL);
$inRedirIniURL    = $inXpiURL                    if !defined($inRedirIniURL);
$cwdBuilder       = "$topsrcdir/xpinstall/wizard/windows/builder";
$gDistInstallPath = "$inDistPath/inst_gre";
$gPackagerPath    = "$topsrcdir/xpinstall/packager";

if(defined($ENV{DEBUG_INSTALLER_BUILD}))
{
  print " build_gre.pl\n";
  print "   topobjdir  : $topobjdir\n";
  print "   inDistPath : $inDistPath\n";
  print "   inStagePath: $inStagePath\n";
}

chdir("$gPackagerPath/win_gre");
if(system("perl makeall.pl -objDir \"$topobjdir\" -stagePath \"$inStagePath\" -distPath \"$inDistPath\" -aurl $inXpiURL -rurl $inRedirIniURL"))
{
  die "\n Error: perl makeall.pl -objDir \"$topobjdir\" -stagePath \"$inStagePath\" -distPath \"$inDistPath\" -aurl $inXpiURL -rurl $inRedirIniURL\n";
}

chdir($cwdBuilder);

# Copy the .xpi files to the same directory as setup.exe.
# This is so that setup.exe can find the .xpi files
# in the same directory and use them.
StageUtils::CopyFiles("$gDistInstallPath/xpi", "$gDistInstallPath");

print "\n";
print "**\n";
print "*\n";
print "*  A self-extracting installer has been built and delivered:\n";
print "*\n";
print "*      $gDistInstallPath/gre-win32-install.exe\n";
print "*\n";
print "**\n";
print "\n";

exit(0);

sub PrintUsage
{
  die "usage: $0 [options]

       options available are:

           -h                - this usage.

           -objDir <path>    - the build directory (defaults to a srcdir build)

           -stagePath <path> - Full path to where the mozilla stage dir is at.
                               Default path, if one is not set, is:
                                 [mozilla]/stage

           -distPath <path>  - Full path to where the mozilla dist dir is at.
                               Default path, if one is not set, is:
                                 [mozilla]/dist

           -aurl <url>       - ftp or http url for where the archives (.xpi, exe, .zip, etx...) are.
                               ie: ftp://my.ftp.com/mysoftware/version1.0/xpi

           -rurl  <url>      - ftp or http url for where the redirect.ini file is located at.
                               ie: ftp://my.ftp.com/mysoftware/version1.0
                   
                               This url can be the same as the archive url.
                               If -rurl is not supplied, it will be assumed that the redirect.ini
                               file is at -arul.

           -DEPTH location of the top of the Mozilla build tree
       \n";
}

sub ParseArgv
{
  my(@myArgv) = @_;
  my($counter);

  for($counter = 0; $counter <= $#myArgv; $counter++)
  {
    if($myArgv[$counter] =~ /^[-,\/]h$/i)
    {
      PrintUsage();
    }
    elsif($myArgv[$counter] =~ /^[-,\/]objDir$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $topobjdir = $myArgv[$counter];
        $topobjdir =~ s/\\/\//g;
      }
    }
    elsif($myArgv[$counter] =~ /^[-,\/]stagePath$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $inStagePath = $myArgv[$counter];
        $inStagePath =~ s/\\/\//g;
      }
    }
    elsif($myArgv[$counter] =~ /^[-,\/]distPath$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $inDistPath = $myArgv[$counter];
        $inDistPath =~ s/\\/\//g;
      }
    }
    elsif($myArgv[$counter] =~ /^[-,\/]aurl$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $inXpiURL = $myArgv[$counter];
        $inRedirIniURL = $inXpiURL;
      }
    }
    elsif($myArgv[$counter] =~ /^[-,\/]rurl$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $inRedirIniURL = $myArgv[$counter];
      }
    }
    elsif($myArgv[$counter] =~ /^[-,\/]depth$/i)
    {
      if($#myArgv >= ($counter + 1))
      {
        ++$counter;
        $DEPTH = $myArgv[$counter];
      }
    }
  }
  PrintUsage() if (!defined($topsrcdir));
  $tmpdir = get_system_cwd();
  chdir("$topsrcdir") || die "$topsrcdir: Directory does not exist!\n";
  $topsrcdir = get_system_cwd();
  chdir("$tmpdir") || die "$tmpdir: Cannot find our way back home\n";
  if (defined($DEPTH)) {
      chdir("$DEPTH") || die "$DEPTH: Directory does not exist!\n";
      $DEPTH = get_system_cwd();
      chdir("$tmpdir") || die "$tmpdir: Cannot find our way back home again\n";
  }
}

sub get_system_cwd {
  my $a = Cwd::getcwd()||`pwd`;
  chomp($a);
  return $a;
}

sub GetVersion
{
  my($depthPath) = @_;
  my($fileMozilla);
  my($fileMozillaVer);
  my($distWinPathName);
  my($monAdjusted);
  my($yy);
  my($mm);
  my($dd);
  my($hh);

  # determine if build was built via gmake
  $distWinPathName = "dist";

  $fileMozilla = "$depthPath/$distWinPathName/bin/mozilla.exe";
  # verify the existance of file
  if(!(-e "$fileMozilla"))
  {
    print "file not found: $fileMozilla\n";
    exit(1);
  }

  ($dev, $ino, $mode, $nlink, $uid, $gid, $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks) = stat $fileMozilla;
  ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime($mtime);

  # calculate year
  # localtime() returns year 2000 as 100, we mod 100 to get at the last 2 digits
  $yy = $year % 100;
  $yy = "20" . sprintf("%.2d", $yy);

  # calculate month
  $monAdjusted = $mon + 1;
  $mm = sprintf("%.2d", $monAdjusted);

  # calculate month day
  $dd = sprintf("%.2d", $mday);

  # calculate day hour
  $hh = sprintf("%.2d", $hour);

  $fileMozillaVer = "$yy$mm$dd$hh";
  print "y2k compliant version string for $fileMozilla: $fileMozillaVer\n";
  return($fileMozillaVer);
}

sub GetTopSrcDir
{
  my($rootDir) = dirname($0) . "/$DEPTH";
  my($savedCwdDir) = cwd();

  chdir($rootDir);
  $rootDir = cwd();
  chdir($savedCwdDir);
  return($rootDir);
}

