%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% For support : Yolande Koh, <ykoh@wspc.com.sg>                         
%% Technical assistance : D. Rajesh Babu, <rajesh@wspc.com.sg>            
%% Book Template (updated on 8-1-2007)
%% Book Trim Size: 9in x 6in
%% Text Area: 7.35in (include runningheads) x 4.5in
%% Main Text: 10/13pt
%%
%% The content, structure, format and layout of this style file is  
%% the property of World Scientific Publishing Co. Pte. Ltd. 
%% Copyright 2007 by World Scientific Publishing Co. 
%% All rights are reserved.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\NeedsTeXFormat{LaTeX2e}[1995/12/01]
\ProvidesClass{ws-book9x6}
              [1996/05/26 v1.3r
 Standard LaTeX document class]
\newif\if@restonecol
\@twosidetrue\@mparswitchfalse\@twocolumnfalse
%
\newif\if@draft
\newif\if@bcrhead
\newif\if@csrhead
\newif\if@acrhead
\newif\if@onethmnum
%
\newdimen\draftrule
\draftrule=0pt
%
\DeclareOption{draft}{\@drafttrue\setlength\overfullrule{5pt}\draftrule0.01pt}%
%
\hoffset=0.15in
\voffset=0.65in
%
\DeclareOption{bcrhead}{\@bcrheadtrue}
\DeclareOption{csrhead}{\@csrheadtrue}
\DeclareOption{acrhead}{\@acrheadtrue}
\DeclareOption{onethmnum}{\@onethmnumtrue}
\DeclareOption{final}{\setlength\overfullrule{0pt}}
\DeclareOption{twocolumn}{\twocolumn\sloppy\flushbottom}
\ExecuteOptions{a4,10pt,twoside,onecolumn,final,openright,bcrhead}  %maintext-10pt
\ProcessOptions
%
\newdimen\trimheight
\newdimen\trimwidth
\newdimen\typeheight
\newdimen\typewidth 
\newdimen\normaltextheight
\newdimen\normaltextwidth
\newdimen\tablewidth
\newdimen\tempdimen
\newdimen\tablewd
%
\newbox\tempbox
%
\newcommand\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
%
\newcount\minute
\newcount\hour
\def\currenttime{%
    \minute\time
    \hour\minute
    \divide\hour60
    \the\hour:\multiply\hour60\advance\minute-\hour\the\minute}
%
\def\infotag{\hbox to \trimwidth{\hskip8pt{\scriptsize\sf \today
\quad\currenttime}\hfill\scriptsize\sf World Scientific Book - 9in x 6in 
\hfill{\scriptsize\sf\jobname}\hskip8pt}}%
%
\newtoks\title
\title{Book Title}
\newcommand\booktitle{\the\title}
\newcommand\contentsname{Contents}
\newcommand\listfigurename{List of Figures}
\newcommand\listtablename{List of Tables}
\newcommand\indexname{Index}
\newcommand\appendixname{Appendix}
\newcommand\prefacename{Preface}
\newcommand\forewordname{Foreword}
\newcommand\acknowledgmentsname{Acknowledgments}
\newcommand\acknowledgmentname{Acknowledgment}
%\newcommand\chapname{Chapter~\thechapter\unskip\hspace*{1em}}
\newcommand\outchapter{\thechapter.\unskip\hspace*{1em}}
\newcommand\appname{Appendix~\theappendix}
%
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}
%%
\newcommand\@pnumwidth{1.55em}
\newcommand\@tocrmarg{2.55em}
\newcommand\@dotsep{4.5}
\setcounter{tocdepth}{2}
\newdimen\@cnumwidth        %ADDED FOR CONTENT
\setlength\@cnumwidth{5pc}  %ADDED FOR CONTENT

\newcommand\tableofcontents{%
    {\cleardoublepage
     \renewcommand\thepage{\roman{page}}
         \thispagestyle{plain}%
         \global\@topnum\z@
         \@afterindentfalse
    \if@twocolumn
        \@restonecoltrue\onecolumn
        \else
        \@restonecolfalse
        \fi
    {\vbox to 115pt{%
    \vspace*{37pt}
    \parindent\z@\raggedright\reset@font
    {\centering{{\CNfont \phantom{Chapter \thechapter}\par}
        \vskip 0.25in
    \vbox{\CTfont\contentsname}\par}\par}\nobreak\vfill}\vskip0.4in}
\markboth{\booktitle}{Contents}
\if@csrhead
\markboth{Contents}{Contents}
\fi
\if@acrhead
\markboth{Contents}{Contents}
\fi
\@starttoc{toc}%
        \if@restonecol \twocolumn\fi}\par\vfill\pagebreak\par}
