%% This file is apjrnl.cls,
%% a class file for Academic Press journals
%%
%%   Copyright (C) 2001 Academic Press
%%
%% This file may be distributed and/or modified under the
%% conditions of the LaTeX Project Public License, either version 1.2
%% of this license or (at your option) any later version.
%% The latest version of this license is in
%%    http://www.latex-project.org/lppl.txt
%% and version 1.2 or later is part of all distributions of LaTeX
%% version 1999/12/01 or later.
%% It is based on the LaTeX class article.cls
%% and requires the apthm.sty package for Academic Press theorem styles.
%% Also required are cite.sty for all options,
%% and paralist.sty and multicol.sty for selected journal options.
%% A journal option must be chosen.
 

%%%Identification
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{apjrnl}[2001/03/07]

%%%%%% Commands needed for option declaration  %%%%%%
\newif\iftwocol\twocolfalse
\newif\ifdraft\draftfalse
\newif\ifruninauthors\runinauthorsfalse
\newif\ifarabictables\arabictablesfalse
\newif\ifswitchfences\switchfencesfalse
\newif\ifnojournal\nojournalfalse
\newif\ifnamedate\namedatefalse
\newif\ifitalcite\italcitefalse
\newcommand{\journalcode}{none}
\newcommand{\journaltitle}{No Journal Option Specified}
\newcommand{\templatename}{\texttt{tmplmain.tex}}


% Commands for trim size options%%%%%%%%%%%%%%%%%%%%%%%

%command for large trim size formatting
\newcommand\largetrim{%
\AtEndOfClass{%
\setlength{\textwidth}{30pc}%
\setlength{\textheight}{50pc}%
}}

%command for small trim size formatting
\newcommand{\smalltrim}{%
\AtEndOfClass{%
\setlength{\textwidth}{27pc}%
\setlength{\textheight}{45pc}%
}}

% command for two column journal formatting 
\newcommand{\twocoltrim}{%
\twocoltrue\AtEndOfClass{%
\setlength{\textwidth}{43pc}
\setlength{\textheight}{54pc}
\AtBeginDocument{%
\setlength{\hoffset}{-0.5in}
\setlength{\voffset}{-0.5in}
\setlength{\textwidth}{43pc}
\setlength{\textheight}{54pc}
}}}%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%Declaration of Options

\DeclareOption{draft}{%
\AtBeginDocument{%
\renewcommand\baselinestretch{2}%
\normalfont\normalsize
\drafttrue}}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% journal option declarations %%%%%%%%%%%%%%%%%%

\DeclareOption{none}{%
\renewcommand{\journalcode}{none}
\renewcommand{\journaltitle}{No Journal Option Specified}
\largetrim \nojournaltrue \arabictablestrue}

\DeclareOption{aama}{%
\renewcommand{\journaltitle}{%
Advances in Applied Mathematics}
\renewcommand{\journalcode}{aama}%
\smalltrim }

\DeclareOption{acha}{%
\renewcommand{\journaltitle}{%
Applied and Computational Harmonic Analysis}
\renewcommand{\journalcode}{acha}%
\largetrim }

\DeclareOption{aima}{%
\renewcommand{\journaltitle}{%
Advances in Mathematics}
\renewcommand{\journalcode}{aima}%
\smalltrim }

\DeclareOption{aphy}{%
\renewcommand{\journaltitle}{%
Annals of Physics}
\renewcommand{\journalcode}{aphy}%
\smalltrim }

\DeclareOption{cviu}{%
\renewcommand{\journaltitle}{%
Computer Vision and Image Understanding}
\renewcommand{\journalcode}{cviu}%
\largetrim \arabictablestrue}

\DeclareOption{drev}{%
\renewcommand{\journaltitle}{%
Developmental Review (APA)}
\renewcommand{\journalcode}{drev}%
\smalltrim \namedatetrue \arabictablestrue}

\DeclareOption{dspr}{%
\renewcommand{\journaltitle}{%
Digital Signal Processing}
\renewcommand{\journalcode}{dspr}%
\renewcommand{\templatename}{\texttt{tmpdspr.tex}}
\largetrim \runinauthorstrue \arabictablestrue}


\DeclareOption{exeh}{%
\renewcommand{\journaltitle}{%
Explorations in Economic History}
\renewcommand{\journalcode}{exeh}%
\smalltrim \namedatetrue \arabictablestrue}

