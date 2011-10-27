%%% ====================================================================
%%% @LaTeX-file{
%%%    filename  = "memo-l.cls",
%%%    version   = "2.01",
%%%    date      = "2007/05/09",
%%%    time      = "13:18:16 EDT",
%%%    checksum  = "44382 126 314 4026",
%%%    author    = "Barbara Beeton",
%%%    address   = "American Mathematical Society, PO Box 6248,
%%%                       Providence, RI 02940, USA",
%%%    supported = "yes",
%%%    email     = "tech-support@ams.org (Internet)",
%%%    abstract  = "This is a LaTeX documentclass for the Memoirs of
%%%                 the American Mathematical Society.",
%%%    docstring = "The checksum field above contains a CRC-16 checksum
%%%                 as the first value, followed by the equivalent of
%%%                 the standard UNIX wc (word count) utility output
%%%                 of lines, words, and characters.  This is produced
%%%                 by Robert Solovay's checksum utility.",
%%% }
%%% ====================================================================

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{memo-l}
              [2007/05/09 v2.01 MEMO Author Class]

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{amsbook}}
\ProcessOptions\relax

\AtBeginDocument{%
  \renewcommand\mainmatter{\cleardoublepage\pagenumbering{arabic}}%
  \renewcommand\backmatter{\if@openright\cleardoublepage\else\clearpage\fi
    \let\chaptername\relax}%
  \renewcommand\chapter{\if@openright\cleardoublepage\else\clearpage\fi
    \thispagestyle{plain}\global\@topnum\z@
    \@afterindenttrue \secdef\@chapter\@schapter}%
  }

\LoadClass{amsbook}[1996/10/24]

%  Footnotes for \thanks, \subjclass and \keywords go on a separate
%  page with the abstract.  Assume that the abstract is more than one
%  line long if it was input before \maketitle.
\def\maketitle{\par
  \@topnum\z@ % this prevents figures from falling at the top of page 1
  \begingroup
  \@maketitle
  \endgroup
  \def\do##1{\let##1\relax}%
  \do\maketitle \do\@maketitle \do\title \do\@xtitle \do\@title
  \do\author \do\@xauthor \do\address \do\@xaddress
  \do\email \do\@xemail \do\curraddr \do\@xcurraddr
  \do\dedicatory \do\@dedicatory
}
%  Add abstract after table of contents
\def\tableofcontents{%
  \@starttoc{toc}\contentsname
  \@setabstract
}
\def\@setabstract{%
  \ifdim\dp\abstractbox>\z@
    {\ifdim\ht\abstractbox<38pc \@openrightfalse \fi
     \def\@tocwriteb##1##2##3{}%
     \chapter*{Abstract}
     \@maketopfootnotes
     \noindent\unvbox\abstractbox
    }%
  \fi
  \c@footnote\z@
}
%
\def\@maketopfootnotes{%
  {\let\@makefnmark\relax  \let\@thefnmark\relax
   \ifx\@empty\@date\else \@footnotetext{\@setdate}\fi
   \ifx\@empty\@subjclass\else \@footnotetext{\@setsubjclass}\fi
   \ifx\@empty\@keywords\else \@footnotetext{\@setkeywords}\fi
   \ifx\@empty\thankses\else \@footnotetext{%
     \def\par{\let\par\@par}\@setthanks}\fi
  }%
  \def\do##1{\let##1\relax}%
  \do\date \do\@date \do\thanks \do\thankses
  \do\keywords \do\@keywords \do\subjclass \do\@subjclass
}
\def\@setdate{Received by the editor \@date\@addpunct.}
\def\@setsubjclass{{\let\\=\space
  {\itshape\subjclassname.}\enspace\@subjclass\@addpunct.}}
\def\@setkeywords{%
  {\itshape \keywordsname.}\enspace \@keywords\@addpunct.}
%
\def\@maketitle{%
  \cleardoublepage \thispagestyle{empty}%
  \begingroup \topskip\z@skip
  \null\vfil
  \begingroup
  \LARGE\bfseries \centering
  \openup\medskipamount
  \@title\par\vspace{24pt}%
  \def\and{\par\medskip}\centering
  \mdseries\authors\par\bigskip
  \endgroup
  \vfil
  \ifx\@empty\addresses\else
  Author address:
  \@setaddresses\fi
  \bigskip
  \vfil
  \begin{center}
    \ifx\@empty\@translators\else\vfil\@settranslators\fi
  \end{center}
  \vfil\vfil
  \endgroup
}
%
\renewenvironment{abstract}{%
  \ifx\maketitle\relax
    \ClassWarning{\@classname}{Abstract should precede
      \protect\maketitle\space in AMS documentclasses; reported}%
  \fi
  \global\setbox\abstractbox=\vtop\bgroup
      \normalsize
      \indent
}{%
    \egroup
}

\endinput
