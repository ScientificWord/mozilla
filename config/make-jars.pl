#!/perl

# make-jars [-f] [-v] [-l] [-x] [-a] [-e] [-d <chromeDir>] [-s <srcdir>] [-t <topsrcdir>] [-c <localedir>] [-j <jarDir>] [-z zipprog] [-o operating-system] < <jar.mn>

my $cygwin_mountprefix = "";
if ($^O eq "cygwin") {
    $cygwin_mountprefix = $ENV{CYGDRIVE_MOUNT};
    if ($cygwin_mountprefix eq "") {
      $cygwin_mountprefix = `mount -p | awk '{ if (/^\\//) { print \$1; exit } }'`;
      if ($cygwin_mountprefix eq "") {
        print "Cannot determine cygwin mount points. Exiting.\n";
        exit(1);
      }
    }
    chomp($cygwin_mountprefix);
    # Remove extra ^M caused by using dos-mode line-endings
    chop $cygwin_mountprefix if (substr($cygwin_mountprefix, -1, 1) eq "\r");
} else {
    # we'll be pulling in some stuff from the script directory
    require FindBin;
    import FindBin;
    push @INC, $FindBin::Bin;
}

use strict;

use Getopt::Std;
use Cwd;
use File::stat;
use Time::localtime;
use File::Copy;
use File::Path;
use File::Spec;
use File::Basename;
use IO::File;
use Config;
require mozLock;
import mozLock;

my $objdir = getcwd;