\DeclareOption{ffta}{%
\renewcommand{\journaltitle}{%
Finite Fields and Their Applications}
\renewcommand{\journalcode}{ffta}%
\smalltrim }

\DeclareOption{game}{%
\renewcommand{\journaltitle}{%
Games and Economic Behavior}
\renewcommand{\journalcode}{game}%
\smalltrim \namedatetrue }

\DeclareOption{gmod}{%
\renewcommand{\journaltitle}{%
Graphical Models}
\renewcommand{\journalcode}{gmod}%
\largetrim }

\DeclareOption{inco}{%
\renewcommand{\journaltitle}{%
Information and Computation}
\renewcommand{\journalcode}{inco}%
\largetrim \arabictablestrue}

\DeclareOption{jabr}{%
\renewcommand{\journaltitle}{%
Journal of Algebra}
\renewcommand{\journalcode}{jabr}%
\smalltrim }

\DeclareOption{jagm}{%
\renewcommand{\journaltitle}{%
Journal of Algorithms}
\renewcommand{\journalcode}{jagm}%
\smalltrim \arabictablestrue}

\DeclareOption{jath}{%
\renewcommand{\journaltitle}{%
Journal of Approximation Theory}
\renewcommand{\journalcode}{jath}%
\smalltrim }

\DeclareOption{jcat}{%
\renewcommand{\journaltitle}{%
Journal of Catalysis}
\renewcommand{\journalcode}{jcat}%
\renewcommand{\templatename}{\texttt{tmpjcat.tex}}
\twocoltrim \runinauthorstrue \switchfencestrue \arabictablestrue}

\DeclareOption{jcec}{%
\renewcommand{\journaltitle}{%
Journal of Comparative Economics}
\renewcommand{\journalcode}{jcec}%
\smalltrim \namedatetrue \arabictablestrue}

\DeclareOption{jcis}{%
\renewcommand{\journaltitle}{%
Journal of Colloid and Interface Science}
\renewcommand{\journalcode}{jcis}%
\renewcommand{\templatename}{\texttt{tmpjcis.tex}}
\twocoltrim \runinauthorstrue \switchfencestrue \arabictablestrue}

\DeclareOption{jcom}{%
\renewcommand{\journaltitle}{%
Journal of Complexity}
\renewcommand{\journalcode}{jcom}%
\smalltrim }

\DeclareOption{jcph}{%
\renewcommand{\journaltitle}{%
Journal of Computational Physics}
\renewcommand{\journalcode}{jcph}%
\renewcommand{\templatename}{\texttt{tmpjcph.tex}}
\largetrim \runinauthorstrue }

\DeclareOption{jcss}{%
\renewcommand{\journaltitle}{%
Journal of Computer and System Sciences}
\renewcommand{\journalcode}{jcss}%
\largetrim \arabictablestrue}

\DeclareOption{jcta}{%
\renewcommand{\journaltitle}{%
Journal of Combinatorial Theory, Series A}
\renewcommand{\journalcode}{jcta}%
\smalltrim }

\DeclareOption{jctb}{%
\renewcommand{\journaltitle}{%
Journal of Combinatorial Theory, Series B}
\renewcommand{\journalcode}{jctb}%
\smalltrim }

\DeclareOption{jdeq}{%
\renewcommand{\journaltitle}{%
Journal of Differential Equations}
\renewcommand{\journalcode}{jdeq}%
\smalltrim }

\DeclareOption{jeth}{%
\renewcommand{\journaltitle}{%
Journal of Economic Theory}
\renewcommand{\journalcode}{jeth}%
\smalltrim }

\DeclareOption{jfan}{%
\renewcommand{\journaltitle}{%
Journal of Functional Analysis}
\renewcommand{\journalcode}{jfan}%
\smalltrim }

\DeclareOption{jfin}{%
\renewcommand{\journaltitle}{%
Journal of Financial Intermediation}
\renewcommand{\journalcode}{jfin}%
\smalltrim \namedatetrue }

\DeclareOption{jhec}{%
\renewcommand{\journaltitle}{%
Journal of Housing Economics}
\renewcommand{\journalcode}{jhec}%
\smalltrim \namedatetrue }

\DeclareOption{jjie}{%
\renewcommand{\journaltitle}{%
Journal of the Japanese and International Economies}
\renewcommand{\journalcode}{jjie}%
\smalltrim \namedatetrue }

