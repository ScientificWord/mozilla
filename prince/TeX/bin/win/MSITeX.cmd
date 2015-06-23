rem @echo off
setx MSITEX @@TeX_Root@@
setx MSITEXBIN @@TeX_Bindir@@
setx MSITEXMF @@TeXmf_Local@@
setx MSIBIBTEX @@BibTeX_Dir@@
setx PATH "c:\Program Files (x86)\gs\gs9.14\bin;c:\Program Files\gs\gs9.14\bin;@@TeX_Bindir@@;%PATH%"
setx BIBINPUTS %MSIBIBTEX%