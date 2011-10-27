%%  
%%  ws-procs11x85.cls  :   02 May 2003                                      
%%
%%  Class file for Proceedings Trim Size [11in x 8.5in] written in Latex2E. 
%%
%%  The content, structure, format and layout of this style file is the     
%%  property of World Scientific Publishing Co. Pte. Ltd.                   
%%  Copyright 1995, 2002 by World Scientific Publishing Co.                 
%%
%%  All rights are reserved.                                                
%%
%%  For Support: ykoh@wspc.com.sg                                            
%%               rajesh@wspc.com.sg                                          
%%


\def\@styver{v1.0 - 23 January 2003.}
\typeout{^^JWorld Scientific Proceeding style for trim size 11 x 8.5 [twocolumn] \@styver^^J}
\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{ws-procs11x85}[\@styver^^J]
%
\newif \if@twocolumn   \@twocolumnfalse
\newif \if@twoside     \@twosidefalse
%
\if@compatibility\else
\DeclareOption{oneside}{\@twosidefalse \@mparswitchfalse}
\fi
\DeclareOption{draft}{\setlength\overfullrule{5pt}}
\DeclareOption{final}{\setlength\overfullrule{0pt}}
\DeclareOption{twoside}{\@twosidetrue\@mparswitchtrue}
%
\ExecuteOptions{a4paper,10pt,twoside,twocolumn,final}
\ProcessOptions
%
\newcommand\contentsname{Contents}
\newcommand\listfigurename{List of Figures}
\newcommand\listtablename{List of Tables}
\newcommand\refname{References}
\newcommand\indexname{Index}
\newcommand\figurename{Figure}
\newcommand\tablename{Table}
\newcommand\partname{Part}
\newcommand\appendixname{Appendix}
\newcommand\abstractname{Abstract}
%
\newcommand\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
%
\newdimen\trimheight
\newdimen\trimwidth
\newdimen\typeheight
\newdimen\typewidth
\newdimen\tempdimen
\newdimen\normaltextheight
%
\def\@xiiipt{13}
\renewcommand\normalsize{%
   \@setfontsize\normalsize\@xpt\@xiiipt
   \abovedisplayskip6\p@\@plus4\p@\@minus2\p@
   \abovedisplayshortskip\abovedisplayskip
   \belowdisplayshortskip\abovedisplayshortskip
   \belowdisplayskip\abovedisplayskip
   \let\@listi\@listI}
