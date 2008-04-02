#!/usr/bin/perl 
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
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

######################################################################
#
# Initial global variable
#
######################################################################
%utot = ();
$ui=0;
$li=0;

######################################################################
#
# Open the unicode database file
#
######################################################################
open ( UNICODATA , "< ../../unicharutil/tools/UnicodeData-Latest.txt") 
   || die "cannot find UnicodeData-Latest.txt";

######################################################################
#
# Open the JIS X 4051 Class file
#
######################################################################
open ( CLASS , "< jisx4501class.txt") 
   || die "cannot find jisx4501class.txt";

######################################################################
#
# Open the JIS X 4051 Class simplified mapping
#
######################################################################
open ( SIMP , "< jisx4501simp.txt") 
   || die "cannot find jisx4501simp.txt";

######################################################################
#
# Open the output file
#
######################################################################
open ( OUT , "> anzx4501.html") 
  || die "cannot open output anzx4501.html file";

######################################################################
#
# Open the output file
#
######################################################################
open ( HEADER , "> ../src/jisx4501class.h") 
  || die "cannot open output ../src/jisx4501class.h file";

######################################################################
#
# Generate license and header
#
######################################################################
$hthmlheader = <<END_OF_HTML;
<HTML>
<HEAD>
<TITLE>
Analysis of JIS X 4051 to Unicode General Category Mapping
</TITLE>
</HEAD>
<BODY>
<H1>
Analysis of JIS X 4051 to Unicode General Category Mapping
</H1>
END_OF_HTML
print OUT $hthmlheader;

######################################################################
#
# Generate license and header
#
######################################################################
$npl = <<END_OF_NPL;
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/* 
    DO NOT EDIT THIS DOCUMENT !!! THIS DOCUMENT IS GENERATED BY
    mozilla/intl/lwbrk/tools/anzx4501.pl
 */
END_OF_NPL
print HEADER $npl;

%occ = ();
%gcat = ();
%dcat = ();
%simp = ();
%gcount = ();
%dcount = ();
%sccount = ();
%rangecount = ();

######################################################################
#
# Process the file line by line
#
######################################################################
while(<UNICODATA>) {
   chop;
   ######################################################################
   #
   # Get value from fields
   #
   ######################################################################
   @f = split(/;/ , $_); 
   $c = $f[0];   # The unicode value
   $g = $f[2]; 
   $d = substr($g, 0, 1);

   $gcat{$c} = $g;
   $dcat{$c} = $d;
   $gcount{$g}++;
   $dcount{$d}++;
}
close(UNIDATA);

while(<SIMP>) {
   chop;
   ######################################################################
   #
   # Get value from fields
   #
   ######################################################################
   @f = split(/;/ , $_); 

   $simp{$f[0]} = $f[1];
   $sccount{$f[1]}++;
}
close(SIMP);

sub GetClass{
  my ($u) = @_;
  my $hex = DecToHex($u);
  $g = $gcat{$hex};
  if($g ne "") {
    return $g;
  } elsif (( 0x3400 <= $u) && ( $u <= 0x9fa5 )  ) {
    return "Han";
  } elsif (( 0xac00 <= $u) && ( $u <= 0xd7a3 )  ) {
    return "Lo";
  } elsif (( 0xd800 <= $u) && ( $u <= 0xdb7f )  ) {
    return "Cs";
  } elsif (( 0xdb80 <= $u) && ( $u <= 0xdbff )  ) {
    return "Cs";
  } elsif (( 0xdc00 <= $u) && ( $u <= 0xdfff )  ) {
    return "Cs";
  } elsif (( 0xe000 <= $u) && ( $u <= 0xf8ff )  ) {
    return "Co";
  } else {
    printf "WARNING !!!! Cannot find General Category for U+%s \n" , $hex;
  }
}
sub GetDClass{
  my ($u) = @_;
  my $hex = DecToHex($u);
  $g = $dcat{$hex};
  if($g ne "") {
    return $g;
  } elsif (( 0x3400 <= $u) && ( $u <= 0x9fa5 )  ) {
    return "Han";
  } elsif (( 0xac00 <= $u) && ( $u <= 0xd7a3 )  ) {
    return "L";
  } elsif (( 0xd800 <= $u) && ( $u <= 0xdb7f )  ) {
    return "C";
  } elsif (( 0xdb80 <= $u) && ( $u <= 0xdbff )  ) {
    return "C";
  } elsif (( 0xdc00 <= $u) && ( $u <= 0xdfff )  ) {
    return "C";
  } elsif (( 0xe000 <= $u) && ( $u <= 0xf8ff )  ) {
    return "C";
  } else {
    printf "WARNING !!!! Cannot find Detailed General Category for U+%s \n" , $hex;
  }
}
sub DecToHex{
     my ($d) = @_;
     return sprintf("%04X", $d); 
}
%gtotal = ();
%dtotal = ();
while(<CLASS>) {
   chop;
   ######################################################################
   #
   # Get value from fields
   #
   ######################################################################
   @f = split(/;/ , $_); 

   if( substr($f[2], 0, 1) ne "a")
   {
     $sc = $simp{$f[2]};
     $l = hex($f[0]);
     if($f[1] eq "")
     {
       $h = $l;
     } else {
       $h = hex($f[1]);
     }
     for($k = $l; $k <= $h ; $k++)
     {
       if( exists($occ{$k}))
       {
          #  printf "WARNING !! Conflict defination!!! U+%s -> [%s] [%s | %s]\n", 
          #         DecToHex($k),  $occ{$k} , $f[2] , $sc;
       }
       else
       {
           $occ{$k} = $sc . " | " . $f[2];
           $gclass = GetClass($k); 
           $dclass = GetDClass($k);
           $gtotal{$sc . $gclass}++;
           $dtotal{$sc . $dclass}++;
           $u = DecToHex($k);
           $rk = " " . substr($u,0,2) . ":" . $sc;
           $rangecount{$rk}++;
       }
     }
  }
}

