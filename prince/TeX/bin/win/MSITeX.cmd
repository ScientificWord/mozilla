rem @echo off
setx MSITEX @@TeX_Root@@
setx MSITEXBIN @@TeX_Bindir@@
setx MSITEXMF @@TeXmf_Local@@
setx MSIBIBTEX @@BibTeX_Dir@@
setx PATH @@TeX_Bindir@@;%PATH%
setx BIBINPUTS %MSIBIBTEX%;
