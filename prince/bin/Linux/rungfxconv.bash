#! /bin/bash

# If you add support for any new graphics type, please add it to the extension list below.
# BEGIN EXTENSION LIST
# bmp tif tiff gif art arw cals cin cmyk cmyka crw cut dcm dcr dcx djvu dot dng dot fax fits gray hdr hrz mat mono
# mrw mtv nef orf otb p7 palm pam pbm pcd pcds pcl pcx pdb pef pfm pgm picon pict pix pnm ppm psd ptif pwp raf
# rgb rgba rla rle sct sfw sgi sun tga tim uil uyvy vicar viff wbmp wpg xbm xcf xpm xwd x3f ycbcr ycbcra yuv
# cdr cdt ccx cdrx cmx cgm xfig sk sk1 aff plt dxf dst pes exp pcs eps ps
# wmf emf
# END EXTENSION LIST

ImageMagick=${ImageMagick:="$5"}
Uniconvertor=${Uniconvertor:="$6"}
wmf2eps=${wmf2eps:="$7"}
inputPath="$1"
declare -l inputFormat
inputFormat=$2
outputPath=$3
mode=$4  # give output format=out; convert for graphic display=disp; give output for tex=outtex; convert for tex=tex

case $inputFormat in
  bmp | tif | tiff | gif | art | arw | cals | cin | cmyk | cymka | crw | cut | dcm | dcr | dcx | djvu | dot | dng | fax | fits | gray | hdr | hrz | mat | mono | mrw | mtv | nef | orf | otb | p7 | palm | pam | pbm | pcd | pcds | pcl | pcx | pdb | pdf | pfm | pgm | picon | pict | pix | pnm | ppm | psd | ptif | pwp | raf | rgb | rgba | rla | rle | sct | sfw | sgi | sun | tga | tim | uil | uvyy | vicar | viff | wbmp | wpg | xbm | xcf | xpm | xwd | x3f | ycbcr | ycbcra | yuv )
    case $mode in
      out | outtex)
        echo png
        ;;
      disp | tex)
        $ImageMagick/convert $inputPath.$2 "$outputPath.png" >"$outputPath.png.log" 2>&1
      if [ ! -f "$outputPath.png" ]
        then
        echo failed >"$outputPath.png.txt"
      fi
        ;;
    esac
    ;;
 
  cdr | ccx | cdrx | cmx | cgm | xfig | sk | sk1 | aff | plt | dxf | dst | pes | exp | pcs | svg | wmf | emf)
    case $mode in
      out)
        echo svg
        ;;
      outtex)
        echo pdf
        ;;
      disp)
        $Uniconvertor/uniconvertor $inputPath.$2 "$outputPath.svg" >"$outputPath.svg.log" 2>&1
        if [ ! -f "$outputPath.svg" ]
          then
          echo failed >"$outputPath.svg.txt"
        fi
        ;;
      tex)
        $Uniconvertor/uniconvertor $inputPath.$2 $outputPath.pdf >"$outputPath.pdf.log" 2>&1
        if [ ! -f "$outputPath.pdf" ]
          then
          echo failed >"$outputPath.pdf.txt"
        fi
        ;;
    esac
    ;;
    
#   wmf | emf)
#     case $mode in
#       out)
#         echo svg
#         ;;
#       outtex)
#         echo pdf
#         ;;
#       disp)
#         $wmf2eps/wmf2epsc $inputPath.$2 "$outputPath.eps" >"$outputPath.svg.log" 2>&1
#         $Uniconvertor/uniconvertor $"outputPath.eps" "$outputPath.svg" >>"$outputPath.svg.log" 2>&1
#         echo Need to put in wmf2eps here
#         if [ ! -f "$outputPath.svg" ]
#           then
#           echo failed >"$outputPath.svg.txt"
#         fi
#         ;;
#       tex)
#         $wmf2eps/wmf2epsc $inputPath.$2 "$outputPath.eps" >"$outputPath.eps.log" 2>&1
#         echo Need to put in wmf2eps here
#         if [ ! -f "$outputPath.eps" ]
#           then
#           echo failed >"$outputPath.eps.txt"
#         fi
#         ;;
#     esac
#     ;;
  *)
  ;;
esac