#print %gtotal;
#print %dtotal;

sub printreport 
{
    print OUT "<TABLE BORDER=3>\n";
    print OUT "<TR BGCOLOR=blue><TH><TH>\n";
    
    foreach $d (sort(keys %dcount)) {
       print OUT "<TD BGCOLOR=red>$d</TD>\n";
    }
    
    print OUT "<TD BGCOLOR=white>Total</TD>\n";
    foreach $g (sort(keys %gcount)) {
       print OUT "<TD BGCOLOR=yellow>$g</TD>\n";
    }
    print OUT "</TR>\n";
    foreach $sc (sort(keys %sccount)) {
    
       print OUT "<TR><TH>$sc<TH>\n";
    
       $total = 0; 
       foreach $d (sort (keys %dcount)) {
         $count = $dtotal{$sc . $d};
         $total += $count;
         print OUT "<TD>$count</TD>\n";
       }
    
       print OUT "<TD BGCOLOR=white>$total</TD>\n";
    
       foreach $g (sort(keys %gcount)) {
         $count = $gtotal{$sc . $g};
         print OUT "<TD>$count</TD>\n";
       }
    
    
       print OUT "</TR>\n";
    }
    print OUT "</TABLE>\n";
    
    
    print OUT "<TABLE BORDER=3>\n";
    print OUT "<TR BGCOLOR=blue><TH><TH>\n";
    
    foreach $sc (sort(keys %sccount)) 
    {
       print OUT "<TD BGCOLOR=red>$sc</TD>\n";
    }
    
    print OUT "</TR>\n";
    
    
    for($rr = 0; $rr < 0x4f; $rr++)
    {
       $empty = 0;
       $r = sprintf("%02X" , $rr) ;
       $tmp = "<TR><TH>" . $r . "<TH>\n";
    
       foreach $sc (sort(keys %sccount)) {
         $count = $rangecount{ " " .$r . ":" .$sc};
         $tmp .= sprintf("<TD>%s</TD>\n", $count);
         $empty += $count;
       }
    
       $tmp .=  "</TR>\n";
    
       if($empty ne 0) 
       {
          print OUT $tmp;
       }
    }
    print OUT "</TABLE>\n";
    
}
printreport();

sub printarray
{
   my($r, $def) = @_;
printf "[%s || %s]\n", $r, $def;
   $k = hex($r) * 256;
   printf HEADER "static const PRUint32 gLBClass%s[32] = {\n", $r;
   for($i = 0 ; $i < 256; $i+= 8)
   {  
      for($j = 7 ; $j >= 0; $j-- )
      {  
          $v = $k + $i + $j;
          if( exists($occ{$v})) 
	  {
             $p = substr($occ{$v}, 1,1);
          } else {
             $p = $def;
          }

          if($j eq 7 ) 
          {
             printf HEADER "0x%s" , $p;
          } else {
             printf HEADER "%s", $p ;
          }
      }
      printf HEADER ", // U+%04X - U+%04X\n", $k + $i ,( $k + $i + 7);
   }
   print HEADER "};\n\n";
}
printarray("00", "7");
printarray("20", "7");
printarray("21", "7");
printarray("30", "5");
printarray("0E", "8");

#print %rangecount;

######################################################################
#
# Close files
#
######################################################################
close(HEADER);
close(CLASS);
close(OUT);

