rem @echo off
setx MSITEX "@@TeX_Root@@"
setx MSITEXBIN "@@TeX_Bindir@@"
setx MSITEXMF "@@TeXmf_Local@@"
setx MSITEXMF_HOME "%MSITEX\texmf-dist"
setx PATH "@@TeX_Bindir@@;%PATH%"
