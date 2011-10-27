%%% This is nuthesis.cls
%%% for writing doctoral dissertations at the 
%%% Mathematics Department of Northwestern University
%%% 
%%% Author: Miguel A. Lerma (mlerma@math.northwestern.edu)
%%% Date:4/21/2009
%%%
%%% ====================================================================
%% \CharacterTable
%%  {Upper-case    \A\B\C\D\E\F\G\H\I\J\K\L\M\N\O\P\Q\R\S\T\U\V\W\X\Y\Z
%%   Lower-case    \a\b\c\d\e\f\g\h\i\j\k\l\m\n\o\p\q\r\s\t\u\v\w\x\y\z
%%   Digits        \0\1\2\3\4\5\6\7\8\9
%%   Exclamation   \!     Double quote  \"     Hash (number) \#
%%   Dollar        \$     Percent       \%     Ampersand     \&
%%   Acute accent  \'     Left paren    \(     Right paren   \)
%%   Asterisk      \*     Plus          \+     Comma         \,
%%   Minus         \-     Point         \.     Solidus       \/
%%   Colon         \:     Semicolon     \;     Less than     \<
%%   Equals        \=     Greater than  \>     Question mark \?
%%   Commercial at \@     Left bracket  \[     Backslash     \\
%%   Right bracket \]     Circumflex    \^     Underscore    \_
%%   Grave accent  \`     Left brace    \{     Vertical bar  \|
%%   Right brace   \}     Tilde         \~}
\NeedsTeXFormat{LaTeX2e}% LaTeX 2.09 can't be used (nor non-LaTeX)
[1994/12/01]% LaTeX date must December 1994 or later
\ProvidesClass{nuthesis}[2009/04/21 v1.9c by Miguel A. Lerma]
\PassOptionsToClass{oneside}{amsbook}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{amsbook}}
\ProcessOptions\relax
\LoadClass{amsbook}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\RequirePackage[bookmarks, bookmarksopen=true, plainpages=false, pdfpagelabels, letterpaper, pdfpagelayout=SinglePage, breaklinks = true]{hyperref} % contributed by Ezra Getzler - 3/15/2005

% ``breaklinks = true'' option added - MAL 4/21/2009

% contributed by Tomasz Strzalecki - 7/27/2008
% there should be a page number on the page which displays the part name
\def\part{\cleardoublepage %
  \null\vfil  \markboth{}{}\secdef\@part\@spart}


%\def\singlespace{\renewcommand{\baselinestretch}{1}}
%\def\singlehalfspace{\renewcommand{\baselinestretch}{1.4}}
%\def\doublespace{\renewcommand{\baselinestretch}{1.8}}

%\RequirePackage{doublespace}       % MAL - 5/8/2001
%\def\onehalfspace{\spacing{1.4}}   % MAL - 5/8/2001
%\def\doublespace{\spacing{1.8}}    % MAL - 5/8/2001

\RequirePackage{setspace}                        % MAL - 4/19/2003
\renewcommand{\onehalfspacing}{\setstretch{1.4}} % MAL - 4/19/2003
\renewcommand{\doublespacing}{\setstretch{1.8}}  % MAL - 4/19/2003

% Page numbering should be at normal size 
% (amsbook.cls yields scriptsize) - MAL 6/17/2003
%\def\ps@plain{\ps@empty
%  \def\@oddfoot{\normalfont\normalsize \hfil\thepage\hfil}%
%  \let\@evenfoot\@oddfoot}



% Another place where amsbook.cls yields scriptsize
% (amsbook.cls yields scriptsize) - MAL 7/15/2003
\def\ps@headings{\ps@empty
  \def\@evenhead{%
    \setTrue{runhead}%
    \normalfont\normalsize        % changed from scriptsize - MAL 7/15/2003
    \rlap{\thepage}\hfil \leftmark{}{}\hfil}%
  \def\@oddhead{%
    \setTrue{runhead}%
    \normalfont\normalsize \hfil  % changed from scriptsize - MAL 7/15/2003
    \rightmark{}{}\hfil \llap{\thepage}}%
  \let\@mkboth\markboth
  \def\partmark{\@secmark\markboth\partrunhead\partname}%
  \def\chaptermark{%
    \@secmark\markboth\chapterrunhead{}}%
  \def\sectionmark{%
    \@secmark\markright\sectionrunhead\sectionname}%
}


% Numbering must be now on the upper right corner always - MAL 4/3/2006
\def\ps@plain{\ps@headings}


% Redefinition of \chapter* contributed by Oliver M. Haynold (5/19/2005)
% to fix a problem with bookmarks in PDF
\newcommand\chapterX{\if@openright\cleardoublepage\else\clearpage\fi
%   \thispagestyle{plain}%
    \global\@topnum\z@
    \@afterindentfalse
    \secdef\@addchap\@saddchap%
}


% from koma-script scrbook.cls - MAL 5/19/2005
\newcommand\addchap{\if@openright\cleardoublepage\else\clearpage\fi
%                    \thispagestyle{plain}%
                    \thispagestyle{myheadings}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@addchap\@saddchap}
\def\@addchap[#1]#2{\typeout{#2}
                    \addcontentsline{toc}{chapter}{#1}%
                    \if@twoside\@mkboth{#1}{}\else\@mkboth{}{#1}\fi
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \if@twocolumn
                         \@topnewpage[\@makeschapterhead{#2}]%
                    \else
                         \@makeschapterhead{#2}%
                         \@afterheading
                    \fi}
\def\@saddchap#1{\@mkboth{}{}
                 \if@twocolumn
                      \@topnewpage[\@makeschapterhead{#1}]%
                 \else
                      \@makeschapterhead{#1}%
                      \@afterheading
                 \fi}




% MAL - 2/15/2006
\def\l@figure{\@tocline{0}{3pt plus2pt}{0pt}{}{}}
%\def\l@figure{\@tocline{0}{3pt plus2pt}{0pt}{1.5pc}{}}
%\def\l@figure{\@tocline{0}{3pt plus2pt}{0pt}{3pc}{}}

\let\l@table=\l@figure


%\setlength{\textheight}{7.5in}
\setlength{\textheight}{8.0in}
\setlength{\topmargin}{0in}
\setlength{\headsep}{0.5in}
\setlength{\footskip}{0.5in}
\setlength{\headheight}{12pt}
%\setlength{\textwidth}{5.8in}
\setlength{\textwidth}{6.3in}
%\setlength{\oddsidemargin}{0.6in}
\setlength{\oddsidemargin}{0.1in}
\setlength{\evensidemargin}{0.1in}

\renewcommand\thesection{\thechapter.\@arabic\c@section}
\renewcommand\thefigure {\thechapter.\arabic{figure}}
\renewcommand\thetable {\thechapter.\arabic{table}}

%% SECTION heading spacing and font
%%
% arguments are: #1 - sectiontype name
% (for \@sect)   #2 - section level
%                #3 - indent
%                #4 - top separation
%                #5 - tempskipa, bottom separation
%                #6 - font control
% if tempskipa <= 0 the heading is a run-in heading

\def\section{\@startsection{section}{1}%
  \z@{.7\linespacing\@plus\linespacing}{.5\linespacing}%
  {\normalfont\bfseries\centering}}
\def\subsection{\@startsection{subsection}{2}%
  \z@{-3.25ex\@plus -1ex \@minus -.2ex}{1.5ex \@plus .2ex}%
  {\normalfont\bfseries}}
\def\subsubsection{\@startsection{subsubsection}{3}%
  \z@\z@{-.5em}%
  {\normalfont\bfseries}}


\def\th@plain{%
  \thm@headfont{\bfseries}
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@preskip.5\baselineskip\@plus.2\baselineskip
                                    \@minus.2\baselineskip
  \thm@postskip\thm@preskip
  \itshape
}
\def\th@definition{%
  \thm@headfont{\bfseries}%
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@preskip.5\baselineskip\@plus.2\baselineskip
                                    \@minus.2\baselineskip
  \thm@postskip\thm@preskip
  \upshape
}
\def\th@remark{%
  \thm@headfont{\bfseries}%
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@preskip.5\baselineskip\@plus.2\baselineskip
                                    \@minus.2\baselineskip
  \thm@postskip\thm@preskip
  \upshape
}


% This redefinition of the proof environment is obsolete - MAL 5/4/2005
%\renewenvironment{proof}[1][\proofname]{\par \normalfont
%  \topsep6\p@\@plus6\p@ \trivlist \itemindent\z@
%  \item[\hskip\labelsep\bfseries
%    #1\@addpunct{.}]\ignorespaces
%}{%
%  \qed\endtrivlist
%}

% Better just redefine the font for proofname 
% - suggested by Ezra Getzler 5/4/2005
\renewcommand{\proofname}{\bfseries{Proof}}


\numberwithin{equation}{chapter}


\def\@captionheadfont{\normalfont}  % \scshape in amsbook.cls


\def\@title{Title of dissertation required}

\def\degree#1{\def\d@gree{#1}}
\def\d@gree{DOCTOR OF PHILOSOPHY}   % spelling corrected - MAL 3/8/2002
\def\field#1{\def\fi@ld{#1}}
\def\fi@ld{Mathematics}
\def\campus#1{\def\c@mpus{#1}}
\def\c@mpus{EVANSTON, ILLINOIS}

\def\graduation@month{\ifcase\month\or
    June\or June\or June\or June\or June\or June\or December\or
    December\or December\or December\or December\or December\fi}
\def\graduation@year{\number\year}

\def\graduationmonth#1{\def\graduation@month{#1}}
\def\graduationyear#1{\def\graduation@year{#1}}

\def\@date{\graduation@month \graduation@year}


\def\frontmatter{%
  \cleardoublepage
  \pagenumbering{arabic}
  \pagestyle{myheadings}
}
\def\mainmatter{} % not needed any more
\def\backmatter{\cleardoublepage}


\def\titlepage{%
  \cleardoublepage
  \thispagestyle{empty}\setcounter{page}{1}}
\def\endtitlepage{\newpage}


\def\copyrightpage{%
  \cleardoublepage
\begin{center}
  \ \\
  \vspace{\fill}
\copyright \ Copyright by \authors{} \graduation@year \\
All Rights Reserved
\vspace{\fill}
\end{center}}
\def\endcopyrightpage{\newpage}


\def\maketitle{\par
  \@topnum\z@ % this prevents figures from falling at the top of page 1
\begin{titlepage}
\begin{center}
NORTHWESTERN UNIVERSITY\par
\vfill
{\large \@title}\par
\vfill
A DISSERTATION\par
\vspace{0.5cm}
SUBMITTED TO THE GRADUATE SCHOOL\par
IN PARTIAL FULFILLMENT OF THE REQUIREMENTS\par
\vspace{0.5cm}
for the degree\par
\vspace{0.5cm}
\d@gree{}\par
\vfill
Field of \fi@ld\par
\vfill
By\par
\vspace{0.5cm}
\ifx\@empty\authors
Author's name required\par
  \else
  \authors\par
  \fi
\vfill
\c@mpus\par
\vspace{0.5cm}
\graduation@month{} \graduation@year{}
\end{center}
\end{titlepage}
}


\renewcommand{\abstract}{%
\chapter*{\abstractname}
\bigskip
\begin{center}
\@title{}\par
\bigskip
\authors{}\par
\end{center}
\bigskip
}
\def\abstractname{ABSTRACT} % capitalized - MAL 5/3/2002


\newcommand{\acknowledgements}{\chapter*{\acknowledgementsname}}
\def\acknowledgementsname{Acknowledgements}
\newcommand{\preface}{\chapter*{\prefacename}}
\def\prefacename{Preface}

% New name for the table of contents MAL 5/9/2006
\renewcommand\contentsname{Table of Contents}

% The introduction must be a chapter - MAL 5/9/2006
%\newcommand{\introduction}{\chapter*{\introductionname}}
%\def\introductionname{Introduction} 

\newcommand{\vita}{\chapter*{\vitaname}}
\def\vitaname{Vita}

% Taken from amsbook.cls and modified.
% The references must use normal size font,
% and be single spaced with double space
% between items.
\renewenvironment{thebibliography}[1]{%
  \@xp\chapter\@xp*\@xp{\bibname}%
%  \normalfont\footnotesize            % commented out (MAL-5/4/2001)
  \labelsep .5em\relax
  \renewcommand\theenumiv{\arabic{enumiv}}\let\p@enumiv\@empty
  \list{\@biblabel{\theenumiv}}{%
    \settowidth\itemsep\@mainsize      % MAL - 5/9/2001
%    \singlespace                       % MAL - 5/9/2001
    \begin{singlespace}
    \settowidth\labelwidth{\@biblabel{#1}}%
    \leftmargin\labelwidth \advance\leftmargin\labelsep
    \usecounter{enumiv}}%
  \sloppy \clubpenalty\@M \widowpenalty\clubpenalty
  \sfcode`\.=\@m
}{%
  \def\@noitemerr{\@latex@warning{Empty `thebibliography' environment}}%
%  \endsinglespace                      % MAL - 5/9/2001
    \end{singlespace}
  \endlist
}
\def\bibname{References}


\def\appendices{\par
  \c@chapter\z@ \c@section\z@
  \let\chaptername\appendixname
  \def\thechapter{\@Alph\c@chapter}}

\def\appendix{\par
  \c@chapter\z@ \c@section\z@
  \let\chaptername\appendixname
  \def\thechapter{{}}
  \renewcommand\thesection{\@arabic\c@section}}



% Scheme enviroment (similar to figure) - MAL 9/25/2008
\def\ftype@scheme{3}
\def\listschemename{List of Schemes}
\def\listofschemes{\@starttoc{los}\listschemename}
\newcommand{\fps@scheme}{tbp}
\newcounter{scheme}[chapter]
\def\ftype@scheme{3}
\def\ext@scheme{los}
\def\fnum@scheme{\schemename\ \thescheme}
\def\schemename{Scheme}
\newenvironment{scheme}{%
  \@float{scheme}%
}{%
  \end@float
}
\newenvironment{scheme*}{%
  \@dblfloat{scheme}%
}{%
  \end@dblfloat
}
%\def\l@scheme{\@tocline{0}{3pt plus2pt}{0pt}{1.5pc}{}}
\def\l@scheme{\@tocline{0}{3pt plus2pt}{3pt}{5pc}{}}

\renewcommand\thescheme{\thechapter.\arabic{scheme}}





\doublespacing


\frontmatter % MAL 4/3/06


\endinput
%%
%% End of file `nuthesis.cls'.

