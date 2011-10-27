%=========================== jjap.cls =============================
%
%-for JJAP %
% Version 1.0.1
%-Released on October 1997
%-Revised on June 1998
%-Copyright (C) Japanese Journal of Applied Physics
%
%=================================================================

\typeout{***** Document Class =<<<JJAP>>>= ver. 1.0.1 ******}

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{jjap}[1998/06/29 v.1.0.1 JJAP document class]

\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}

\setlength\lineskip{0\p@}
\setlength\normallineskip{0\p@}
\renewcommand\baselinestretch{1}
\setlength\parskip{0\p@}
\setlength\parindent{10\p@}

\setlength\paperheight{297mm}
\setlength\paperwidth{210mm}
\setlength\textheight{260mm}
\setlength\textwidth{177mm}
\setlength\oddsidemargin{-1cm}
\setlength\topmargin{-1cm}
\setlength\headheight{\z@}
\setlength\headsep{-12\p@} 
\setlength\topskip{12\p@}
%%\setlength\footheight{12\p@}
\setlength\footskip{18\p@}
\addtolength\textheight{\topskip}

\setlength{\skip\footins}{9\p@ \@plus 4\p@ \@minus 2\p@}
\setlength\floatsep    {12\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\textfloatsep{20\p@ \@plus 2\p@ \@minus 4\p@}
\setlength\intextsep   {12\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\dblfloatsep    {12\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\dbltextfloatsep{20\p@ \@plus 2\p@ \@minus 4\p@}
\setlength\@fptop{0\p@ \@plus 1fil}
\setlength\@fpsep{8\p@ \@plus 2fil}
\setlength\@fpbot{0\p@ \@plus 1fil}
\setlength\@dblfptop{0\p@ \@plus 1fil}
\setlength\@dblfpsep{8\p@ \@plus 2fil}
\setlength\@dblfpbot{0\p@ \@plus 1fil}
\setlength\partopsep{0\p@}

\setlength\arraycolsep {6pt} 
\setlength\tabcolsep {6pt}
\setlength\arrayrulewidth {.25pt}
\setlength\doublerulesep {2pt}
\setlength\tabbingsep {\labelsep}
\setlength\fboxsep{3pt} 
\setlength\fboxrule {.25pt}
\setlength\marginparwidth {.75in} 
\setlength\marginparsep {12pt}

\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\setcounter{topnumber}{15}
\renewcommand\topfraction{.9}
\setcounter{bottomnumber}{20}
\renewcommand\bottomfraction{.9}
\setcounter{totalnumber}{35}
\renewcommand\textfraction{.1}
\renewcommand\floatpagefraction{.9}
\setcounter{dbltopnumber}{2}
\renewcommand\dbltopfraction{.7}
\renewcommand\dblfloatpagefraction{1}
\raggedbottom

\renewcommand\normalsize{%
   \@setfontsize\normalsize\@xpt\@xiipt
   \abovedisplayskip 6\p@ 
   \abovedisplayshortskip 6\p@
   \belowdisplayshortskip 6\p@
   \belowdisplayskip \abovedisplayskip
   \setlength\jot{6\p@}
   \let\@listi\@listI}
\normalsize
\newcommand\small{%
   \@setfontsize\small\@ixpt{11}%
   \abovedisplayskip 5.5\p@
   \abovedisplayshortskip \abovedisplayskip
   \belowdisplayshortskip \abovedisplayskip
   \def\@listi{\leftmargin\leftmargini
               \topsep 4\p@ \@plus2\p@ \@minus2\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand\footnotesize{%
   \@setfontsize\footnotesize\@viiipt{9.5}%
   \abovedisplayskip 4\p@
   \abovedisplayshortskip \abovedisplayskip
   \belowdisplayshortskip \abovedisplayskip
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand\scriptsize{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand\tiny{\@setfontsize\tiny\@vpt\@vipt}
\newcommand\large{\@setfontsize\large\@xiipt{16}}
\newcommand\Large{\@setfontsize\Large\@xivpt{18}}
\newcommand\LARGE{\@setfontsize\LARGE\@xviipt{22}}
\newcommand\huge{\@setfontsize\huge\@xxpt{25}}
\newcommand\Huge{\@setfontsize\Huge\@xxvpt{30}}

%%\@mparswitchfalse
\newcounter {section}
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]

\renewcommand\thesection {\@arabic\c@section}
\renewcommand\thesubsection {\thesection.\@arabic\c@subsection}
\renewcommand\thesubsubsection {\thesubsection.\@arabic\c@subsubsection}
\renewcommand\theparagraph {\thesubsubsection.\@arabic\c@paragraph}
\renewcommand\thesubparagraph {\theparagraph.\@arabic\c@subparagraph}
%

\newcommand\@secapp{}

\newcommand\section{\@startsection{section}{1}{\z@}
                   {8truept}{4truept}{\normalfont\normalsize\bfseries}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}
                   {12truept}{.5pt \@plus -.5pt \@minus .5pt}
                   {\normalfont\normalsize\itshape}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}
                   {.5pt plus -.5pt minus .5pt}{.5pt plus -.5pt minus .5pt}
                   {\normalfont\normalsize\itshape}} 
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}
                   {\z@}{-10\p@}
                   {\normalfont\normalsize\bfseries}}
\newcommand\subparagraph{\@startsection{subparagraph}{4}{\parindent}
                   {\z@}{-10\p@}
                   {\normalfont\normalsize\itshape}}


\def\@nullstring{}
\def\@appendixstring{Appendix}

\def\@sect#1#2#3#4#5#6[#7]#8{\ifnum #2>\c@secnumdepth
     \def\@svsec{}\else
       \ifnum #2=1		% TRUE if \section{}
         \refstepcounter{#1}
         \def\@appendixtitle{#8}
         \ifx\@secapp\@appendixstring
         \ifx\@appendixtitle\@nullstring
             \edef\@svsec{\@secapp \csname the#1\endcsname\hskip10\p@}
         \else
             \edef\@svsec{\@secapp \csname the#1\endcsname:\hskip10\p@}
         \fi
         \else
           \ifx\thesection\@nullstring
             \edef\@svsec{\@secapp \csname the#1\endcsname\hskip10\p@}
           \else
             \edef\@svsec{\@secapp \csname the#1\endcsname.\hskip10\p@}
           \fi
         \fi
       \else
         \refstepcounter{#1}\edef\@svsec{\csname the#1\endcsname\hskip10\p@}
       \fi
     \fi
     \@tempskipa #5\relax
      \ifdim \@tempskipa>\z@
        \begingroup #6\relax
        \@hangfrom{\hskip #3\relax\@svsec}{\interlinepenalty \@M #8\par}%
        \endgroup
       \csname #1mark\endcsname{#7}
      \else
        \def\@svsechd{#6\hskip #3\@svsec #8\csname #1mark\endcsname{#7}}
      \fi
     \@xsect{#5}}

\setcounter{secnumdepth}{3}

\renewcommand\theenumi{\@arabic\c@enumi}
\renewcommand\theenumii{\@alph\c@enumii}
\renewcommand\theenumiii{\@roman\c@enumiii}
\renewcommand\theenumiv{\@Alph\c@enumiv}
\newcommand\labelenumi{(\theenumi)}
\newcommand\labelenumii{\theenumii.}
\newcommand\labelenumiii{\theenumiii.}
\newcommand\labelenumiv{\theenumiv.}
\renewcommand\p@enumii{\theenumi}
\renewcommand\p@enumiii{\theenumi(\theenumii)}
\renewcommand\p@enumiv{\p@enumiii\theenumiii}
\newcommand\labelitemi{\textbullet}
\newcommand\labelitemii{\normalfont\bfseries \textendash}
\newcommand\labelitemiii{\textasteriskcentered}
\newcommand\labelitemiv{\textperiodcentered}

\setlength\leftmargini {20\p@}
\setlength\leftmarginii {10\p@} 
\setlength\leftmarginiii {10\p@} 
\setlength\leftmarginiv {10\p@} 
\setlength\leftmarginv {10\p@}
\setlength\leftmarginvi {10\p@}
\setlength\leftmargin{\leftmargini}
\setlength\labelwidth{\leftmargini\advance\labelwidth-\labelsep}
\setlength\labelsep {5\p@} 
\setlength\parsep {\z@}
 
\def\@listi{\leftmargin\leftmargini}
\def\@listii{\leftmargin\leftmarginii
 \labelwidth\leftmarginii\advance\labelwidth-\labelsep
 \topsep\z@ 
 \parsep\z@ 
 \itemsep \parsep}
\def\@listiii{\leftmargin\leftmarginiii
 \labelwidth\leftmarginiii\advance\labelwidth-\labelsep
 \topsep\z@
 \parsep\z@
\partopsep\z@
 \itemsep=\z@ \topsep\z@}
\def\@listiv{\leftmargin\leftmarginiv
 \labelwidth\leftmarginiv\advance\labelwidth-\labelsep}
\def\@listv{\leftmargin\leftmarginv
 \labelwidth\leftmarginv\advance\labelwidth-\labelsep}
\def\@listvi{\leftmargin\leftmarginvi
 \labelwidth\leftmarginvi\advance\labelwidth-\labelsep}
 
\newenvironment{verse}
               {\let\\\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -15\p@%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 15\p@}%
                \item\relax}
               {\endlist}
\newenvironment{quotation}
               {\list{}{\listparindent 15\p@%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item\relax}
               {\endlist}
               
\newenvironment{quote}
               {\list{}{\rightmargin\leftmargin}%
                \item\relax}
               {\endlist}

\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
                                \normalfont\bfseries #1}


\def\thebibliography#1{%
\newpage\section*{References}
\list{\footnotesize\arabic{enumi})}{\footnotesize\settowidth%
 \labelwidth{[#1]}
 \leftmargin\labelwidth
 \advance\leftmargin\labelsep
 \usecounter{enumi}}
 \sloppy
 \sfcode`\.=500\relax}
\let\endthebibliography=\endlist

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\footnoterule{\noindent\vskip1\baselineskip \kern-3\p@
 \hrule width \halftextwidth
 \kern 2.6\p@}

%%%%%%% footnote symbol %%%%%%%%
\def\numberfootnotes{\setcounter{footnote}{0}%
		\gdef\thefootnote{\arabic{footnote}}}
\def\letterfootnotes{\setcounter{footnote}{0}%
		\gdef\thefootnote{\rm\alph{footnote})}}
\def\symbolfootnotes{\setcounter{footnote}{0}%
		\gdef\thefootnote{\fnsymbol{footnote}}}
%
\gdef\@thanks{}\symbolfootnotes
%%%%%%%%%%%%%%%%%%%%%%

\long\def\@makefntext#1{\vskip3pt \hsize=86truemm \par
\@ifundefined{manyfootnote@2}%
  {\par\hangindent 6pt}{\par\hangindent 9pt}
\noindent
\footnotesize \hskip6pt $^{\@thefnmark}$\hskip-1pt#1\vskip-3pt}

%%%%%%%%%%%%%%%%%%%%%%%%%
%--- Modification of halftext, fulltext ---- by S. Suzuki 
%   \halfflag is declared, which tells the current mode,  half or full.
%
\newcount\halfflag
\halfflag=0
%
\def\halftext{%
\halfflag=1
\global\hsize=86truemm\global\textwidth\hsize%
  \global\linewidth\textwidth}
\def\halftextwidth{86truemm}

%
\def\fulltext{%
\halfflag=0%
\global\hsize=177truemm\global\textwidth\hsize%
  \global\linewidth\textwidth}
\def\fulltextwidth{177truemm}

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ------------ Modification of @makecaption --------------- 
%              by S. Suzuki
%
%	   Figure captions are just stored in named box.
%	   Table caption are written.
%	   The position of captions depend on \halftext value.
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

\newcount\@figureindex
\@figureindex=1

\newcount\@tmpfigureindex
\@tmpfigureindex=1

\newcount\@boxcreated
\@boxcreated=1

\def\setfigurecaptionbox#1{%
\ifnum \@figureindex=1
\newbox{#1}
\global\advance\@boxcreated by 1
\fi
}

\def\makefigurecaptions{%
%
\ifnum \@figureindex>\@boxcreated
  \typeout{MAKEFIGURECAPTIONS: Number of Figures has changed.}
  \else
   \newpage
   \fulltext
   \parindent=\z@ 
    \ifnum \@figureindex>2
      \section*{Figure captions}
    \else
      \section*{Figure caption}
    \fi     
   \vskip.5\baselineskip
   \@whilenum\@figureindex>\@tmpfigureindex \do{%
   \usebox{\csname @figcapbox\@roman\@tmpfigureindex \endcsname}
   \vskip1\baselineskip
   \advance\@tmpfigureindex by 1 }
  \fi
}

\def\localsbox#1#2{%
\ifnum \@figureindex<\@boxcreated
   \ifnum \halfflag>0
      \global\setbox#1=\hbox{\begin{minipage}{\hsize}
      \par\hangindent=8truept\hangafter=1\noindent #2 \end{minipage}}\relax
      \else
      \global\setbox#1=\hbox{\begin{minipage}{15.5truecm}
      \par\hangindent=16truept\hangafter=1\noindent #2 \end{minipage}}\relax
   \fi
\else
   \typeout{Number of Figures has chagned, one more LaTeX needed.}
\fi
}

\global\def\captionfigure{figure}%

\long\def\@makecaption#1#2#3{\footnotesize
\ifx#1\captionfigure% 
%%%%%%%%%%% caption of figures %%%%%%%%
  \def\tempcapstring{\string\@figcapbox\@roman{\@figureindex}}
  \if@filesw \immediate\write\@auxout
       {\string\setfigurecaptionbox{\tempcapstring}}
  \fi
  \ifnum \@boxcreated>1
    \vskip .5\baselineskip
    \ifnum \halfflag>0
      \hsize=86truemm
      \global\setbox\@tempboxa\hbox{#2.\hskip10\p@#3}
        \ifdim \wd\@tempboxa >\hsize
        \par\hangindent=8truept\hangafter=1\noindent
         \localsbox{\csname @figcapbox\@roman\@figureindex\endcsname}
          {\unhbox\@tempboxa}
      \else
         \localsbox{\csname @figcapbox\@roman\@figureindex\endcsname}
	      {\hbox to\hsize{#2.\hskip10\p@#3\hfil}}
      \fi 
    \else
      \hsize=155truemm
      \setbox\@tempboxa\hbox{#2.\hskip10\p@#3}
      \ifdim \wd\@tempboxa >\hsize
      \par\hangindent=16truept\hangafter=1\noindent 
         \localsbox{\csname @figcapbox\@roman\@figureindex\endcsname}%
         {#2.\hskip10\p@#3}
      \else
         \localsbox{\csname @figcapbox\@roman\@figureindex\endcsname}
	      {\hbox to\hsize{#2.\hskip10\p@#3\hfill}}
      \fi 
    \fi
  \fi
  \global\advance\@figureindex by 1
\else	%%%%%%%% caption for tables  %%%%%%%%%%%
  \vskip 2\baselineskip
  \ifnum \halfflag>0
    \hsize=86truemm
    \setbox\@tempboxa\hbox{#2.\hskip10\p@#3}
    \ifdim \wd\@tempboxa>\hsize 
    \par\hangindent=8truept\hangafter=1\noindent
    \unhbox\@tempboxa\par%
    \else \hbox to\hsize{\hfil\box\@tempboxa\hfil}\par%
    \fi 
  \else
    \hsize=166truemm
    \setbox\@tempboxa\hbox{#2.\hskip10\p@#3}
    \ifdim \wd\@tempboxa<155truemm%
    \hbox to 177truemm{\hfil\box\@tempboxa\hfil}\par%
    \else
    \par\hangindent=48truept\hangafter=1\noindent\hskip32truept%
    \unhbox\@tempboxa\par%
    \fi  
  \fi 
\fi
} 

\long\def\@caption#1[#2]#3{\par
  \begingroup
    \@parboxrestore
    \normalsize
    \@makecaption{#1}{\csname fnum@#1\endcsname}{\ignorespaces #3}\par
  \endgroup}

\newcounter{figure}
\def\thecaption{\@arabic\c@figure}
\def\fps@figure{htbp}
\def\ftype@figure{1}
\def\fnum@figure{Fig.\hskip5\p@\thefigure}
\newcounter{table}
\def\thetable{\@Roman\c@table}
\def\fps@table{p}
\def\ftype@table{2} 
\def\fnum@table{Table\hskip5\p@\thetable}

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Table environment by S. Suzuki
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\figure{\@float{figure}}
\let\endfigure\end@float
\@namedef{figure*}{\@dblfloat{figure}}
\@namedef{endfigure*}{\end@dblfloat}
\def\fullfigure{\footnotesize\fulltext\@float{figure}\vspace{-4pt}}
\def\endfullfigure{\end@float\halftext}
\def\halffigure{\footnotesize\halftext\@float{figure}\vspace{-4pt}}
\def\endhalffigure{\end@float\halftext}

\def\table{\footnotesize\@float{table}}
\let\endtable\end@float
\@namedef{table*}{\@dblfloat{table}}
\@namedef{endtable*}{\end@dblfloat}

\def\fulltable{\vskip24pt\footnotesize\fulltext\@float{table}[p]}
\def\endfulltable{\end@float\halftext}

\def\halftable{\vskip24pt\footnotesize\halftext\@float{table}[p]}
\let\endhalftable\end@float

\def\halftabular{\halftext\footnotesize\vskip5pt%
    \def\@halignto{to \halftextwidth}\@tabular}
\def\fulltabular{\fulltext\footnotesize\vskip5pt%
    \def\@halignto{to \fulltextwidth}\@tabular}

\def\endhalftabular{\crcr\egroup\egroup $\egroup}
\def\endfulltabular{\crcr\egroup\egroup $\egroup}

\def\arraystretch{1.2}
\def\hline{\noalign{\ifnum0=`}\fi \hrule \@height
\arrayrulewidth \futurelet \@tempa\@xhline%
} 

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% definition of footnote mark by S. Suzuki
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\@fnsymbols#1{\ifcase#1\or *1\or *2\or *3\or
   *4\or *5\or *6\or *7\or *8\or *9 \else\@ctrerr\fi\relax}

\def\@fnsymbol#1{\ifcase#1\or *\or **\or ***\or
   ****\or *****\or ******\or *******\or ********
   \or ********* \else\@ctrerr\fi\relax}

\def\fnsymbol#1{\@fnsymbol{\@nameuse{c@#1}}}
\def\fnsymbols#1{\@fnsymbols{\@nameuse{c@#1}}}

\newcount\@footnoteindex
\@footnoteindex=0

\def\setmanyfootnote#1{%
  \global\@namedef{manyfootnote@#1}{1}
}

\def\footnote{%
 \global\advance \@footnoteindex by 1
 \if@filesw \immediate\write\@auxout
       {\string\setmanyfootnote{\the\@footnoteindex}}
 \fi
 \@localfootnote
}

\def\@localfootnote{\@ifnextchar[{\@xfootnote}{\stepcounter
   {\@mpfn}\xdef\@thefnmark{\hskip-4pt\thempfn}\@footnotemark\@footnotetext}}

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% definition of maketitle  by T. Inoue
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\maketitle{\par
 \begingroup
 \@ifundefined{manyfootnote@2}{\def\thefootnote{\fnsymbol{footnote}}}
                            {\def\thefootnote{\fnsymbols{footnote}}}

\def\@makefnmark{\hskip-1pt{$^{\@thefnmark}$}}

 \if@twocolumn
 \twocolumn[\@maketitle]
 \else %\newpage
 \global\@topnum\z@ \@maketitle \fi\thispagestyle{plain}\@thanks
 \endgroup
 \setcounter{footnote}{0}
 \let\maketitle\relax
 \let\@maketitle\relax
 \gdef\@thanks{}\gdef\@author{}\gdef\@inst{}\gdef\@title{}\let\thanks\relax}
\def\@maketitle{%
\null
\vskip.5\baselineskip

\def\re{\par\hangindent=0.5cm\hangafter=1\noindent}
\def\hang{\par\hangindent=\z@\hangafter=1\noindent}

{\large\bfseries 
\hang
\@title \par} 
\vskip 4truept
{%
\hang
\@author
}
\vskip 2truept
{\footnotesize\itshape
\hang
\@inst \par
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%     Date insertion,
%     desigend by Y. Yamamoto
%
%     A binary program extracts the strings ``(Received ...'' in the dvi
%     file is suported by Y. Yamamoto
%
\long\gdef\dmy_msg{\footnotesize 
(Received date; revised manuscript received date;
accepted for publication date --- These dates will be written in
by the editor, hence please let these lines remain unchanged even though
your preprint looks a bit ugly.) \par}
\vskip 2pt

\@ifundefined{@recdate}{\def\@recdate{}}{}
\@ifundefined{@revdate}{\def\@revdate{}}{}
\@ifundefined{@accdate}{\def\@accdate{}}{}
\ifx\@recdate\empty{\footnotesize\hang (Received Month date, year;
accepted for publication month date, year)}
\else\ifx\@accdate\empty{\footnotesize\hang (Received month date, year;
accepted for publication month date, year)}
\else
\vskip2pt
{\footnotesize\hang (Received \@recdate{}; \ifx\@revdate\empty accepted
for publication \@accdate{})\else revised manuscript received
\@revdate{}; accepted for publication \@accdate{})\fi}
\fi\fi
\gdef\@recdate{}\gdef\@revdate{}\gdef\@accdate{}
%
 \par
 \vskip6truept
 \@ifundefined{@abst}{}{%
\begingroup
{\noindent\parbox[t]{160mm}{\small \hspace*{9pt}
  \@abst} \par} 
  \gdef\@abst{}}
\endgroup
\vskip6truept
\@ifundefined{@kword}{}{%
\hang
\begingroup
{\footnotesize\sffamily KEYWORDS: \ 
\parbox[t]{140truemm}{\@kword}}
  \vskip 12truept
  \gdef\@kword{}}\par
\endgroup
\newpage
\halftext}

\@nameuse{@printstyle}
\@ifundefined{@printstyle}{}{\newpage}

\long\def\abst#1{\long\gdef\@abst{#1}}
\long\def\inst#1{\long\gdef\@inst{#1}}
\long\def\kword#1{\long\gdef\@kword{#1}}
%
\def\recdate#1{\gdef\@recdate{#1}}
\def\revdate#1{\gdef\@revdate{#1}}
\def\accdate#1{\gdef\@accdate{#1}}

\mark{{}{}}

\def\today{\ifcase\month\or
 January\or February\or March\or April\or May\or June\or
 July\or August\or September\or October\or November\or December\fi
 \space\number\day, \number\year}

%%%For math enviornment%%%
\def\eqnarray{%
  \stepcounter{equation}%
  \let\@currentlabel=\theequation
  \global\@eqnswtrue
  \global\@eqcnt\z@
  \tabskip\@centering
  \let\\=\@eqncr
  $$\halign to \displaywidth\bgroup\@eqnsel\hskip\@centering
  $\displaystyle\tabskip\z@{##}$&\global\@eqcnt\@ne
  \hfil$\displaystyle{{}##{}}$\hfil
  &\global\@eqcnt\tw@$\displaystyle\tabskip\z@{##}$\hfil
  \tabskip\@centering&\llap{##}\tabskip\z@\cr}
  
%%\def\dfrac#1#2{{\displaystyle\frac{#1}{#2}}} 
%%\def\cfrac#1#2{\dfrac{\mathstrut #1}{\mathstrut #2}}
\def\mbi#1{\mbox{\boldmath$#1$}}

\pagenumbering{arabic} \onecolumn

\def\sloppy{\tolerance=9999
\hfuzz=.5\p@ \vfuzz=.5\p@}

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Acknowledgements by S. Suzuki
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\acknowledgement{\par
 \setcounter{section}{0}
 \def\@secapp{Acknowledgement}
 \def\thesection{}
 \section{}
}

\def\acknowledgements{\par
 \setcounter{section}{0}
 \def\@secapp{Acknowledgements}
%%% \def\@chapend{}
 \def\thesection{}
 \section{}
}

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% \appendix  by S. Suzuki
%
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\newcount\@appendixflag
\@appendixflag=0
\newcount\@appendixindex
\@appendixindex=0

\def\getappendix#1{\global\@namedef{appnedix@#1}{1}}

\def\appendix#1{\par
 \c@table=0
 \c@figure=0
 \setcounter{equation}{0}
 \ifnum \@appendixflag=0
   \setcounter{section}{0} \@appendixflag=1
   \def\@secapp{Appendix}
   \def\thetable{\Alph{section}$\cdot$\@arabic\c@table}%
   \def\thefigure{\Alph{section}$\cdot$\@arabic\c@figure}
   \def\theequation{\Alph{section}\raise.5ex\hbox{.}\arabic{equation}}
 \fi
%
 \@ifundefined {appnedix@1}{\def\thesection{}}
   {\def\thesection{\ \Alph{section}}}
%
 \def\thesubsection {\Alph{section}.\arabic{subsection}}
 \if@filesw \immediate\write\@auxout
       {\string\getappendix{\the\@appendixindex}}
 \advance \@appendixindex by 1 \fi
 \section{#1}
}
%%%%%%%%% end of appendix %%%%%%%%%%%%%
%  
\RequirePackage{amsmath}[1997/03/20]
\RequirePackage{amssymb}
\RequirePackage{latexsym} 
\RequirePackage{overcite} 
\def\@cite#1{\mbox{$\m@th^{\hbox{\@ove@rcfont#1)}}$}}
%
\renewcommand\theequation{\@arabic\c@equation}
\DeclareOption{seceq}{%
\@addtoreset{equation}{section}
\renewcommand \theequation {\@arabic\c@section.\@arabic\c@equation}
}
\DeclareOption{jjapprn}{\input{jjapprn.clo}}
\ProcessOptions*\relax
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\pagestyle{plain}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\endinput
%%
%% End of file `jjap.cls'.