\DeclareOption{jmaa}{%
\renewcommand{\journaltitle}{%
Journal of Mathematical Analysis and Applications}
\renewcommand{\journalcode}{jmaa}%
\smalltrim }

\DeclareOption{jmps}{%
\renewcommand{\journaltitle}{%
Journal of Mathematical Psychology (APA)}
\renewcommand{\journalcode}{jmps}%
\largetrim \namedatetrue \arabictablestrue}

\DeclareOption{jmre}{%
\renewcommand{\journaltitle}{%
Journal of Magnetic Resonance}
\renewcommand{\journalcode}{jmre}%
\renewcommand{\templatename}{\texttt{tmpjmre.tex}}
\twocoltrim \runinauthorstrue \italcitetrue \arabictablestrue}

\DeclareOption{jmva}{%
\renewcommand{\journaltitle}{%
Journal of Multivariate Analysis}
\renewcommand{\journalcode}{jmva}%
\smalltrim }

\DeclareOption{jnth}{%
\renewcommand{\journaltitle}{%
Journal of Number Theory}
\renewcommand{\journalcode}{jnth}%
\smalltrim }

\DeclareOption{jpdc}{%
\renewcommand{\journaltitle}{%
Journal of Parallel and Distributed Computing}
\renewcommand{\journalcode}{jpdc}%
\largetrim \arabictablestrue}

\DeclareOption{jssc}{%
\renewcommand{\journaltitle}{%
Journal of Solid State Chemistry}
\renewcommand{\journalcode}{jssc}%
\renewcommand{\templatename}{\texttt{tmpjssc.tex}}
\twocoltrim \runinauthorstrue \switchfencestrue \arabictablestrue}

\DeclareOption{juec}{%
\renewcommand{\journaltitle}{%
Journal of Urban Economics}
\renewcommand{\journalcode}{juec}%
\smalltrim \arabictablestrue}

\DeclareOption{jvbe}{%
\renewcommand{\journaltitle}{%
Journal of Vocational Behavior (APA)}
\renewcommand{\journalcode}{jvbe}%
\smalltrim \namedatetrue \arabictablestrue}

\DeclareOption{jvic}{%
\renewcommand{\journaltitle}{%
Journal of Visual Communication and Image Representation}
\renewcommand{\journalcode}{jvic}%
\largetrim \arabictablestrue}

\DeclareOption{mben}{%
\renewcommand{\journaltitle}{%
Metabolic Engineering}
\renewcommand{\journalcode}{mben}%
\renewcommand{\templatename}{\texttt{tmpmben.tex}}
\twocoltrim \namedatetrue \arabictablestrue}

\DeclareOption{ofte}{%
\renewcommand{\journaltitle}{%
Optical Fiber Technology}
\renewcommand{\journalcode}{ofte}%
\largetrim \arabictablestrue}

\DeclareOption{redy}{%
\renewcommand{\journaltitle}{%
Review of Economic Dynamics}
\renewcommand{\journalcode}{redy}%
\smalltrim \namedatetrue }

\DeclareOption{ssre}{%
\renewcommand{\journaltitle}{%
Social Science Research}
\renewcommand{\journalcode}{ssre}%
\smalltrim \namedatetrue \arabictablestrue}

%%%%% End of journal option declarations  %%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%Activate paper size options for article.cls
\PassOptionsToClass{a4paper,a5paper,b5paper,letterpaper,executivepaper}{article}

%%%Execution of Options
\ExecuteOptions{none}

\ProcessOptions

%%%Package Loading
\LoadClass[oneside]{article}
\RequirePackage{apthm}
\ifruninauthors\RequirePackage{paralist}\fi
\iftwocol\RequirePackage{multicol}\fi
%%%Main Code