\newcommand*\l@part[2]{%
  \ifnum \c@tocdepth >-2\relax
    \addpenalty{-\@highpenalty}%
    \addvspace{2.25em \@plus\p@}%
    \begingroup
      \setlength\@tempdima{3em}%   %3em
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      {\leavevmode
       \large \bfseries #1\hfil \hb@xt@\@pnumwidth{\hss #2}}\par
       \nobreak
         \global\@nobreaktrue
         \everypar{\global\@nobreakfalse\everypar{}}%
    \endgroup
  \fi}
%
%CHANGE HERE FOR CONTENT TEXT
\newcommand*\l@chapter[2]{%
  \ifnum \c@tocdepth >\m@ne
    \addpenalty{-\@highpenalty}%
    \vskip 1.0em \@plus\p@         %CONTROL CHAPTER HEADS
    \setlength\@tempdima{2.7em}%   %CONTROL P1-heads TURNLINES
    \begingroup
      \parindent \z@ \rightskip \@cnumwidth      %ADDED HERE
      \parfillskip -\@cnumwidth                  %ADDED HERE
      \leavevmode %\bfseries
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      {#1}\nobreak\hfill \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
      \vskip4pt    %more space
      \penalty\@highpenalty
    \endgroup
  \fi}
%
\newcommand*\l@appenx[2]{%
  \ifnum \c@tocdepth >\m@ne
    \addpenalty{-\@highpenalty}%
    \vskip 1.0em  \@plus\p@
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
%
%CHANGE HERE FOR CONTENT TEXT of \chapter*
\newcommand*\l@schapter[2]{%
  \ifnum \c@tocdepth >\m@ne
   \addpenalty{-\@highpenalty}%
   \vskip 1em \@plus\p@         %CONTROL SPACE FOR TOC AS IN \chapter
   \setlength\@tempdima{1.5em}%
    \begingroup
      \parindent \z@ \rightskip \@pnumwidth
      \parfillskip -\@pnumwidth
      \leavevmode        %roman content.toc pg nos.
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      {\it #1}\nobreak\hfil \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
      \vskip-3pt    %more space
      \penalty\@highpenalty
    \endgroup
  \fi}
%
\newcommand*\l@author[2]{%
  \ifnum \c@tocdepth >\m@ne
    \addpenalty{-\@highpenalty}%
    \vskip0em \@plus\p@
     \setlength\@tempdima{1.8em}%
     \begingroup
      \parindent1.9em \rightskip \@cnumwidth
      \parfillskip -\@cnumwidth
      \leavevmode
      \advance\leftskip\@tempdima
      \hskip -\leftskip
      {\it#1}\nobreak\hfill \nobreak\hb@xt@\@pnumwidth{\hss \phantom{#2}}\par
      \vskip0.6em \@plus\p@
      \penalty\@highpenalty
    \endgroup
  \fi}
%
%CONTROL THE TOC TEXT:-
%\newcommand*\l@section{\@dottedtocline{1}{1.8em}{2.1em}}       %1.7
%\newcommand*\l@subsection{\@dottedtocline{2}{3.9em}{2.9em}}    %1.7.1
%\newcommand*\l@subsubsection{\@dottedtocline{3}{6.9em}{3.7em}} %1.7.1.1

\newcommand*\l@section{\@dottedtocline{1}{1.8em}{2.8em}}        %1.12
\newcommand*\l@subsection{\@dottedtocline{2}{4.6em}{3.5em}}     %1.12.1
\newcommand*\l@subsubsection{\@dottedtocline{3}{7.8em}{4.2em}}  %1.12.1.1

\newcommand*\l@paragraph{\@dottedtocline{4}{8.7em}{5em}}%10em
\newcommand*\l@subparagraph{\@dottedtocline{5}{12em}{6em}}
\newcommand*\l@appsection{\@dottedtocline{1}{\z@}{6.5em}}
\newcommand*\l@appsubsection{\@dottedtocline{2}{6.5em}{3.4em}}
%
\newcommand\listoffigures{%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\listfigurename}%
      \@mkboth{\MakeUppercase\listfigurename}%
              {\MakeUppercase\listfigurename}%
    \@starttoc{lof}%
    \if@restonecol\twocolumn\fi
    \cleardoublepage
    }
\newcommand*\l@figure{\@dottedtocline{1}{0em}{2.3em}}
\newcommand\listoftables{%
    \if@twocolumn
      \@restonecoltrue\onecolumn
    \else
      \@restonecolfalse
    \fi
    \chapter*{\listtablename}%
      \@mkboth{%
          \MakeUppercase\listtablename}%
         {\MakeUppercase\listtablename}%
    \@starttoc{lot}%
    \if@restonecol\twocolumn\fi
    \cleardoublepage
    }
\let\l@table\l@figure
%
\renewcommand\normalsize{%
   \@setfontsize\normalsize\@xpt{13}  %Main Text is 10/13pt
   \abovedisplayskip6\p@\@plus4\p@\@minus6\p@
   \abovedisplayshortskip \abovedisplayskip
   \belowdisplayshortskip \abovedisplayskip
   \belowdisplayskip \abovedisplayskip
   \let\@listi\@listI}
\normalsize
%
\newcommand\small{%
   \@setfontsize\small\@ixpt{11}%
   \abovedisplayskip 9.5\p@ \@plus3\p@
   \abovedisplayshortskip \z@ \@plus2\p@
   \belowdisplayshortskip 5\p@ \@plus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 5\p@ \@plus3\p@
               \parsep 2\p@ \@plus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip}
%
\newcommand\footnotesize{%
   \@setfontsize\footnotesize\@viiipt{10}%
   \abovedisplayskip 6\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 3\p@ \@plus\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FONTS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\rhfont{\footnotesize}
\def\PNfont{\Large{}}
\def\PTfont{\LARGE\bfseries\boldmath{}}
\def\CNfont{\large\bfseries}
\def\CTfont{\Large\bfseries\boldmath{}}

\def\sectionfont{\fontsize{10}{13}\bfseries\boldmath\selectfont\rightskip2pc}
\def\subsectionfont{\fontsize{10}{13}\bfseries\boldmath\itshape\selectfont\rightskip2pc}
\def\subsubsectionfont{\fontsize{10}{13}\itshape\selectfont\rightskip3pc}%
\def\paragraphfont{\fontsize{10}{13}\bfseries\boldmath\selectfont\rightskip3pc}
%
\def\figurecaptionfont{\footnotesize}
\def\tablefont{\footnotesize}
\def\tablecaptionfont{\footnotesize}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% FONTS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcommand\scriptsize{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand\tiny{\@setfontsize\tiny\@vpt\@vipt}
\newcommand\large{\@setfontsize\large\@xiipt{14}}%12/14
\newcommand\Large{\@setfontsize\Large\@xviipt{21}}%17/21
\newcommand\LARGE{\@setfontsize\LARGE\@xxvpt{25}}%25/25
%
\trimheight8.97in       %measure is 9in
\trimwidth5.97in
\typeheight7.35in
%
\setlength\parindent{15pt}
\setlength\headheight{5\p@}  %8
\setlength\headsep   {22pt}  
\setlength\topskip  {7\p@}
\setlength\footskip{0.36in}   
\setlength\maxdepth{4\p@}
\setlength\@maxdepth\maxdepth
\setlength\textwidth{4.5truein}
\normaltextwidth\textwidth
\setlength\textheight\typeheight
\advance\textheight-\headheight
\advance\textheight-\headsep
\advance\textheight-2pt
\setlength\normaltextheight{\textheight}
\setlength\oddsidemargin{.73in}%   %.65in
\setlength\evensidemargin{.73in}%  %.65in
\setlength\marginparwidth{.5in}
\setlength\marginparsep{7\p@}
\setlength\marginparpush{5\p@}
\setlength\topmargin{0.5in}%
\setlength\footnotesep{6\p@}
\setlength{\skip\footins}{6\p@ \@plus 4\p@}%
\setlength\floatsep    {18\p@ \@plus 2\p@ \@minus 2\p@}
\setlength\textfloatsep{15.1\p@ \@plus 6\p@}
\setlength\intextsep   {12\p@ \@plus 2\p@}
\setlength\dblfloatsep    {12\p@ \@plus 2\p@}
\setlength\dbltextfloatsep{20\p@ \@plus 2\p@}
\setlength\@fptop{0\p@ \@plus 1fil}
\setlength\@fpsep{8\p@ \@plus 2fil}
\setlength\@fpbot{0\p@ \@plus 1fil}
\setlength\@dblfptop{0\p@ \@plus 1fil}
\setlength\@dblfpsep{8\p@ \@plus 2fil}
\setlength\@dblfpbot{0\p@ \@plus 1fil}
\setlength\partopsep{2\p@ \@plus 1\p@}
\setlength\lineskip{1\p@}
\setlength\normallineskip{1\p@}
\renewcommand\baselinestretch{}
\setlength\parskip{0\p@}
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
%
\setcounter{secnumdepth}{3}
\newcounter {part}
\newcounter {chapter}
\newcounter {section}[chapter]
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\newcounter {paragraph}[subsubsection]
\newcounter {subparagraph}[paragraph]
%
\renewcommand\thepart{\arabic{part}}
\renewcommand\thechapter{\arabic{chapter}}
\renewcommand\thesection{\thechapter.\arabic{section}}
\renewcommand\thesubsection{\thesection.\arabic{subsection}}
\renewcommand\thesubsubsection{\thesubsection .\arabic{subsubsection}}
%\renewcommand\theparagraph{\thesubsubsection.\arabic{paragraph}}
\renewcommand\theparagraph{\relax}
%
\newcommand\section{\@startsection {section}{1}{\z@}{-20pt \@plus-3pt minus6pt}{12pt}{\sectionfont}}
\newcommand\subsection{\@startsection{subsection}{2}{\z@}{-16.5pt \@plus-3pt minus6pt}{6.25pt}{\subsectionfont}}
\newcommand\subsubsection{\@startsection{subsubsection}{3}{\z@}{-12.75pt \@plus-3pt minus3pt}{6pt}{\subsubsectionfont}}
\newcommand\paragraph{\@startsection{paragraph}{4}{\z@}{-8pt \@plus -3pt}{-1em}{\paragraphfont}}
%
\def\@seccntformat#1{{\upshape\csname the#1\endcsname}\hskip 1em}
%
\newcommand\part{\cleardoublepage
                 \renewcommand\thepage{\arabic{page}}
                 \thispagestyle{pblind}%
                 \global\@topnum\z@
                 \@afterindentfalse
                 \secdef\@part\@spart}
%
\def\@part[#1]#2#3{%
 {\vspace*{3pc}
%\vbox to 6.85pc{\parindent \z@ \raggedright \reset@font
  \vbox{\parindent\z@ \reset@font
         \refstepcounter{part}
    \addcontentsline{toc}{part}{#2}
%   \addcontentsline{toc}{part}{Part~\thepart\quad #1}
         \vskip 1.4in
         {\centering{\CNfont PART \thepart}
%          \vskip-2.95pt
%         \hbox to \hsize{\hss\vrule height1pt width28pc}%
         \par
         \vskip 12\p@
    \vbox{\CTfont #1}\par}
    \vskip 106\p@
    \vbox{\noindent #3}\par
    \nobreak}}
  \@endpart}
%
\def\@spart#1{%
    {\PTfont #1\par}%
    \@endpart}
%
\def\@endpart{\vfil\newpage
              \if@twoside
                \hbox{}%
                \thispagestyle{pblind}%
                \newpage
              \fi}
%
\newcommand\chapter{\cleardoublepage
                    \thispagestyle{plain}%
                    \global\@topnum\z@
                    \@afterindentfalse
                    \secdef\@chapter\@schapter}
%
\def\@chapter[#1]#2{\ifnum \c@secnumdepth >\m@ne
            \renewcommand\thepage{\arabic{page}}
                        \else \fi
                    \chaptermark{#1}%
                    \addtocontents{lof}{\protect\addvspace{10\p@}}%
                    \addtocontents{lot}{\protect\addvspace{10\p@}}%
                    \if@twocolumn
                      \@topnewpage[\@makechapterhead{#2}]%
                    \else
                      \@makechapterhead{#2}%
                       \ifnum\arabic{chapter}=1\addcontentsline{toc}{author}{\vspace*{-20pt}}\fi
                       \addcontentsline{toc}{chapter}{\outchapter#1}
                      \@afterheading
                    \fi}
%
\def\@makechapterhead#1{%
    {\vbox to 115pt{%      
    \refstepcounter{chapter}%
%   \addcontentsline{toc}{chapter}{\outchapter#1}
    \vspace*{37pt}      %VSPACE FROM TRIM SIZE
        \parindent\z@\raggedright\reset@font
        {\centering{{\CNfont Chapter~\thechapter\par}%
         \vskip 0.25in
    \vbox{
    \CTfont #1\par
    }\par}\par}\nobreak\vfill}}\vskip0.4in}%
%
\def\@schapter#1{
    \markboth{#1}{#1}
        \@makeschapterhead{#1}%
        \@afterheading}
%
\def\@makeschapterhead#1{%
    {\addcontentsline{toc}{schapter}{#1}
    \vbox to 115pt{%    
    \vspace*{37pt}  %change pt here
    \parindent\z@\raggedright\reset@font
        {\centering{{\CNfont \phantom{Chapter \thechapter}\par}
        \vskip 0.25in 
        \vbox{\CTfont #1}\par}\par}\nobreak\vfill}}\vskip0.4in}
%
\def\authorfont{\fontsize{10}{12}\selectfont}

\def\author{\@ifnextchar[{\@bauthor}{\@nauthor}}

\def\@nauthor#1{\authormark{#1}\par
    {\centering{\authorfont#1}\par\vspace*{0.05in}}
\addcontentsline{toc}{author}{#1}}

\def\@bauthor[#1]#2{\authormark{#1}\par
    {\centering{\authorfont#2}\par\vspace*{0.05in}}
    \ifx#1\empty\else
\addcontentsline{toc}{author}{#1}\fi}
%
\def\addressfont{\fontsize{10}{12}\selectfont}      
\def\address#1{\par
    {\centering\it{\addressfont#1}\par\vspace*{11pt}}  
     %\addcontentsline{toc}{author}{#1}
}
%
\setcounter{topnumber}{4}
\renewcommand\topfraction{.9}
\setcounter{bottomnumber}{4}
\renewcommand\bottomfraction{.9}
\setcounter{totalnumber}{8}
\renewcommand\textfraction{.1}
\renewcommand\floatpagefraction{.9991}
\setcounter{dbltopnumber}{5}
\renewcommand\dbltopfraction{.9}
\renewcommand\dblfloatpagefraction{.9}
%
\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{8\p@ plus3pt minus3pt}
\setlength\belowcaptionskip{-1\p@}
%
\newbox\captionbox
%
\def\FigName{figure}%
%
\long\def\@makecaption#1#2{%
  \ifx\FigName\@captype
        \vskip\abovecaptionskip
    \setbox\captionbox\hbox{%
    {\figurecaptionfont#1\unskip\ignorespaces\hskip1em#2\par}}
    \ifdim\wd\captionbox>\hsize
    {\figurecaptionfont#1\unskip\ignorespaces\hskip1em#2\par}\par
    \else
    \centerline{\figurecaptionfont#1\unskip\ignorespaces\hskip1em#2}\par
    \fi\else\global\setbox\captionbox\hbox{%
    {\tablecaptionfont#1\ignorespaces\hskip1em#2\par}}
    \fi
}
%
\def\figsubcap#1{\par\noindent\centering\footnotesize(#1)}
\def\figurename{Fig.}
\newcounter{figure}[chapter]
\renewcommand\thefigure{\thechapter.\@arabic\c@figure}
\def\fps@figure{htbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{\figurename~\thefigure\nobreak}
\newenvironment{figure}
               {\@float{figure}}
               {\end@float}
\newenvironment{figure*}
               {\@dblfloat{figure}}
               {\end@dblfloat}
%
\newenvironment{tablehere}
  {\parskip0pt\topskip 13pt plus 2pt minus 1pt\def\@captype{table}\begin{small}\begin{center}}
  {\end{center}\end{small}\vskip 13pt plus 2pt minus 1pt}
  
\newenvironment{figurehere}
  {\parskip0pt\topskip 13pt plus 2pt minus 1pt\def\@captype{figure}}
  {\vskip 13pt plus 2pt minus 1pt}
%
\def\sbs{\hsize=.45\textwidth\parindent=0pt\centering}
\long\def\sidebyside#1#2{\figurecaptionfont
\hbox to\textwidth{\vtop{\sbs#1\vskip1sp}\hfill\vtop{\sbs#2}}}
%
\newenvironment{boxedtxt}
               {\par\vskip 6pt plus2pt minus1pt
        \parindent0pt\leftskip0pt
        \itemindent0pt\leftmargin0pt
        \fboxsep=8pt
        \normalfont
        \begin{boxedminipage}[0.5pt]{\hsize}}
               {\par\end{boxedminipage}
        \vskip 6pt plus2pt minus1pt}
%
\def\ArtDir{\jobname/}
\def\figurebox#1#2#3{%
        \def\arg{#3}%
        \ifx\arg\empty
        {\hfill\vbox{\hsize#2\hrule\hbox to #2{%
                \vrule\hfill\vbox to #1{\hsize#2\vfill}\vrule}\hrule}\hfill}%
        \else
        \hbox to \hsize{\hfill\epsfbox{\ArtDir#3.eps}\hfill}
        \fi}
%
\InputIfFileExists{psfig.sty}{\typeout{^^Jpsfig.sty inputed...ok}}{\typeout{^^JWarning: psfig.sty could be be found.^^J}}
\InputIfFileExists{epsfsafe.tex}{\typeout{^^Jepsfsafe.tex inputed...ok}}
            {\typeout{^^JWarning: epsfsafe.tex could not be found.^^J}}
\InputIfFileExists{epsfig.sty}{\typeout{^^Jepsfig.sty inputed...ok}}{\typeout{^^JWarning: epsfig.sty could not be found.^^J}}
\InputIfFileExists{epsf.sty}{\typeout{^^Jepsf.sty inputed...ok}}{\typeout{^^JWarning: epsf.sty could not be found.^^J}}%
%
\def\ArtWork#1{\noindent\hfill\epsfbox{#1}\hfill}%
%
\newcounter{table}[chapter]
\renewcommand\thetable{\thechapter.\@arabic\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{Table~\thetable}
\newenvironment{table}
               {\@float{table}}
               {\vskip5pt\end@float}
\newenvironment{table*}
               {\@dblfloat{table}}
               {\end@dblfloat}
%
\newdimen\tabledim
%
\long\def\tbl#1#2{%
    \parindent\z@\ignorespaces\noindent\tablecaptionfont
    \caption{#1}%
    \par\setbox\tempbox\hbox{\tablefont #2}%
    \tablewd\hsize\advance\tablewd-\wd\tempbox\global\divide\tablewd\tw@
    \ifdim\wd\captionbox<\wd\tempbox\centerline{\unhbox\captionbox}
    \else\leftskip\tablewd\rightskip\leftskip{\unhbox\captionbox}\par
    \fi\centerline{\box\tempbox}
}%
%
\def\Hline{\noalign{\ifnum0=`}\fi\hrule \@height .5pt
        \futurelet\@tempa\@xhline}%
\def\toprule{\\[-6pt]\hline\\[-5.5pt]}
\def\colrule{\\[-7.5pt]\hline\\[-5.5pt]}
\def\botrule{\\[-7pt]\hline\\[-8.5pt]}
\newcommand{\centre}[2]{\multicolumn{#1}{c}{#2}}
%
\newenvironment{tabnote}{%
    \par\vskip5pt\footnotesize%
    \@ifnextchar[{\@tabnote}{\@tabnote[]}}{%
    \par\vskip-5pt}
\def\@tabnote[#1]{%
    \leftskip\tablewd\rightskip\leftskip {\it #1} \ignorespaces}
%
\long\def\ttbl#1#2#3{%
    \parindent\z@
    \noindent
    \tablecaptionfont
    \caption{#2}%
    \tablewd\hsize
    \advance\tablewd-#1
    \global\divide\tablewd\tw@
    \ifdim \wd\captionbox<#1
    \centerline{\unhbox\captionbox}
    \else
    {\leftskip\tablewd% plus1fill{}
    \rightskip\tablewd% plus1fill{}
    \unhbox\captionbox\par}
    \fi
    \vskip-8pt
    \centerline{#3}
}%
%
\newenvironment{InTextTable}{%
    \par\addvspace{16pt plus6pt minus6pt}%
    \begin{center}\tablecaptionfont\noindent\ignorespaces
}{%
    \end{center}\par\addvspace{17pt plus6pt minus6pt}}
%
%%%%%%%%%%%%%%%%%%%%%% Flush Left Lists %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\@listI{\leftmargin\leftmargini
            \listparindent\itemindent
            \parsep \z@\labelsep.5em
            \topsep 6.5\p@ \@plus3\p@% \@minus2\p@
            \itemsep0\p@}
%
\let\@listi\@listI
\@listi
%
\def\@listii {\leftmargin\leftmarginii
              \labelwidth\leftmarginii
              \advance\labelwidth-\labelsep
              \topsep    3.5\p@ \@plus2\p@ \@minus\p@
              \parsep    \z@
              \itemsep\z@}
\def\@listiii{\leftmargin\leftmarginiii
              \labelwidth\leftmarginiii
              \advance\labelwidth-\labelsep
              \topsep 3.5\p@ \@plus\p@\@minus\p@
              \parsep \z@
             % \partopsep \p@ \@plus\z@ \@minus\p@
              \itemsep0\p@}%
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
\setlength\leftmargini{3pc}
\setlength\leftmarginii{2.2em}
\setlength\leftmarginiii{1.87em}
\setlength\leftmarginiv{1.7em}
\setlength\leftmarginv{1em}
\setlength\leftmarginvi{1em}
\setlength\leftmargin{\leftmargini}
\setlength\listparindent{\parindent}
\setlength\labelsep{.5em}
\setlength\labelwidth{\leftmargini}
\addtolength\labelwidth{-\labelsep}
\renewcommand\theenumi{\arabic{enumi}}
\renewcommand\theenumii{\alph{enumii}}
\renewcommand\theenumiii{\roman{enumiii}}
\renewcommand\theenumiv{\Alph{enumiv}}
\newcommand\labelenumi{(\theenumi)}
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
\def\enummax#1{\setbox\tempbox=\hbox{#1\hskip\labelsep}%
  \expandafter\global\csname leftmargin\romannumeral\the\@enumdepth\endcsname\wd\tempbox}
%
\def\enumerate{\@ifnextchar[{\@enumerate}{\@enumerate[\csname label\@enumctr\endcsname]}}
%
\def\@enumerate[#1]{\ifnum \@enumdepth >3 \@toodeep\else
      \advance\@enumdepth \@ne\edef\@enumctr{enum\romannumeral\the\@enumdepth}%
      \enummax{#1}\list
      {\csname label\@enumctr\endcsname}{\usecounter
        {\@enumctr}\def\makelabel##1{\hss\llap{##1}}}\fi}
%
\let\Item\item
\newenvironment{enumeroman}{%
  \def\theenumi{\roman{enumi}}\def\theenumii{\roman{enumii}}%
  \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
        \let\item\Item
  \begin{enumerate}%
}{%
  \end{enumerate}}
%
\newenvironment{alphlist}{%
  \def\theenumi{\alph{enumi}}\def\theenumii{\alph{enumii}}%
  \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
        \let\item\Item
  \begin{enumerate}%
}{%
  \end{enumerate}}
%
\newenvironment{Alphlist}{%
  \def\theenumi{\Alph{enumi}}\def\theenumii{\Alph{enumii}}%
  \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
        \let\item\Item
  \begin{enumerate}%
}{%
  \end{enumerate}}
%
\newenvironment{arabiclist}{%
    \def\theenumi{\arabic{enumi}}\def\theenumii{\arabic{enumii}}
   \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}% %(2)
%  \def\labelenumi{\theenumi.}\def\labelenumii{(\theenumii)}%  %2. 
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{romanlist}{%
    \def\theenumi{\roman{enumi}}\def\theenumii{\roman{enumii}}
    \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{Romanlist}{%
    \def\theenumi{\Roman{enumi}}\def\theenumii{\Romanlist{enumii}}
    \def\labelenumi{(\theenumi)}\def\labelenumii{(\theenumii)}%
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{itemlist}{%
    \def\labelenumi{\labelitemi}
    \let\item\Item
    \begin{enumerate}
    }{%
    \end{enumerate}}
%
\newenvironment{unnumlist}{%
    \let\item\Item
    \leftmargini2pc
    \ifnum \@enumdepth >3 \@toodeep\else
        \advance\@enumdepth \@ne
        \list{}{\itemindent-2pc\topsep6pt
              \def\makelabel##1{\hss\llap{##1}}}%
   \fi
}{%
  \endlist}
%
\newenvironment{description}
               {\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
                                \normalfont\bfseries #1}
\newenvironment{verse}
               {\let\\\@centercr
                \list{}{\itemsep      \z@
                        \itemindent   -1.5em%
                        \listparindent\itemindent
                        \rightmargin  \leftmargin
                        \advance\leftmargin 1.5em}%
                \item\relax}
               {\endlist}
\newenvironment{quotation}
               {\list{}{\listparindent 1.5em%
                        \itemindent    \listparindent
                        \rightmargin   \leftmargin
                        \parsep        \z@ \@plus\p@}%
                \item\relax}
               {\endlist}
\newenvironment{quote}
               {\small\list{}{\rightmargin\leftmargin}%
                \item\relax}
               {\endlist}
%
%%%%%%%%%%%%%%%%%%%%%%%% End Flush Left Lists %%%%%%%%%%%%%%%%%%%%%%%%%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Theorem %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\@xthm#1#2{\@begintheorem{#2}{\csname the#1\endcsname}{}\ignorespaces}
\def\@ythm#1#2[#3]{\@opargbegintheorem{#2}{\csname the#1\endcsname}{#3}\ignorespaces}
\def\@opargbegintheorem#1#2#3{\trivlist
      \item[\hskip \labelsep{\bfseries #1\ #2\ (#3).}]\itshape}
%
\let\Algorithmfont\upshape
\let\Answerfont\upshape
\let\Assertionfont\upshape
\let\Assumptionfont\upshape
\let\Casefont\upshape
\let\Commentfont\upshape
\let\Conditionfont\upshape
\let\Conventionfont\upshape
\let\Criterionfont\upshape
\let\Definitionfont\upshape
\let\Notationfont\upshape
\let\Notefont\upshape
\let\Remarkfont\upshape
\let\Examplefont\upshape
\let\Propfont\upshape
\let\Observationfont\upshape
\let\Solutionfont\upshape
\let\Stepfont\upshape
\let\Summaryfont\upshape
\let\Claimfont\itshape
\let\Conjecturefont\itshape
\let\Corollaryfont\itshape
\let\Lemmafont\itshape
\let\Theoremfont\itshape
\let\Problemfont\itshape
\let\Propositionfont\itshape
\let\Questionfont\itshape
%
\let\Algorithmheadfont\bfseries
\let\Answerheadfont\bfseries
\let\Assertionheadfont\bfseries
\let\Assumptionheadfont\bfseries
\let\Caseheadfont\bfseries
\let\Claimheadfont\bfseries
\let\Commentheadfont\bfseries
\let\Conditionheadfont\bfseries
\let\Conjectureheadfont\bfseries
\let\Conventionheadfont\bfseries
\let\Corollaryheadfont\bfseries
\let\Criterionheadfont\bfseries
\let\Definitionheadfont\bfseries
\let\Exampleheadfont\bfseries
\let\Lemmaheadfont\bfseries
\let\Notationheadfont\bfseries
\let\Noteheadfont\bfseries
\let\Observationheadfont\bfseries
\let\Problemheadfont\bfseries
\let\Propheadfont\bfseries
\let\Propositionheadfont\bfseries
\let\Questionheadfont\bfseries
\let\Remarkheadfont\bfseries
\let\Solutionheadfont\bfseries
\let\Stepheadfont\bfseries
\let\Summaryheadfont\bfseries
\let\Theoremheadfont\bfseries
%
\def\@begintheorem#1#2#3{\par\addvspace{8pt plus3pt minus2pt}%
              \noindent{\csname#1headfont\endcsname#1\ \ignorespaces#3 #2.}%
              \csname#1font\endcsname\hskip.4em\ignorespaces}
\def\@endtheorem{\par\addvspace{8pt plus3pt minus2pt}\@endparenv}
%
\newtheorem{theorem}{Theorem}[chapter]
\if@onethmnum
    \newtheorem{algorithm}[theorem]{Algorithm}
%   \newtheorem{answer}[theorem]{Answer}
    \newtheorem{assertion}[theorem]{Assertion}
    \newtheorem{assumption}[theorem]{Assumption}
%   \newtheorem{case}[theorem]{Case}
    \newtheorem{claim}[theorem]{Claim}
%   \newtheorem{comment}[theorem]{Comment}
%   \newtheorem{condition}[theorem]{Condition}
    \newtheorem{conjecture}[theorem]{Conjecture}
    \newtheorem{convention}[theorem]{Convention}
    \newtheorem{corollary}[theorem]{Corollary}
    \newtheorem{criterion}[theorem]{Criterion}
    \newtheorem{definition}[theorem]{Definition}
    \newtheorem{example}[theorem]{Example}
    \newtheorem{lemma}[theorem]{Lemma}
    \newtheorem{notation}[theorem]{Notation}
%   \newtheorem{note}[theorem]{Note}
    \newtheorem{observation}[theorem]{Observation}
    \newtheorem{problem}[theorem]{Problem}
    \newtheorem{prop}[theorem]{Prop}
    \newtheorem{proposition}[theorem]{Proposition}
    \newtheorem{question}[theorem]{Question}
    \newtheorem{remark}[theorem]{Remark}
%   \newtheorem{solution}[theorem]{Solution}
%   \newtheorem{step}[theorem]{Step}
%   \newtheorem{summary}[theorem]{Summary}
\else
    \newtheorem{algorithm}{Algorithm}[chapter]
%   \newtheorem{answer}{Answer}[chapter]
    \newtheorem{assertion}{Assertion}[chapter]
    \newtheorem{assumption}{Assumption}[chapter]
%   \newtheorem{case}{Case}[chapter]
    \newtheorem{claim}{Claim}[chapter]
%   \newtheorem{comment}{Comment}[chapter]
%   \newtheorem{condition}{Condition}[chapter]
    \newtheorem{conjecture}{Conjecture}[chapter]
    \newtheorem{convention}{Convention}[chapter]
    \newtheorem{corollary}{Corollary}[chapter]
    \newtheorem{criterion}{Criterion}[chapter]
    \newtheorem{definition}{Definition}[chapter]
    \newtheorem{example}{Example}[chapter]
    \newtheorem{lemma}{Lemma}[chapter]
    \newtheorem{notation}{Notation}[chapter]
%   \newtheorem{note}{Note}[chapter]
    \newtheorem{observation}{Observation}[chapter]
    \newtheorem{problem}{Problem}[chapter]
    \newtheorem{prop}{Prop}[chapter]
    \newtheorem{proposition}{Proposition}[chapter]
    \newtheorem{question}{Question}[chapter]
    \newtheorem{remark}{Remark}[chapter]
%   \newtheorem{solution}{Solution}[chapter]
%   \newtheorem{step}{Step}[chapter]
%   \newtheorem{summary}{Summary}[chapter]
\fi
%
\newenvironment{answer}[1][Answer]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{case}[1][Case]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{comment}[1][Comment]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{note}[1][Note]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{solution}[1][Solution]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{step}[1][Step]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newenvironment{summary}[1][Summary]{\begin{trivlist}
\item[\hskip \labelsep {\bfseries #1.}]}{\end{trivlist}}
%
\newif\if@halmos
\@halmostrue
\font\msa=msam10 at 10pt
\def\halmos{\par
    \vskip-\baselineskip
    \hbox to \hsize{\hfill\hskip1.5pc\msa\char'003}\global\@halmosfalse}
%
\newenvironment{proof}{%
 \par\addvspace{6pt plus3pt minus2pt}%
 \noindent{\bfseries\itshape Proof.}\hskip1pc\ignorespaces}{%
  \if@halmos\halmos\fi
  \par\addvspace{6pt plus3pt minus2pt}\@endparenv\global\@halmostrue}
%
\setlength\arraycolsep{1.5pt}   
\setlength\tabcolsep{6\p@}
\setlength\arrayrulewidth{.4\p@}
\setlength\doublerulesep{2\p@}
\setlength\tabbingsep{\labelsep}
\skip\@mpfootins = \skip\footins
\setlength\fboxsep{3\p@}
\setlength\fboxrule{.4\p@}
%
\@addtoreset{equation}{chapter}
\renewcommand\theequation{\thechapter.\@arabic\c@equation} %(1.1)
%\renewcommand\theequation{\@arabic\c@equation} %(1)
%
%
%\renewcommand\footnoterule{}  %comment this defition to have footnote rule
%\renewcommand\thefootnote{\fnsymbol{footnote}}
\renewcommand\thefootnote{\arabic{footnote}}
\@addtoreset{footnote}{chapter}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%TURNLINES FLUSH LEFT:-
\long\def\@makefntext#1{\parindent 1em\noindent
\hb@xt@1em{\hss$^{\@thefnmark}$}#1}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\RightMark{}
\def\authormark#1{\relax}
\mark{{}{}}
%
\def\ps@plain{%
     \let\@mkboth\@gobbletwo
     \let\@oddhead\@empty
     \def\@oddfoot{\hfill\rhfont\thepage\hfill}%
     \let\@evenhead\@empty
     \let\@evenfoot\@oddfoot}
%
\def\ps@csrhead{%
   \let\@oddfoot\@empty\let\@evenfoot\@empty
   \def\@evenhead{\hbox to 4.5in{\rhfont\thepage\hfil{\it\leftmark}}}%
   \def\@oddhead{\hbox to 4.5in{\rhfont{\it\rightmark}\hfil\thepage}}%
   \let\@mkboth\markboth
   \def\chaptermark##1{\markboth{##1}{}}%
   \def\sectionmark##1{\markright{\thesection.\enskip##1}}}
%
\def\ps@bcrhead{%
   \let\@oddfoot\@empty\let\@evenfoot\@empty
   \def\@evenhead{{\rhfont\thepage}\hfill{\rhfont\it\booktitle}\hfill}%
   \def\@oddhead{\hfill{\rhfont\it\rightmark}\hfill{\rhfont\thepage}}%
   \let\@mkboth\@gobbletwo
   \def\chaptermark##1{\markboth{}{##1}}%
   \let\sectionmark\@gobble}
%
\def\ps@acrhead{%
    \let\@oddfoot\@empty
    \let\@evenfoot\@empty
    \def\@evenhead{{\rhfont\thepage}\hfill{\rhfont\it\leftmark}\hfill}%
    \def\@oddhead{\hfill{\rhfont\it\rightmark}\hfill{\rhfont\thepage}}%
    \let\@mkboth\markboth
    \def\authormark##1{\markboth{##1}{\RightMark}}%
    \def\chaptermark##1{\gdef\RightMark{##1}\markright{##1}}}%
%
\def\foliofont{\fontsize{8}{8}\selectfont}
%      
\def\ps@pblind{%    
    \let\@mkboth\@gobbletwo
    \def\@evenhead{}%
    \def\@oddhead{}
    \def\@oddfoot{\hfill\mbox{\lower7.5pc\hbox{\rhfont\thepage}}\hfill}
   \let\@evenfoot\@oddfoot}
%
%%%%%%%%%%%%%%%%%%%%%%%%Appendix %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcounter{appendix}
\renewcommand\theappendix{\Alph{appendix}}
\def\@makeappendixhead#1{%
    \vbox to 115pt{%
    \refstepcounter{appendix}
%   \addcontentsline{toc}{chapter}{\appname#1}
    \vspace*{37pt}
    \parindent\z@\raggedright\reset@font
    {\centering{{\CNfont\appendixname\  \theappendix}\par
    \vskip 0.25in
    \vbox{\CTfont #1}\par}\par}\nobreak
    \vfill\@afterheading\@afterindentfalse}\vskip0.4in}
%
\newenvironment{appendix}{\par%
    \let\@makechapterhead\@makeappendixhead
    \let\thechapter\theappendix%
    \renewcommand\outchapter{\appname\unskip\hspace*{1em}}
    \renewcommand\thesection{\thechapter.\arabic{section}}%
    \renewcommand\thetable{\thechapter.\arabic{table}}%
    \@addtoreset{equation}{appendix}
    \@addtoreset{section}{appendix}
    \@addtoreset{table}{appendix}
    \@addtoreset{figure}{appendix}
}
    {\par\vfill\pagebreak\par}%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%End Appendix %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcounter{ptitl}
\def\aindx#1{\index{author}{#1}}    %For Author index
\def\sindx#1{\index{subject}{#1}}   %For Subject index
\def\gindx#1{\index{general}{#1}}   %For General index
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Index %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newenvironment{theindex}
    {\cleardoublepage
     \raggedright\small
    \if@twocolumn
            \@restonecolfalse
         \else
            \@restonecoltrue
         \fi
         \columnseprule \z@
         \columnsep 18\p@
                \twocolumn[\chapter*{\indexname}
    %\@makeschapterhead{\indexname}
     \vspace*{4pt}]%
                %\@mkboth{\indexname}{\indexname}%
        \@mkboth{\booktitle}{Index}
        \if@csrhead
        \markboth{Index}{Index}
        \fi
        \if@acrhead
        \markboth{Index}{Index}
        \fi        
                \thispagestyle{plain}\parindent\z@
                \parskip\z@ \@plus .3\p@\relax
                \let\item\@idxitem}
               {\if@restonecol\onecolumn\else\clearpage\fi}
\newcommand\@idxitem  {\par\hangindent 1em}
\newcommand\subitem   {\par\hangindent 40\p@ \hspace*{10\p@}}
\newcommand\subsubitem{\par\hangindent 40\p@ \hspace*{30\p@}}
\newcommand\indexspace{\par\vskip\baselineskip}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Index %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcommand{\sitem}{\hspace*{20pt}}
\newcommand{\ssitem}{\hspace*{30pt}}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CITATIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\@up#1{\raise.2ex\hbox{#1}}
%
\let\@internalcite\cite
\def\cite{\def\@citeseppen{-1000}%
\def\@cite##1##2{\@up[\nobreak\hskip 0in{##1\if@tempswa , ##2\fi}\@up]}%
    \def\citeauthoryear##1##2{##1, ##2}\@internalcite}
\def\shortcite{\def\@citeseppen{1000}%
    \def\@cite##1##2{\@up[{##1\if@tempswa , ##2\fi}\@up]}%
\def\citeauthoryear##1##2{##2}\@internalcite}
%
\def\citeauthor#1{\def\@citeseppen{1000}%
    \def\@cite##1##2{{##1\if@tempswa , ##2\fi}}%
\def\citeauthoryear##1##2{##1}\@citedata{#1}}
    \def\citeyear#1{\def\@citeseppen{1000}%
\def\@cite##1##2{{##1\if@tempswa , ##2\fi}}%
\def\citeauthoryear##1##2{##2}\@citedata{#1}}
%
\def\@citedata#1{\@tempswafalse%
    \if@filesw\immediate\write\@auxout{\string\citation{#1}}\fi
  \def\@citea{}\@cite{\@for\@citeb:=#1\do
    {\@citea\def\@citea{,\penalty\@citeseppen\ }\@ifundefined
       {b@\@citeb}{{\bf ?}\@warning
       {Citation `\@citeb' on page \thepage \space undefined}}%
{\csname b@\@citeb\endcsname}}}{}}
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout{\string\citation{#2}}\fi
 \let\@citea\@empty
  \@cite{\@for\@citeb:=#2\do
    {\@citea\def\@citea{;\penalty\@citeseppen\ }%
     \def\@tempa##1##2\@nil{\edef\@citeb{\if##1\space##2\else##1##2\fi}}%
     \expandafter\@tempa\@citeb\@nil
     \@ifundefined{b@\@citeb}{{\bf ?}\@warning
       {Citation `\@citeb' on page \thepage \space undefined}}%
     {\csname b@\@citeb\endcsname}}}{#1}}
\def\@biblabel#1{\def\citeauthoryear##1##2{##1, ##2}}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CITATIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\newcommand{\eref}[1]{Eq.~(\ref{#1})}
\newcommand{\sref}[1]{Sec.~\ref{#1}}
\newcommand{\cref}[1]{Chap.~\ref{#1}}
\newcommand{\fref}[1]{Fig.~\ref{#1}}
\newcommand{\tref}[1]{Table~\ref{#1}}
\newcommand{\Eref}[1]{Equation (\ref{#1})}
\newcommand{\Sref}[1]{Section~\ref{#1}}
\newcommand{\Cref}[1]{Chapter~\ref{#1}}
\newcommand{\Fref}[1]{Figure~\ref{#1}}
\newcommand{\Tref}[1]{Table~\ref{#1}}
%
%%%%%%%%%%%%%%% BIBLIOGRAPHY TEXT WITHOUT NOS. AT END OF BOOK %%%%%%%%%%%%%%
%%
\def\bibfont{\fontsize{9}{11}\selectfont}
%
\newenvironment{thebibliography}[1]{\chapter*{Bibliography}
    \markboth{\booktitle}{Bibliography}
\if@csrhead
\markboth{Bibliography}{Bibliography}
\fi
\if@acrhead
\markboth{Bibliography}{Bibliography}
\fi    
    \bibfont
    \vspace*{4pt}
    \list{\@biblabel{\arabic{enumiv}}}
    {\settowidth\labelwidth{\@biblabel{#1}}%
        \leftmargin\labelwidth\advance\leftmargin by 24pt
    \labelsep0pt\itemsep0pt\parsep0pt\itemindent-24pt
    \advance\leftmargin\labelsep
    \usecounter{enumiv}%
        \let\p@enumiv\@empty
        \def\theenumiv{\arabic{enumiv}}}%
        \def\newblock{\hskip .11em plus.33em minus.07em}%
        \sloppy\clubpenalty4000\widowpenalty4000
        \sfcode`\.=1000\relax}
     {\def\@noitemerr
       {\@latex@warning{Empty `thebibliography' environment}}%
      \endlist}
%
\newcommand\newblock{}
%
\def\bib{B\kern-.05em{I}\kern-.025em{B}\kern-.08em}
\def\btex{B\kern-.05em{I}\kern-.025em{B}\kern-.08em\TeX}
%
%%%%%%%%%%%%%%% BIBLIOGRAPHY TEXT WITHOUT NOS. AT END OF BOOK %%%%%%%%%%%%%%
%
\def\makerobust#1{%
  \@ifundefined{fr@gile\expandafter\@gobble\string#1}%
  {\expandafter
    \let\csname fr@gile\expandafter\@gobble\string#1\endcsname#1%
   \edef#1{\noexpand\protect\expandafter\noexpand
               \csname fr@gile\expandafter\@gobble\string#1\endcsname}}%
  {\@warning{\string#1 is already robust (command ignored)}}}
%
\newenvironment{titlepages}{%
    \thispagestyle{pblind}
    \renewcommand\thepage{\roman{page}}
    \par\ \par
    \vfill
    Publishers' page\par
    \vfill
    \pagebreak\par
    \thispagestyle{pblind}
    \ \par
    \vfill
    Publishers' page\par
    \vfill
    \pagebreak\par
    \thispagestyle{pblind}
    \ \par
    \vfill
    Publishers' page\par
    \vfill
    \pagebreak\par
    \thispagestyle{pblind}
    \ \par
    \vfill
    Publishers' page\par
    \vfill
    \pagebreak\par}{}%
%   
\newenvironment{preface}{%
    \cleardoublepage
    \chapter*{\prefacename}\normalsize
\markboth{\booktitle}{Preface}
\if@csrhead
\markboth{Preface}{Preface}
\fi
\if@acrhead
\markboth{Preface}{Preface}
\fi
}{\par\vfill\pagebreak\par}%
%
\newenvironment{foreword}{%
    \cleardoublepage
    \chapter*{\forewordname}\normalsize
\markboth{\booktitle}{Foreword}
\if@csrhead
\markboth{Foreword}{Foreword}
\fi
\if@acrhead
\markboth{Foreword}{Foreword}
\fi
}{\par\vfill\pagebreak\par}%
%
\newenvironment{acknowledgments}{%
    \cleardoublepage
    \chapter*{\acknowledgmentsname}\normalsize
\markboth{\booktitle}{Acknowledgments}
\if@csrhead
\markboth{Acknowledgments}{Acknowledgments}
\fi
\if@acrhead
\markboth{Acknowledgments}{Acknowledgments}
\fi
}{\par\vfill\pagebreak\par}%
%
\newenvironment{acknowledgment}{%
    \cleardoublepage
    \chapter*{\acknowledgmentname}\normalsize
\markboth{\booktitle}{Acknowledgment}
\if@csrhead
\markboth{Acknowledgment}{Acknowledgment}
\fi
\if@acrhead
\markboth{Acknowledgment}{Acknowledgment}
\fi
}{\par\vfill\pagebreak\par}%
%
\newenvironment{tblofcontents}{%
    \markboth{\booktitle}{Contents}
    \if@csrhead
    \markboth{Contents}{Contents}
    \fi
    \if@acrhead
    \markboth{Contents}{Contents}
    \fi
    \renewcommand\thepage{\roman{page}}
    \tableofcontents}{\par\vfill\pagebreak\par}%
%
\newenvironment{dedication}{%
    \normalsize
    \thispagestyle{pblind}
    \renewcommand\thepage{\roman{page}}
    \ \par
    \vspace*{10pc}\begin{center}}{\end{center}\par\vfill\par}%
%   
%%%%%%%%%%%%%%%%%%%%%%%%INVERTED L CROPMARKS%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\cropmarks{\nointerlineskip%
    \vbox to 0pt{\vskip-8.6pt\parindent0pt\infotag 
    \par\advance\trimheight by 2pc
    \vbox to \trimheight{\offinterlineskip\parindent0pt\vskip-2pc
    \hbox to \trimwidth{\raise2.6pc\hbox{\vrule height2pc width0.3pt}\raisebox{2pc}
   {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill\raisebox{2pc}
   {\vrule width2pc height0.3pt}\hspace*{-2.6pc}\raise2.6pc\hbox{\vrule height2pc width0.3pt}}\vfill
     \hbox to \trimwidth{\lower1.2pc\hbox{\vrule height2pc width0.3pt}\raisebox{1.4pc}
       {\hspace*{-2.6pc}\vrule width2pc height0.3pt}\hfill 
    \lower1.2pc\hbox{\vrule height2pc width0.3pt}\rlap{\hspace*{0.6pc}\raisebox{1.4pc}{\vrule width2pc height0.3pt}}}}}  %for draftrule
     \vbox to 0pt{\offinterlineskip\parindent0pt\vskip\topmargin%\vskip1.3pt
     \tempdimen\normaltextheight
     \advance\tempdimen27pt
     \moveright\@themargin
     \vbox to \tempdimen{\hrule height\draftrule 
     \vbox to 0pt{\vskip\headheight\vskip\headsep%\vskip3pt
                \vrule height\draftrule width\textwidth}%
      \hbox to \textwidth{\vrule width\draftrule \hfill\vrule height\tempdimen width\draftrule}%
           \hrule height\draftrule}}}%%
%
%%%%%%%%%%%%%%%%%%%%%%%%INVERTED L CROPMARKS%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
\def\@outputpage{%
\begingroup           % the \endgroup is put in by \aftergroup
  \let \protect \noexpand
  \@resetactivechars
  \let\-\@dischyph
  \let\'\@acci\let\`\@accii\let\=\@acciii
  \let\\\@normalcr
  \let\par\@@par
  \shipout \vbox{%
    \set@typeset@protect
    \aftergroup \endgroup
    \aftergroup \set@typeset@protect
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
    \@begindvi\cropmarks
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
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% RUNNING HEAD %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Even Pg: Book Title, Odd Pg: Chapter Title, *default style*
\if@bcrhead\pagestyle{bcrhead}\fi
%
% Even Pg: Chapter Title, Odd Pg: Section Title
\if@csrhead\pagestyle{csrhead}\fi
%
% Even Pg: Author Name, Odd Pg: Chapter Title
\if@acrhead\pagestyle{acrhead}\fi
%
\pagenumbering{arabic}
\makerobust{\cite}
\def\sloppy{\tolerance=100000\hfuzz=\maxdimen\vfuzz=\maxdimen}
\vbadness=12000
\hbadness=12000
\sloppy
\usepackage[figuresright]{rotating}
\usepackage{amsfonts,amssymb,amsmath}
\usepackage{boxedminipage}
\usepackage{makeidx}
\usepackage{url}
\flushbottom
\onecolumn
\endinput
%
