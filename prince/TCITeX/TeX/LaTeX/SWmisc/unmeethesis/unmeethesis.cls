%% 
%% This is file `unmeethesis.cls', generated 
%% on <1995/10/31> with the jwhhacks utility (v3.2zzz).
%% 
%% The original source files were:
%% 
%% unmeethesis.dtx 
%% 
%% 
%% This is a generated file. 
%% 
%% Copyright 1995 by James W. Howse IV. All rights reserved. 
%% Modified with permission by Neall E. Doren, January 27, 2000. 
%% Further mods as per D. Bader, EECE, by N. Doren, March 07, 2000.
%% 
%% This file is distributed in the hope that it will be useful, 
%% but WITHOUT ANY WARRANTY; without even the implied warranty of 
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
%% 
%% This file is a modified form of the standard book format book.cls.  It is
%% based on book.cls version 1.3g dated 1995/06/26.
%% Modification history:
%%   /9/17/01: Slight margin changes (top and bottom).  N. Doren
%%   /3/27/02: Slight margin changes for title boxes.  N. Doren
%% The names of the source files used are shown above. 
%% 
%% 
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
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{unmeethesis}
              [1995/10/31 
 UNM EECE dissertation class-mod 03/27/02]
\newcommand\@ptsize{}
\newif\if@restonecol
\newif\if@openright
\newif\if@openbib
\newif\if@draft
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Added an 'if' to check whether the 'draft' or 'final' option is 
%  selected
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newif\if@draft
\newif\if@nobox
\newif\if@botnum
\@openbibfalse
\newif\if@mainmatter \@mainmattertrue
\DeclareOption{a4paper}
   {\setlength\paperheight {297mm}%
    \setlength\paperwidth  {210mm}}
\DeclareOption{a5paper}
   {\setlength\paperheight {210mm}%
    \setlength\paperwidth  {148mm}}
\DeclareOption{b5paper}
   {\setlength\paperheight {250mm}%
    \setlength\paperwidth  {176mm}}
\DeclareOption{letterpaper}
   {\setlength\paperheight {10.92in}%
    \setlength\paperwidth  {8.5in}}
\DeclareOption{legalpaper}
   {\setlength\paperheight {14in}%
    \setlength\paperwidth  {8.5in}}
\DeclareOption{executivepaper}
   {\setlength\paperheight {10.5in}%
    \setlength\paperwidth  {7.25in}}
\DeclareOption{landscape}
   {\setlength\@tempdima   {\paperheight}%
    \setlength\paperheight {\paperwidth}%
    \setlength\paperwidth  {\@tempdima}}
\DeclareOption{10pt}{\renewcommand\@ptsize{0}}
\DeclareOption{11pt}{\renewcommand\@ptsize{1}}
\DeclareOption{12pt}{\renewcommand\@ptsize{2}}
\DeclareOption{oneside}{\@twosidefalse \@mparswitchfalse}
\DeclareOption{twoside}{\@twosidetrue  \@mparswitchtrue}
\DeclareOption{botnum}{\@botnumtrue}
\DeclareOption{draft}{\@drafttrue \setlength\overfullrule{5pt}}
\DeclareOption{final}{\@draftfalse \setlength\overfullrule{0pt}}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Removed 'notitlepage' option
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\DeclareOption{openright}{\@openrighttrue}
\DeclareOption{openany}{\@openrightfalse}
\DeclareOption{nobox}{\@noboxtrue}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Removed 'twocolumn' option
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\DeclareOption{leqno}{\input{leqno.clo}}
\DeclareOption{fleqn}{\input{fleqn.clo}}
\DeclareOption{openbib}{\@openbibtrue}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Changed the default option from final to draft
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\ExecuteOptions{letterpaper,12pt,oneside,draft,openright}
\ProcessOptions
   \if@botnum
      \setlength\paperheight {10.7in}%
   \fi
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  In order to generate the red box around the title pages, the 'color'
%  package MUST be loaded
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\if@draft
\else
  \RequirePackage{color}
\fi
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Changed the input file from {bk1\@ptsize.clo} since the bk??.clo 
%  files only support two-sided formatting
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\input{unm1\@ptsize.clo}
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Modified \baselinestretch to appear "double" spaced.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\renewcommand\baselinestretch{1.45}
\newlength\@chosenpointsize
\setlength\@chosenpointsize{1\@ptsize\p@}
\newlength\@scaledpointsize
\setlength\@scaledpointsize{\baselinestretch\@chosenpointsize}
\setlength\parskip{0.5\@scaledpointsize \@plus \p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\setcounter{topnumber}{2}
\renewcommand\topfraction{.7}
\setcounter{bottomnumber}{1}
\renewcommand\bottomfraction{.7}
\setcounter{totalnumber}{3}
\renewcommand\textfraction{0}
\renewcommand\floatpagefraction{1}
\setcounter{dbltopnumber}{2}
\renewcommand\dbltopfraction{.7}
\renewcommand\dblfloatpagefraction{.5}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Made the following changes to the default headings.
%
%  1)  The headings are no longer upper case.
%  2)  Removed the \MakeUppercase command from the 'tableofcontents',
%      'listoffigures', and 'listoftables' commands and from the
%      'thebibliography', and 'theindex' environments.
%  3)  Due to Graduate Studies requirements the frontmatter and the 
%      mainmatter have different headings.  Specifically, the frontmatter
%      has the page numbers at the bottom center of the page, and the 
%      mainmatter has the page numbers at the top right corner of the page.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\if@twoside
  \def\ps@mainheadings{%
   \if@botnum
      \def\@oddfoot{\hfil\thepage\hfil}\let\@evenfoot\@oddfoot
      \def\@evenhead{\hfil{\slshape\leftmark}}%
      \def\@oddhead{{\slshape\rightmark}\hfil}%
   \else
      \let\@oddfoot\@empty\let\@evenfoot\@empty
      \def\@evenhead{\thepage\hfil{\slshape\leftmark}}%
      \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
   \fi
      \let\@mkboth\markboth
    \def\chaptermark##1{%
      \markboth {%
        \ifnum \c@secnumdepth >\m@ne
          \if@mainmatter
             \@chapapp\ \thechapter. \ %
          \fi
        \fi
        ##1}{}}%
    \def\sectionmark##1{%
      \markright {%
        \ifnum \c@secnumdepth >\z@
          \thesection. \ %
        \fi
        ##1}}}
\else
  \def\ps@mainheadings{%
    \let\@oddfoot\@empty

   \if@botnum
      \def\@oddfoot{\hfil\thepage\hfil}
   \else
      \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
   \fi

%   \def\@evenhead{\thepage\hfil{\slshape\leftmark}}%
    \let\@mkboth\markboth
    \def\chaptermark##1{%
      \markright {%
       \ifnum \c@secnumdepth >\m@ne
          \if@mainmatter
           \@chapapp\ \thechapter. \ %
          \fi
       \fi
        ##1}}}
\fi

\if@twoside
  \def\ps@frontheadings{%

   \if@botnum
      \def\@oddhead{{\slshape\rightmark}\hfil}%
      \def\@evenhead{\hfil{\slshape\leftmark}}%
      \def\@oddfoot{\hfil\thepage\hfil}\let\@evenfoot\@oddfoot
   \else
      \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
      \def\@evenhead{\thepage\hfil{\slshape\leftmark}}%
      \let\@oddfoot\@empty
   \fi

%     \def\@evenhead{\hfil{\slshape\leftmark}}%
      \let\@mkboth\markboth
    \def\chaptermark##1{%
      \markboth {##1}{}}%
    \def\sectionmark##1{%
      \markright {##1}}}
\else
  \def\ps@frontheadings{%
    \let\@oddfoot\@empty
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%

   \if@botnum
      \def\@oddfoot{\hfil\thepage\hfil}
      \def\@oddhead{{\slshape\rightmark}\hfil}%
   \else
      \def\@evenhead{\thepage\hfil{\slshape\leftmark}}%
   \fi

      \let\@mkboth\markboth
    \def\chaptermark##1{%
      \markright {##1}}}
\fi

%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\def\ps@myheadings{%
    \let\@oddfoot\@empty\let\@evenfoot\@empty
    \def\@evenhead{\thepage\hfil\slshape\leftmark}%
    \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
    \let\@mkboth\@gobbletwo
    \let\chaptermark\@gobble
    \let\sectionmark\@gobble
    }
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
% comment out whole routine between colon separators for bottom page numbering
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
   \if@botnum
   \else
      \def\ps@plain{%
        \let\@oddfoot\@empty\let\@evenfoot\@empty
 %      \def\@evenhead{\thepage\hfil\slshape\leftmark}%
 %      \def\@oddhead{{\slshape\rightmark}\hfil\thepage}%
        \def\@oddhead{\hfil\thepage}%
        \let\@mkboth\@gobbletwo
        \let\chaptermark\@gobble
        \let\sectionmark\@gobble
        }
    \fi

%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
% This material was taken from fancybox.sty by Timothy Van Zandt.  It is all
% part of the definition for the command '\thisfancypage' which allows a box
% to be put around an entire page.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

\newbox\@fancybox
\@ifundefined{@tempdimc}{\newdimen\@tempdimc}{}

% \singlebox

\def\singlebox{\VerbBox\@singlebox}
\def\@singlebox#1{%
  \begingroup
    \setbox\@fancybox\hbox{{#1}}%
    \if@nobox
       \fboxrule=0pt%
    \else
       \fboxrule=2pt%
    \fi
    \if@draft
      \if@nobox
         \fbox{\hspace{-2pt} \box\@fancybox}%
      \else
         \fbox{\box\@fancybox}%
      \fi

    \else
      \fcolorbox{red}{white}{\box\@fancybox}%
    \fi
  \endgroup}

% \VerbBox

\newtoks\do@VerbBox

\def\VerbBox#1{%
  \do@VerbBox{#1}%
  \afterassignment\begin@VerbBox
  \setbox\@fancybox=\hbox}
\def\begin@VerbBox{\aftergroup\end@VerbBox}
\def\end@VerbBox{\the\do@VerbBox{\box\@fancybox}}%

% \fb@outputpage
% This is a modification of the version of \@outputpage copied from the file
% latex.ltx, stored in /usr/local/texmf/lib/tex/latex/misc.
        
\def\fb@outputpage{%
  \let \protect \noexpand
  \shipout \vbox{%
    \set@typeset@protect
    \aftergroup\set@typeset@protect
    \if@specialpage
      \global\@specialpagefalse\@nameuse{ps@\@specialstyle}%
    \fi
    \if@twoside
      \ifodd\count\z@ \let\@thehead\@oddhead \let\@thefoot\@oddfoot
          \let\@themargin\oddsidemargin
      \else \let\@thehead\@evenhead
         \let\@thefoot\@evenfoot \let\@themargin\evensidemargin
      \fi
    \fi
  \reset@font
  \normalsize
  \baselineskip\z@skip \lineskip\z@skip \lineskiplimit\z@
%    \@shipoutsetup
    \@begindvi
%
    \ifx\this@fancypage\@empty\else
      \this@fancypage
      \gdef\this@fancypage{}%
    \fi
    \ifx\the@fancypage\@empty\else
      \setbox\@outputbox\hbox{\the@fancypage{\box\@outputbox}}%
    \fi
%
    \setbox\@outputbox \vbox{%
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
%
    \ifx\the@@fancypage\@empty\else
      \setbox\@outputbox\hbox{\the@@fancypage{\box\@outputbox}}%
    \fi
%
    \vbox{%
%
%      \ifnum\fancyput@flag>-1 \do@fancyput\fi
%
      \vskip \topmargin
      \moveright\@themargin\box\@outputbox
    }%
  }%
  \global \@colht \textheight
  \stepcounter{page}%
  \let\firstmark\botmark
}

% \fancypage

\def\the@fancypage{}
\def\the@@fancypage{}
\def\this@fancypage{}

\def\fancypage#1#2{%
  \def\the@fancypage{#1}%
  \def\the@@fancypage{#2}%
  \def\@outputpage{\fb@outputpage}}

\def\thisfancypage#1#2{%
  \gdef\@outputpage{\fb@outputpage}%
  \gdef\this@fancypage{\def\the@fancypage{#1}\def\the@@fancypage{#2}}}

%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the title page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\def\title#1{\gdef\@title{#1}}
\def\author#1{\gdef\@author{#1}}
\def\previousdegrees#1{\gdef\@previousdegrees{#1}}
\def\date#1{\gdef\@date{#1}}
  \date{\today}
\def\degree#1{\gdef\@degree{#1}}
  \degree{Doctor of Philosophy \\ Engineering}
\def\degreesubject#1{\gdef\@degreesubject{#1}}
  \degreesubject{Ph.D., Electrical Engineering}
\def\documenttype#1{\gdef\@documenttype{#1}}
  \documenttype{Dissertation}
\def\schoolname#1{\gdef\@schoolname{#1}} 
  \schoolname{University of New Mexico}
\def\schooladdress#1{\gdef\@schooladdress{#1}} 
  \schooladdress{Albuquerque, New Mexico}
\def\degreetext#1{\gdef\@degreetext{#1}} 
  \degreetext{Submitted in Partial Fulfillment of the \\
    Requirements for the Degree of}

\newlength\@temptextheight
\setlength\@temptextheight{\textheight}
\newlength\@temptextwidth
\setlength\@temptextwidth{\textwidth}
\newlength\@temptopmargin
\setlength\@temptopmargin{\topmargin}
\newlength\@tempheadheight
\setlength\@tempheadheight{\headheight}
\newlength\@tempheadsep
\setlength\@tempheadsep{\headsep}
\newlength\@tempfootskip
\setlength\@tempfootskip{\footskip}

\newcommand\@resetpagesize{%
  \setlength{\topmargin}{\@temptopmargin}%
  \setlength{\headheight}{\@tempheadheight}%
  \setlength{\headsep}{\@tempheadsep}%
  \setlength{\footskip}{\@tempfootskip}%
  \setlength{\textheight}{\@temptextheight}%
  \setlength{\textwidth}{\@temptextwidth}%
}

\newcommand\maketitle{%
   \thisfancypage{}{%
     \setlength{\fboxsep}{-6pt}%
     \singlebox
}
   \@maketitle
   \clearpage%
   \mbox{\@resetpagesize}%
   \if@twoside%
     \thispagestyle{empty}%
     \mbox{}%
     \clearpage%
   \fi
}

\def\@maketitle{%
  \thispagestyle{empty}
%
  \setlength{\textheight}{648pt}
  \setlength{\textwidth}{427.5pt}
  \if@botnum
      \setlength{\topmargin}{-6pt}
  \else
      \setlength{\topmargin}{-4pt}
  \fi
 
  \setlength{\headheight}{21pt}
  \setlength{\headsep}{9pt}
  \if@botnum
     \setlength{\footskip}{40pt}
  \else
     \setlength{\footskip}{20pt}
  \fi
%
  \let\footnotesize\small
%  \let\footnoterule\relax
  \if@twoside%
    \setcounter{page}{1}%
  \else
    \setcounter{page}{2}%
  \fi
  \null\vfil%
  \begin{center}%
  \renewcommand\baselinestretch{0.75}
    {\LARGE\bf \@title}
    \par\vskip 60pt plus2pt minus30pt
    by
    \par\vskip 9pt
    {\large\bf \@author}
    \par\vskip 24pt
    {\large \@previousdegrees}
    %\par\vskip 120pt plus6pt minus35pt
    \par\vskip 80pt plus6pt minus35pt
    {\large \MakeUppercase{\@documenttype}}
    \par\vskip 12pt
    \@degreetext
    \par\vskip 12pt
    {\large \@degree}
    \par\vskip 6pt
    The \@schoolname \par
    \@schooladdress
    \par\vskip 6pt
    \@date
    \par
  \end{center}
  \vfil\null%
  \setcounter{footnote}{0}%
%  \if@twoside\begin{titlepage} \setcounter{page}{0}\mbox{} \end{titlepage}\fi
\let\maketitle\relax}%

%\newcount\@Signatures 
%\def\Signatures#1{\global\@Signatures=#1\relax}
%  \Signatures{3} % number of signature lines
\renewcommand\baselinestretch{1.45}

%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the copywrite page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand\makecopyright{%

  \thispagestyle{plain}
  \if@botnum
    \setlength{\headsep}{-50pt}
    \setlength{\topmargin}{3pt}
% extra:
    \setlength{\footskip}{58pt}
  \else
     \setlength{\topmargin}{-8pt}
  \fi
  \null\vfil%
  \begin{center}%
    \vspace{+0.56in}
    \copyright \thisyear, \hskip 0.5em \@author
  \end{center}%
  \vfil\vfil\null%
  \if@twoside%
    \clearpage%
    \thispagestyle{plain}%
    \mbox{}%
  \fi
\let\makecopyright\relax}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the dedication page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newenvironment{dedication}{%
  \clearpage
  \thispagestyle{plain}
  \if@botnum
    % \setlength{\headsep}{-42pt}
    % \setlength{\topmargin}{-6pt}
  \else
%     \setlength{\headheight}{-2pt}
%     \setlength{\topmargin}{-6pt}
  \fi
 \null\vfil%
  \begin{flushleft}
     \if@botnum
        \vspace{.82in} %change this if "Dedication" is too high/low
     \else
        \vspace{-.12in} %change this if "Dedication" is too high/low
     \fi 
     \Huge \bfseries Dedication
  \end{flushleft}
  \itshape
  \null\vfil%
  \begin{flushright}}%
{\end{flushright}\par\vfil\vfil\null% 
   \if@twoside%
     \clearpage%
     \thispagestyle{plain}%
     \mbox{}%
   \fi%
}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the 'singlespace' environment
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\def\singlespace{%
%  \vskip\parskip%
%  \vskip\baselineskip%
  \def\baselinestretch{1}%
  \ifx\@currsize\normalsize\@normalsize\else\@currsize\fi%
%  \vskip-\parskip%
%  \vskip-\baselineskip%
}

\def\endsinglespace{\null}%\par
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the acknowledgments page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newenvironment{acknowledgments}{%
  \if@botnum
     \setlength{\headsep}{-50pt}
  \else
     \setlength{\topmargin}{-6pt}
  \fi
  \clearpage
  \thispagestyle{plain}
  \singlespace
  \null\vfil%
  \begin{flushleft}%
    \vspace{+0.84in}
    \Huge \bfseries \acknowledgname
    \@endparpenalty\@M
  \end{flushleft}}%
{\endsinglespace\vfil\vfil\null  
   \if@twoside%
     \clearpage%
     \thispagestyle{plain}%
     \mbox{}%
   \fi
}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the abstract title page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand\maketitleabstract{%
   \clearpage
   \thisfancypage{}{%
     \setlength{\fboxsep}{-6pt}%
     \singlebox
}
   \@maketitleabstract
%   \if@twoside\begin{titlepage}\mbox{}\end{titlepage}\fi
   \clearpage
   \@resetpagesize
                    % \addcontentsline{toc}{chapter}{Abstract}%
   \if@twoside%
     \thispagestyle{empty}%
     \mbox{}%
     \clearpage%
   \fi
}

\newcommand\@maketitleabstract{%
  \thispagestyle{empty}
%
  \setlength{\textheight}{614pt}
  \setlength{\textwidth}{427.5pt}
  \setlength{\topmargin}{-4pt}
  \setlength{\headheight}{4pt}
  \setlength{\headsep}{4pt}
  \setlength{\footskip}{4pt}
%
  \let\footnotesize\small
%  \let\footnoterule\relax
  \null\vfil%
  \begin{center}%
    {\LARGE\bf \@title}
    \par\vskip 60pt plus2pt minus30pt
    by
    \par\vskip 9pt
    {\large\bf \@author}
    \par\vskip 20pt plus6pt minus51pt
    {\large \MakeUppercase{\abstractname\ of \@documenttype}}
    \par\vskip 12pt
    \@degreetext
    \par\vskip 12pt
    {\large \@degree}
    \par\vskip 6pt
    The \@schoolname \par
    \@schooladdress
    \par\vskip 6pt
    \@date
    \par
  \end{center}
   \vfil\null%
%\if@twoside\begin{titlepage} \setcounter{page}{0}\mbox{} \end{titlepage}\fi
\let\maketitleabstract\relax}%
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  The following commands define the abstract page
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newenvironment{abstract}{%
  \if@botnum
     \setlength{\headsep}{07pt}
  \else
     \setlength{\headsep}{22pt}
  \fi
  \if@twoside
    \setlength{\headsep}{25pt}
   \fi
  \thispagestyle{plain}
%  \null\vfil%
   \null%
  \begin{center}%
    {\LARGE\bf \@title}
    \par\vskip 20pt plus2pt minus9pt
    by
    \par\vskip 6pt
    {\large\bf \@author}
    \par\vskip 12pt
    {\large \@previousdegrees}
    \par
    {\large \@degreesubject, \@schoolname, \thisyear}
    \par\vskip 36pt
    \large \bfseries \abstractname
    \@endparpenalty\@M
  \end{center}}%

{\vfill}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand*\chaptermark[1]{}
\setcounter{secnumdepth}{2}
\newcounter {part}
\newcounter {chapter}
\newcounter {section}[chapter]
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]
\renewcommand\thepart         {\@Roman\c@part}
\renewcommand\thechapter      {\@arabic\c@chapter}
\renewcommand\thesection      {\thechapter.\@arabic\c@section}
\renewcommand\thesubsection   {\thesection.\@arabic\c@subsection}
\renewcommand\thesubsubsection{\thesubsection .\@arabic\c@subsubsection}
\renewcommand\theparagraph    {\thesubsubsection.\@arabic\c@paragraph}
\renewcommand\thesubparagraph {\theparagraph.\@arabic\c@subparagraph}
\newcommand\@chapapp{\chaptername}
\newcommand\frontmatter{\cleardoublepage
           \@mainmatterfalse\pagestyle{frontheadings}\pagenumbering{roman}}
\newcommand\mainmatter{\cleardoublepage
       \@mainmattertrue\pagestyle{mainheadings}\pagenumbering{arabic}}
\newcommand\backmatter{\if@openright\cleardoublepage\else\clearpage\fi
      \@mainmatterfalse}
\newcommand\part{\cleardoublepage
                 \thispagestyle{plain}%
                 \hbox{}\vfil
                 \secdef\@part\@spart}

\def\@part[#1]#2{%
    \ifnum \c@secnumdepth >-2\relax
      \refstepcounter{part}%
      \addcontentsline{toc}{part}{\thepart\hspace{1em}#1}%
    \else
      \addcontentsline{toc}{part}{#1}%
    \fi
    \markboth{}{}
    {\centering
     \interlinepenalty \@M
     \normalfont
     \ifnum \c@secnumdepth >-2\relax
       \huge\bfseries \partname~\thepart
       \par
       \vskip 20\p@
     \fi
     \Huge \bfseries #2\par}%
    \@endpart}
\def\@spart#1{%
    {\centering
     \interlinepenalty \@M
     \normalfont
     \Huge \bfseries #1\par}%
    \@endpart}
\def\@endpart{\vfil\newpage
              \if@twoside
                \hbox{}%
                \thispagestyle{plain}%
                \newpage
              \fi}
\newcommand\chapter{\if@openright\cleardoublepage\else\clearpage\fi
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Modified from \thispagestyle{plain} in order to conform to 
%  specifications in the dissertation submission guidelines.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                    \thispagestyle{plain}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@chapter\@schapter}
\def\@chapter[#1]#2{\ifnum \c@secnumdepth >\m@ne
                       \if@mainmatter
                         \refstepcounter{chapter}%
                         \typeout{\@chapapp\space\thechapter.}%
                         \addcontentsline{toc}{chapter}%
                                   {\protect\numberline{\thechapter}#1}%
                       \else
                         \addcontentsline{toc}{chapter}{#1}%
                       \fi
                    \else
                      \addcontentsline{toc}{chapter}{#1}%
                    \fi
                    \chaptermark{#1}%
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \@makechapterhead{#2}%
                    \@afterheading}
\def\@makechapterhead#1{%
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Changed spacing from 50pt to 40pt
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%\setlength{\textheight}{-11pt}
%\begin{flushright}
%  \raisebox{0in}{\thepage} 
%\end{flushright}
  \vspace*{46\p@}%
  {\parindent \z@ \raggedright \normalfont
    \ifnum \c@secnumdepth >\m@ne
      \if@mainmatter
        \huge\bfseries \@chapapp{} \thechapter
        \par\nobreak
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Changed spacing from 20pt to 15pt
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        \vskip 15\p@
      \fi
    \fi
    \interlinepenalty\@M
    \Huge \bfseries #1\par\nobreak
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Changed spacing from 40pt to 35pt
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    \vskip 45\p@
  }}
\def\@schapter#1{\@makeschapterhead{#1}%
                 \@afterheading}
\def\@makeschapterhead#1{%
  \vspace*{50\p@}%
  {\parindent \z@ \raggedright
    \normalfont
    \interlinepenalty\@M
    \Huge \bfseries  #1\par\nobreak
    \vskip 40\p@
  }}
\newcommand\section{\@startsection {section}{1}{\z@}%
                                   {-3.5ex \@plus -1ex \@minus -.2ex}%
                                   {2.3ex \@plus.2ex}%
                                   {\normalfont\Large\bfseries}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                                     {\normalfont\large\bfseries}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                                     {\normalfont\normalsize\bfseries}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
                                    {3.25ex \@plus1ex \@minus.2ex}%
                                    {-1em}%
                                    {\normalfont\normalsize\bfseries}}
\newcommand\subparagraph{\@startsection{subparagraph}{5}{\parindent}%
                                       {3.25ex \@plus1ex \@minus .2ex}%
                                       {-1em}%
                                      {\normalfont\normalsize\bfseries}}
\setlength\leftmargini  {2.5em}
\setlength\leftmarginii  {2.2em}
\setlength\leftmarginiii {1.87em}
\setlength\leftmarginiv  {1.7em}
\setlength\leftmarginv  {1em}
\setlength\leftmarginvi {1em}
\setlength\leftmargin    {\leftmargini}
\setlength  \labelsep  {.5em}
\setlength  \labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
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
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
                                \normalfont\bfseries #1}
\newenvironment{verse}
               {\let\\=\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -1.5em%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 1.5em}%
                \item[]}
               {\endlist}
\newenvironment{quotation}
               {\list{}{\listparindent 1.5em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item[]}
               {\endlist}
\newenvironment{quote}
               {\list{}{\rightmargin\leftmargin}%
                \item[]}
               {\endlist}
\newenvironment{titlepage}
    {%
      \cleardoublepage
      \newpage
      \thispagestyle{empty}%
      \if@compatibility
        \setcounter{page}{0}
      \fi}%
    {\newpage}
\newcommand\appendix{\par
  \setcounter{chapter}{0}%
  \setcounter{section}{0}%
  \renewcommand\@chapapp{\appendixname}%
  \renewcommand\thechapter{\@Alph\c@chapter}}
\setlength\arraycolsep{5\p@}
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
\@addtoreset{equation}{chapter}
\renewcommand\theequation{\thechapter.\@arabic\c@equation}
\newcounter{figure}[chapter]
\renewcommand\thefigure{\thechapter.\@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figurename~\thefigure}
\newenvironment{figure}
               {\singlespace\@float{figure}}
               {\endsinglespace\end@float}
\newenvironment{figure*}
               {\singlespace\@dblfloat{figure}}
               {\endsinglespace\end@dblfloat}
\newcounter{table}[chapter]
\renewcommand\thetable{\thechapter.\@arabic\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{\tablename~\thetable}
\newenvironment{table}
               {\singlespace\@float{table}}
               {\endsinglespace\end@float}
\newenvironment{table*}
               {\singlespace\@dblfloat{table}}
               {\endsinglespace\end@dblfloat}
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10\p@}
\setlength\belowcaptionskip{0\p@}
\long\def\@makecaption#1#2{%
  \vskip\abovecaptionskip
  \sbox\@tempboxa{#1: #2}%
  \ifdim \wd\@tempboxa >\hsize
    #1: #2\par
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand*\mit{\@fontswitch{\relax}{\mathnormal}}
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
\setcounter{tocdepth}{2}
\newcommand\tableofcontents{%
    \chapter*{\contentsname
        \@mkboth{%
           \contentsname}{\contentsname}}%
        \thispagestyle{plain}
    \@starttoc{toc}%
    }
\newcommand*\l@part[2]{%
  \ifnum \c@tocdepth >-2\relax
    \addpenalty{-\@highpenalty}%
    \addvspace{2.25em \@plus\p@}%
    \begingroup
      \setlength\@tempdima{3em}%
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      {\leavevmode
       \large \bfseries #1\hfil \hb@xt@\@pnumwidth{\hss #2}}\par
       \nobreak
         \global\@nobreaktrue
         \everypar{\global\@nobreakfalse\everypar{}}
    \endgroup
  \fi}
\newcommand*\l@chapter[2]{%
  \ifnum \c@tocdepth >\m@ne
    \addpenalty{-\@highpenalty}%
    \vskip 1.0em \@plus\p@
    \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode \bfseries
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      #1\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
      \penalty\@highpenalty
    \endgroup
  \fi}
\newcommand*\l@section{\@dottedtocline{1}{1.5em}{2.3em}}
\newcommand*\l@subsection{\@dottedtocline{2}{3.8em}{3.2em}}
\newcommand*\l@subsubsection{\@dottedtocline{3}{7.0em}{4.1em}}
\newcommand*\l@paragraph{\@dottedtocline{4}{10em}{5em}}
\newcommand*\l@subparagraph{\@dottedtocline{5}{12em}{6em}}
\newcommand\listoffigures{%
    \chapter*{\listfigurename
                      \addcontentsline{toc}{chapter}{List of Figures}%
      \@mkboth{\listfigurename}%
              {\listfigurename}}%
      \thispagestyle{plain}
    \@starttoc{lof}%
    }
\newcommand*\l@figure{\@dottedtocline{2}{1.5em}{2.8em}}
\newcommand\listoftables{%
    \chapter*{\listtablename
                      \addcontentsline{toc}{chapter}{List of Tables}%
      \@mkboth{%
          \listtablename}{\listtablename}}%
      \thispagestyle{plain}
    \@starttoc{lot}%
    }
\let\l@table\l@figure
\newdimen\bibindent
\bibindent=1.5em
\newenvironment{thebibliography}[1]
     {\chapter*{\bibname
        \@mkboth{\bibname}{\bibname}}%
                      \addcontentsline{toc}{chapter}{References}%
      \list{\@biblabel{\@arabic\c@enumiv}}%
           {\settowidth\labelwidth{\@biblabel{#1}}%
            \leftmargin\labelwidth
            \advance\leftmargin\labelsep
            \if@openbib
              \advance\leftmargin\bibindent
              \itemindent -\bibindent
              \listparindent \itemindent
              \parsep \z@
            \fi
% remove next line if double-spacing the bibliography.
% Single spacing is allowed for more than 60 references.
\singlespace
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand\theenumiv{\@arabic\c@enumiv}}%
      \if@openbib
        \renewcommand\newblock{\par}%
      \else
        \renewcommand\newblock{\hskip .11em \@plus.33em \@minus.07em}%
      \fi
      \sloppy\clubpenalty4000\widowpenalty4000%
      \sfcode`\.=\@m}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist}
