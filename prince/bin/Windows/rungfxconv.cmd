@echo off
setlocal
REM If you add support for any new graphics type, please add it to the extension list below.
REM BEGIN EXTENSION LIST
REM bmp tif tiff gif art arw cals cin cmyk cmyka crw cut dcm dcr dcx djvu dot dng dot fax fits gray hdr hrz mat mono
REM mrw mtv nef orf otb p7 palm pam pbm pcd pcds pcl pcx pdb pef pfm pgm picon pict pix pnm ppm psd ptif pwp raf
REM rgb rgba rla rle sct sfw sgi sun tga tim uil uyvy vicar viff wbmp wpg xbm xcf xpm xwd x3f ycbcr ycbcra yuv
REM cdr cdt ccx cdrx cmx cgm xfig sk sk1 aff plt dxf dst pes exp pcs eps ps
REM wmf emf
REM END EXTENSION LIST

if %ImageMagick%x==x set ImageMagick=%5
if %Uniconvertor%x==x set Uniconvertor=%6
if %wmf2epsDir%x==x set wmf2epsDir=%7
set inputPath=%1
set inputFormat=%2
set outputPath=%3
set mode=%4
set outputFile=%8
set extraFiles=()
REM give output format=out; convert for graphic display=disp; give output for tex=outtex; convert for tex=tex
for %%E in (bmp tif tiff gif art arw cals cin cmyk cmyka crw cut dcm dcr dcx djvu dot dng dot fax fits gray hdr hrz mat mono mrw mtv nef orf otb p7 palm pam pbm pcd pcds pcl pcx pdb pef pfm pgm picon pict pix pnm ppm psd ptif pwp raf rgb rgba rla rle sct sfw sgi sun tga tim uil uyvy vicar viff wbmp wpg xbm xcf xpm xwd x3f ycbcr ycbcra yuv) do (if %%Ex==%inputFormat%x goto imMagick)
for %%E in (cdr cdt ccx cdrx cmx cgm xfig sk sk1 aff plt dxf dst pes exp pcs eps ps svg) do (if %%Ex==%inputFormat%x goto UniConv)
for %%E in (wmf emf) do (if %%Ex==%inputFormat%x goto wmfeps)
goto finish
:imMagick
set outputFormat=png
if %mode%x==outx goto echoIt
if %mode%x==outtexx goto echoIt
%ImageMagick%\convert %inputPath%.%inputFormat% %outputPath%.png >%outputPath%.png.log 2>&1
goto finish
:UniConv
set outputFormat=svg
if %mode%x==texx set outputFormat=pdf
if %mode%x==outtexx set outputFormat=pdf
if %mode%x==outx goto echoIt
if %mode%x==outtexx goto echoIt
call %Uniconvertor%\uniconvertor "%~1.%inputFormat%" "%~3.%outputFormat%" >%outputPath%.%outputFormat%.log 2>&1
goto finish
:wmfeps
set outputFormat=svg
if %mode%x==outx set extraFiles=(eps)
if %mode%x==texx set outputFormat=eps
if %mode%x==outtexx set outputFormat=eps
if %mode%x==outx goto echoIt
if %mode%x==outtexx goto echoIt
copy /Y %inputPath%.%inputFormat% %outputPath%.%inputFormat%
%wmf2epsDir%\wmf2eps %outputPath%.%inputFormat% >%outputPath%.%outputFormat%.log 2>&1
call %Uniconvertor%/uniconvertor "%~3.eps" %~3.%outputFormat%" >>%outputPath%.%outputFormat%.log 2>&1
goto finish
:echoIt
if %outputFile%x==x goto noextra
for %%g in %extraFiles% do echo %%g >>%outputFile%
:noextra
if not %outputFile%x==x echo %outputFormat% >>%outputFile%
echo %outputFormat%
goto endIt
:finish
if not exist %outputPath%.%outputFormat% echo failed >%outputPath%.%outputFormat%.txt
:endIt
endlocal
