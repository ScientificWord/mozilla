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
# Portions created by the Initial Developer are Copyright (C) 1998-2000
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
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

package mozBDate;

use strict;
use IO::File;

BEGIN {
    use Exporter ();
    use vars qw ($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $milestone);

    $VERSION = 1.00;
    @ISA = qw(Exporter);
    @EXPORT = qw(&UpdateBuildNumber &SubstituteBuildNumber &SetMilestone);
    %EXPORT_TAGS = ( );
    @EXPORT_OK = qw();
}

local $mozBDate::milestone = "0.0";

sub write_number($) {
    my ($file, $num) = @_;
    unlink($file);
    open(OUT, ">$file") || die "$file: $!\n";
    print OUT "$num\n";
    close(OUT);
}

sub UpdateBuildNumber($$) {

    my ($outfile, $official) = @_;
    my $given_date = $ENV{"MOZ_BUILD_DATE"};
    my $build_number;

    if ($given_date eq "") {
	# XP way of doing the build date.
	# 1998091509 = 1998, September, 15th, 9am local time zone
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =
	    localtime(time);

	# localtime returns year minus 1900
	$year = $year + 1900;
	$build_number = sprintf("%04d%02d%02d%02d", $year, 1+$mon,
				$mday, $hour);
    }
    else {
	$build_number = $given_date;
    }

    if ("$outfile" eq "") {
        print "$build_number\n";
        return;
    }

    if (!$official) {
	$build_number = "0000000000";
    }

    my $old_num = "";
    
    # Don't overwrite $outfile if its contents won't change
    if ( -e $outfile ) {
        open(OLD, "<$outfile") || die "$outfile: $!\n";
        $old_num = <OLD>;
        chomp($old_num);
        close(OLD);
    }

    if ($old_num ne $build_number) {
        &write_number($outfile, $build_number);
    }
    return;
}

sub SubstituteBuildNumber($$$) {

    my ($outfile, $build_num, $infile) = @_;
    my $INFILE = new IO::File;
    my $OUTFILE = new IO::File;

    open $INFILE, "<$build_num";
    my $build = <$INFILE>;
    close $INFILE;
    chomp $build;
    chop $build if (substr($build, -1, 1) eq "\r");
    
    if ($infile ne "") {
        open($INFILE, "< $infile") || die "$infile: $!\n";
    } else {
        open($INFILE, "< $outfile") || die "$outfile: $!\n";
    }
    open $OUTFILE, ">${outfile}.old" || die;
    
    while (<$INFILE>) {
        my $id = $_;
        my $temp;
        if ($id =~ "Build ID:") {
            $temp = "Build ID: " . $build;
            $id =~ s/Build ID:\s\d+/$temp/;
            print $OUTFILE $id;
        }
        elsif ($id =~ "NS_BUILD_ID") {
            $temp = "NS_BUILD_ID " . $build;
            $id =~ s/NS_BUILD_ID\s\d+/$temp/;
            print $OUTFILE $id;
        }
        elsif ($id =~ "GRE_BUILD_ID") {
            if (defined($ENV{'MOZ_MILESTONE_RELEASE'}) &&
                $ENV{'MOZ_MILESTONE_RELEASE'} ne "") {
                $temp = "GRE_BUILD_ID \"$milestone\"";
            } else {
                $temp = "GRE_BUILD_ID \"${milestone}_${build}\"";
            }
            $id =~ s/GRE_BUILD_ID\s\"\d+\"/$temp/;
            print $OUTFILE $id;
        }
        else {
            print $OUTFILE $_;
        }
    }

    close $INFILE;
    close $OUTFILE;

    unlink $outfile;
    rename "${outfile}.old", "$outfile";
}

sub SetMilestone($) {
    my ($mstone) = (@_);
    $milestone = $mstone if ($mstone ne "");
}

END {};

1;