\newcommand\newblock{}
\newenvironment{theindex}
               {\columnseprule \z@
                \columnsep 35\p@
                \twocolumn[\@makeschapterhead{\indexname}]%
                \@mkboth{\indexname}%
                        {\indexname}%
                \thispagestyle{plain}\parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
               {\clearpage}
\newcommand\@idxitem{\par\hangindent 40\p@}
\newcommand\subitem{\par\hangindent 40\p@ \hspace*{20\p@}}
\newcommand\subsubitem{\par\hangindent 40\p@ \hspace*{30\p@}}
\newcommand\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Redefined the environment 'glossary'
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\renewenvironment{glossary}[1]%
     {\chapter*{\glossaryname
                      \addcontentsline{toc}{chapter}{Glossary}%
        \@mkboth{\glossaryname}{\glossaryname}}%
     \thispagestyle{plain}
     \list{}{\let\makelabel\glossarylabel%
       \settowidth\labelwidth{#1}%
       \labelsep     2\labelsep%
       \itemindent   0in%
       \leftmargin   \labelwidth%
       \advance\leftmargin\labelsep}}
     {\endlist}
\newcommand*\glossarylabel[1]{#1\hfil}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\renewcommand\footnoterule{%
  \kern-3\p@
  \hrule\@width.4\columnwidth
  \kern2.6\p@}
\@addtoreset{footnote}{chapter}
\long\def\@makefntext#1{%
    \singlespace
    \parindent 1em%
    \noindent
    \hb@xt@1.8em{\hss\@makefnmark}#1\endsinglespace}
\newcommand\contentsname{Contents}
\newcommand\listfigurename{List of Figures}
\newcommand\listtablename{List of Tables}
\newcommand\bibname{References}
\newcommand\indexname{Index}
\newcommand\figurename{Figure}
\newcommand\tablename{Table}
\newcommand\partname{Part}
\newcommand\chaptername{Chapter}
\newcommand\appendixname{Appendix}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Defined new commands \abstractname, \acknowledgname and \glossaryname.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand\abstractname{Abstract}
\newcommand\acknowledgname{Acknowledgments}
\newcommand\glossaryname{Glossary}
\newcommand\symbolsname{Symbols}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Modified \today to return just the month and year.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  , \number\year}
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
%  Defined new command \thisyear.
%::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
\newcommand\thisyear{\number\year}
\setlength\columnsep{10\p@}
\setlength\columnseprule{0\p@}
\pagestyle{mainheadings}
\pagenumbering{arabic}
\onecolumn
\endinput
%% 
%% End of file `unmeethesis.cls'.
