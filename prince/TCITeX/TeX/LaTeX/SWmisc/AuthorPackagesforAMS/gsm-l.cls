%%
%% This is file `gsm-l.cls',
%% generated with the docstrip utility.
%%
%% The original source files were:
%%
%% gsm-l.dtx 
%% 
%%% ====================================================================
%%% @LaTeX-file{
%%%   filename  = "gsm-l.dtx",
%%%   version   = "2.05",
%%%   date      = "2008/09/26",
%%%   time      = "14:17:49 EDT",
%%%   checksum  = "41839 657 2064 21049",
%%%   author    = "American Mathematical Society",
%%%   copyright = "Copyright (C) 1995,2001 American Mathematical Society,
%%%                all rights reserved.  Copying of this file is
%%%                authorized only if either:
%%%                (1) you make absolutely no changes to your copy,
%%%                including name; OR
%%%                (2) if you do make changes, you first rename it
%%%                to some other name.",
%%%   address   = "American Mathematical Society,
%%%                Technical Support,
%%%                Publications Technical Group,
%%%                P. O. Box 6248,
%%%                Providence, RI 02940,
%%%                USA",
%%%   telephone = "401-455-4080 or (in the USA and Canada)
%%%                800-321-4AMS (321-4267)",
%%%   FAX       = "401-331-3842",
%%%   email     = "tech-support@ams.org (Internet)",
%%%   codetable = "ISO/ASCII",
%%%   keywords  = "",
%%%   supported = "yes",
%%%   abstract  = "This is the source for the document class for the
%%%                AMS book series Graduate Studies in Mathematics.
%%%                This is designed for use with LaTeX.",
%%%   docstring = "The checksum field above contains a CRC-16
%%%                checksum as the first value, followed by the
%%%                equivalent of the standard UNIX wc (word
%%%                count) utility output of lines, words, and
%%%                characters.  This is produced by Robert
%%%                Solovay's checksum utility.",
%%% }
%%% ====================================================================
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{gsm-l}[2008/09/26 v2.05 GSM Author Class]
\providecommand\@classname{gsm-l}
\newcommand\DelayedOption[2]{\@ifclasswith{\@classname}{#1}{#2}{}}
\DeclareOption{theorema}{}
\DeclareOption{theoremb}{}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{amsbook}}
\ExecuteOptions{theorema}
\ProcessOptions\relax
\LoadClass{amsbook}[1996/10/24]
\parskip 3\p@\relax
\toks@\@xp{\@listi}%
\edef\@listi{\the\toks@ \itemsep1\parskip plus\p@\relax}
\let\@listI\@listi \@listI
\def\@mainsize{11}\def\@ptsize{1}%
\def\@typesizes{%
  \or{6}{7}\or{7}{8}\or{8}{10}\or{9}{11}\or{10}{12}%
  \or{\@xipt}{13.5}% normalsize
  \or{\@xiipt}{14}\or{\@xivpt}{17}\or{\@xviipt}{20}%
  \or{\@xxpt}{24}\or{\@xxvpt}{30}}%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\@ifundefined{cleardouble@page}{%
  \let\cleardouble@page\cleardoublepage
  \def\cleardoublepage{\clearpage{\pagestyle{empty}\cleardouble@page}}%
}{}
\def\@part[#1]#2{%
  \begingroup \Huge\bfseries
  \hsize24pc \raggedright
  \ifnum \c@secnumdepth >-2\relax
    \refstepcounter{part}%
    {\LARGE\normalfont\itshape
      \top@space{18pc plus-1fil}% cancel \null\vfil done by \part
      \partname\ \thepart\par
    }%
    \bb@space{48pt}%
    \ignorespaces#2\unskip
    \addcontentsline{toc}{part}{%
      \partname\ \thepart.\protect\enspace\protect\noindent#1}%
  \else
    \ignorespaces#2\unskip
    \addcontentsline{toc}{part}{#1}%
  \fi
  \par
  \endgroup
  \thispagestyle{plain}%
  \newpage
  \thispagestyle{empty}}

\def\@spart#1{%
  \begingroup \c@secnumdepth=-2 \@part[#1]{#1}\endgroup
}
\providecommand\bb@space[1]{%
  \skip@#1\advance\skip@-\baselineskip\vskip\skip@
}
\providecommand\top@space[1]{%
  \par
  \ifdim\pagegoal<\maxdimen \else\hbox{}\fi
  \skip@#1\advance\skip@-\topskip
  \advance\skip@-\headsep\advance\skip@-\headheight
  \advance\skip@-\baselineskip\vskip\skip@
}
\def\chapter{\cleardoublepage\thispagestyle{plain}\global\@topnum\z@
  \@afterindentfalse \secdef\@chapter\@schapter}
\def\chapter@number{%
  \ifnum\c@secnumdepth>\m@ne
    \vskip-\headsep\vskip5pt
    \vbox{\LARGE\normalfont\itshape
      \hbox to\textwidth{\hss\rule{8pc}{6pt}}%
      \hbox to\textwidth{\hss\chaptername\space\thechapter}%
    }%
  \fi
}
\providecommand{\indexintro}{}
\newif\if@index  \@indexfalse
\def\@makechapterhead#1{\global\topskip\normaltopskip
  \begingroup
  \vbox to\topskip{%
    \chapter@number
    \vss
  }\penalty\@M
  \Huge\bfseries \hsize24pc \raggedright
  \top@space{11pc}%
  \noindent\ignorespaces #1\par \endgroup
  \if@index
    \ifx\@empty\indexintro
      \bb@space{7.5pc}%
    \else
      \vspace{3pc}%
      \begingroup \small
        \parbox[t]{27pc}{\leftskip3pc\normalfont\indexintro\par}%
      \endgroup
      \bb@space{3pc}%
    \fi
  \else
      \@dropfolio
      \bb@space{7.5pc}%
  \fi
  \@afterheading
}
\def\@makeschapterhead#1{\begingroup
  \c@secnumdepth\m@ne \@makechapterhead{#1}%
  \endgroup
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\section{\@startsection{section}{1}%
  \z@{-.7\linespacing\@plus-\linespacing}{.5\linespacing}%
  {\large\normalfont\bfseries \raggedright}}
\def\specialsection{\@startsection{section}{1}%
  \z@{-2\linespacing\@plus-\linespacing}{.5\linespacing}%
  {\Large\normalfont\bfseries \raggedright}}
\def\subsection{\@startsection{subsection}{2}%
  \z@{.5\linespacing\@plus.7\linespacing}{-.5em}%
  {\normalfont\bfseries}}
\def\subsubsection{\@startsection{subsubsection}{3}%
  \z@\z@{-.5em}%
  {\normalfont\itshape}}
\renewenvironment{xcb}{\par
  \ifdim\lastskip>\z@\vskip-\lastskip\fi
  \penalty-9999
  \vskip18\p@ plus2\p@ \leaders\hrule width\columnwidth\vskip.5\p@\relax
  \settowidth{\leftmargini}{00.\hskip\labelsep}%
  \settowidth{\leftmarginii}{(d.)\hskip\labelsep}%
  \@startsection{section}% counter name; ignored because of the * below
  {1}% sectioning level
  {\z@}% indent to the left of the title
  {-1sp}% vertical space above (faked, see leaders above)
  {20\p@}% Space below
  {\Large\bfseries % The font of the title
   \raggedright\leftskip\columnwidth \advance\leftskip-12pc
   \xcb@rule}%
  *% always unnumbered
}{%
  \par
}
\def\xcb@rule{\par\nointerlineskip%
  {\parskip\z@skip\noindent}\rule{12pc}{4pt}\break\ignorespaces}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\th@plain{%
  \let\thm@indent\noindent
  \thm@headfont{\bfseries}%
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@space@setup
  \itshape
}
\def\th@definition{%
  \let\thm@indent\noindent
  \thm@headfont{\bfseries}%
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@space@setup
  \upshape
}
\def\th@remark{%
  \let\thm@indent\noindent
  \thm@headfont{\bfseries}% heading font small caps
  \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
  \thm@space@setup
  \upshape
}
\@ifundefined{swappedhead@plain}{%
  \let\swappedhead@plain=\swappedhead
}{}
\@ifundefined{pushQED}{%
  \let\pushQED\@gobble
  \def\popQED{\qed}%
}{}
\renewenvironment{proof}[1][\proofname]{\par
  \pushQED{\qed}\relax
  \normalfont \topsep6\p@\@plus6\p@ \relax
  \trivlist
  \itemindent\z@
  \item[\hskip\labelsep
        \bfseries
    #1\@addpunct{.}]\ignorespaces
}{%
  \popQED\endtrivlist\@endpefalse
}
\DelayedOption{theoremb}{%
  \def\th@plain{%
    \let\thm@indent\indent
    \thm@headfont{\scshape}%
    \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
    \thm@space@setup
    \itshape
  }%
  %
  \def\th@definition{%
    \let\thm@indent\indent
    \thm@headfont{\scshape}%
    \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
    \thm@space@setup
    \upshape
  }%
  %
  \def\th@remark{%
    \let\thm@indent\indent
    \thm@headfont{\itshape}%
    \let\thmhead\thmhead@plain \let\swappedhead\swappedhead@plain
    \thm@space@setup
    \upshape
  }%
  %
  \renewenvironment{proof}[1][\proofname]{\par
    \pushQED{\qed}\relax
    \normalfont \topsep6\p@\@plus6\p@\relax
    \trivlist
    \itemindent\normalparindent
    \item[\hskip\labelsep
          \itshape
      #1\@addpunct{.}]\ignorespaces
  }{%
    \popQED\endtrivlist\@endpefalse
  }%
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\renewenvironment{thebibliography}[1]{%
  \@xp\chapter\@xp*\@xp{\bibname}%
  \normalfont\smaller[2]\labelsep .5em\relax
  \renewcommand\theenumiv{\arabic{enumiv}}\let\p@enumiv\@empty
  \list{\@biblabel{\theenumiv}}{\settowidth\labelwidth{\@biblabel{#1}}%
    \leftmargin\labelwidth \advance\leftmargin\labelsep
    \itemsep3\p@ plus\p@ minus\p@
    \usecounter{enumiv}}%
  \sloppy \clubpenalty\@M \widowpenalty\clubpenalty
  \sfcode`\.=\@m
}{%
  \def\@noitemerr{\@latex@warning{Empty `thebibliography' environment}}%
  \endlist
}
\AtBeginDocument{%
  \def\indexchap#1{%
    \twocolumn[%
      \vskip-\topskip\vskip-\baselineskip % compensate for twocolumn fudging
      \c@secnumdepth\m@ne
      \@indextrue
      \@makechapterhead{#1}]%
    \@dropfolio
  }%
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\ps@headings{\ps@empty
  \def\@evenhead{\rh@underline{\unskip
    \rlap{\thepage}\hfil {\slshape\leftmark{}{}}}}%
  \def\@oddhead{\rh@underline{\unskip
    {\slshape\rightmark{}{}}\hfil \llap{\thepage}}}%
  \let\@mkboth\markboth
  \def\partmark{\@secmark\markboth\partrunhead\partname}%
  \def\chaptermark{%
    \@secmark\markboth\chapterrunhead{}}%
  \def\sectionmark{%
    \@secmark\markright\sectionrunhead\sectionname}%
}
\def\partrunhead#1#2#3{%
  \@ifnotempty{#2}{{\@ifnotempty{#1}{#1 }#2}\@ifnotempty{#3}{. }}#3}
\let\chapterrunhead\partrunhead
\let\sectionrunhead\partrunhead
\def\ps@plain{\ps@empty
  \def\@oddhead{\rh@underline{%
    \vrule width\z@ height\headheight}}%
  \let\@evenhead\@oddhead
}
\def\rh@underline#1{\vbox to\headheight{%
  \hbox to\textwidth{\hskip\z@\@plus\textwidth\relax#1}%
  \baselineskip5\p@
  \hbox to\textwidth{\leaders \hrule\@height.5\p@\hfil}%
  \vss
}}
\headsep3pc \advance\headsep5.5pt \advance\headsep-\topskip
\newinsert\copyins
\skip\copyins=2pc
\count\copyins=999 % magnification factor, 1000 = 100%
\dimen\copyins=.5\textheight % maximum allowed per page
\def\@dropfolio{%
  \ifnum\count\copyins<\@m
    \global\count\copyins=\@m % reset to 999 in output routine
    \insert\copyins{\@parboxrestore
      \hsize\textwidth \raggedleft \normalfont\normalsize \everypar{}%
      \vss\nointerlineskip
      \vbox{\vskip-\skip\copyins
        \baselineskip=11\p@\relax % instead of 13.5
        \ialign{\hfil##\cr
          \vrule width\z@ height\skip\copyins\relax
          \rule{18\p@}{3\p@}\cr\thepage\cr}}%
    }%
  \fi
}
\def\@combinefloats{%
  \ifx \@toplist\@empty \else \@cflt \fi
  \ifx \@botlist\@empty \else \@cflb \fi
  \ifvoid\copyins \else \@cflci \fi
}
\def\@cflci{%
  \setbox\@outputbox\vbox{%
    \unvbox\@outputbox
    \vskip\skip\copyins
    \if@twocolumn \else \vskip\z@ plus\p@ \fi
    \hbox to\columnwidth{%
      \hss\vbox to\z@{\vss
        \if@twocolumn
          \if@firstcolumn \else \unvbox\copyins \fi
        \else
          \unvbox\copyins
        \fi
        }}}%
  \if@twocolumn \if@firstcolumn
    \insert\copyins{\unvbox\copyins}%
  \fi\fi
  \global\count\copyins=999 \relax
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\@captionheadfont{\bfseries}
\def\@captionfont{\normalfont\Small}
\long\def\@footnotetext#1{\insert\footins{%
  \normalfont\footnotesize
  \interlinepenalty\interfootnotelinepenalty
  \splittopskip\footnotesep \splitmaxdepth \dp\strutbox
  \floatingpenalty\@MM \hsize\columnwidth
  \@parboxrestore \parindent\normalparindent \sloppy
  \edef\@currentlabel{\p@footnote\@thefnmark}%
  \@makefntext{\rule\z@\footnotesep\ignorespaces#1\unskip\strut\par}}}
\def\footnotesize{\SMALL}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\l@part#1#2{\@tocline{-1}{12pt plus2pt}{0pt}{}{\bfseries}{#1}{}}
\def\l@section{\@tocline{1}{3\p@\@plus\p@\@minus\p@}{1pc}{}{}}
\renewcommand{\tocsection}[3]{%
  \indentlabel{\@ifnotempty{#2}{\S#2.\quad}}#3}
\pagestyle{headings}
\textheight=50pc \advance\textheight 9.5\p@
\textwidth=30pc \calclayout
\endinput %
\endinput
%%
%% End of file `gsm-l.cls'.
