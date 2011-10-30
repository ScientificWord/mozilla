%%%%%%%%%% jltp.cls %%%%%%%%%%
% Version 0.2: May 13, 1998
%
% D. Murphy
%
% Usage: \documentclass[]{jltp}
%
% Based on article style with twoside option, LaTeX2e
%

\NeedsTeXFormat{LaTeX2e}[1995/06/01]
\ProvidesClass{jltp}[1998/5/13 v0.2 JLTP Camera Ready Format]

\DeclareOption{twocolumn}{\OptionNotUsed}
\DeclareOption{titlepage}{\OptionNotUsed}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions\relax
\LoadClass[twoside,11pt]{article}

% Indent paragraph after section heading
% We could have used the indentfirst package, but don't want to rely on it
\let\@afterindentfalse\@afterindenttrue
\@afterindenttrue

% We need address
\newcommand{\address}[1]{\gdef\@address{#1}}
\newcommand{\@address}{\@latex@warning@no@line{No \noexpand\address given}}

% Reformat the section commands
\renewcommand\section{\@startsection {section}{1}{\z@}%
                                   {-6.0ex \@plus -1ex \@minus -.2ex}%
                                   {2.0ex \@plus.2ex}%
                                   {\normalfont\normalsize\bfseries\centering}}
\renewcommand\subsection{\@startsection{subsection}{2}{\z@}%
                                   {-6.0ex \@plus -1ex \@minus -.2ex}%
                                   {2.0ex \@plus.2ex}%
                                     {\normalfont\normalsize\bfseries\centering}}
\renewcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
                                   {-6.0ex \@plus -1ex \@minus -.2ex}%
                                   {2.0ex \@plus.2ex}%
                                     {\normalfont\normalsize\itshape\centering}}
\renewcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
                                    {3.25ex \@plus1ex \@minus.2ex}%
                                    {-1em}%
                                    {\normalfont\normalsize\bfseries\centering}}
\renewcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}%
                                       {3.25ex \@plus1ex \@minus .2ex}%
                                       {-1em}%
                                      {\normalfont\normalsize\bfseries\centering}}

% Change the numbering of sections so that numbers end in period
\renewcommand\thepart         {\@Roman\c@part .}
\renewcommand\thesection      {\@arabic\c@section .}
\renewcommand\thesubsection   {\thesection\@arabic\c@subsection .}
\renewcommand\thesubsubsection{\thesubsection\@arabic\c@subsubsection .}
\renewcommand\theparagraph    {\thesubsubsection\@arabic\c@paragraph .}
\renewcommand\thesubparagraph {\theparagraph\@arabic\c@subparagraph .}

% Change appendix name
% Needs Appendix in front of letter
\renewcommand\appendix{\par
  \setcounter{section}{0}%
  \setcounter{subsection}{0}%
  \renewcommand\thesection{Appendix \@Alph\c@section .}
  \setcounter{equation}{0}
  \renewcommand\theequation{\@Alph\c@section.\@arabic\c@equation}}

