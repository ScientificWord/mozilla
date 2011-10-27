This is slightly revised documentation for the 1995 version of the 
Springer-Verlag LaTeX209 and LaTeX2e macros.

For the LaTeX209 macros (svsing [for single-author books], svcon [for 
contributors to multiauthor books], and svmult [for editors of multiauthor 
books]), begin your files as follows:
  \documentstyle[<svmult, svcon, or svsing>,<other style files>]{report}

LaTeX209 documentation is contained in the file pub/tex/latex209/svlatex.doc; 
svcon.sty is needed to TeX and print the file.

For the LaTeX2e macros (svsing2e [for single-author books], svcon2e [for 
contributors to multiauthor books], and svmult2e [for editors of multiauthor 
books]), begin your files as follows:
  \documentclass{report}
  \usepackage{<svmult2e, svcon2e, or svsing2e>}

LaTeX2e documentation is contained in the file pub/tex/latex2e/svlatex.doc; 
svcon2e.sty is needed to TeX and print the file.

Use of these packages follows the standard LaTeX209 or LaTeX2e system, except 
for one small feature. If you need to supply a short title for your running 
head in svmult, svmult2e, svcon, or svcon2e, use
  \shortchapname{<short title>}
  \shortauthname{<short title>}

If any problems crop up, please email

Frederick H. Bartlett
Supervisor, Electronic Production
texhelp@springer-ny.com

For some features and new macros (for instance, those that would change the 
fonts used from Computer Modern to Times or alter the page size), the approval 
of your Springer-Verlag editor may be necessary.

		GENERAL ADVICE

Calls to TeX primitives and plain TeX macros should only be made by 
higher-level macros defined in your style file. (There is, however, a large 
class of exceptions to this stricture: any macro defined with \chardef or 
\accent, can be used anywhere in horizontal mode, as can the plain TeX macros 
used for setting mathematics, such as \eqalignno and \left.

The use of commands like \vskip, \smallskip, \medskip, \bigskip, \parskip, 
\vglue, \hglue, \kern, \break, \eject, \hfil, \vfil, \hfill, \newline, 
\clearpage, \cleardoublepage, \newpage, or \vfill in your source files is  
discouraged, since their use may cause problems at a later stage of the 
publishing process.  While \break and \\ may be used to solve problems with 
Overfull \hboxes, it is best to fix the problem with \-.

Please do not begin paragraphs with displayed equations. That will cause the 
spacing to be inconsistent.