\normalsize
\newcommand\small{%
   \@setfontsize\small\@ixpt{11}%
   \abovedisplayskip 8.5\p@ \@plus3\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus2\p@
   \belowdisplayshortskip 4\p@ \@plus2\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 4\p@ \@plus2\p@ \@minus2\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand\footnotesize{%
   \@setfontsize\footnotesize\@viiipt{10}%
   \abovedisplayskip 6\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 3\p@ \@plus\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
%
\newcommand\scriptsize{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand\tiny{\@setfontsize\tiny\@vpt\@vipt}
\newcommand\large{\@setfontsize\large\@xiipt{14}}
\newcommand\Large{\@setfontsize\Large\@xivpt{18}}
\newcommand\LARGE{\@setfontsize\LARGE\@xviipt{22}}
\newcommand\huge{\@setfontsize\huge\@xxpt{25}}
\newcommand\Huge{\@setfontsize\Huge\@xxvpt{30}}
\setlength\parindent{0.25in}
\setlength\smallskipamount{3\p@ \@plus 1\p@ \@minus 1\p@}
\setlength\medskipamount{6\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\bigskipamount{12\p@ \@plus 4\p@ \@minus 4\p@}
%%%%%%%
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}
%
\DeclareOldFontCommand{\bi}{\bfseries\itshape}{\bfseries\itshape}
\renewcommand{\rmdefault}{cmr}%
%
\DeclareMathSizes{28}{28}{25}{20}
\DeclareMathSizes{24}{24}{21}{18}
\DeclareMathSizes{22}{22}{19}{17}
\DeclareMathSizes{20}{20}{17}{15}
\DeclareMathSizes{18}{18}{15}{10}
\DeclareMathSizes{12}{11}{8}{5}
\DeclareMathSizes{11}{11}{8}{5}
\DeclareMathSizes{10}{10}{7}{5}
\DeclareMathSizes{9.5}{9}{7}{5}
\DeclareMathSizes{9}{9}{7}{5}
\DeclareMathSizes{8}{8}{6}{5}
\DeclareMathSizes{7}{7}{5}{5}
\DeclareMathSizes{6}{6}{5}{5}
\DeclareMathSizes{5}{5}{5}{5}
\def\bm#1{\mathchoice
    {\mbox{\boldmath$#1$}}%
          {\mbox{\boldmath$#1$}}%
          {\mbox{\boldmath$\scriptstyle#1$}}%
          {\mbox{\boldmath$\scriptscriptstyle#1$}}}%
%
\trimheight11truein
\advance\trimheight by 0.01truein

\trimwidth8.5truein
\advance\trimwidth by -0.02truein

\typeheight9.25in

%
\setlength\columnsep{0.3in}
\setlength\topmargin{.5in}
\setlength\headheight{8\p@}%8
\setlength\topskip  {0.7\p@}
\setlength\headsep{0.3in}%25
\setlength\footskip{0.3in}
\setlength\maxdepth{4\p@} 
\setlength\@maxdepth\maxdepth
\setlength\textwidth{6.6truein}
\setlength\textheight\typeheight
\advance\textheight-\headheight
\advance\textheight-\headsep
\advance\textheight-2pt
%\setlength\textheight{8.8truein}
\setlength\normaltextheight{\textheight}
\setlength\marginparsep {10\p@}
\setlength\marginparpush{5\p@}
\setlength\oddsidemargin  {0.0truein}
\setlength\evensidemargin {-\textwidth}
\addtolength\evensidemargin{-\oddsidemargin}
\setlength\marginparwidth {48\p@}
\setlength\footnotesep{6.65\p@}
\setlength{\skip\footins}{12\p@ \@plus 4\p@ \@minus 2\p@}%9
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
\setlength\partopsep{2\p@ \@plus 1\p@ \@minus 1\p@}
%
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
\renewcommand\baselinestretch{}
\setlength\parskip{0\p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
%
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
%%
\newcommand\maketitle{\par
  \begingroup
    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    \long\def\@makefntext##1{\parindent 1em\noindent
            \hb@xt@1.8em{%
                \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
 \if@twocolumn
      \ifnum \col@number=\@ne
        \@maketitle
      \else
        \twocolumn[\@maketitle]%
      \fi
    \else
      \newpage
      \global\@topnum\z@   % Prevents figures from going at top of page.
      \@maketitle
    \fi
    \thispagestyle{plain}\@thanks
  \endgroup
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
}
%
\def\title#1{\gdef\@title{\uppercase{#1}}}
\def\@title{\@latex@error{No \expand\title given}\@ehc}
%%
\def\@maketitle{%
  \newpage
  \null
\vspace*{-0.10in}
  \begin{center}%
  \let \footnote \thanks
    {\bf \@title \par}%
    \vskip 2em%
   \@aabuffer
\end{center}%
 \par}
%
%%%%%%%%%%%%
%
\def\and{\lowercase{and\ }}
\def\@aabuffer{}
\def\author #1{\expandafter\def\expandafter\@aabuffer\expandafter
    {\@aabuffer\normalsize\rm\uppercase{#1}\relax\par
    \vspace*{2pt}}}%\vspace{0.75em}
\def\address#1{\expandafter\def\expandafter\@aabuffer\expandafter
    {\@aabuffer\normalsize\it #1\relax\par
    \vspace{1.5em}}}
%%%
\long\def\abstract#1{
    \begin{center}
    \begin{minipage}{6truein}
    \footnotesize
    \parindent1em\noindent\ignorespaces#1
    \end{minipage}
    \end{center}
    \vskip1.5em}
%
%ACKNOWLEDGEMENT: this portion is from John Hershberger 
%
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout
    {\string\citation{#2}}\fi
\def\@citea{}\@cite{\@for\@citeb:=#2\do
    {\@citea\def\@citea{,}\@ifundefined
    {b@\@citeb}{{\bf ?}\@warning
    {Citation `\@citeb' on page \thepage \space undefined}}
    {\csname b@\@citeb\endcsname}}}{#1}}
%
\newif\if@cghi
\def\cite{\@cghitrue\@ifnextchar [{\@tempswatrue
    \@citex}{\@tempswafalse\@citex[]}}
\def\citelow{\@cghifalse\@ifnextchar [{\@tempswatrue
    \@citex}{\@tempswafalse\@citex[]}}
\def\@cite#1#2{{$^{#1}$\if@tempswa\typeout
    {WSPC warning: optional citation argument 
    ignored: `#2'} \fi}}
\newcommand{\citeup}{\cite}
%
\newenvironment{quotation}
               {\list{}{\listparindent 1.5em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item\relax}
               {\endlist}
%
\newenvironment{quote}
               {\list{}{\rightmargin\leftmargin}%
                \item\relax}
               {\endlist}
%
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
                                \normalfont\bfseries #1}
%%%%%
%
\def\@listi{\leftmargin\leftmargini
            \parsep 4\p@ \@plus2\p@ \@minus\p@
            \topsep 8\p@ \@plus2\p@ \@minus4\p@
            \itemsep4\p@ \@plus2\p@ \@minus\p@}
\let\@listI\@listi
\@listi
\def\@listii {\leftmargin\leftmarginii
              \labelwidth\leftmarginii
              \advance\labelwidth-\labelsep
              \topsep    4\p@ \@plus2\p@ \@minus\p@
              \parsep    2\p@ \@plus\p@  \@minus\p@
              \itemsep   \parsep}
\def\@listiii{\leftmargin\leftmarginiii
              \labelwidth\leftmarginiii
              \advance\labelwidth-\labelsep
              \topsep    2\p@ \@plus\p@\@minus\p@
              \parsep    \z@
              \partopsep \p@ \@plus\z@ \@minus\p@
              \itemsep   \topsep}
\def\@listiv {\leftmargin\leftmarginiv
              \labelwidth\leftmarginiv
              \advance\labelwidth-\labelsep}
\def\@listv  {\leftmargin\leftmarginv
              \labelwidth\leftmarginv
              \advance\labelwidth-\labelsep}
\def\@listvi {\leftmargin\leftmarginvi
              \labelwidth\leftmarginvi
              \advance\labelwidth-\labelsep}
%
\setlength\leftmargini{2em}
\leftmargin\leftmargini
\setlength\leftmarginii{2.2em}
\setlength\leftmarginiii{1.87em}
\setlength\leftmarginiv{1.7em}
\setlength\leftmarginv{.5em}
\setlength\leftmarginvi{.5em}
\setlength\labelsep{.5em}
\setlength\labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\renewcommand\theenumi{\@arabic\c@enumi}
\renewcommand\theenumii{\@alph\c@enumii}
\renewcommand\theenumiii{\@roman\c@enumiii}
\renewcommand\theenumiv{\@Alph\c@enumiv}
\newcommand\labelenumi{\theenumi.}
\newcommand\labelenumii{(\theenumii)}
\newcommand\labelenumiii{\theenumiii.}
\newcommand\labelenumiv{\theenumiv.}
\renewcommand\p@enumii{\theenumi}
\renewcommand\p@enumiii{\theenumi(\theenumii)}
\renewcommand\p@enumiv{\p@enumiii\theenumiii}
\newcommand\labelitemi{$\m@th\bullet$}
\newcommand\labelitemii{\normalfont\bfseries --}
\newcommand\labelitemiii{$\m@th\ast$}
\newcommand\labelitemiv{$\m@th\cdot$}
%
%FOLLOWING THREE COMMANDS ARE FOR `LIST' COMMAND.
\topsep=0in
\parsep=0in
\itemsep=0in
%
\newcounter{arabiclistc}
\newenvironment{arabiclist}
    {\setcounter{arabiclistc}{0}
     \begin{list}{\arabic{arabiclistc}}
    {\usecounter{arabiclistc}
     \setlength{\parsep}{0pt}
     \setlength{\itemsep}{0pt}}}{\end{list}}
%
\setcounter{secnumdepth}{2}
\newcounter {section}
\newcounter {subsection}[section]
\renewcommand\thesection      {\@arabic\c@section}
\renewcommand\thesubsection   {\thesection.\@arabic\c@subsection}
%
\newcommand\section{\@startsection {section}{1}{\z@}%
                    {-3.5ex \@plus -1ex \@minus -.2ex}%
                    {2.3ex \@plus.2ex}%
                    {\rightskip1pc plus1fil\normalfont\normalsize\bfseries}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}%
                    {-3.25ex\@plus -1ex \@minus -.2ex}%
                    {1.5ex \@plus .2ex}%
                    {\rightskip1pc plus1fil\normalfont\normalsize\it }}
%
\setcounter{topnumber}{2}
\renewcommand\topfraction{.7}
\setcounter{bottomnumber}{1}
\renewcommand\bottomfraction{.3}
\setcounter{totalnumber}{3}
\renewcommand\textfraction{.2}
\renewcommand\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\renewcommand\dbltopfraction{.7}
\renewcommand\dblfloatpagefraction{.5}
%
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{6\p@}
\setlength\belowcaptionskip{6\p@}
%%
\InputIfFileExists{psfig.sty}{}{}
\InputIfFileExists{epsfsafe.tex}{}{}
\InputIfFileExists{epsfig.sty}{}{}
\InputIfFileExists{epsf.sty}{}{}
%
\def\captionfont{\normalfont\footnotesize{}}

\long\def\@makecaption#1#2{%
    \captionfont
 \vskip\abovecaptionskip
  \sbox\@tempboxa{#1.\hskip.5em#2}%
  \ifdim \wd\@tempboxa >\hsize
    \noindent #1.\hskip.5em#2\par
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}
%
\newcounter{figure}
\renewcommand\thefigure{\@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figurename~\thefigure}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
%
\def\figurebox#1#2#3{%
    \def\arg{#3}%
    \ifx\arg\empty
    {\hfill\vbox{\hsize#2\hrule\hbox to #2{\vrule\hfill\vbox to #1{\hsize#2\vfill}\vrule}\hrule}\hfill}%
    \else
    {\hfill\epsfbox{#3}\hfill}% 
    \fi}
%
\def\@figurecaption#1#2{\unskip\vskip10pt{#1.\hskip.5em#2\par}}
%

\def\tablefont{\normalfont\footnotesize{}}
%\def\endtabular{\crcr\egroup\egroup $\egroup\endgroup}              %% Rajesh
%\def\tabular{\tablefont\begingroup\let\@halignto\@empty\@tabular}

\newcounter{table}
\renewcommand\thetable{\@arabic\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\tablename~\thetable}
\newenvironment{table}
               {\@float{table}\tablefont\begingroup}
               {\endgroup\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}

%
\setlength\arraycolsep{1.5\p@}%5pt
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins=\skip\footins
\setlength\fboxsep{0\p@}
\setlength\fboxrule{.2\p@}
\renewcommand\theequation{\@arabic\c@equation}
%
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
%
\renewcommand\footnoterule{%
  \kern-3\p@
  \hrule\@width.4\columnwidth
  \kern2.6\p@}
\newcommand\@makefntext[1]{%
    \parindent 1em%
    \noindent
     \@makefnmark#1}
%
\def\thefootnote{\alph{footnote}}
\def\@makefnmark{{$^{\@thefnmark}$}}
%%%%% page style %%%
\mark{{}{}}%
%

\def\ps@plain{\let\@mkboth\@gobbletwo 
    \let\@oddhead\@empty\def\@oddfoot{\hfill\thepage\hfill}%@empty 
         \def\@oddfoot{\hfill\mbox{\lower8pt\hbox{\thepage}}\hfill}
  \let\@evenhead\@empty\let\@evenfoot\@oddfoot} 

\def\RightMark{}
%
\def\foliofont{\fontsize{12}{12}\selectfont}
\def\ps@headings{%
    \def\@oddfoot{\vbox{\vskip22pt
      \hbox to 6.6truein{\hfill\reset@font\normalsize\phantom{777}\hfill}
      \vskip0.3truein
      \hbox to 6.6truein{\relax}}}
    \let\@evenfoot\@oddfoot
        \def\@evenhead{\vbox{\par
    \hbox to 6.6in{\thepage\hfill}}}
        \def\@oddhead{\vbox{\par
    \hbox to 6.6in{\hfill\thepage}}}%
      \let\@mkboth\markboth
    \def\sectionmark##1{%
      \markboth {\MakeUppercase{%
        \ifnum \c@secnumdepth >\z@
          \thesection\quad
        \fi
        ##1}}{}}%
    \def\subsectionmark##1{%
      \markright {%
        \ifnum \c@secnumdepth >\@ne
          \thesubsection\quad
        \fi
        ##1}}}
%
\def\ps@myheadings{%
    \def\@oddfoot{\vbox{\vskip22pt
      \hbox to 6.6truein{\hfill\reset@font\normalsize\phantom{777}\hfill}
      \vskip0.4truein
      \hbox to 6.6truein{\rightnote\hfil\fbox{{\bf \thepage}}\par}}}
    \let\@evenfoot\@oddfoot
        \def\@evenhead{\hbox to 6.6in{\leftmark\hfill}}%
        \def\@oddhead{\hbox to 6.6in{\hfill\rightmark}}%
 \let\@mkboth\markboth
    \def\authormark##1{\markboth{##1}{\RightMark}}%
    \def\chaptermark##1{\gdef\RightMark{##1}\markright{##1}}}%
%
\newenvironment{thebibliography}[1]
    {\section*{\refname}
    \begin{list}{\arabic{enumi}.}
    {\usecounter{enumi}\setlength{\parsep}{0pt}
     \setlength{\itemsep}{0pt} \settowidth
    {\labelwidth}{#1.}\sloppy}}{\end{list}}
%
\newcommand\appendix{\par
  \setcounter{section}{0}%
  \setcounter{subsection}{0}%
  \renewcommand\thesection{\@Alph\c@section}
    \section*{\appendixname}}
%
\newenvironment{tablehere}
  {\parskip0pt\topskip 13pt plus 2pt minus 1pt\def\@captype{table}\begin{small}\begin{center}}
  {\end{center}\end{small}\vskip 13pt plus 2pt minus 1pt}

\newenvironment{theindex}
               {\if@twocolumn
                  \@restonecolfalse
                \else
                  \@restonecoltrue
                \fi
                \columnseprule \z@
                \columnsep 35\p@
                \twocolumn[\section*{\indexname}]%
                \@mkboth{\MakeUppercase\indexname}%
                        {\MakeUppercase\indexname}%
                \thispagestyle{headings}\parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
               {\if@restonecol\onecolumn\else\clearpage\fi}
\newcommand\@idxitem{\par\hangindent 40\p@}
\newcommand\subitem{\@idxitem \hspace*{20\p@}}
\newcommand\subsubitem{\@idxitem \hspace*{30\p@}}
\newcommand\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
%
\def\rightnote{}%
\def\cropmarks{\nointerlineskip%
     \vbox to 0pt{\vskip-8.6pt\parindent0pt%\infotag
     \par\advance\trimheight by 2pc
     \vbox to \trimheight{\offinterlineskip\parindent0pt\vskip-2pc
     \hbox to \trimwidth{\raise2.6pc\hbox{\vrule height2pc width0.3pt}\raisebox{2pc}
   {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill\raisebox{2pc}
   {\vrule width2pc height0.3pt}\hspace*{-2.6pc}\raise2.6pc\hbox{\vrule height2pc width0.3pt}}\vfill
     \hbox to \trimwidth{\lower1.2pc\hbox{\vrule height2pc width0.3pt}\raisebox{1.4pc}
        {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill 
    \lower1.2pc\hbox{\vrule height2pc width0.3pt}\rlap{\hspace*{0.6pc}\raisebox{1.4pc}{\vrule width2pc height0.3pt}}}}}}%%
%
%
\def\@outputpage{%
\begingroup           % the \endgroup is put in by \aftergroup
  \let \protect \noexpand
  \@resetactivechars
  \let\-\@dischyph
  \let\'\@acci\let\`\@accii\let\=\@acciii
  \let\\\@normalcr
  \let\par\@@par          %% 15 Sep 87
  \shipout \vbox{%
    \set@typeset@protect
    \aftergroup \endgroup
    \aftergroup \set@typeset@protect
                                % correct? or just restore by ending
                                % the group?
  \if@specialpage
    \global\@specialpagefalse\@nameuse{ps@\@specialstyle}%
  \fi
  \if@twoside
    \ifodd\count\z@ \let\@thehead\@oddhead \let\@thefoot\@oddfoot
         \let\@themargin\oddsidemargin
    \else \let\@thehead\@evenhead
       \let\@thefoot\@evenfoot \let\@themargin\oddsidemargin
    \fi
  \fi
  \reset@font
  \normalsize
  \baselineskip\z@skip \lineskip\z@skip \lineskiplimit\z@
    \@begindvi%\cropmarks
    \vskip \topmargin
    \moveright\@themargin \vbox {%
      \setbox\@tempboxa \vbox to\headheight{%
        \vfil
        \color@hbox
          \normalcolor
          \hb@xt@\textwidth {%
            \let \label \@gobble
            \let \index \@gobble
            \let \glossary \@gobble %% 21 Jun 91
            \@thehead
            }%
        \color@endbox
        }%                        %% 22 Feb 87
      \dp\@tempboxa \z@
      \box\@tempboxa
      \vskip \headsep
      \box\@outputbox
      \baselineskip \footskip
      \color@hbox
        \normalcolor
        \hb@xt@\textwidth{%
          \let \label \@gobble
          \let \index \@gobble      %% 22 Feb 87
          \let \glossary \@gobble   %% 21 Jun 91
          \@thefoot
          }%
      \color@endbox
      }%
    }%
\global \@colht \textheight
\stepcounter{page}%
\let\firstmark\botmark
}
%%

\setlength\columnseprule{0\p@}
\def\sloppy{\tolerance=100000\hfuzz=\maxdimen\vfuzz=\maxdimen}
\vbadness=12000
\hbadness=12000
\hoffset=-.5cm
\pagenumbering{arabic}
\pagestyle{headings}
%\twocolumn
\sloppy
\flushbottom
\endinput
%%
%% End of file
%% For Support: ykoh@wspc.com.sg, rajesh@wspc.com.sg
%% ws-procs11x85.cls (C) World Scientific Publishing Co.
%%