# if there's a "--", everything after it goes into $defines.  We don't do
# this with the remaining args in @ARGV after the getopts call because
# old versions of Getopt::Std don't understand "--".
my $ddindex = 0;
foreach my $arg (@ARGV) {
  ++$ddindex;
  last if ($arg eq "--");
}
my $defines = join(' ', @ARGV[ $ddindex .. $#ARGV ]);

getopts("d:s:t:c:j:f:avlD:o:p:xz:e");

my $baseFilesDir = ".";
if (defined($::opt_s)) {
    $baseFilesDir = $::opt_s;
}

my $topSrcDir;
if (defined($::opt_t)) {
    $topSrcDir = $::opt_t;
}

my $localeDir;
if (defined($::opt_c)) {
    $localeDir = $::opt_c;
}

my $maxCmdline = 4000;
if ($Config{'archname'} =~ /VMS/) {
    $maxCmdline = 200;
}

my $chromeDir = ".";
if (defined($::opt_d)) {
    $chromeDir = $::opt_d;
}

my $jarDir = $chromeDir;
if (defined($::opt_j)) {
    $jarDir = $::opt_j;
}

my $verbose = 0;
if (defined($::opt_v)) {
    $verbose = 1;
}

my $fileformat = "jar";
if (defined($::opt_f)) {
    ($fileformat = $::opt_f) =~ tr/A-Z/a-z/;
}

if ("$fileformat" ne "jar" &&
    "$fileformat" ne "flat" &&
    "$fileformat" ne "symlink" &&
    "$fileformat" ne "both") {
    print "File format specified by -f option must be one of: jar, flat, both, or symlink.\n";
    exit(1);
}

my $zipmoveopt = "";
if ("$fileformat" eq "jar") {
    $zipmoveopt = "-m -0";
}
if ("$fileformat" eq "both") {
    $zipmoveopt = "-0";
}

my $nofilelocks = 0;
if (defined($::opt_l)) {
    $nofilelocks = 1;
}

my $autoreg = 1;
if (defined($::opt_a)) {
    $autoreg = 0;
}

my $useExtensionManifest = 0;
if (defined($::opt_e)) {
    $useExtensionManifest = 1;
}

my $preprocessor = "";
if (defined($::opt_p)) {
    $preprocessor = $::opt_p;
}

my $force_x11 = 0;
if (defined($::opt_x)) {
    $force_x11 = 1;
}

my $zipprog = $ENV{ZIP};
if (defined($::opt_z)) {
    $zipprog = $::opt_z;
}

if ($zipprog eq "") {
    print "A valid zip program must be given via the -z option or the ZIP environment variable. Exiting.\n";
    exit(1);
}

my $force_os;
if (defined($::opt_o)) {
    $force_os = $::opt_o;
}

if ($verbose) {
    print "make-jars "
        . "-v -d $chromeDir "
        . "-j $jarDir "
        . "-z $zipprog "
        . ($fileformat ? "-f $fileformat " : "")
        . ($nofilelocks ? "-l " : "")
        . ($baseFilesDir ? "-s $baseFilesDir " : "")
        . "\n";
}

my $win32 = ($^O =~ /((MS)?win32)|msys|cygwin|os2/i) ? 1 : 0;
my $macos = ($^O =~ /MacOS|darwin/i) ? 1 : 0;
my $unix  = !($win32 || $macos) ? 1 : 0;
my $vms   = ($^O =~ /VMS/i) ? 1 : 0;

if ($force_x11) {
    $win32 = 0;
    $macos = 0;
    $unix = 1;
}

if (defined($force_os)) {
    $win32 = 0;
    $macos = 0;
    $unix = 0;
    if ($force_os eq "WINNT") {
    $win32 = 1;
    } elsif ($force_os eq "OS2") {
    $win32 = 1;
    } elsif ($force_os eq "Darwin") {
    $macos = 1;
    } else {
    $unix = 1;
    }
}

sub foreignPlatformFile
{
   my ($jarfile) = @_;
   
   if (!$win32 && index($jarfile, "-win") != -1) {
     return 1;
   }
   
   if (!$unix && index($jarfile, "-unix") != -1) {
     return 1; 
   }

   if (!$macos && index($jarfile, "-mac") != -1) {
     return 1;
   }

   return 0;
}

sub foreignPlatformPath
{
   my ($jarpath) = @_;
   
   if (!$win32 && index($jarpath, "-platform/win") != -1) {
     return 1;
   }
   
   if (!$unix && index($jarpath, "-platform/unix") != -1) {
     return 1; 
   }

   if (!$macos && index($jarpath, "-platform/mac") != -1) {
     return 1;
   }

   return 0;
}

sub zipErrorCheck($$)
{
    my ($err,$lockfile) = @_;
    return if ($err == 0 || $err == 12);
    mozUnlock($lockfile) if (!$nofilelocks);
    die ("Error invoking zip: $err");
}

sub JarIt
{
    my ($destPath, $jarPath, $jarfile, $args, $overrides) = @_;
    my $oldDir = cwd();
    my $jarchive = _moz_abs2rel("$jarPath/$jarfile.jar", "$destPath/$jarfile", 1);
    chdir("$destPath/$jarfile");

    if ("$fileformat" eq "flat" || "$fileformat" eq "symlink") {
        unlink($jarchive) if ( -e $jarchive);
        chdir($oldDir);
        return 0;
    }

    #print "cd $destPath/$jarfile\n";
    my $argOpt = "-X";
    $argOpt = "-uX" if ( -e $jarchive);

    my $lockfile = "../$jarfile.lck";

    mozLock($lockfile) if (!$nofilelocks);

    if (!($args eq "")) {
        my $err = 0; 

        #print "$zipprog $zipmoveopt -uX $jarchive $args\n";

        # Handle posix cmdline limits
        while (length($args) > $maxCmdline) {
            #print "Exceeding POSIX cmdline limit: " . length($args) . "\n";
            my $subargs = substr($args, 0, $maxCmdline-1);
            my $pos = rindex($subargs, " ");
            $subargs = substr($args, 0, $pos);
            $args = substr($args, $pos);

            #print "$zipprog $zipmoveopt -uX $jarchive $subargs\n";
            #print "Length of subargs: " . length($subargs) . "\n";
            system("$zipprog $zipmoveopt $argOpt $jarchive $subargs") == 0 or
                $err = $? >> 8;
            zipErrorCheck($err,$lockfile);
        }
        #print "Length of args: " . length($args) . "\n";
        #print "$zipprog $zipmoveopt -uX $jarchive $args\n";
        system("$zipprog $zipmoveopt $argOpt $jarchive $args") == 0 or
            $err = $? >> 8;
        zipErrorCheck($err,$lockfile);
    }

    if (!($overrides eq "")) {
        my $err = 0; 
        print "+++ overriding $overrides\n";
          
        while (length($overrides) > $maxCmdline) {
            #print "Exceeding POSIX cmdline limit: " . length($overrides) . "\n";
            my $subargs = substr($overrides, 0, $maxCmdline-1);
            my $pos = rindex($subargs, " ");
            $subargs = substr($overrides, 0, $pos);
            $overrides = substr($overrides, $pos);

            #print "$zipprog $zipmoveopt -X $jarchive $subargs\n";       
            #print "Length of subargs: " . length($subargs) . "\n";
            system("$zipprog $zipmoveopt -X $jarchive $subargs") == 0 or
                $err = $? >> 8;
            zipErrorCheck($err,$lockfile);
        }
        #print "Length of args: " . length($overrides) . "\n";
        #print "$zipprog $zipmoveopt -X $jarchive $overrides\n";
        system("$zipprog $zipmoveopt -X $jarchive $overrides\n") == 0 or 
        $err = $? >> 8;
        zipErrorCheck($err,$lockfile);
    }
    mozUnlock($lockfile) if (!$nofilelocks);
    chdir($oldDir);
    #print "cd $oldDir\n";
}

sub _moz_rel2abs
{
    my ($path, $isdir) = @_;
    $path = File::Spec->catfile(getcwd, $path)
        unless File::Spec->file_name_is_absolute($path);
    $path = File::Spec->canonpath($path);
    $path =~ s|\\|/|g if $path =~ s/^([A-Z]:\\)/\L$1/;
    my (@dirs) = reverse split(m:/:, $path);
    shift @dirs unless $isdir;
    my ($up) = File::Spec->updir();
    foreach (reverse 0 .. $#dirs) {
        splice(@dirs, $_, 2) if ($dirs[$_] eq $up);
    }
    return reverse @dirs;
}

sub _moz_abs2rel
{
    my ($target, $basedir, $isdir) = @_;
    my (@target) = _moz_rel2abs($target, 1);
    my (@basedir) = _moz_rel2abs($basedir, $isdir);
    shift @target, shift @basedir
        while @target && @basedir && $target[0] eq $basedir[0];
    return File::Spec->catfile((File::Spec->updir()) x @basedir, @target);
}

sub UniqIt
{
    my $manifest = shift;

    return if (scalar(@_) == 0); # no entries, don't bother

    my %lines = map { $_ => 1 } @_;

    my $lockfile = "$manifest.lck";
    print "+++ updating chrome $manifest\n";

    my $dir = dirname($manifest);
    mkpath($dir, 0, 0755);

    mozLock($lockfile) if (!$nofilelocks);
    if (-f $manifest) {
        unless (open(FILE, "<$manifest")) {
            mozUnlock($lockfile) if (!$nofilelocks);
            die "error: can't open $manifest: $!";
        };

        # Read through the file: if the lines already exist, no need to write
        # them again.
        while (defined($_ = <FILE>)) {
            chomp;
            delete $lines{$_};
        }
        close(FILE);
    }

    unless (open(FILE, ">>$manifest")) {
        mozUnlock($lockfile) if (!$nofilelocks);
        die "error: can't open $manifest: $!";
    };

    print FILE map("$_\n", keys(%lines));
    close(FILE) or my $err = 1;
    mozUnlock($lockfile) if (!$nofilelocks);

    if ($err) {
        die "error: can't close $manifest: $!";
    }
}

sub RegIt
{
    my ($chromeDir, $jarFileName, $chromeType, $pkgName) = @_;\
    chop($pkgName) if ($pkgName =~ m/\/$/);
    #print "RegIt:  $chromeDir, $jarFileName, $chromeType, $pkgName\n";

    my $line;
    if ($fileformat eq "flat" || $fileformat eq "symlink") {
        $line = "$chromeType,install,url,resource:/chrome/$jarFileName/$chromeType/$pkgName/";
    } else {
        $line = "$chromeType,install,url,jar:resource:/chrome/$jarFileName.jar!/$chromeType/$pkgName/";
    }
    my $installedChromeFile = "$jarDir/installed-chrome.txt";
    UniqIt($installedChromeFile, $line);
}

sub EnsureFileInDir
{
    my ($destPath, $srcPath, $destFile, $srcFile, $override, $preproc) = @_;
    my $objPath;

    #print "EnsureFileInDir($destPath, $srcPath, $destFile, $srcFile, $override)\n";

    my $src = $srcFile;
    if (defined($src)) {
        if ($src =~ m|^/|) {
            # "absolute" patch from topSrcDir
            defined($topSrcDir) || die("Command-line option -t <topsrcdir> missing.");
            $src = $topSrcDir.$srcFile;
        } elsif ($srcFile =~ s|^\%|/|) {
            defined($localeDir) || die("Command-line option -c <localedir> missing.");
            $src = $localeDir.$srcFile;
        } elsif (! -e $src ) {
            $src = "$srcPath/$srcFile";
        }
    }
    else {
        $src = "$srcPath/$destFile";
        # check for the complete jar path in the dest dir
        if (!-e $src) {
            #else check for just the file name in the dest dir
            my $dir = "";
            my $file;
            if ($destFile =~ /([\w\d.\-\_\\\/\+]+)[\\\/]([\w\d.\-\_]+)/) {
                $dir = $1;
                $file = $2;
            }
            else {
                die "file not found: $srcPath/$destFile";
            }
            $src = "$srcPath/$file";
            if (!-e $src) {
                die "file not found: $srcPath/$destFile";
            }
        }
    }

    $srcPath = $src;
    $destPath = "$destPath/$destFile";

    my $srcStat = stat($srcPath);
    my $srcMtime = $srcStat ? $srcStat->mtime : 0;

    my $destStat = stat($destPath);
    my $destMtime = $destStat ? $destStat->mtime : 0;
    #print "destMtime = $destMtime, srcMtime = $srcMtime\n";

    if (!-e $destPath || $destMtime < $srcMtime || $override) {
        #print "copying $destPath, from $srcPath\n";
        my $dir = "";
        my $file;
        if ($destPath =~ /(.+)[\\\/]([\w\d.\-\_]+)/) {
            $dir = $1;
            $file = $2;
        }
        else {
            $file = $destPath;
        }

        if ($srcPath) {
            $file = $srcPath;
        }
        $objPath = "$objdir/$destFile";

        if (!-e $file) {
            if (!-e $objPath) {
                die "error: file '$file' doesn't exist";
            } else {
                $file = "$objPath";
            }
        }
        if (!-e $dir) {
            mkpath($dir, 0, 0775) || die "can't mkpath $dir: $!";
        }
        unlink $destPath;       # in case we had a symlink on unix
        if ($preproc) {
            my $preproc_flags = '';
            if ($srcPath =~ /\.css$/o) {
                $preproc_flags = '--marker=%';
            }

            my $preproc_file = $file;
            if ($^O eq 'cygwin' && $file =~ /^[a-zA-Z]:/) {
                # convert to a cygwin path
                $preproc_file =~ s|^([a-zA-Z]):|$cygwin_mountprefix/\1|;
            }
            if ($vms) {
                # use a temporary file otherwise cmd is too long for system()
                my $tmpFile = "$destPath.tmp";
                open(TMP, ">$tmpFile") || die("$tmpFile: $!");
                print(TMP "$^X $preprocessor $preproc_flags $defines $preproc_file > $destPath");
                close(TMP);
                print "+++ preprocessing $preproc_file > $destPath\n";
                if (system("bash \"$tmpFile\"") != 0) {
                    die "Preprocessing of $file failed (VMS): ".($? >> 8);
                }
                unlink("$tmpFile") || die("$tmpFile: $!");
            } else {
                if (system("$^X $preprocessor $preproc_flags $defines $preproc_file > $destPath") != 0) {
                    die "Preprocessing of $file failed: ".($? >> 8);
                }
            }
        } elsif ("$fileformat" eq "symlink") {
            $file = _moz_abs2rel($file, $destPath);
            symlink($file, $destPath) || die "symlink($file, $destPath) failed: $!";
            return 1;
        } else {
            copy($file, $destPath) || die "copy($file, $destPath) failed: $!";
        }

        # fix the mod date so we don't jar everything (is this faster than just jarring everything?)
        my $mtime = stat($file)->mtime || die $!;
        my $atime = stat($file)->atime;
        $atime = $mtime if !defined($atime);
        utime($atime, $mtime, $destPath);

        return 1;
    }
    return 0;
}

my @gLines = <STDIN>;

while (defined($_ = shift @gLines)) {
    chomp;

start: 
    if (/^([\w\d.\-\_\\\/]+).jar\:\s*$/) {
        my $jarfile = $1;
        my $args = "";
        my $overrides = "";
        my $cwd = cwd();
        my @manifestLines;

        print "+++ making chrome $cwd  => $jarDir/$jarfile.jar\n";
        while (defined($_ = shift @gLines)) {
            if (/^\s+([\w\d.\-\_\\\/\+]+)\s*(\(\%?[\w\d.\-\_\\\/]+\))?$\s*/) {
                my $dest = $1;
                my $srcPath = defined($2) ? substr($2, 1, -1) : $2;
                EnsureFileInDir("$chromeDir/$jarfile", $baseFilesDir, $dest, $srcPath, 0, 0);
                $args = "$args$dest ";
                if (!foreignPlatformFile($jarfile) && !foreignPlatformPath($dest) && $autoreg &&
                    $dest =~ /([\w\d.\-\_\+]+)\/([\w\d.\-\_\\\/]+)contents.rdf/)
                {
                    my $chrome_type = $1;
                    my $pkg_name = $2;
                    RegIt($chromeDir, $jarfile, $chrome_type, $pkg_name);
                }
            } elsif (/^\+\s+([\w\d.\-\_\\\/\+]+)\s*(\(\%?[\w\d.\-\_\\\/]+\))?$\s*/) {
                my $dest = $1;
                my $srcPath = defined($2) ? substr($2, 1, -1) : $2;
                EnsureFileInDir("$chromeDir/$jarfile", $baseFilesDir, $dest, $srcPath, 1, 0);
                $overrides = "$overrides$dest ";
                if (!foreignPlatformFile($jarfile) && !foreignPlatformPath($dest) && $autoreg &&
                    $dest =~ /([\w\d.\-\_\+]+)\/([\w\d.\-\_\\\/]+)contents.rdf/)
                {
                    my $chrome_type = $1;
                    my $pkg_name = $2;
                    RegIt($chromeDir, $jarfile, $chrome_type, $pkg_name);
                }
            } elsif (/^\*\+?\s+([\w\d.\-\_\\\/\+]+)\s*(\(\%?[\w\d.\-\_\\\/]+\))?$\s*/) {
                # preprocessed (always override)
                my $dest = $1;
                my $srcPath = defined($2) ? substr($2, 1, -1) : $2;
                EnsureFileInDir("$chromeDir/$jarfile", $baseFilesDir, $dest, $srcPath, 1, 1);
                $overrides = "$overrides$dest ";
                if (!foreignPlatformFile($jarfile) && !foreignPlatformPath($dest) && $autoreg &&
                    $dest =~ /([\w\d.\-\_\+]+)\/([\w\d.\-\_\\\/]+)contents.rdf/)
                {
                    my $chrome_type = $1;
                    my $pkg_name = $2;
                    RegIt($chromeDir, $jarfile, $chrome_type, $pkg_name);
                }
            } elsif (/^\%\s+(.*)$/) {
                my $path = $1;

                my $jarpath = $jarfile;
                $jarpath = "chrome/".$jarfile if $useExtensionManifest;

                if ($fileformat eq "flat" || $fileformat eq "symlink") {
                    $path =~ s|\%|$jarpath/$1|;
                } else {
                    $path =~ s|\%|jar:$jarpath.jar!/$1|;
                }

                push @manifestLines, $path;
            } elsif (/^\s*$/) {
                # end with blank line
                last;
            } else {
                my $manifest = "$jarDir/$jarfile.manifest";
                my $manifest = "$jarDir/../chrome.manifest" if $useExtensionManifest;
                UniqIt($manifest, @manifestLines);
                JarIt($chromeDir, $jarDir, $jarfile, $args, $overrides);
                goto start;
            }
        }
        my $manifest = "$jarDir/$jarfile.manifest";
        $manifest = "$jarDir/../chrome.manifest" if $useExtensionManifest;
        UniqIt($manifest, @manifestLines);
        JarIt($chromeDir, $jarDir, $jarfile, $args, $overrides);

    } elsif (/^\s*\#.*$/) {
        # skip comments
    } elsif (/^\s*$/) {
        # skip blank lines
    } else {
        close;
        die "bad jar rule head at: $_";
    }
}