% First page has empty pagestyle
\renewcommand{\maketitle}{\par
  \begingroup
    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    \long\def\@makefntext##1{\parindent 1em\noindent
            \hb@xt@1.8em{%
                \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
%    \if@twocolumn
%      \ifnum \col@number=\@ne
%        \@maketitle
%      \else
%        \twocolumn[\@maketitle]%
%      \fi
%    \else
      \newpage
      \global\@topnum\z@   % Prevents figures from going at top of page.
      \@maketitle
%    \fi
%    \thispagestyle{plain}\@thanks
    \thispagestyle{empty}\@thanks
  \endgroup
  \setcounter{footnote}{0}%
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@address\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\address\relax
  \global\let\date\relax
  \global\let\and\relax
}

% Reformat title
% Font is Large and boldfaced
% Author is boldfaced, normalsized
% No date
\renewcommand{\@maketitle}{%
  \newpage
  \null
  \vskip 3em%
  \begin{center}%
  \let \footnote \thanks
%    {\LARGE \@title \par}%
    {\Large\bfseries \@title \par}%
    \vskip 1.5em%
%    {\large
    {\bfseries
      \lineskip .5em%
      \begin{tabular}[t]{c}%
        \@author
      \end{tabular}\par}%
    \vskip 1.0em%
%    {\large
    {\small\itshape
      \lineskip .5em%
      \begin{tabular}[t]{c}%
        \@address
      \end{tabular}\par}%
%    \vskip 1em%
%    {\large \@date}%
  \end{center}%
  \par
  \vskip 2.0em}
%}

% Change the format of the abstract
% Italicized, no indentation
% Two lines after end of abstract (6 ex)
\renewenvironment{abstract}{
  \itshape
  \setlength{\parskip}{3pt}  %Abstracts should be one paragraph; parskip for PACS
	 \setlength{\parindent}{0pt}  %No indentation for abstract
}{\par\vskip 6.0ex}

% Change the format of in-text citations
% Superscripted, no square brackets
% No space after comma
%\def\@citex[#1]#2{%
%  \let\@citea\@empty
%  \@cite{\@for\@citeb:=#2\do
%%    {\@citea\def\@citea{,\penalty\@m\ }%
%    {\@citea\def\@citea{,}%
%     \edef\@citeb{\expandafter\@firstofone\@citeb}%
%     \if@filesw\immediate\write\@auxout{\string\citation{\@citeb}}\fi
%     \@ifundefined{b@\@citeb}{\mbox{\reset@font\bfseries ?}%
%       \G@refundefinedtrue
%       \@latex@warning
%         {Citation `\@citeb' on page \thepage \space undefined}}%
%       {\hbox{\csname b@\@citeb\endcsname}}}}{#1}}
\newcount\@tempcntc
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout{\string\citation{#2}}\fi
 \@tempcnta\z@\@tempcntb\m@ne\def\@citea{}\@cite{\@for\@citeb:=#2\do
  {\@ifundefined
   {b@\@citeb}{\@citeo\@tempcntb\m@ne\@citea\def\@citea{,}{\bfseries ?}\@warning
   {Citation `\@citeb' on page \thepage \space undefined}}%
  {\setbox\z@\hbox{\global\@tempcntc0\csname b@\@citeb\endcsname\relax}%
   \ifnum\@tempcntc=\z@ \@citeo\@tempcntb\m@ne
    \@citea\def\@citea{,}\hbox{\csname b@\@citeb\endcsname}%
   \else
    \advance\@tempcntb\@ne
    \ifnum\@tempcntb=\@tempcntc
    \else\advance\@tempcntb\m@ne\@citeo
    \@tempcnta\@tempcntc\@tempcntb\@tempcntc\fi\fi}}\@citeo}{#1}}
\def\@citeo{\ifnum\@tempcnta>\@tempcntb\else\@citea\def\@citea{,}%
 \ifnum\@tempcnta=\@tempcntb\the\@tempcnta\else
  {\advance\@tempcnta\@ne\ifnum\@tempcnta=\@tempcntb \else \def\@citea{--}\fi
   \advance\@tempcnta\m@ne\the\@tempcnta\@citea\the\@tempcntb}\fi\fi}

\renewcommand{\@cite}[2]{\textsuperscript{#1\if@tempswa , #2\fi}}

% Add onlinecite command for non-superscripted citations DJM 1998/5/13

\DeclareRobustCommand\onlinecite{%
  \@ifnextchar [{\@tempswatrue\@onlinecitex}{\@tempswafalse\@onlinecitex[]}}

\def\@onlinecitex[#1]#2{\if@filesw\immediate\write\@auxout{\string\citation{#2}}\fi
 \@tempcnta\z@\@tempcntb\m@ne\def\@citea{}\@onlinecite{\@for\@citeb:=#2\do
  {\@ifundefined
   {b@\@citeb}{\@citeo\@tempcntb\m@ne\@citea\def\@citea{,}{\bfseries ?}\@warning
   {Citation `\@citeb' on page \thepage \space undefined}}%
  {\setbox\z@\hbox{\global\@tempcntc0\csname b@\@citeb\endcsname\relax}%
   \ifnum\@tempcntc=\z@ \@citeo\@tempcntb\m@ne
    \@citea\def\@citea{,}\hbox{\csname b@\@citeb\endcsname}%
   \else
    \advance\@tempcntb\@ne
    \ifnum\@tempcntb=\@tempcntc
    \else\advance\@tempcntb\m@ne\@citeo
    \@tempcnta\@tempcntc\@tempcntb\@tempcntc\fi\fi}}\@citeo}{#1}}
\def\@citeo{\ifnum\@tempcnta>\@tempcntb\else\@citea\def\@citea{,}%
 \ifnum\@tempcnta=\@tempcntb\the\@tempcnta\else
  {\advance\@tempcnta\@ne\ifnum\@tempcnta=\@tempcntb \else \def\@citea{--}\fi
   \advance\@tempcnta\m@ne\the\@tempcnta\@citea\the\@tempcntb}\fi\fi}

\newcommand{\@onlinecite}[2]{#1\if@tempswa , #2\fi}

% Change the format of the bibliography
\renewenvironment{thebibliography}[1]
     {\section*{\MakeUppercase\refname
        \@mkboth{\MakeUppercase\refname}{\MakeUppercase\refname}}%
%      \list{\@biblabel{\@arabic\c@enumiv}}%
%           {\settowidth\labelwidth{\@biblabel{#1}}%
      \small\list{\@arabic\c@enumiv .}  %No brackets around ref. number, small font
           {\settowidth\labelwidth{#1.}  %No brackets around ref. number
            \leftmargin\labelwidth
            \setlength{\parsep}{0pt}  %Don't want space between references
            \setlength{\itemsep}{0pt}  %Don't want space between references
            \advance\leftmargin\labelsep
            \@openbib@code
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand\theenumiv{\@arabic\c@enumiv}}%
      \sloppy\clubpenalty4000\widowpenalty4000%
      \sfcode`\.\@m}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist}

% Change the dimensions of the page
\setlength{\textwidth}{5in}
\setlength{\textheight}{564pt}
\setlength{\oddsidemargin}{0.875in}
\setlength{\evensidemargin}{0.875in}
\setlength{\topmargin}{31.5pt}
\setlength{\headheight}{12pt}
\setlength{\headsep}{12pt}
\setlength{\footskip}{30pt}

% Change paragraph lengths
% No space between paragraphs, 2em indentation
\setlength{\parskip}{0pt}
\setlength{\parindent}{2em}

% Change caption lengths
% Caption is closer to object
\setlength{\abovecaptionskip}{5\p@}

% Change caption format
% Separator is ., not :
% Caption also flush left
\renewcommand{\@makecaption}[2]{%
  \vskip\abovecaptionskip
%  \sbox\@tempboxa{#1: #2}%
  \sbox\@tempboxa{#1. #2}  %Change the separator
  \ifdim \wd\@tempboxa >\hsize
%    #1: #2\par
    #1. #2\par  %Change the separator
  \else
    \global \@minipagefalse
%    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
    \hb@xt@\hsize{\box\@tempboxa}  %Want the caption flushleft, not centered
  \fi
  \vskip\belowcaptionskip}

% Change figure name from Figure to Fig.
\renewcommand{\figurename}{Fig.}

% Change float separations
\setlength{\floatsep}{1.5\baselineskip plus 4pt minus 4pt}
\setlength{\textfloatsep}{\floatsep}
\setlength{\intextsep}{\floatsep}

% Change the headers and footers
% Headers are small, boldfaced, and centered, no page numbers
% Footers are empty
\renewcommand{\ps@myheadings}{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
%    \def\@evenhead{\thepage\hfil\slshape\leftmark}%
%    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \def\@evenhead{\hfil{\small\bfseries\leftmark}\hfil}%
    \def\@oddhead{\hfil{\small\bfseries\rightmark}\hfil}%
    \let\@mkboth\@gobbletwo
    \let\sectionmark\@gobble
    \let\subsectionmark\@gobble
    }

% Set page style to the above
\pagestyle{myheadings}

% No footnoterule
\renewcommand{\footnoterule}

% Define a running head command
\newcommand\runninghead[2]{\markboth{#1}{#2}}

