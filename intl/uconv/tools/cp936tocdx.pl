#!/user/local/bin/perl
# -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
# The Original Code is Mozilla Communicator client code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 1998
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
$rowwidth = ((0xff - 0x80)+(0x7f - 0x40));
sub cp936tonum()
{
   my($cp936) = (@_);
   my($first,$second,$jnum);
   $first = hex(substr($cp936,2,2));
   $second = hex(substr($cp936,4,2));
   $jnum = ($first - 0x81 ) * $rowwidth;
   if($second >= 0x80)
   {
       $jnum += $second - 0x80 + (0x7f-0x40);
   }
   else
   {
       $jnum += $second - 0x40;
   }
   return $jnum;
}

@map = {};
sub readtable()
{
open(CP936, "<gbkcommon.txt") || die "cannot open gbkcommon.txt";
while(<CP936>)
{
   if(! /^#/) {
        chop();
        ($j, $u, $r) = split(/\t/,$_);
        if(length($j) > 4)
        {
        $n = &cp936tonum($j);
        $map{$n} = $u;
        }
   } 
}
}


sub printtable()
{
  for($i=0;$i<126;$i++)
  {
     printf ( "/* 0x%2XXX */\n", ( $i + 0x81));
     for($j=0;$j<(0x7f-0x40);$j++)
     {
         if("" eq ($map{($i * $rowwidth + $j)}))
         {
            printf "0xFFFD,"
         } 
         else 
         {   
            printf $map{($i * $rowwidth + $j)} . ",";
         }
         if( 0 == (($j + 1) % 8))
         {
            printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 4+($j/16), (7==($j%16))?0:8;
         }
     }
     
	 print "0xFFFD,";

     printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 4+($j/16),(7==($j%16))?0:8;
     for($j=0;$j < (0xff-0x80);$j++)
     {
         if("" eq ($map{($i * $rowwidth + $j + 0x3f)}))		# user defined chars map to 0xFFFD
         {

			if ( ( $i == 125 ) and ( $j == (0xff - 0x80 - 1 )))
			{
				printf "0xFFFD";							#has no ',' followed last item
			}
			else
			{
				printf "0xFFFD,";
			}
         } 
		 else
		 {
			if ( ( $i == 125 ) and ( $j == (0xff - 0x80 - 1 )))
			{
				printf $map{($i * $rowwidth + $j + 0x3f)};	#has no ',' followed last item
			}
			else
			{
				printf $map{($i * $rowwidth + $j + 0x3f)} . ",";
			}
		 }
		  	
         if( 0 == (($j + 1) % 8))
         {
            printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 8+($j/16), (7==($j%16))?0:8;
         }
     }
     printf "       /* 0x%2X%1X%1X*/\n", $i+0x81, 8+($j/16),(7==($j%16))?0:8;
  }
}
sub printnpl()
{
$npl = <<END_OF_NPL;
/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
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
END_OF_NPL
print $npl;
}
sub printdontmodify()
{
$dont_modify = <<END_OF_DONT_MODIFY;
/*
  This file is generated by mozilla/intl/uconv/tools/cp936tocdx.pl
  Please do not modify this file by hand
  Instead, you should download CP936.TXT from
  http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/
  and put under mozilla/intl/uconv/toools
  and run perl cp936tocdx.pl > ../ucvcn/cp936map.h
  If you have question, mailto:ftan\@netscape.com
 */
END_OF_DONT_MODIFY
print $dont_modify;
}

&readtable();
&printnpl();
&printdontmodify();
&printtable();