\newcommand\eqnref[1]{%
\normalsize \normalfont (\ref{#1})}%

%%% if commands for journal styles  %%%%%%%%%%%%%%%%%%%%%%%
%% commands for paren citations and bracket equations
\ifswitchfences%
\renewcommand{\@cite}[2]{(#1\if@tempswa , #2\fi)}
%
\renewcommand{\@eqnnum}{[\theequation]}
\renewcommand{\@biblabel}[1]{(#1)}
\AtBeginDocument{%
\renewcommand\eqnref[1]{%
\normalsize \normalfont [\ref{#1}]}}

\fi
%
%commands for name date references
\ifnamedate
\renewcommand{\@biblabel}[1]{}
\fi
%
%commands for run in authors%
\ifruninauthors
\AtBeginDocument{\newcommand\additem[2][]{%
\renewcommand\labelenumi{\theenumi}
\renewcommand\theenumi{\fnsymbol{enumi}}
\item\label{#1}\textit{#2}
}}
\fi
%
%command for arabic table numbers%
\renewcommand{\thetable}{\Roman{table}}%
\ifarabictables
\renewcommand{\thetable}{\arabic{table}}%
\fi
%
%%% Italic citation format command for JMRE
\ifitalcite{%
\AtBeginDocument{%
\renewcommand{\@cite}[2]{\textit{(#1\if@tempswa , #2\fi)}}
\renewcommand\eqnref[1]{%
\normalsize \normalfont [\ref{#1}]}%
\renewcommand{\@eqnnum}{[\theequation]}
\renewcommand{\@biblabel}[1]{\textit{(#1)}}}
\fi
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%Section and subsection header styles
\renewcommand{\section}{\@startsection {section}{1}{\z@}%
                                   {-3.5ex \@plus -1ex \@minus -.2ex}%
                                   {2.3ex \@plus.2ex}%
                                   {\centering\normalfont\normalsize\uppercase}}
\renewcommand{\subsubsection}{\@startsection{subsubsection}{2}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                                     {\normalfont\normalsize\itshape}}

\renewcommand{\subsection}{\@startsection{subsection}{2}{\z@}%
                                     {-3.25ex\@plus -1ex \@minus -.2ex}%
                                     {1.5ex \@plus .2ex}%
                          				 {\normalfont\normalsize\bfseries\centering}}
\renewcommand{\paragraph}{\@startsection{paragraph}{4}{\parindent}%
                                    {3.25ex \@plus1ex \@minus.2ex}%
                                    {-1em}%
                                    {\normalfont\normalsize\itshape}}

\renewcommand{\@seccntformat}[1]{{\csname the#1\endcsname}.\hspace{1em}}




\let\@afterindentfalse\@afterindenttrue
\@afterindenttrue

%%Provide command for section numbers on equations
\newcommand\equationsection{\renewcommand{\theequation}{\thesection.\arabic{equation}}}

\newcommand\titlehead{please enter a title head}
\newcommand\authorhead{please enter an author head}



%%%command for spanning columns

\newcommand\startwide{\end{multicols}\begin{multicols}{2}\hrule\end{multicols}}
\newcommand\stopwide{\par\hspace{21pc}\hrulefill\begin{multicols}{2}}

%%% Redefine maketitle
\renewcommand{\maketitle}{%
 \newpage%
  \null%
\ifdraft\renewcommand\baselinestretch{2}\fi%
   \begin{center}%
  \let \thanks \footnote%
    {\LARGE \@title}%
 \vskip 1.5em%
    \lineskip .5em %
\renewcommand\baselinestretch{1}%
\ifdraft\renewcommand\baselinestretch{2}\fi%
\normalsize %
\ifruninauthors\begin{inparaenum}\@author\end{inparaenum}
\else\@author
\fi%
    \vskip 2em%
    {Version: \@date}%
  \end{center}%
\iftwocol\begin{multicols}{2}\fi%
}



%%Front matter commands

\renewcommand{\and}{\\[6pt] and \\[10pt]}

\renewenvironment{abstract}{\normalfont \small \par}{\normalfont\normalsize\par\bigskip}

\newenvironment{keywords}{\par\textit{Key Words: }}{\par}

\newenvironment{subject}[1][Subject Classification]%
{\par\textit{{#1}: }}%
{\par}

\newcommand{\dedication}[1]{\centering \normalfont \textsc{\lowercase{#1}}}


%%End matter environments and commands
\newenvironment{acknowledge}{\addvspace{18pt}\normalfont \normalsize
 \begin{center} \normalfont\normalsize ACKNOWLEDGMENTS
 \end{center} \small \addvspace{6pt} \par}%
{\par\addvspace{6pt}\normalfont \normalsize}

\newenvironment{noteinproof}{%
\small \normalfont \par \textit{Note added in proof}. \hskip\labelsep}%
{\par \addvspace{6pt}\normalfont \normalsize}

\renewcommand{\refname}{REFERENCES}

%%% Table and figure commands
\renewcommand{\tablename}{TABLE}
\renewcommand{\figurename}{FIG.}

\newcommand{\@makefigcaption}[2]{%
  \vskip\abovecaptionskip
  \sbox\@tempboxa{\textbf{#1} #2}%
  \ifdim \wd\@tempboxa >\hsize
    \textbf{#1} #2\par
  \else
    \global \@minipagefalse
    \hb@xt@\hsize{\hfil\box\@tempboxa\hfil}%
  \fi
  \vskip\belowcaptionskip}

\newcommand{\@maketablecaption}[2]{%
  \vskip\abovecaptionskip
    \begin{center}#1\par #2\par\end{center}
  \vskip\belowcaptionskip}

\renewcommand{\figure}%
	{\let\@makecaption\@makefigcaption\@float{figure}}
\renewcommand{\table}%
	{\let\@makecaption\@maketablecaption\@float{table}}

\iftwocol\AtBeginDocument{%
\renewenvironment{figure}{\begin{figure*}}{\end{figure*}}}
\fi

%\renewcommand{\figure*}%
%	{\let\@makecaption\@makefigcaption\@float{figure}}
%\renewcommand{\table*}%
%	{\let\@makecaption\@maketablecaption\@float{table}}


%%Appendix

\renewcommand{\appendix}{%
    \renewcommand{\section}{%
        \secdef\Appendix\sAppendix}%
    \setcounter{section}{0}%
    \renewcommand{\thesection}{\Alph{section}}%
}
\renewcommand{\appendixname}{APPENDIX}
\newcommand{\Appendix}[2][?]{%
    \refstepcounter{section}%
    \addcontentsline{toc}{appendix}%
        {\protect\numberline{\appendixname~\thesection} #1}%
    {\vspace{\baselineskip}\par\normalfont\normalsize\uppercase{%
        \centering\appendixname\ \thesection{:\ #2}\par}}
    \sectionmark{#1}\vspace{\baselineskip}}

\newcommand{\sAppendix}[1]{%
    \refstepcounter{section}%
    {\vspace{\baselineskip}\par\normalfont\normalsize\uppercase{%
        \centering\appendixname\ \thesection\par #1}\par}
    \vspace{\baselineskip}}


%%Tweak the bibliography formatting

\renewenvironment{thebibliography}[1]{%
\renewcommand{\section}{\@startsection {section}{1}{\z@}%
                                   {-3.5ex \@plus -1ex \@minus -.2ex}%
                                   {2.3ex \@plus.2ex}%
                                   {\centering\normalfont\normalsize\uppercase}}
\section*{\refname
        \@mkboth{\MakeUppercase\refname}{\MakeUppercase\refname}}%
      \list{\@biblabel{\@arabic\c@enumiv}}%
           {\settowidth\labelwidth{\@biblabel{#1}}%
            \leftmargin\labelwidth
            \advance\leftmargin\labelsep
            \@openbib@code
            \usecounter{enumiv}%
            \let\p@enumiv\@empty
            \renewcommand\theenumiv{\@arabic\c@enumiv}}%
      \sloppy
      \clubpenalty4000
      \@clubpenalty \clubpenalty
      \widowpenalty4000%
      \sfcode`\.\@m}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist}

%%%Create environments for figures and table sections
%
%\newenvironment{figuresec}%
%{\clearpage\renewcommand\baselinestretch{2}%
%{\listoffigures}\clearpage}%
%{}
%\newenvironment{tablesec}{\clearpage\renewcommand\baselinestretch{1}}{}
%%%Redefine formatting for \listoffigures
%\renewcommand*{\l@figure}[2]{%
%    \addpenalty\@secpenalty
%    \addvspace{1.0em \@plus\p@}%
%    \setlength\@tempdima{1.5em}%
%    \begingroup
%      \parindent \z@ \rightskip \@pnumwidth
%      \parfillskip -\@pnumwidth
%      \leavevmode 
%      \advance\leftskip\@tempdima
%      \hskip -\leftskip
%     FIG.\ #1\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss }
%\par
%    \endgroup
%}





%for use in sample files
%short command for formatting command strings
\newcommand\cmd[1]{%
\texttt{%
$\backslash$%
{#1}}%
}