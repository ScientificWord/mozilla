\def\currversion{October 11, 2001}
%% This file is bjour.cls/sty
%%
%% It can be renamed to either
%% bjour.cls or bjour.sty depending on whether you are
%% using LaTeX2.09 (bjour.sty) or LaTeX2e (bjour.cls).
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% LaTeX Style file for Articles in
%% `B' Style  Journals Published by
%% Academic Press
%%
%% Prepared by Amy Hendrickson, TeXnology Inc.             
%%                                                            
%% TeXnology Inc.: 617 738-8029                            
%%             amyh@TeXnology.com
%%                                                          
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\typeout{^^J^^J
`B' Journal Style, Academic Press
^^J
This version is dated \currversion^^J^^J}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Switch for final production, to put in copyright info.
\newif\iffinaltypesetters
\def\finaltypesetting{\global\finaltypesetterstrue}
%%%%% 2e switches ===>>
\newif\ifll
\expandafter\ifx\csname LaTeXe\endcsname\relax % LaTeX2.09 is used
\else% LaTeX2e is being used, so set ll true
\global\lltrue\fi
\ifll
\else
\gdef\@plus{plus}
\gdef\@minus{minus}
\global\@maxsep 20pt
\global\@dblmaxsep 20pt
\fi
%% 2e ==>>>
\newif\if@openbib
%\@openbibtrue
\@openbibfalse
\newif\if@openright
\newif\if@mainmatter
\@mainmattertrue
\newif\if@restonecol
\newif\if@titlepage
\newif\ifdraft
\ifll \let\dooptions\ProcessOptions
\else
\let\dooptions\@options\fi
\dooptions
\ifll
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{bjour}
              [05/15/1999]
\newcommand{\@ptsize}{}
\renewcommand{\normalsize}{%
\ifdraft
   \@setfontsize\normalsize\@xpt\@xviipt
\else
   \@setfontsize\normalsize\@xpt\@xiipt
\fi
   \abovedisplayskip 10\p@ \@plus2\p@ \@minus5\p@
   \abovedisplayshortskip \z@ \@plus3\p@
   \belowdisplayshortskip 6\p@ \@plus3\p@ \@minus3\p@
   \belowdisplayskip \abovedisplayskip
   \let\@listi\@listI}
\normalsize
\newcommand{\small}{%
\ifdraft
   \@setfontsize\small\@ixpt{17}%
\else
   \@setfontsize\small\@ixpt{11}%
\fi
   \abovedisplayskip 8.5\p@ \@plus3\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus2\p@
   \belowdisplayshortskip 4\p@ \@plus2\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 4\p@ \@plus2\p@ \@minus2\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand{\footnotesize}{%
\ifdraft
   \@setfontsize\footnotesize\@viiipt{17}%
\else
   \@setfontsize\footnotesize\@viiipt{9.5}%
\fi
   \abovedisplayskip 6\p@ \@plus2\p@ \@minus4\p@
   \abovedisplayshortskip \z@ \@plus\p@
   \belowdisplayshortskip 3\p@ \@plus\p@ \@minus2\p@
   \def\@listi{\leftmargin\leftmargini
               \topsep 3\p@ \@plus\p@ \@minus\p@
               \parsep 2\p@ \@plus\p@ \@minus\p@
               \itemsep \parsep}%
   \belowdisplayskip \abovedisplayskip
}
\newcommand{\scriptsize}{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand{\tiny}{\@setfontsize\tiny\@vpt\@vipt}
\newcommand{\large}{\@setfontsize\large\@xiipt{14}}
\newcommand{\Large}{\@setfontsize\Large\@xivpt{18}}
\newcommand{\LARGE}{\@setfontsize\LARGE\@xviipt{22}}
\newcommand{\huge}{\@setfontsize\huge\@xxpt{25}}
\newcommand{\Huge}{\@setfontsize\Huge\@xxvpt{30}}
\@maxdepth\maxdepth
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*{\cal}{\@fontswitch{\relax}{\mathcal}}
\DeclareRobustCommand*{\mit}{\@fontswitch{\relax}{\mathnormal}}
\else %% LaTeX2.09
%% To get bold super and sub-scripts in the right size:
\gdef\xpt{\textfont\z@\tenrm
  \scriptfont\z@\sevrm \scriptscriptfont\z@\fivrm
\textfont\@ne\tenmi \scriptfont\@ne\sevmi \scriptscriptfont\@ne\fivmi
\textfont\tw@\tensy \scriptfont\tw@\sevsy \scriptscriptfont\tw@\fivsy
\textfont\thr@@\tenex \scriptfont\thr@@\tenex \scriptscriptfont\thr@@\tenex
\def\unboldmath{\everymath{}\everydisplay{}\@nomath\unboldmath
          \textfont\@ne\tenmi
          \textfont\tw@\tensy \textfont\lyfam\tenly
          \@boldfalse}\@boldfalse
\def\boldmath{\@ifundefined{tenmib}{\global\font\tenmib\@mbi
   \global\font\tensyb\@mbsy
   \global\font\tenlyb\@lasyb\relax\@addfontinfo\@xpt
   {\def\boldmath{\everymath{\mit}\everydisplay{\mit}\@prtct\@nomathbold
        \textfont\@ne\tenmib \textfont\tw@\tensyb
        \textfont\lyfam\tenlyb \@prtct\@boldtrue}}}{}\@xpt\boldmath}%
\def\prm{\fam\z@\tenrm}%
\def\pit{\fam\itfam\tenit}\textfont\itfam\tenit \scriptfont\itfam\sevit
    \scriptscriptfont\itfam\sevit
\def\psl{\fam\slfam\tensl}\textfont\slfam\tensl
     \scriptfont\slfam\tensl \scriptscriptfont\slfam\tensl
\def\pbf{\fam\bffam\tenbf}\textfont\bffam\tenbf
    \scriptfont\bffam\sevbf \scriptscriptfont\bffam\fivbf
\def\ptt{\fam\ttfam\tentt}\textfont\ttfam\tentt
    \scriptfont\ttfam\tentt \scriptscriptfont\ttfam\tentt
\def\psf{\fam\sffam\tensf}\textfont\sffam\tensf
    \scriptfont\sffam\tensf \scriptscriptfont\sffam\tensf
\def\psc{\@getfont\psc\scfam\@xpt{\@mcsc}}%
\def\ly{\fam\lyfam\tenly}\textfont\lyfam\tenly
   \scriptfont\lyfam\sevly \scriptscriptfont\lyfam\fivly
\@setstrut \rm}
%
\gdef\@normalsize{\ifdraft
\@setsize\normalsize{17pt}\xpt\@xpt
\else
\@setsize\normalsize{12pt}\xpt\@xpt
\fi
\abovedisplayskip 10\p@ plus2\p@ minus5\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip  \z@ plus3\p@
\belowdisplayshortskip  6\p@ plus3\p@ minus3\p@
\let\@listi\@listI}
\gdef\small{\ifdraft
\@setsize\small{17pt}\ixpt\@ixpt
\else
\@setsize\small{11pt}\ixpt\@ixpt
\fi
\abovedisplayskip 8.5\p@ plus3\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus2\p@
\belowdisplayshortskip 4\p@ plus2\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 4\p@ plus2\p@ minus2\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}
\gdef\footnotesize{\ifdraft
\@setsize\footnotesize{17pt}\viiipt\@viiipt
\else
\@setsize\footnotesize{9.5pt}\viiipt\@viiipt
\fi
\abovedisplayskip 6\p@ plus2\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus\p@
\belowdisplayshortskip 3\p@ plus\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 3\p@ plus\p@ minus\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}
%
\gdef\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt}
\gdef\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
\gdef\large{\@setsize\large{14pt}\xiipt\@xiipt}
\gdef\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
\gdef\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
\gdef\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
\gdef\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
%
\gdef\bfseries{\bf}
\fi
%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% <===== end 2e switches
%%%%%
%%%%%%%
%% LaTeX book.sty,
%% was the original basis for this style, but
%% many changes have been made to it.
\def\@ptsize{0}
\@twosidetrue     %  Defines twoside option.
\@mparswitchtrue  %  Marginpars go on outside of page.
\def\ds@draft{\overfullrule 5pt}  % Causes overfull hboxes to be marked.
%% Special text placement: quotation, quote
\def\quotation{\footnotesize\list{}{\listparindent 1.5em
    \itemindent\listparindent
    \rightmargin\leftmargin
\parsep=0pt plus .001pt \topsep=6.5pt plus.001pt}\item[]}
\let\endquotation=\endlist
\def\quote{\list{}{\rightmargin\leftmargin}\item[]}
\let\endquote=\endlist
%%%%%
%% Default Dimension Settings
\arraycolsep 3pt % was 5pt
% Half the space between columns in an array environment.
\tabcolsep 6pt
% Half the space between columns in a tabular environment.
\arrayrulewidth .4pt
% Width of rules in array and tabular environment.
\doublerulesep 2pt
% Space between adjacent rules in array or tabular env.
\tabbingsep \labelsep
% Space used by the \' command.  (See LaTeX manual.)
\skip\@mpfootins = \skip\footins
\fboxsep = 3pt   
% Space left between box and text by \fbox and \framebox.
\fboxrule = .4pt 
% Width of rules in box made by \fbox and \framebox.
%% Setting up counters
\newcounter {chapter} %% used to reset other counters in article.
\newcounter {section}[chapter]
\newcounter {subsection}[section]
\newcounter {subsubsection}[subsection]
\def\@chapapp{\chaptername}
\newdimen\sectionwrap
\let\savedblbslash=\\
%%%%%%%%%%%%%
%% Description of parameters having to do with float placement:
%  \c@topnumber            : Number of floats allowed at the top of a column.
%  \topfraction            : Fraction of column that can be devoted to floats.
%  \c@dbltopnumber, \dbltopfraction : Same as above, but for double-column
%                          floats.
%  \c@bottomnumber, \bottomfraction : Same as above for bottom of page.
%  \c@totalnumber          : Number of floats allowed in a single column,
%                          including in-text floats.
%  \textfraction         : Minimum fraction of column that must contain text.
%  \floatpagefraction    : Minimum fraction of page that must be taken
%                          up by float page.
\def\@xfloat#1[#2]{\ifhmode\@bsphack\@floatpenalty -\@Mii
\else\@floatpenalty -\@Miii\fi
\def\@captype{#1}\ifinner \@parmoderr \@floatpenalty \z@ \else
\@next\@currbox\@freelist
{\@tempcnta \csname ftype@#1\endcsname \multiply \@tempcnta \@xxxii
\advance \@tempcnta \sixt@@n \@tfor \@tempa :=#2\do {\if \@tempa h\advance
\@tempcnta \@ne \fi \if \@tempa t\advance \@tempcnta \tw@ \fi \if \@tempa
b\advance \@tempcnta 4\relax \fi \if \@tempa p\advance \@tempcnta 8\relax
\fi }\global\count\@currbox\@tempcnta }\@fltovf \fi
\global \setbox \@currbox \vbox\bgroup%
\ifx\@captype\xtable\tabletextsize\fi%
\hsize\columnwidth\@parboxrestore}
%% These parameters are reset to more generous values to make it easier
%% to position floats.
%% Original values, then new values:
% \setcounter{topnumber}{2}
\setcounter{topnumber}{10}
% \def\topfraction{.7}
\def\topfraction{.99}
% \setcounter{bottomnumber}{1}
\setcounter{bottomnumber}{10}
% \def\bottomfraction{.3}
\def\bottomfraction{.99}
% \setcounter{totalnumber}{3}
\setcounter{totalnumber}{20} 
% \def\textfraction{.2}
\def\textfraction{.01}
%% These aren't relevant but LaTeX looks for them anyway.
\def\floatpagefraction{.5}
\setcounter{dbltopnumber}{2}
\def\dbltopfraction{.7}
\def\dblfloatpagefraction{.5}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%
%% Figure counters and parameters.
\newcounter{figure}[chapter]
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\fnum@figure{\figurename~\thefigure}
\def\figure{\@float{figure}}
\def\endfigure{\end@float\gdef\@currentlabel{}}
%% Table counters and parameters.
\newcounter{table}[chapter]
\def\fps@table{tbp}
\def\ftype@table{2}
\def\fnum@table{\tablename~\thetable}
%\let\endtable\end@float
%%%%%%%
\mark{{}{}}   % Initializes TeX's marks
%%%%%%%
\@addtoreset{footnote}{chapter}   % Makes \chapter reset 'footnote' counter.
%%%%%%%
%%%% Macros added or changed from original LaTeX, General tools:
%% i) Math macros: Lettered equation numbers. Split math. Left align eqns.
%%    Size changes correctly when \bf is used in sub- and superscripts.
%% ii)  Lettered captions for tables and figures,
%%      Multiple caption lines may center automatically.
%% iii) Tables, captions will adjust horizontal size to match width
%%      of table. Ruled lines added to top or bottom of table.
%%      Table content will extend to the left and right of columns,
%%      no extra horizontal space left.
%% iv)  Landscape tables and figures
%%%%%
%% (Generic Macros to be used to make particular styles
%% follow these general tools.)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% i) Math macros: Lettered equation numbers. Split math.
%%     Size changes correctly when \bf is used in sub- and superscripts.
%% Change definition below to make all equation numbers
%%   use [] rather than ()
\def\@eqnnum{{\normalfont \normalcolor [\theequation]}}
%% Special equation numbers. i.e., 16'
\def\specialeqnum#1{\def\theequation{#1}}
%% Bold math
\def\bbf#1{\mathchoice{\hbox{\boldmath$\displaystyle#1$}}
{\hbox{\boldmath$\textstyle#1$}}
{\hbox{\boldmath$\scriptstyle#1$}}
{\hbox{\boldmath$\scriptscriptstyle#1$}}
}
%% Math Letters:
\def\themathletter{\relax}
\def\mathletter#1{\gdef\themathletter{#1}}
\def\xrelax{\relax}
\let\templabel\relax
\def\xlabel#1{\gdef\templabel{#1}}
\newif\iffirsttime
\global\firsttimetrue
\def\equation{$$ % $$ BRACE MATCHING HACK
\setbox0=\hbox\bgroup $ \displaystyle%
\let\label\xlabel}
\let\savelabel\label
\def\endequation{$\egroup%
%
\ifx\themathletter\xrelax\global\firsttimetrue%
\refstepcounter{equation}\else%
\iffirsttime\global\firsttimefalse\refstepcounter{equation}\fi\fi%
%
\setbox1=\hbox{\copy0\@eqnnum}%
%
\ifdim\wd1>\linewidth%
\vbox{\unskip\noindent\hbox to\hsize{\hss\unhbox0\hss}\vskip3pt%
\noindent\hbox to\hsize{\hfill\displaywidth\linewidth\llap{\@eqnnum}}}%
\else%
\vbox{\unskip\noindent\hbox to\linewidth{%
\hfil\unhbox0\hfil\displaywidth\linewidth\llap{\@eqnnum}}}%
\fi%
\let\@currentlabel=\theequation%
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax%
\gdef\theequation{\savetheequation}%
\gdef\themathletter{\relax}%
\global\@ignoretrue$$}
\def\@@eqncr{\let\@tempa\relax
    \ifcase\@eqcnt \def\@tempa{& & &}\or \def\@tempa{& &}
     \else \def\@tempa{&}\fi
     \@tempa
\ifx\themathletter\xrelax
%% make equation letters work ==>
\if@eqnsw\stepcounter{equation}\fi%% <==
\else\iffirsttime\global\firsttimefalse\stepcounter{equation}\fi\fi%
\if@eqnsw\@eqnnum\fi%
\ifx\templabel\relax\else\savelabel{\templabel}\fi%%% <<=== make label
\global\let\templabel\relax          % catch current number and letter
\global\let\themathletter\xrelax     % <<===
\gdef\theequation{\savetheequation}%
\global\@eqnswtrue\global\@eqcnt\z@\cr}
\def\eqnarray{\let\@currentlabel=\theequation
\global\firsttimetrue%
\global\@eqnswtrue
\let\label\xlabel
\global\@eqnswtrue\m@th
\global\@eqcnt\z@\tabskip\@centering\let\\\@eqncr
$$\halign to\displaywidth\bgroup\@eqnsel\hskip\@centering
  $\displaystyle\tabskip\z@{##}$&\global\@eqcnt\@ne
  \hskip 2\arraycolsep \hfil${##}$\hfil
&\global\@eqcnt\tw@ \hskip 2\arraycolsep $\displaystyle\tabskip\z@{##}$\hfil
   \tabskip\@centering&\llap{##}\tabskip\z@\cr}
\def\endeqnarray{\@@eqncr\egroup
\global\firsttimetrue
$$\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax
\gdef\theequation{\savetheequation}
\def\themathletter{\relax}\global\@ignoretrue}
%%%%%
%% Splitmath, to break equations into two lines easily.
%%   To use:
%% \begin{splitmath} xyz\\ 123\end{splitmath},
%%   or, to have an equation number,
%% \begin{eqsplitmath} xyz\\ 123\end{eqsplitmath}
\newif\ifCenterSplitMathEqno
\def\eatone#1{}%% Used to disable macro that takes one argument.
\long\def\eqsplitmath#1\\ #2\end#3{
\setbox1=\hbox{\let\label\eatone$#2$}
\ifx\themathletter\xrelax\global\firsttimetrue%
\refstepcounter{equation}\else%
\iffirsttime\global\firsttimefalse\refstepcounter{equation}\fi\fi%
\let\label\xlabel
\setbox2=\hbox{\theequation}
$$
\vcenter{\hbox to\hsize{$\displaystyle#1$\hfil}
\ifCenterSplitMathEqno
\vskip-3pt
\vbox to0pt{\vss\hbox to\hsize{\hfill[\theequation]}}
\else\vskip2pt\fi
\hbox to\hsize{\hfil$\displaystyle#2$
\ifCenterSplitMathEqno
\hskip\wd2\qquad\else\qquad\hbox{[\theequation]}\fi}%
}% end vcenter
\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax
\gdef\themathletter{\relax}%
$$\aftergroup{\end{eqsplitmath}}}
\def\endeqsplitmath{\gdef\theequation{\savetheequation}
\noindent\hskip-3pt}%% a kludge, but it seems ok.
\long\def\splitmath#1\\ #2\end#3{$$
\vcenter{\hbox to\hsize{$\displaystyle#1$\hfil}
\vskip3pt
\hbox to\hsize{\hfil$\displaystyle#2$}%
}% end vcenter
$$\aftergroup{\end{splitmath}}}
\let\endsplitmath\endeqsplitmath
%%%%
\def\specialeqnum#1{\global\firsttimefalse
\mathletter{xxx}%% mathletter to keep the equation counter
                                    %%from advancing
\gdef\theequation{#1}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% ii)  Lettered captions for tables and figures,
%%      Multiple caption lines may center automatically.
\def\theletter{\relax}
\newif\ifxfirsttime
\xfirsttimetrue
\def\xtable{table}
\def\xrelax{\relax}
\newif\ifcontinued
\def\continuedcaption{\global\continuedtrue\caption{(continued)}}
\def\caption{\ifx\@captype\xtable
%
\ifx\theletter\xrelax\global\xfirsttimetrue
   \ifcontinued\global\continuedfalse\else
   \refstepcounter{table}
   \fi
\else
  \ifxfirsttime\global\xfirsttimefalse
     \ifcontinued\global\continuedfalse
     \else\refstepcounter{table}
     \fi
  \fi
\fi
%
\else% figure
  \ifx\theletter\xrelax\global\xfirsttimetrue
     \ifcontinued\global\continuedfalse\else%
     \refstepcounter\@captype\fi
  \else%
    \ifxfirsttime\global\xfirsttimefalse%
      \ifcontinued\global\continuedfalse\else%
       \refstepcounter\@captype\fi
    \fi
  \fi
\fi%
\xdef\currcaptype{\@captype}%
\xdef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}%
\@dblarg{\@caption\@captype}}
\def\letteredcaption#1{\gdef\theletter{#1}\caption}
\def\letteredcontinuedcaption#1{\gdef\theletter{#1}\continuedcaption}
\def\appcaption{\ifx\@captype\xtable\centermultiplelinesfalse
     \ifx\theletter\xrelax%
     \global\xfirsttimetrue \ifcontinued\global\continuedfalse\else%
     \global\advance\c@apptable by 1\fi\else%
     \ifxfirsttime\global\xfirsttimefalse%
     \ifcontinued\global\continuedfalse\else%
     \global\advance\c@apptable by1\relax\fi\fi\fi
\else%
     \ifx\theletter\xrelax%
     \global\xfirsttimetrue\ifcontinued\global\continuedfalse\else%
     \global\advance\c@appfigure by 1\fi\else%
     \ifxfirsttime\global\xfirsttimefalse%
     \ifcontinued\global\continuedfalse\else%
     \global\advance\c@appfigure by1\relax\fi\fi\fi\fi%
%
\xdef\currcaptype{\@captype}%
\@dblarg{\@caption\@captype}}
\newskip\abovefigcaptionskip
\newskip\abovetabcaptionskip
\newskip\abovetableskip
\newif\ifcentermultiplelines
\newif\ifcentersingleline
\newif\ifcentersingletabline
\newif\iflineabovetabcaption
\newif\iflinebelowtabcaption
\newdimen\belowtabcaptionskip
\long\def\makecenterlines#1{\setbox1=\vbox{\parindent=0pt
\tolerance=1000
\parfillskip=0pt
    \parshape6
          .05\hsize .9\hsize
           .1\hsize .8\hsize
          .15\hsize .7\hsize
          .2\hsize .6\hsize
          .25\hsize .5\hsize
          .3\hsize .4\hsize
#1}%
\vbox{\unvbox1\setbox0=\lastbox%
\hbox to\hsize{\hss\ifhbox0\unhbox0\fi\hss}}}
\long\def\topmakecenterlines#1{\setbox1=\vtop{\parindent=0pt %%<== was vbox
\tolerance=1000
\parfillskip=0pt
    \parshape6
          .05\hsize .9\hsize
           .1\hsize .8\hsize
          .15\hsize .7\hsize
          .2\hsize .6\hsize
          .25\hsize .5\hsize
          .3\hsize .4\hsize
#1}%
\vbox{\unvbox1\setbox0=\lastbox%
\hbox to\hsize{\hss\ifhbox0\unhbox0\fi\hss}}}
\def\xxtest{\ignorespaces }
\newbox\testtabcaptionheight
%% Caption that allows for TABLE on its own line, with
%% caption lines centered underneath.
\long\def\@makecaption#1#2{%
\ifx\@captype\xtable% TABLE CAPTION
\centermultiplelinestrue
%%
  \iflandscapetable\hsize=.95\textheight\fi
  \iflineabovetabcaption\hrule\fi%
  \vskip\abovetabcaptionskip%
%% in all cases, put TABLE on its own line:
\hbox to\hsize{\iflandscapetable\hskip5pc\fi
\hfill\tablenamefont#1\hfill}%++
%%
\def\xtest{#2}
 \ifx\xtext\xxtest %% no caption
 \else
 \setbox\@tempboxa\hbox{\captionsize\boldmath
\tablenamefont#2}%
   \ifdim \wd\@tempboxa >\hsize
%
   \vskip5pt
%%
      \setbox\testtabcaptionheight\vbox{\makecenterlines{%
            \captionsize\boldmath
\tablenamefont#2\vskip1sp\par}
                    }%
%%
     \ifdim\ht\testtabcaptionheight>28pt %% more than 3 lines in table caption
    \iflandscapetable\moveright26pt\fi%
      \vbox{\noindent\captionsize\boldmath
\tablenamefont#2\vskip1sp}%
      \par
     \else
    \iflandscapetable\moveright26pt\fi%
      \vbox{\makecenterlines{%
            \captionsize\boldmath\tablenamefont#2\vskip1sp\par}
           }%
     \fi
%
   \else
   \hbox to\hsize{\iflandscapetable\hskip5pc\fi
\hfill\captionsize\boldmath\tablenamefont#2\hfill}%++
   \vskip1pt
   \fi
 \fi% end test to see if caption exists
%
\else% FIGURE CAPTION
 \centermultiplelinesfalse
 \vskip\abovefigcaptionskip
 \iflandscape\hsize=.95\textheight\fi
%
  \setbox\@tempboxa\hbox{\captionsize\captionnamefont#1~~%
  \captiontextfont#2}%
%
 \ifdim \wd\@tempboxa >\hsize
    \iflandscape\moveright26pt\fi\vbox{
              \captionsize\captionnamefont#1.~~\captiontextfont#2
                                      }\par%
 \else% less than hsize in caption
\noindent\hbox to\hsize{%
\captionsize%
  \iflandscape\hskip5pc\fi
  \ifcentersingleline\hfill\fi%
  \captionnamefont#1.~~\captiontextfont#2\hfill
                       }
 \fi%
\fi% end figure caption
%%%
%% either figure or table
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}%
%
\ifx\@captype\xtable
  \iflinebelowtabcaption\vskip\belowtabcaptionskip\thetablelines\fi
  \vskip\abovetableskip
\fi
\ifhere\else\gdef\theletter{\relax}\fi%
}
%%%% side by side captions
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% to use:
% \begin{figure or table} \sidebyside{}{} \end{figure or table}
%
% \begin{table}
% \sidebyside{\caption{table caption} first table}
% {\caption{table caption} second table}
% \end{table}
% or
% \begin{figure}
%\vskip<dimen>
% \sidebyside{\caption{first fig caption}}
% {\caption{second fig caption}}
% \end{figure}
% Or, if you want different spaces above the two
% figure captions, you can put the \vskip or \vspace
% <inside> the {}{}, ie.:
% \begin{figure}
% \sidebyside{\vskip<dimen>\caption{fig caption}}
% {\vskip<dimen>\caption{fig caption}}
% \end{figure}
% To call for line breaks, you can use \\ , but
% also supply [], i.e., \caption[]{asdf}.
% This will send whatever is in [] to the List of Tables. If
% you are not using that you can just have facing square
% brackets. If you are using List of Tables, you can supply
% a title without \\ in it:
% \caption[$hA_3f_4K_2l_5$ 60-01-05]{$hA_3f_4K_2l_5$\\ 60-01-05}}
% If you are not using List of Tables, just supply []'s, i.e.,
%\begin{figure}[h]
%\vspace{15pc}
%\sidebyside{
%\caption[]{$hA_3f_4K_2l_5$\\ 60-01-05}}
%{\caption[]{$hD_8p_4F_xd_7$\\ 60-01-10}}
%\end{figure}
% xref labels work the same as normally:
%\begin{figure}[h]
%\vspace{4pc}
%\sidebyside{\caption{Hexiamond board.\label{three}}}
%{\caption{O'Beirne's?\label{four}}}
%\end{figure}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\specialendtable{\vskip1sp\end@float}
\long\def\sidebyside#1#2{%
\hbox to\hsize{\let\caption\savecaption
\ifcaptypefig\def\@captype{figure}\else\def\@captype{table}
\footnotesize
\fi%
\vtop{\hsize=.5\hsize%
\advance\hsize by -.5\columnsep
\parindent=0pt
\centering
#1}\hskip\columnsep\vtop{\hsize=.5\hsize%
\advance\hsize by -.5\columnsep
\parindent=0pt\centering
#2
}}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% iii) Tables, captions will adjust horizontal size to match width
%%      of table. Ruled lines added to top or bottom of table.
%%      Table content will extend to the left and right of columns,
%%      no extra horizontal space left.
\long\def\@caption#1[#2]#3{\par
\begingroup
    \@parboxrestore
    \normalsize
    \@makecaption{\csname fnum@#1\endcsname}{\ignorespaces#3}\par
  \endgroup}
\def\catchlabel{}
%\let\savelabel\label
\def\savetable{\@float{table}}
\let\endsavetable\end@float
%% Made \begin{specialtable}... \end{specialtable} for cases in which there
%% is a tabular inside another tabular, or for when author wants to
%% use \begin{table}...\end{table} with something other than
%% \begin{tabular}...\end{tabular} in it.
\def\specialtable{\def\@captype{table}%
\ifcontinued\else\refstepcounter{table}\fi%
\let\tabular\savetabular\let\endtabular\saveendtabular}
\def\endspecialtable{\ifnobottomtabline\global\nobottomtablinefalse\else
\vskip6pt
\thetablelines\fi}
\def\tableterms{\bgroup\everycr={\noalign{\vskip1pt}}%
\let\\=\cr\halign\bgroup\hfill## :&\ ##\hfill\cr}
\def\endtableterms{\crcr\egroup\vskip4pt\egroup}
%% table if outside two columns??
\newif\iffullwidthtable
\def\table{\def\tabular{\tabletextsize\xtabular}
\let\endtabular\endxtabular
\long\def\tablenotes##1{\def\thetabnotes{{\rm\vskip1sp
\leftskip=6pt \parindent=-6pt \rightskip=0pt \parfillskip=0pt plus 1fil
\tablenotesize##1\vskip1sp}}}
\expandafter\def\csname tabular*\endcsname ##1{%
\global\setbox\savecurrtable\hbox\bgroup\tabletextsize%
\def\@halignto{to ##1}\@tabular}
\expandafter \let \csname endtabular*\endcsname = \endxtabular
\def\label##1{\gdef\catchlabel{##1}}%
\def\caption##1{\gdef\catchcaption{##1}}%
\tabletextsize%\@float{table}
}
\let\saveendtable\endtable
\newif\ifnobottomtabline
\def\nobottomline{\global\nobottomtablinetrue}
\let\nobottomlines\nobottomline
\def\endtable{%
\ifx\catchcaption\empty\else
\ifx\theletter\xrelax
\global\xfirsttimetrue\ifcontinued\global\continuedfalse\else
\refstepcounter{table}\fi\else
\ifxfirsttime\global\xfirsttimefalse\ifcontinued\global\continuedfalse
\else\refstepcounter{table}\fi\fi\fi\fi
\vskip1sp
\noindent
\iflandscapetable
\dimen0=\textheight
\advance\dimen0 by-\textwidth
\hsize=\textheight\else%\hsize=\textwidth
\hskip-.5\dimen0\fi%
%%??
\hbox to\hsize{\hss%
\vbox{\ifdim\wd\savecurrtable>0pt%
      \iffullwidthtable%\hsize=\textwidth
\centering\else%
      \hsize=\wd\savecurrtable\fi\fi%
\ifx\catchcaption\empty\else\vskip1sp{\savecaption{\catchcaption}}\fi%
%
\ifdim\wd\savecurrtable>0pt\relax%
\unhbox\savecurrtable%
\ifnobottomtabline\global\nobottomtablinefalse\else
\vskip1sp%??
\thetablelines\fi
\vskip3pt
\thetabnotes%
\gdef\thetabnotes{\relax}
\else\vskip1sp\fi
}\hss}%
%
\ifx\catchlabel\empty\else%
\xdef\@currentlabel{\thetable}%
\savelabel{\catchlabel}\fi%
\gdef\@currentlabel{}%
\gdef\catchlabel{}%
\gdef\catchcaption{}%
%\saveendtable%
\global\wd\savecurrtable=0pt}
\long\def\tablenotes#1{\vskip3pt%
{\rm\leftskip=6pt \parindent=-6pt \rightskip=0pt \parfillskip=0pt plus 1fil%
\tablenotesize#1\vskip1sp}\vskip1sp
}
\let\thetabnotes\relax
\let\catchcaption\empty
\newbox\savecurrtable
\let\savetabular\tabular
\let\saveendtabular\endtabular
\newskip\abovetabularskip
\newskip\belowtabularskip
\def\tabular{\vskip\abovetabularskip\vbox\bgroup\tabletextsize
\parindent=0pt\savetabular}
\def\endtabular{\saveendtabular\egroup
\vskip\belowtabularskip}
\def\xtabular{\global\setbox\savecurrtable\hbox\bgroup%
\tabletextsize\unskip\savetabular}
\def\endxtabular{\saveendtabular
\ifnobottomtabline\global\nobottomtablinefalse\else
\vskip1sp%??
\thetablelines\fi
\egroup}
\def\xtable{table}
\def\@array[#1]#2{\setbox\@arstrutbox=\hbox{\vrule
     height\arraystretch \ht\strutbox
     depth\arraystretch \dp\strutbox
     width\z@}\@mkpream{#2}\edef\@preamble{\halign \noexpand\@halignto
\bgroup%
\tabskip\z@\@arstrut\@preamble
\ifx\@captype\xtable\hskip-\tabcolsep\fi%% <==== Changed
\tabskip\z@ \cr}%
\let\@startpbox\@@startpbox \let\@endpbox\@@endpbox%
  \if #1t\vtop \else \if#1b\vbox \else \vcenter \fi\fi%
  \bgroup\let\par\relax%
  \let\@sharp##\let\protect\relax \lineskip\z@\baselineskip\z@\@preamble}
\def\new@tabacol{\edef\@preamble{\@preamble\hskip0pt}}
\def\@tabclassz{\ifcase \@lastchclass\@acolampacol%
\or \@ampacol \or
   \or \or \@addamp \or \@acolampacol\or \@firstampfalse
\ifx\@captype\xtable \new@tabacol\else\@tabacol \fi%
\fi%
\edef\@preamble{\@preamble%
  \ifcase \@chnum%
     \hfil\ignorespaces\@sharp\unskip\hfil%
     \or \ignorespaces\@sharp\unskip\hfil%
     \or \hfil\hskip\z@ \ignorespaces\@sharp\unskip\fi}}
\let\savehline\hline
\def\hline{\noalign{\vskip3pt
\iffullwidthtable \hrule width\hsize%\textwidth
\else\hrule\fi%
\vskip3pt}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% iv)  Landscape figures and tables
\newif\iflandscape
\newif\iflandscapetable
\def\landscapefigure{\clearpage\global\landscapetrue
\thispagestyle{empty}
\figure[h]\vskip.95\textwidth\relax}
\def\endlandscapefigure{\endfigure\global\landscapefalse\clearpage}
\def\landscapetable{\clearpage\global\landscapetabletrue
\thispagestyle{empty}
\table[h]}
\def\endlandscapetable{\endtable
\global\landscapetablefalse
\clearpage}
\def\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
\newif\ifdraft
\def\draft{\global\drafttrue
\def\@oddfoot{\vtop to 0pt{\vskip24pt\large\tt
\hsize=41pc
D R A F T\hfill\today, \realtime\hfill D R A F T\global\titlefalse\vss}}
\def\@evenfoot{\@oddfoot}\normalsize}
\def\fix{\ifcase\oldtime 0\or0\or0\or%
0\or0\or0\or0\or0\or0\or0\fi}
\def\fixtiming{\ifcase\timing 0\or0\or0\or%
0\or0\or0\or0\or0\or0\or0\fi}
\newcount\timing
\newcount\hourcount
\newcount\oldtime
\def\realtime{\timing=\time \oldtime=\time
\ifnum\timing>60 \divide\timing by 60
\hourcount=\the\timing
\multiply\timing by 60
\advance\oldtime by-\timing
\ifnum\hourcount<12 \number\hourcount:\fix\number\oldtime am\fi%
\ifnum\hourcount=12 \number\hourcount:\fix\number\oldtime pm\fi%
\ifnum\hourcount>12 \advance\hourcount by-12
\number\hourcount:\fix\number\oldtime pm\fi
\else12:\fixtiming\number\timing am\fi}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%
%% Generic macros to make specific style possible:
%% 1) Running Head and Folio
%% 2) Article title page, including capability for multiple \thanks,
%%    end article.
%% 3) Theorems, Proofs, Examples, etc.
%% 4) Algorithm macros
%% 5) Footnotes, endnotes
%% 6) Acknowledgments
%% 7) Appendices
%% 8) References
%% 9) Received lines
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newif\iffootlinefolio
\newif\iffootlinehrule
%%%%%%
%% 1) Running Head and Folio
\def\ps@headings{%
\def\@oddfoot{\jfootline}\def\@evenfoot{\jfootline}
\def\@evenhead{\jheadline}%
\def\@oddhead{\jheadline}%
}
\ps@headings
\pagenumbering{arabic}
\onecolumn
%%%%%%
%% 2) Article title page, including capability for multiple \thanks,
%%    end article.
\newcount\totpages
\def\mrule{\raise.75ex\hbox{\vrule width6pt height.5pt}}
\def\lastpage{%
\expandafter\ifx\csname endpage\the\c@chapter\endcsname\relax%
 ?? \else
--%
\csname endpage\the\c@chapter\endcsname\fi}
\newif\ifleadingzeros
\def\startingpage{%
\xdef\firstpage{\ifleadingzeros\ifnum\c@page<10 000\the\c@page\else%
\ifnum\c@page<100 00\the\c@page\else%
\ifnum\c@page<1000 0\the\c@page\else\the\c@page\fi\fi\fi
\else\the\c@page\fi}
\def\thepages{pp.~\firstpage\relax\lastpage\relax}}
\def\smcopyright{{\rm\ooalign{\hfil\hskip-1pt\raise.02ex\hbox{c}\hfil\crcr%
\mathhexbox 20D}}}
\def\smcopyright{%
\ooalign {\hfil\raise .17ex\hbox{\footnotesize c}\hfil\crcr%
\raise.17ex\hbox{$\scriptstyle\bigcirc$}}}
\def\yearofpublication#1{\def\theyear{#1}}
\def\monthofpublication#1{\def\themonth{#1}}
\def\volume#1{\def\thevolume{#1}}
\def\issuenumber#1{\def\theissuenumber{#1}}
\def\cccline#1{\def\thecccline{#1}}
\def\journame#1{\def\thejourname{\uppercase{#1}}}
\def\articlenumber#1{\def\thearticlenumber{#1}}
\yearofpublication{Year ??}
\volume{Volume ??}
\issuenumber{Number ??}
\def\editor#1{%
\def\theeditor{\vskip\aboveacceptedskip\noindent Recommending editors: #1}}
\def\received#1{%
\def\thereceived{Received #1}}
\def\revised#1{\def\therevised{revised #1}}
\def\accepted#1{%
\def\theaccepted{accepted #1}}
\cccline{???} %% need to be used once for each journal, do not reset
              %% for each article.
%% To be used in specific style part of file, below:
\newif\ifauthorUC
\newif\iftitleUC
\def\authorrunninghead#1{\def\theauthors{\ifauthorUC\uppercase\fi{#1}}}
\def\titlerunninghead#1{\def\thetitle{\iftitleUC\uppercase\fi{#1}}}
%% Reset default values
\def\resetdefaults{%
\def\theauthors{!! Please write
{\tt\string\authorrunninghead\string{\string<Author Name(s)\string>\string}}
in file !!}
\def\thetitle{!! Please write
{\tt\string\titlerunninghead\string{\string<(Shortened) Article Title\string>%
\string}} in file !!}
\yearofpublication{???}
\monthofpublication{???}
\volume{???}
\gdef\thereceived{}
\gdef\therevised{}
\gdef\theaccepted{}
\gdef\thededication{}
\gdef\theeditor{}
\global\dothanks=0
\global\thanksnum=0
\global\setbox\barticle=\vbox{}
\gdef\theabstract{}
\gdef\thekeywords{}
\gdef\thecommline{}
\global\c@figure=0
\global\c@table=0
\global\appendonfalse\relax}
\newdimen\abovetitleskip
\newdimen\belowtitleskip
\newdimen\abovesubtitleskip
\newdimen\belowsubtitleskip
\newdimen\aboveauthorskip
\newdimen\belowauthorskip
%% Making \thanks work multiple times in the title and in
%% authors line.
\let\savefootnotetext\footnotetext
\newcount\thanksnum
\newcount\dothanks
\def\dothankssymbol{\ifcase\thanksnum\or *\or $\dagger$\or
$\ddagger$\or {$\mathchar "278$}\or {$\mathchar "27B$}\or $\|$\or $\dagger\dagger$
   \or $\ddagger\ddagger$ \else\@ctrerr\fi\relax}
\newcount\thankscounter
\def\thanks#1{\global\advance\thanksnum by1\relax
\global\advance\dothanks by1
\setbox1=\hbox{X}\ifnum\thanksnum=1\relax\else
\raise.25\ht1\fi\vbox{\hbox{\dothankssymbol}\vfill}}
\def\maketemp#1{\global\advance\thankscounter by1\relax
{\footnotesize%
\expandafter\gdef\csname tempthanks\the\thankscounter\endcsname{#1}}}
%% gdef instead of xdef
\newif\iftitle
\def\StartOnNewPage{\clearpage
\ifodd\c@page\else\null\clearpage\fi}
\newcount\currarticlenumber
%%%%%%%%%%%%%%%%%%%%
%% Separate \thanks environment for title so we get number instead of
%%   symbol
\newcount\titlethanksnum
\newcount\dotitlethanks
\newcount\titlethankscounter
\long\def\titlethanks{\futurelet\next\lookforast}
\def\lookforast{\ifx\next*\let\go\authorthanks\else%
\let\go\xtitlethanks\fi\go}
\long\def\xtitlethanks#1{\global\advance\c@footnote by1\relax%
\global\advance\titlethanksnum by1\relax%
\global\advance\dotitlethanks by1\relax%
\setbox1=\hbox{X}%
\unskip\vbox to\ht1{\hbox{\titlethanksfont\the\titlethanksnum}\vfill}}
\long\def\subtitlethanks{\futurelet\next\lookforaster}
\def\lookforaster{\ifx\next*\let\go\authorthanks\else
\let\go\xsubtitlethanks\fi\go}
\long\def\xsubtitlethanks#1{\global\advance\titlethanksnum by1\relax
\global\advance\dotitlethanks by1
\setbox1=\hbox{X}%
\,\vbox to1.2\ht1{\hbox{\subtitlethanksfont\the\titlethanksnum}\vfill}}
\long\def\smalltitlethanks{\futurelet\next\lookforaster}
\def\lookforaster{\ifx\next*\let\go\authorthanks\else
\let\go\xsmalltitlethanks\fi\go}
\long\def\xsmalltitlethanks#1{\global\advance\titlethanksnum by1\relax
\global\advance\dotitlethanks by1 \setbox1=\hbox{X}%
$^{\the\titlethanksnum}$}
\def\titlethankssymbol{\titlethanksfont\the\thanksnum}
\long\def\titlemaketemp{\futurelet\next\tempast}
\long\def\tempast{\ifx\next*\let\go\maketemp\else\let\go\xtitlemaketemp\fi\go}
\long\def\xtitlemaketemp#1{\global\advance\titlethankscounter by1\relax
\expandafter\gdef\csname temptitlethanks\the\titlethankscounter\endcsname{#1}}
%% Making \thanks work multiple times in the title and in
%% authors line.
%% \dothankssymbol may be used later, even if it is not being use now==>
\let\savefootnotetext\footnotetext
\newcount\thanksnum
\newcount\dothanks
\def\dothankssymbol{\ifcase\thanksnum\or*\or$\dagger$\or
$\ddagger$\or $\mathchar "278$\or $\mathchar "27B$\or $\|$\or $\dagger\dagger$
   \or $\ddagger\ddagger$ \else\@ctrerr\fi\relax}
\newcount\thankscounter
\long\def\thanks#1{\global\advance\thanksnum by1\relax
\global\advance\dothanks by1
\setbox1=\hbox{X}\vbox to\ht1{\hbox{\dothankssymbol}\vfill}}
%%% NOW used for unnumbered thanks!!!
\long\def\authorthanks#1#2{\global\advance\thanksnum by1\relax
\global\advance\dothanks by1
%\raise.8ex\hbox{\/\footnotesize\dothankssymbol}
}
\long\def\maketemp#1#2{\global\advance\thankscounter by1\relax
\expandafter\gdef\csname tempthanks\the\thankscounter\endcsname{#2}}
%%%%%%%%%%%%%%%%%%%%
\long\def\title#1{\startingpage
\global\titletrue
%
%
\vbox{\vskip\abovetitleskip}
{%
\let\thanks\titlethanks
\centering\def\\ {\vskip1pt}%
\LARGE\titlefont
\baselineskip16pt#1\vrule depth\belowtitleskip
width0pt height 0pt
\vskip1sp%
}%
\setbox1=\hbox{\let\\ \relax\let\thanks\titlemaketemp #1}
\normalsize}
\long\def\subtitle#1{\vbox{\vskip\abovesubtitleskip
\let\thanks\subtitlethanks
\centering\def\\ {\vskip1pt}%
{\large\baselineskip=15pt%
\subtitlefont#1\vrule depth \belowsubtitleskip
width0pt height 0pt
\vskip1sp}%
}%
\setbox1=\hbox{\let\\ \relax\let\thanks\titlemaketemp #1}}
\newif\ifCenterArticleHead
\long\def\author#1{%
\vskip\aboveauthorskip
\hbox to\textwidth{\ifCenterArticleHead\hss\fi%
\let\thanks\smalltitlethanks
\let\\ =\ %
\frenchspacing\authorfont#1\vrule width0pt depth\belowauthorskip\hss}
\setbox1=\hbox{\let\thanks\titlemaketemp #1}\vskip1sp}
\def\and{\vskip.5\aboveauthorskip
\hbox to\textwidth{\hfill and\hfill}
\vskip-.25\aboveauthorskip}
\let\authors\author
\newdimen\aboveaffilskip
\newdimen\belowaffilskip
\long\def\affil#1{%
\vskip1sp
\hbox to\textwidth{\hss
\vbox{\hsize=40pc\centering%
{\let\thanks\smalltitlethanks\frenchspacing\baselineskip=10pt\affilfont#1\vrule
width0pt depth\belowaffilskip\vskip1sp}}
\hss}\setbox1=\hbox{\let\thanks\titlemaketemp#1}\vskip1sp}
\long\def\commline#1{\def\thecommline{\vskip3pt
\hbox to\textwidth{\let\thanks\smalltitlethanks%
\setbox1=\hbox{\let\thanks\titletemp#1}\hfill
\communicatedfont #1\hfill}\vskip4pt}}
\def\thecommline{}
\long\def\dedication#1{\def\thededication{\uppercase{#1}}}
\def\xdedication{\vskip2pt\hbox to \textwidth{\hss \dedicationfont
\thededication\hss}\vskip-2pt}
\long\def\email#1{\vskip-14pt
\hbox to \textwidth{\hss \emailfont E-mail: #1\vrule
depth10pt width0pt\hss}\vskip1sp}
\newif\ifabstractname
\newdimen\aboveabstractskip
\newdimen\belowabstractskip
\newdimen\belowabstractnameskip
\newdimen\abstractmargin
\abstractmargin=0pt
\long\def\abstract#1{\def\theabstract{
{\hsize=21pc
%  \vbadness10001
 \hbadness5000
  \tolerance5000
\leftskip=\abstractmargin \rightskip=\leftskip
\abstractsize
\abstractfont\indent\ignorespaces\vrule height 19pt width 0pt #1%
\iffinaltypesetters
\hskip1em\hbox{\footnotesize
\copyright\hskip2pt\abstractcopyrightfont\the\year\ Academic Press}\fi%
%
}}}
%% If ABSTRACT should be printed, set to true
\abstractnamefalse
\newdimen\abovekeywordsskip
\def\keywords#1{\def\thekeywords{{
\small\keywordfont\indent
{\keywordnamefont Key Words:} #1.}}}
%%%%%%%%%%%%
%% Begin, End article
\newskip\abovereceivedskip
\newskip\aboverevisedskip
\newskip\aboveacceptedskip
\abovereceivedskip=4pt
%% \refstepcounter{chapter}
%% will reset footnote num, section, theorem, table and figure
\let\footnotefont\rm %% default, may be redefined in particular .sty file.
\newbox\barticle
\newbox\thanksbox
\newbox\titlethanksbox
\newcount\c@appendnum
\newif\ifdocumentationextraspace
\def\article{\refstepcounter{chapter} % resets fig. etc. counters with
                                      % each article. We never actually
                                      % use the chapter counter.
%
\global\currarticlenumber=\the\c@page
\ifx\thecommline\empty\else\thecommline\fi
\ifx\thereceived\empty\else
\vskip\abovereceivedskip
\hbox to \textwidth{\hfill
\receivedfont
\ifx\thereceived\empty\else\noindent\thereceived\fi%
\ifx\therevised\empty\else;\enspace\therevised\fi%
\ifx\theaccepted\empty\else;\enspace\theaccepted\fi.%
\ifx\theeditor\empty\else\theeditor.\fi
\hfill}\fi
%
\ifx\thededication\empty\else\xdedication\fi
%
\global\saveparskip=\parskip
\gdef\applett{}
\global\c@appendnum=0 \global\appendonfalse
\vskip1sp
%%%%%%
%%%%%%
\ifnum\dothanks>0
\dothanks=0
\ifnum\thanksnum>0 \global\thanksnum=0
\global\setbox\thanksbox=\vbox{%
\parindent=6pt
\hsize=21pc
\loop
\vskip1pt
\ifnum\thanksnum<\thankscounter
\global\advance\thanksnum by1\relax
\vskip1sp
\footnotesize\noindent\vrule height 8.5pt width0pt%
\hskip\parindent
\csname tempthanks\the\thanksnum\endcsname
\vskip1sp
\repeat
}% end \thanksbox
\fi\fi%
 \ifnum\titlethanksnum>0 \global\titlethanksnum=0
\global\setbox\titlethanksbox=\vbox{%
\parindent=-6pt
\leftskip=6pt
\hsize=21pc
\footnotesize
 \loop\ifnum\titlethanksnum<\titlethankscounter
 \global\advance\titlethanksnum by1\relax
\vskip1pt\indent
\hbox to 6pt{$^\the\titlethanksnum$\hss}%
\csname temptitlethanks\the\titlethanksnum\endcsname
 \repeat
 \global\titlethanksnum=0 \global\titlethankscounter=0
\vskip1sp
}
\fi %% end titlethanksnum
\global\thanksnum=0 \global\thankscounter=0
\global\setbox\barticle=\vbox{\hsize=\mycolumnwidth
%
\ifvoid\thanksbox\else
\unvbox\thanksbox\vskip3pt\fi
%
\ifvoid\titlethanksbox
\else
\unvbox\titlethanksbox\vskip3pt\fi
%
\vskip1sp
}
%   %%%% \advance\vsize by\ht\barticle
\global\dothanks=0 \global\thanksnum=0
\normalsize
\ifdocumentationextraspace
\vskip12pt\fi
\vbox to6pt{\vfill}
\twocolumns
\ifx\theabstract\empty
\else
\vtop{\vskip-7pt
\hrule height .5pt
\theabstract%
\ifx\thekeywords\empty\else
\vskip2pt
\thekeywords\fi\vrule depth 10pt width0pt
\hrule height .5pt\vskip1sp
}
\vskip1pt
\fi
\global\firstsectiontrue\everypar={\global\firstsectionfalse\everypar={}}
\global\savefigandtabnumber=1
\ifdim\ht\barticle>2pt
\begin{figure}[b]
\ifvoid\barticle\else
\hrule width 48pt height .5pt
\vskip4pt
\unvbox\barticle\vskip-12pt\fi
\end{figure}
\fi
\vskip12pt
}
\def\endarticle{\notes
\endtwocolumns
\normalsize
\ifnum\pagefloatnumber>\pageloopnum
\newpage
\insertpage{\hsize=\textwidth
\linewidth=\textwidth
\csname pagefloat\the\pageloopnum \endcsname}\fi%
\xdef\doitnow{\write\@auxout{\string\expandafter%
\string\gdef\string\csname\space
endpage\the\c@chapter\endcsname{\the\c@page}}}
\doitnow
\global\parskip=\saveparskip
\newpage
%% just in case...
\ifdim\ht\dbltopins>0pt \unvbox\dbltopins\fi
\ifdim\ht\dblbotins>0pt \unvbox\dblbotins\fi
\resetdefaults
}
%%%%%%%%%%%
%% 3) Theorems, Proofs, Examples, etc.
\newif\ifperiodaftertheorem
\def\MakePeriodAfterTheorem{\global\periodaftertheoremtrue}
\newif\ifthmsectionnum
\newskip\abovetheoremskip
\newskip\belowtheoremskip
\abovetheoremskip=6pt plus .1pt
\belowtheoremskip=6pt
\def\@xnthm#1#2[#3]{\expandafter\@ifdefinable\csname #1\endcsname
{\@definecounter{#1}\@addtoreset{#1}{#3}%
\expandafter%
\xdef\csname the#1\endcsname{\@thmcounter{#1}}%
\global\@namedef{#1}{\@thm{#1}{#2}%%
}\global\@namedef{end#1}{\@endtheorem}}}
\newtheorem{theorem}{Theorem}
%%%% Redefine these two in particular style if necessary.
\def\TheoremTypeEnv#1{\gdef\saveone{#1}\futurelet\next\tlookforbracket}
\def\tlookforbracket{\ifx\next[\let\go\xTheoremWithTitle
\else\let\go\xTheoremTypeEnv\fi\go}
\def\xTheoremTypeEnv{\vskip\abovetheoremskip
\indent{\theoremfont\saveone%
\ifperiodaftertheorem\@thmcountersep\fi}\enskip\hfourbfont}
\def\xTheoremWithTitle[#1]{\vskip\abovetheoremskip
\noindent{\theoremfont\saveone%
\quad #1\ifperiodaftertheorem\@thmcountersep\fi}\enskip\hfourbfont}
\def\ExampleTypeEnv#1{\trivlist \item[\hskip \labelsep{\it#1:}]}
%%%%
\def\@endtheorem{\if@newlist\@noitemerr\fi
   \if@inlabel\indent\fi
   \ifhmode\unskip \par\fi
\everypar={} %<===
\vskip\belowtheoremskip
\indent % <===
}
%%% Demo and Proclaim
\def\demo#1{\trivlist \item[\hskip \labelsep{\bf#1%
\ifperiodaftertheorem\@thmcountersep\fi}]}
\let\enddemo\@endtheorem
\let\proclaim\TheoremTypeEnv
\let\endproclaim\@endtheorem
%%%
%%%%%%%%%%%
%% 4) Algorithm
\newcount\algoline
\def\doalgoskip{\vskip6pt}
\def\lookatalgo{\vskip1sp\ifx\next[\let\go\xalgoinsides\else
\let\go\yalgoinsides\fi\go}
\newcount\algocount
\def\algorithm{\vskip12pt
\obeyspaces
\global\algoline=0
\global\advance\algocount by1
\ifnevernums
\gdef\@currentlabel{\the\algocount}\fi
\futurelet\next\lookatalgo}
\def\numberedpars{\global\everypar={\ifnonumline
\ifnevernums\else
\global\nonumlinefalse\fi\else
\global\advance\algoline by1
\hskip\parindent\hbox to 0pt{\hss\rm\the\algoline.\hskip8pt}\fi}}
\def\xalgoinsides[#1]{\noindent%
{\theoremfont Algorithm \the\algocount\ (#1).}%
\vskip4pt
\bgroup
\let\\ \doalgoskip
\let\note\algonote
\obeylines\obeyspaces
\numberedpars
\ifnevernums\else
\gdef\@currentlabel{\the\algoline}\fi}
\def\yalgoinsides{\noindent{\theoremfont Algorithm \the\algocount.}
\vskip4pt
\bgroup
\let\note\algonote
\let\\ \doalgoskip
\obeylines\obeyspaces
\numberedpars
\ifnevernums\else
\gdef\@currentlabel{\the\algoline}\fi}
\def\endalgorithm{\vskip1sp\egroup\gdef\@currentlabel{}
\vskip12pt\relax\global\nevernumsfalse
\global\nonumlinefalse
\global\everypar={}}
\def\algonote#1{\hfill{\it #1}}
\newif\ifnonumline
\newif\ifnevernums
\def\nonumline{\global\nonumlinetrue\relax}
\def\nonumalgorithm{\global\nonumlinetrue\global\nevernumstrue\relax
\algorithm}
\let\endnonumalgorithm\endalgorithm
%%%%%%%%%%
%% 5) Footnotes, endnotes
 \newcount\footnum
 \newcount\savefootnum
\def\notes{\global\let\section\savesection \global\appendonfalse
{\ifnum\footnum=0\else
\savefootnum=\footnum
\footnum=0
\section*{Notes}%
\noindent\loop\ifnum\savefootnum>\footnum%
\global\advance\footnum by1\relax%
\csname foot\the\footnum\endcsname\relax%
\expandafter\gdef\csname foot\the\footnum\endcsname{\relax}\relax%
\repeat\global\footnum=0\relax\fi}}
\long\def\endnotes#1{\global\advance\footnum by 1\relax$^{\the\footnum}$%
\long\expandafter\gdef\csname foot\the\footnum\endcsname{%
\vbox{\footnotesize\everypar={\hskip-10pt\everypar={}}
\leftskip=10pt\relax
\noindent\hbox to10pt{\the\footnum.\hfill}{\def\@currentlabel{\the\footnum}%
#1\strut\vskip1sp}}\vskip1pt}}
\def\endnotetext#1{\global\advance\footnum by 1\relax
\long\expandafter\gdef\csname foot\the\footnum\endcsname{%
\vtop{\footnotesize%\hsize=\textwidth
\leftskip=12pt\relax\parindent=-12pt
\indent\hbox to12pt{\the\footnum.\hfill}{#1\strut\vskip1sp}}\vskip1pt}}
%%%%%%%%
%% 6) Acknowledgments
\newskip\ackskip
\def\acknowledgments{\goodbreak\vskip\ackskip\section*{Acknowledgments}}
\let\acknowledgements\acknowledgments
\let\acknowledgement\acknowledgments
\let\acknowledgment\acknowledgments
\let\endacknowledgment\relax
\let\endacknowledgments\relax
\let\endacknowledgement\relax
\let\endacknowledgements\relax
%%%%%%%%
%% 7) Appendices
% Appendix takes an argument: \appendix{A}, \appendix{B} or \appendix{}
%
\newif\ifappendon
\newif\ifupperappend
\def\applett{A}
\def\howmany#1#2*{\def\xtwo{#2}}
\def\appendix#1{\global\appendontrue\goodbreak\def\xone{#1}%
\global\advance\c@appendnum by 1
\gdef\applett{\Alph{appendnum}}%
\refstepcounter{chapter}%
\refstepcounter{section}%
\global\c@section=0
\section*{\appendixfont\appendixname%
\ifx\xone\empty%\gdef\applett{A}
\else%
\expandafter\howmany\xone*%
\appendixfont\ifx\xtwo\empty\ #1\gdef\applett{#1}\else%
\unskip:\ \ifupperappend\uppercase{#1}%
\else#1\fi\fi\fi}}%
\def\appendixtitle#1{\vskip-\lastskip\vskip1sp\subsection*{#1}}
\let\titledappendix\appendix
%%%%%%%%
%% 8) References
%% Changed to make cites be in parens rather than []
\def\@cite#1#2{({#1\if@tempswa , #2\fi})}
\def\@biblabel#1{#1.}
%% changes to default to avoid problem in references
\def\@bibitem#1{\item
%\if@filesw
\immediate \write \@auxout {\string \bibcite {#1}
{\the \value {\@listctr }}}
%\fi
\ignorespaces}
\def\@biblabel#1{#1.}
\def\referencesize{\footnotesize}
\def\references{\notes
\savesection*{References}
\bgroup
\referencesize\itemsep=2pt
\list{\@biblabel{\arabic{enumiv}}}
{\settowidth\labelwidth{\@biblabel{10.}}%
    \leftmargin\labelwidth
%    \advance\leftmargin\labelsep
    \usecounter{enumiv}%
    \let\p@enumiv\@empty
    \def\theenumiv{\arabic{enumiv}}}%
    \def\newblock{\hskip .11em plus.33em minus.07em}%
    \sloppy\clubpenalty4000\widowpenalty4000 \frenchspacing
    \sfcode`\.=\@m}
% same as \references, except for #1 which doesn't get used, in
% case author supplies \thebibliography{10}, which now will not be used.
\def\bibreferences#1{\notes
\savesection*{References}
\bgroup
\referencesize\itemsep=2pt
\list{\@biblabel{\arabic{enumiv}}}
{\settowidth\labelwidth{\@biblabel{10.}}%
    \leftmargin\labelwidth
%    \advance\leftmargin\labelsep
    \usecounter{enumiv}%
    \let\p@enumiv\@empty
    \def\theenumiv{\arabic{enumiv}}}%
    \def\newblock{\hskip .11em plus.33em minus.07em}%
    \sloppy\clubpenalty4000\widowpenalty4000 \frenchspacing
    \sfcode`\.=\@m}
\def\endreferences{\endlist\egroup}
\let\endbibreferences\endreferences
\let\thebibliography\bibreferences
\let\endthebibliography\endreferences
%%%%%% Article Bibliography Using BibTeX
\def\bblname#1{\def\currentfilename{#1}}
\def\include#1{\relax\ifnum\@auxout=\@partaux
\@latex@error {\string \include \space cannot be nested}\@eha
\else\gdef\currfile{#1} \@include #1 \fi }
\def\articlebibliography#1{%
{\let\thebibliography\references
\let\endthebibliography\endreferences
\@input {\currfile.bbl}}
}
%%%%%%%%
%% 9) Received lines
%% defined below
%%%%%%%%
%% 10) PostScript Times Roman text fonts, and Bold Math in large sizes
\newdimen\titlesize
\let\sectiontitlemath\boldmath
\ifll\else
\gdef\@xxxpt{}
\gdef\xxxpt{\textfont\z@\twfvrm
  \scriptfont\z@\twtyrm \scriptscriptfont\z@\svtnrm
\textfont\@ne\twtymi \scriptfont\@ne\twtymi \scriptscriptfont\@ne\svtnmi
\textfont\tw@\twtysy \scriptfont\tw@\twtysy \scriptscriptfont\tw@\svtnsy
\textfont\thr@@\tenex \scriptfont\thr@@\tenex \scriptscriptfont\thr@@\tenex
%\def\unboldmath{\everymath{}\everydisplay{}\@nomath\unboldmath
%        \textfont\@ne\twtymi \textfont\tw@\twtysy \textfont\lyfam\twtyly
%        \@boldfalse}\@boldfalse
%\def\boldmath{\@subfont\boldmath\unboldmath}%
\def\unboldmath{\everymath{}\everydisplay{}\@nomath\unboldmath
          \textfont\@ne\tenmi \textfont\tw@\tensy
          \textfont\lyfam\tenly \@boldfalse}\@boldfalse
\def\boldmath{\@ifundefined{thirtymib}{\global\font
        \thirtymib\@mbi\space at\space \titlesize%\@magscale5
\global\font\thirtysyb\@mbsy\space at \titlesize %\@magscale5
         \global\font\thirtylyb\@lasyb\space at \titlesize
%\@magscale5\relax
\@addfontinfo\@xxxpt
               {\def\boldmath{\everymath
                {\mit}\everydisplay{\mit}\@prtct\@nomathbold
              \textfont\@ne\thirtymib \textfont\tw@\thirtysyb
            \textfont\lyfam\thirtylyb\@prtct\@boldtrue}}}{}\@xxxpt\boldmath}%
\def\prm{\fam\z@\twfvrm}%
\def\pit{\it\rm}%
\def\psl{\sl\rm}%
\def\pbf{\rm}%
\def\ptt{\rm}%
\def\psf{\@subfont\sf\rm}%
\def\psc{\@subfont\sc\rm}%
\def\ly{\fam\lyfam\twtyly}\textfont\lyfam\twtyly
   \scriptfont\lyfam\twtyly \scriptscriptfont\lyfam\svtnly
\@setstrut \bf}
\fi
\def\endash{--} %% Need these if PostScript fonts do not
\def\emdash{---}%% make these dashes join together.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% LaTeX Style for Multiple Columns with Floating Insertions            %%
%%                                                                      %%
%% Copyright 1999, Amy Hendrickson, TeXnology Inc.                      %%
%% All rights reserved                                                  %%
%%                                                                      %%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%
%%% Double column output routines,
%%% with single and double column insertions,
%%% to be positioned at page or column top, page or column bottom, page
%%% or column center, or full page floating insertion.
%%% This code was originally based on Frank Mittlebach's multicol.sty.
%%% The concept of how to balance columns is due to Donald Knuth. The
%%% way to integrate the changed output routine into the standard LaTeX
%%% output routine is due to Frank.
%%% The method of making the single and double column insertions and the
%%% changes in the output routines to accomodate these features are mine.
%%% A.H., January 1999
%%
%% Parameters you can set:
\columnsep=12pt
\newdimen\saveparskip
\saveparskip=\parskip
%%
\newdimen\captionskip %% space between caption and figure or table
\captionskip=6pt
%%%
\newdimen\PushOneColTopFig
\newdimen\PushOneColBotFig
\PushOneColTopFig=3.5pt %% push down from top of text page
\PushOneColBotFig=1.5pt   %% push up from bottom of text page
%%
\newdimen\PushOneColTopTab
\newdimen\PushOneColBotTab
\PushOneColTopTab=2.5pt  %% push down from top of text page
\PushOneColBotTab=0pt %1.5pt %% push up from bottom of text page
%%
\newdimen\PushTwoColTopFig
\newdimen\PushTwoColBotFig
\PushTwoColTopFig=2.5pt %% push down from top of text page
\PushTwoColBotFig=1.5pt   %% push up from bottom of text page
%%
\newdimen\PushTwoColTopTab
\newdimen\PushTwoColBotTab
\PushTwoColTopTab=1.5pt %% push down from top of text page
\PushTwoColBotTab=1.5pt %% push up from bottom of text page
%%
\newskip\intextfloatskip
\intextfloatskip=8pt plus 1pt %% Space between figure or table and text.
%% Normal and Appendix Figure and Table Numbers
\def\figurename{FIG.}
\def\thefigure{\@arabic\c@figure\theletter}
\def\tablename{TABLE}
\def\thetable{\@arabic\c@table\theletter}
\def\appthefigure{\applett\the\c@appfigure\theletter}
\def\appthetable{\applett\the\c@apptable\theletter}
\newcount\c@appfigure
\newcount\c@apptable
%% <=== end of parameters you can set
%%% New version of \@makecol that includes dbltopins and dblbotins,
%%% insertions that span both columns at the top and bottom of text page.
\def\new@makecol{\setbox\@outputbox
     \vbox{\boxmaxdepth \maxdepth
\ifvoid\dbltopins\else\unvbox\dbltopins\fi
     \unvbox\@cclv
\ifvoid\dblbotins\else\unvbox\dblbotins\fi
\ifvoid\footins\else\vskip\skip\footins\footnoterule\unvbox\footins\fi\vss}
\global\savefigandtabnumber\figandtabnumber
%\global\advance\savefigandtabnumber by 1 %% Because loop stops one short
                                         %% of the total number of figs
\global\savedblfigandtabnumber\dblfigandtabnumber
   \xdef\@freelist{\@freelist\@midlist}\gdef\@midlist{}\@combinefloats
   \setbox\@outputbox\vbox to\@colht{\boxmaxdepth\maxdepth
   \@texttop\dimen128=\dp\@outputbox\unvbox\@outputbox
   \vskip-\dimen128\@textbottom}%
   \global\maxdepth\@maxdepth}
%%
\let\@makecol\new@makecol %%
%% Default Latex output routine, to return to after balance columns
%% This should make it possible to capture the output routine of
%% any version of 2e as well as 2.09. ==>>
\edef\curroutput{\the\output}
\let\latexoutput\curroutput
%\def\latexoutput{\ifnum\outputpenalty <-\@M\@specialoutput
%\else
%  \@makecol\@opcol\@floatplacement\@startcolumn
%  \@whilesw\if@fcolmade \fi{\@opcol\@startcolumn}\fi
%  \global\vsize\ifnum\outputpenalty >-\@Miv \@colroom
%                  \else \maxdimen\fi}
%% table* and figure*  outside two columns keep default definitions.
%% Begin twocolumns ==>>
\newdimen\checkforspace
\def\twocolumns{\par \penalty\z@
\checkforspace=\pagegoal
\advance\checkforspace-\pagetotal
\advance\checkforspace-\ht\partialpage
\advance\checkforspace-\dp\partialpage
\advance\checkforspace by -2\ht\dbltopins
\advance\checkforspace by -2\dp\dbltopins
\advance\checkforspace by -2\ht\dblbotins
\advance\checkforspace by -2\dp\dblbotins
\ifdim\checkforspace<36pt\newpage\fi
\begingroup
%
\@namedef{figure*}{\twocolfig}
\@namedef{endfigure*}{\endtwocolfloat}
%
\@namedef{table*}{\twocoltable}
\@namedef{endtable*}{\endtwocolfloat}
%
\let\figure\onecolfig
\let\table\onecoltable
%
\let\@makecol\new@makecol %%
%%      Confines new def of \@makecol to
%%      two column material, preserves normal topskip after twocolumns is
%%      finished.
%%
\output={\global\setbox\partialpage=\vbox{\unvbox\@cclv}}
\vskip1sp %%
\eject
  \vbadness10001 \hbadness5000
  \tolerance5000
\parskip  \saveparskip plus .001pt %
\ifdraft
\baselineskip=17pt\else
\baselineskip=12pt  plus .001pt
  \fi% this allows stretch between lines
   %  \pretolerance\m@ne, keeps TeX from trying to make up paragraph
   %  without discretionary hyphens
  \advance\@colroom-\ht\partialpage
  \advance\@colroom-\dp\partialpage
\vsize=\@colroom
% Changed to \@colroom rather than 2\@colroom. Each column is output
% separately so we can find out if there are top or bottom inserts
% in first column.
%%
  \hsize\textwidth \advance\hsize-\columnsep
  \divide\hsize by 2
  \linewidth\hsize %% particular to LaTeX
  \columnwidth=\hsize
%% OR, you can substitute these lines:
%\hsize=\mycolumnwidth
%\linewidth=\hsize
%%
%% Output:
%  Set output to firstcolout, which checks to see if there are inserts
%  then doubles the vsize and send material back into input stream, then
%  sets the next output routine to \makeuppage
%
\output={\firstcolout}
\vskip2sp\ignorespaces}
\newbox\savetopinsert
\newbox\savebotinsert
\newbox\secondsavetopinsert
\newbox\secondsavebotinsert
\newbox\firstcolbox
\newbox\secondcolbox
\newbox\thefirstbox
\newbox\thesecondbox
\newdimen\firstcol
\newdimen\secondcol
\newdimen\checkheight
\newcount\loopnum
\newcount\firstcolsavefignum
\def\firstcolout{%
\global\firstcolsavefignum=\figandtabnumber
%\global\advance\firstcolsavefignum by 1
    %
\global\advance\vsize by\@colroom
\topskip=10pt
       \unvbox\@cclv
       \penalty\outputpenalty
\global\output={\makeuppage}}
%% This is where the floats and text are put together to make page:
\newbox\footnotebox
\newdimen\ColUsed
\newdimen\FigSpace
\newbox\testdblbotins
\def\makeuppage{%
%%
\FigSpace=.9\@colroom %%
\advance\FigSpace by-\ht\testdblbotins
\advance\FigSpace by-\dp\testdblbotins
\advance\FigSpace by-\ht\dbltopins
\advance\FigSpace by-\dp\dbltopins
%
%% Save top inserts from first column:
\global\setbox\savetopinsert=\vbox{%
% write top figures and tables into this box
\redefiningtrue
\global\loopnum=0 \global\checkheight=0pt
\loop\ifnum\loopnum<\savefigandtabnumber
  %
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\ifdim\checkheight>\FigSpace \relax
\global\loopnum=10000 % jump out of loop
%
\else
\csname topfloat\the\loopnum\endcsname
\fi
  %
\global\advance\loopnum by1
\repeat\relax
}
\ColUsed=\FigSpace %%
\advance\ColUsed by-\ht\savetopinsert
\advance\ColUsed by-\dp\savetopinsert
  %%
\global\setbox\savebotinsert=\vbox{%
  %% Write bottom figures and tables into this box
\redefiningtrue
\global\loopnum=0
\global\checkheight=0pt
\loop\ifnum\loopnum<\savefigandtabnumber
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\ifdim\checkheight>\ColUsed
\global\loopnum=10000 % jump out of loop
\else%
\csname botfloat\the\loopnum\endcsname
\global\expandafter\expandafter\let\csname botfloat\the\loopnum\endcsname\relax
\fi
  %
\global\advance\loopnum by1
\repeat
} %
  %
%
\global\setbox\secondsavetopinsert=\vbox{%
%% Write top figures and tables into this box
%
\global\loopnum=0 \global\checkheight=0pt
\redefiningtrue
\loop\ifnum\loopnum<\firstcolsavefignum
%
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\ifdim\checkheight>\FigSpace
\global\loopnum=10000 % jump out of loop
%
\else%
\csname topfloat\the\loopnum\endcsname
\fi
%
\global\advance\loopnum by1\repeat
}
%
\ColUsed=\FigSpace %%
\advance\ColUsed by-\ht\secondsavetopinsert
\advance\ColUsed by-\dp\secondsavetopinsert
%
\global\setbox\secondsavebotinsert=\vbox{%
%% Write bottom figures and tables into this box
%
\redefiningtrue
\global\loopnum=0 \global\checkheight=0pt
\loop\ifnum\loopnum<\firstcolsavefignum
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\ifdim\checkheight>\ColUsed
\global\loopnum=10000 % jump out of loop
%
\else%
\csname botfloat\the\loopnum\endcsname
\fi
%
\global\advance\loopnum by1\repeat
}
%
\global\dp\secondsavebotinsert=2.5pt
\global\dp\savebotinsert=2.5pt
%
   \splittopskip=\topskip
   \splitmaxdepth\maxdepth
   \dimen@\@colroom
%% Leave space if there is a two-column wide bottom insertion:
%
\global\setbox\dblbotins=\vbox{%
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dblbotins %% avoid eradicating this if it has anything in it
%% Write dblspan bottom figures and tables into this box
\redefiningtrue
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\savedblfigandtabnumber %%
\csname dblbotfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dblbotfloat\the\dblloopnum\endcsname
}
   \ifvoid\dblbotins \else
      \advance\dimen@-\ht\dblbotins  
      \advance\dimen@-\dp\dblbotins  
\fi
%
%% Leave space if there is a two-column wide top insertion:
   \ifvoid\dbltopins\else
      \advance\dimen@-\ht\dbltopins
      \advance\dimen@-\dp\dbltopins
\fi
%%
%% These changes are made to cut the column size down if we need
%% to fit an insert into the column. This allows the right and
%% left column to have differing amounts of text cut from box 255,
%% called box \@cclv in LaTeX. See \vsplit below.
\firstcol=\dimen@
\ifvoid\savetopinsert\else
\ifdim\ht\savetopinsert>0pt
\advance\firstcol by-\ht\savetopinsert
\advance\firstcol by-\dp\savetopinsert\fi\fi
\ifvoid\savebotinsert\else
\ifdim\ht\savebotinsert>0pt
\advance\firstcol by-\ht\savebotinsert
\advance\firstcol by-\dp\savebotinsert\fi\fi%
%%% KLUDGE below
\advance\firstcol by-3pt %% We need this to avoid       
                         %% error messages about overfull boxes.
%
\secondcol=\dimen@
%%
\ifvoid\secondsavetopinsert\else
\ifdim\ht\secondsavetopinsert>0pt
\advance\secondcol by-\ht\secondsavetopinsert
\advance\secondcol by-\dp\secondsavetopinsert\fi\fi
\ifvoid\secondsavebotinsert\else
\ifdim\ht\secondsavebotinsert>0pt
\advance\secondcol by-\ht\secondsavebotinsert
\advance\secondcol by-\dp\secondsavebotinsert\fi\fi
\ifvoid\footins\else
\ifdim\ht\footins>0pt
\global\setbox\footnotebox=\vbox{\vskip\skip\footins
\footnoterule\unvbox\footins\vskip1sp}
\advance\secondcol by-\ht\footnotebox
\advance\secondcol by-\dp\footnotebox\fi\fi
%%% KLUDGE below
\advance\secondcol by-3pt %% We need this to avoid
                          %% error messages about overfull boxes.
%
%% Cutting the amount of text that will fit from box255:
   \splittopskip=\topskip %%
   \splitmaxdepth\maxdepth
\setbox\thefirstbox \vsplit\@cclv to\firstcol
\setbox\thesecondbox \vsplit\@cclv to\secondcol
%
%% Making a text box that includes the inserts, column to the left:
\setbox\firstcolbox\vbox to\dimen@{%
\ifvoid\savetopinsert\else\unvbox\savetopinsert\fi%
\box\thefirstbox%
\ifvoid\savebotinsert\else\vfill\unvbox\savebotinsert\fi%
}
%
%% Making a text box that includes the inserts, column to the right:
\setbox\secondcolbox\vbox to\dimen@{%
\ifvoid\secondsavetopinsert\else\unvbox\secondsavetopinsert\fi%
\box\thesecondbox%%
\ifvoid\secondsavebotinsert\else\vfill\unvbox\secondsavebotinsert\vskip1sp\fi%
\ifvoid\footnotebox\else\unvbox\footnotebox\fi%
}
% Put back any text material that is not used:
  \ifvoid\@cclv \else
       \unvbox\@cclv
       \penalty\outputpenalty\fi
   \setbox\@cclv\vbox{\page@sofar}%
   \@makecol\@outputpage
   \global\@colroom\@colht
% Changed this so that each column will be output separately,
% so we have a chance to see if there are any insertions in the first
% column.
%
%% Do Page Inserts here, so that if there are any two col top inserts
%% They will be numbered after Page Inserts.
%%
\expandafter\ifx\csname pagefloat\the\pageloopnum\endcsname\relax
\else
\expandafter\ifx\csname pagefloat\the\pageloopnum\endcsname\empty
\else
\insertpage\fi\fi
%
%** doing this now gets the numbers right.
\global\setbox\dbltopins=\vbox{%+++
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dbltopins
%% Write dblspan bottom figures and tables into this box
\redefiningtrue %
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\savedblfigandtabnumber
\csname dbltopfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dbltopfloat\the\dblloopnum\endcsname%
                                %Save one for next page?
}
%**
%
%% dblfigandtabnumber rather than savedblfigandtabnumber here
\global\setbox\testdblbotins=\vbox{%
\let\c@figure\bogus
\let\c@table\bogus
\hsize=\textwidth
\linewidth=\textwidth
\copy\dblbotins %% avoid eradicating this if it has anything in it
%% Write dblspan bottom figures and tables into this box
\redefiningfalse
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber %%
\csname dblbotfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dblbotfloat\the\dblloopnum\endcsname
}
%
\ifbalancing
\global\balancingfalse
\global\vsize=2\@colroom
\else
\global\vsize\@colroom
\global\advance\vsize -\ht\dbltopins
\global\advance\vsize -\dp\dbltopins
\global\advance\vsize -\ht\testdblbotins
\global\advance\vsize -\dp\testdblbotins
\global\output={\firstcolout}%
\fi
}
\newcount\pageloopnum
\def\secondoutput{{\makeuppage }}
\newbox\endcolsavetopinsert
\newbox\endcolsavebotinsert
\newbox\testpagesize
\newif\ifbalancing
\newdimen\savelastskip
%%%
\def\endtwocolumns{\ifvmode\global\savelastskip=\lastskip\else
\savelastskip=0pt\relax\fi
\par\penalty\z@
\output={\global\setbox\testpagesize=\vbox{%
\unvbox\@cclv\ifdim\savelastskip>0pt\vskip-\savelastskip\fi}}\eject
 %%
 %% We reset output routine below to \latexoutput
 %% since we had to set multicolout to be the global output routine.
 %%
\dimen0=\ht\testpagesize
\advance\dimen0 by\dp\testpagesize
\unvbox\testpagesize
%\par\penalty\z@
%%
%% Above needed to see if we have more than one page worth of text.
%%
\ifdim\dimen0>2\@colroom
%\message{More than one page===================}
 %% More than one page of text:
 %%
\balancingtrue %% need this to avoid turning on firstcolout output routine.
\global\output={\settwocolboxes\makeuppage
 \global\output={\setboxesandredefine
\balance@columns\global\output={\latexoutput}}}
 \par
 \eject
\else
 %% Less than one page of text, now will figures fit too?
 %%
\setboxesandtest
\advance\dimen0 by\checkheight %% dimen set in setboxes
\advance\dimen0 by 2\ht\xdblbotins
\advance\dimen0 by 2\dp\xdblbotins
%% Above uses All the figures; if makeuppage is called not all figs will
%% be used, but that shouldn't hurt anything.
%%
\ifdim\dimen0>2\@colroom
%\message{More than one page, including one col figs===================}
\typeout{^^J^^J
C A R E F U L!^^J
You may want to change position of figures to make up page \the\c@page\space more^^J
elegantly!^^J^^J}
 %% If there is not enough space for all the figures, do this:
 \balancingtrue %% need this to avoid turning on firstcolout output routine.
 \global\output={\settwocolboxes\makeuppage
 \global\output={\setboxesandredefine
 \balance@columns\global\output={\latexoutput}}}
 \par
 \eject
\else
 %% There is enough space for all the figures, so do this:
% \message{Not more than one page, including one col figs===================}
\global\output={\setboxesandredefine
\balance@columns\global\output={\latexoutput}}\eject%
\fi\fi
\endgroup
\par\penalty\z@\relax
}
\def\setboxesandredefine{%
\global\setbox\dbltopins=\vbox{%+++
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dbltopins
% Write dblspan bottom figures and tables into this box
\redefiningtrue %
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber
\csname dbltopfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dbltopfloat\the\dblloopnum\endcsname%
%                           %Save one for next page?
}
%
\global\setbox\endcolsavetopinsert=\vbox{%
%% dropping figures and tables into this box
\loopnum=0
\redefiningtrue
\loop\ifnum\loopnum<\figandtabnumber
\csname topfloat\the\loopnum\endcsname
\global\advance\loopnum by1\repeat
\csname topfloat\the\loopnum\endcsname}
%
\global\setbox\endcolsavebotinsert=\vbox{
%% drop figures and tables into this box
\global\loopnum=0
\redefiningtrue
\loop\ifnum\loopnum<\figandtabnumber
\csname botfloat\the\loopnum\endcsname
\global\advance\loopnum by1\repeat
\csname botfloat\the\loopnum\endcsname}
%
\global\setbox\dblbotins=\vbox{%
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dblbotins %% avoid eradicating this if it has anything in it
% Write dblspan bottom figures and tables into this box
\redefiningtrue
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber %%
\csname dblbotfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dblbotfloat\the\dblloopnum\endcsname}
}
\def\settwocolboxes{%
\global\setbox\dbltopins=\vbox{%+++
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dbltopins
% Write dblspan bottom figures and tables into this box
\redefiningtrue %
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber
\csname dbltopfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dbltopfloat\the\dblloopnum\endcsname%
%                           %Save one for next page?
%}
%
%\global\setbox\dblbotins=\vbox{%
\hsize=\textwidth
\linewidth=\textwidth
\unvbox\dblbotins %% avoid eradicating this if it has anything in it
% Write dblspan bottom figures and tables into this box
\redefiningtrue
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber %%
\csname dblbotfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dblbotfloat\the\dblloopnum\endcsname}
}
%===
\newcount\bogus
\newbox\xdblbotins
\def\setboxesandtest{%
%
%% Checking height of one col figures:
\global\loopnum=0 \global\checkheight=0pt
%
\loop\ifnum\loopnum<\figandtabnumber
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\global\advance\loopnum by1\repeat
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname topfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\empty
\else
\expandafter\ifx\csname botfloat\the\loopnum\endcsname\relax
\else
\global\advance\checkheight by
\expandafter\csname totalht\the\loopnum\endcsname\relax
\fi\fi
\global\loopnum=0
\global\dblloopnum=0 \relax
\global\setbox\xdblbotins=\vbox{%
\let\c@figure\bogus
\let\c@table\bogus
\hsize=\textwidth
\linewidth=\textwidth
    % avoid eradicating both of these if it has anything in it:
\copy\dbltopins %
\copy\dblbotins %
% Write dblspan bottom figures and tables into this box
\redefiningfalse
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber %%
\csname dbltopfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dbltopfloat\the\dblloopnum\endcsname
\global\dblloopnum=0
\loop\ifnum\dblloopnum<\dblfigandtabnumber %%
\csname dblbotfloat\the\dblloopnum\endcsname
\global\advance\dblloopnum by1\repeat
\csname dblbotfloat\the\dblloopnum\endcsname}
}
\newbox\partialpage
\def\process@cols#1#2{\count@#1\relax
     \loop #2%
     \advance\count@\tw@
     \ifnum\count@<4
   \repeat}
%%%   Version for balanced column output.
%% this version to get topskip right on balanced page at end of article.
\def\Bpage@sofar{\unvbox\partialpage
%% major kludge! but it works ==>>
\null
\vskip-\topskip
\vskip-12pt
%% <<==
   \process@cols\z@{\wd\count@\hsize}%
   \hbox to\textwidth{%
     \process@cols\tw@{\box\count@
       \hss\vrule\@width\columnseprule\hss}%
     \box\z@}}
%%% AH: Version for two column output. %%
\def\page@sofar{\unvbox\partialpage
\wd\firstcolbox\hsize
\wd\secondcolbox\hsize
\hbox to\textwidth{%
\box\firstcolbox
   \hss\vrule\@width\columnseprule\hss
\box\secondcolbox}%%
}
%% Variant on \@outputpage to use for floating page insert:
\def\insertpage{\begingroup\catcode`\ =10
     \let\-\@dischyph \let\'\@acci \let\`\@accii \let\=\@acciii
     \if@twoside
       \ifodd\count\z@ \let\@thehead\@oddhead \let\@thefoot\@oddfoot
            \let\@themargin\oddsidemargin
          \else \let\@thehead\@evenhead
          \let\@thefoot\@evenfoot \let\@themargin\evensidemargin
     \fi\fi
     \shipout
     \vbox{\normalsize \baselineskip\z@ \lineskip\z@
           \let\par\@@par %%
           \vskip \topmargin \moveright\@themargin
           \vbox{\setbox\@tempboxa
                   \vbox to\headheight{\vfil \hbox to\textwidth
                                       {\let\label\@gobble \let\index\@gobble
                                         \@thehead}} %%
                 \dp\@tempboxa\z@
                 \box\@tempboxa
                 \vskip \headsep
%% change from \@outputpage below
\vbox to\textheight{\vfil
\hsize=\textwidth
\linewidth=\textwidth
\csname pagefloat\the\pageloopnum \endcsname
}
                 \baselineskip\footskip
                 \hbox to\textwidth{\let\label\@gobble
                           \let\index\@gobble  %%
                           \@thefoot}}}\global\@colht\textheight
           \endgroup\stepcounter{page}\let\firstmark\botmark
\global\advance\pageloopnum by1
\expandafter\ifx\csname pagefloat\the\pageloopnum\endcsname\relax\else%
\insertpage\fi%
}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\def\balance@columns{%
   \splittopskip=\topskip
   \splitmaxdepth\maxdepth
   \setbox\z@\vbox{
   \ifvoid\savetopinsert\else\unvbox\savetopinsert\fi
   \ifvoid\endcolsavetopinsert\else\unvbox\endcolsavetopinsert\fi
   \unvbox\@cclv
   \ifvoid\savebotinsert\else\unvbox\savebotinsert\fi
   \ifvoid\endcolsavebotinsert\else\unvbox\endcolsavebotinsert
\vskip-2.5pt\fi}
%
\dimen@\ht\z@
\advance\dimen@\dp\z@
\advance\dimen@ 2\topskip
   \advance\dimen@-2\baselineskip %% ?? was 4\baselineskip
%
   \divide\dimen@2%
%
{\vbadness\@M
   \loop{\process@cols\@ne{\global\setbox\count@
                             \box\voidb@x}}%
     \global\setbox\@ne\copy\z@
   {\process@cols\thr@@{\global\setbox\count@
                     \vsplit\@ne to\dimen@}}%
    \ifdim\ht\@ne >\ht\thr@@
    \global\advance\dimen@\p@
    \repeat}
%
   \dimen@\ht\thr@@
   \process@cols\z@{\@tempcnta\count@
        \advance\@tempcnta\@ne
        \setbox\count@\vtop to\dimen@ %% ??? vtop in Mittlebach
           {\unvbox\@tempcnta}}%
   \global\vsize\@colroom
   \global\advance\vsize\ht\partialpage
   \Bpage@sofar}
\@ifundefined{emergencystretch}
     {\newdimen\emergencystretch}{}
\emergencystretch 1pt
\def\setemergencystretch#1#2{%
   \emergencystretch 4pt
   \multiply\emergencystretch#1}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Figure and table captions in two column text and spanning both
%%% columns.
%%%%%%%%%%%%%%%%%%%%%%%%%
%% Single column inserts:
\let\savecaption\caption
\def\onecolfig{\ifvmode\else\unskip\vskip1pt\fi%
\global\captypefigtrue%
\@ifnextchar[\lookforposition{\lookforposition[e]}}
\def\onecoltable{\ifvmode\else\unskip\vskip1pt\fi\tabletextsize
\global\captypefigfalse
\@ifnextchar[\lookforposition{\lookforposition[e]}}
%% This is used in case we get more than one letter, i.e.,
%% \begin{figure}[htp]. This macro picks up only the first letter.
\def\defone#1#2*{\def\one{#1}}
\newcount\figandtabnumber
\newbox\figandtabbox
\newif\ifcaptypefig
\long\def\catchcaption#1{%
\ifcaptypefig%
\expandafter\gdef\csname caption\the\figandtabnumber\endcsname{%
\def\@captype{figure}\savecaption{#1}}%
\else%
\expandafter\gdef\csname caption\the\figandtabnumber\endcsname{%
\def\@captype{table}\savecaption{#1}}%
\fi}
\long\def\dblcatchcaption#1{%
\ifcaptypefig%
\expandafter\gdef\csname dblcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{figure}\savecaption{#1}}%
\else%
\expandafter\gdef\csname dblcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{table}\savecaption{#1}}%
\fi}
\long\def\pcatchcaption#1{%
\ifcaptypefig%
\expandafter\gdef\csname pcaption\the\pagefloatnumber\endcsname{%
\def\@captype{figure}\savecaption{#1}}%
\else%
\expandafter\gdef\csname pcaption\the\pagefloatnumber\endcsname{%
\def\@captype{table}\savecaption{#1}}%
\fi}
%%%
\long\def\catchletteredcaption#1#2{%
\ifcaptypefig%
\expandafter\gdef\csname letteredcaption\the\figandtabnumber\endcsname{%
\def\@captype{figure}\letteredcaption{#1}{#2}}%
\else%
\expandafter\gdef\csname letteredcaption\the\figandtabnumber\endcsname{%
\def\@captype{table}\letteredcaption{#1}{#2}}%
\fi}
\long\def\dblcatchletteredcaption#1#2{%
\ifcaptypefig%
\expandafter\gdef\csname dblletteredcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{figure}%
\letteredcaption{#1}{#2}}%
\else%
\expandafter\gdef\csname dblletteredcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{table}%
\letteredcaption{#1}{#2}}%
\fi}
\long\def\pcatchletteredcaption#1#2{%
\ifcaptypefig%
\expandafter\gdef\csname pletteredcaption\the\pagefloatnumber\endcsname{%
\def\@captype{figure}%
\letteredcaption{#1}{#2}}%
\else%
\expandafter\gdef\csname pletteredcaption\the\pagefloatnumber\endcsname{%
\def\@captype{table}%
\letteredcaption{#1}{#2}}%
\fi}
%%%
\long\def\catchcontinuedcaption{%
\ifcaptypefig%
\expandafter\gdef\csname continuedcaption\the\figandtabnumber\endcsname{%
\def\@captype{figure}\continuedcaption}%
\else%
\expandafter\gdef\csname continuedcaption\the\figandtabnumber\endcsname{%
\def\@captype{table}\continuedcaption}%
\fi}
\long\def\dblcatchcontinuedcaption{%
\ifcaptypefig%
\expandafter\gdef\csname dblcontinuedcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{figure}\continuedcaption}%
\else%
\expandafter\gdef\csname dblcontinuedcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{table}\continuedcaption}%
\fi}
\long\def\pcatchcontinuedcaption{%
\ifcaptypefig%
\expandafter\gdef\csname pcontinuedcaption\the\pagefloatnumber\endcsname{%
\def\@captype{figure}\continuedcaption}%
\else%
\expandafter\gdef\csname pcontinuedcaption\the\pagefloatnumber\endcsname{%
\def\@captype{table}\continuedcaption}%
\fi}
%%%
\long\def\catchletteredcontinuedcaption#1{%
\ifcaptypefig%
\expandafter\gdef\csname letteredcontcaption\the\figandtabnumber\endcsname{%
\def\@captype{figure}\letteredcontinuedcaption{#1}}%
\else%
\expandafter\gdef\csname letteredcontcaption\the\figandtabnumber\endcsname{%
\def\@captype{table}\letteredcontinuedcaption{#1}}%
\fi}
\long\def\dblcatchletteredcontinuedcaption#1{%
\ifcaptypefig%
\expandafter%
\gdef\csname dblletteredcontcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{figure}\letteredcontinuedcaption{#1}}%
\else%
\expandafter%
\gdef\csname dblletteredcontcaption\the\dblfigandtabnumber\endcsname{%
\def\@captype{table}\letteredcontinuedcaption{#1}}%
\fi}
\long\def\pcatchletteredcontinuedcaption#1{%
\ifcaptypefig%
\expandafter%
\gdef\csname pletteredcontcaption\the\pagefloatnumber\endcsname{%
\def\@captype{figure}\letteredcontinuedcaption{#1}}%
\else%
\expandafter%
\gdef\csname pletteredcontcaption\the\pagefloatnumber\endcsname{%
\def\@captype{table}\letteredcontinuedcaption{#1}}%
\fi}
%%%
\newcount\pagefloatnumber
\let\savelabel\label
\def\catchlabel#1{%
\expandafter\gdef\csname label\the\figandtabnumber\endcsname{\savelabel{#1}}}
\def\pcatchlabel#1{%
\expandafter\gdef\csname plabel\the\pagefloatnumber\endcsname{\savelabel{#1}}}
\def\dblcatchlabel#1{\expandafter%
\gdef\csname dbllabel\the\dblfigandtabnumber\endcsname{\savelabel{#1}}}
%% 
\def\startonecolfloat{%
\expandafter\newbox\csname figandtabbox\the\figandtabnumber\endcsname%
\global\setbox\csname figandtabbox\the\figandtabnumber\endcsname=
\vbox\bgroup% 
\ifcaptypefig\else%
\def\@captype{table}\centering\tabletextsize\fi%
\let\continuedcaption\catchcontinuedcaption%
\let\letteredcaption\catchletteredcaption%
\let\letteredcontinuedcaption\catchletteredcontinuedcaption%
\let\caption\catchcaption%
\let\label\catchlabel}
\def\xtopfloat{topfloat}
\def\xbotfloat{botfloat}
\newif\ifredefining
\newbox\sizeoffloat
\def\endonecolfloat{%
\ifcaptypefig\else%
\ifnobottomtabline\global\nobottomtablinefalse\vskip-6.5pt\else%
\vskip2pt\thetablelines\fi\fi%
\egroup%
\DonormalEndcol%
%% make room in column for this insert:
\setbox\sizeoffloat=\vbox{\let\unvbox\copy%
\let\c@table\bogus%
\let\c@figure\bogus%
\redefiningfalse \loopnum=\figandtabnumber%
\expandafter\csname \toporbotfloat\the\figandtabnumber\endcsname}%
%%
\expandafter\xdef\csname totalht\the\figandtabnumber\endcsname{%
\the\ht\sizeoffloat}% <=== use before \sizeoffloat is unboxed
\global\advance\figandtabnumber by1
%
\ignorespaces}
\def\DonormalEndcol{%% top float ==>
\ifx\toporbotfloat\xtopfloat%
%% figure ==>
  \ifcaptypefig%
  \expandafter\gdef\csname topfloat\the\figandtabnumber\endcsname{%
  \vbox{\vskip\PushOneColTopFig%
  \unvbox\csname figandtabbox\the\loopnum\endcsname%
  \vskip\captionskip%
  \csname caption\the\loopnum\endcsname%
  \csname letteredcaption\the\loopnum\endcsname%
  \csname continuedcaption\the\loopnum\endcsname%
  \csname letteredcontcaption\the\loopnum\endcsname}%
  \vskip\intextfloatskip %%
  \ifredefining%
  \csname label\the\loopnum\endcsname%
  \expandafter\gdef\csname topfloat\the\loopnum\endcsname{}\fi}%
%
\else% table ==>
 \expandafter\gdef\csname topfloat\the\figandtabnumber\endcsname{%
 \vbox{\vskip\PushOneColTopTab%%
 \csname caption\the\loopnum\endcsname%
  \csname letteredcaption\the\loopnum\endcsname%
  \csname continuedcaption\the\loopnum\endcsname%
  \csname letteredcontcaption\the\loopnum\endcsname%
  \vskip\captionskip%
  \unvbox\csname figandtabbox\the\loopnum\endcsname%
}
  \vskip\intextfloatskip %%
\ifredefining%
\csname label\the\loopnum\endcsname%
\expandafter\gdef\csname topfloat\the\loopnum\endcsname{}\fi}%
\fi%
%
\else% bottom float
%
\ifcaptypefig%
\expandafter\gdef\csname botfloat\the\figandtabnumber\endcsname{%
\vskip\intextfloatskip%
\vbox{\unvbox\csname figandtabbox\the\loopnum\endcsname%
  \vskip\intextfloatskip%
  \csname caption\the\loopnum\endcsname%
  \csname letteredcaption\the\loopnum\endcsname%
  \csname continuedcaption\the\loopnum\endcsname%
  \csname letteredcontcaption\the\loopnum\endcsname%
\vskip\PushOneColBotFig%%
}%
\ifredefining%
%\xdef\@currentlabel{\thefigure}%
\csname label\the\loopnum\endcsname%
\expandafter\gdef\csname botfloat\the\loopnum\endcsname{}\fi}%
  \else% TABLE
\expandafter\gdef\csname botfloat\the\figandtabnumber\endcsname{%
  \vskip\intextfloatskip%
\vbox{\csname caption\the\loopnum\endcsname%
  \csname letteredcaption\the\loopnum\endcsname%
  \csname continuedcaption\the\loopnum\endcsname%
  \csname letteredcontcaption\the\loopnum\endcsname%
  \vskip.5\intextfloatskip%
  \unvbox\csname figandtabbox\the\loopnum\endcsname%
\vskip\PushOneColBotTab%
}%
\ifredefining%
\csname label\the\loopnum\endcsname%
\expandafter\gdef\csname botfloat\the\loopnum\endcsname{}\fi}%
\fi\fi}
%
\def\startpagefloat{%
\expandafter\newbox\csname pagebox\the\pagefloatnumber\endcsname%
\global\setbox\csname pagebox\the\pagefloatnumber\endcsname=\vbox\bgroup%
\hsize=\textwidth%
\linewidth=\textwidth%
\let\label\pcatchlabel\ifcaptypefig\else\centering\fi%
\let\caption\pcatchcaption \let\letteredcaption\pcatchletteredcaption
\let\continuedcaption\pcatchcontinuedcaption
\let\letteredcontinuedcaption\pcatchletteredcontinuedcaption
}
\def\endpagefloat{\egroup
\DoNormalPage
\global\advance\pagefloatnumber by1
}
\def\DoNormalPage{\ifcaptypefig
%
\expandafter\gdef\csname pagefloat\the\pagefloatnumber\endcsname{%
\vbox{\hsize=\textwidth
\linewidth=\textwidth
\unvbox\csname pagebox\the\pageloopnum\endcsname
  \vskip\captionskip
  \csname pcaption\the\pageloopnum\endcsname
  \csname pletteredcaption\the\pageloopnum\endcsname
  \csname pcontinuedcaption\the\pageloopnum\endcsname
  \csname pletteredcontcaption\the\pageloopnum\endcsname
}
  \vskip\intextfloatskip %%
\xdef\@currentlabel{\thefigure}%
  \csname plabel\the\pageloopnum\endcsname
\expandafter\gdef\csname pagefloat\the\pageloopnum\endcsname{}}%
\else
\expandafter\gdef\csname pagefloat\the\pagefloatnumber\endcsname{%
\vbox{%%
\hsize=\textwidth
\linewidth=\textwidth
\iflandscapetable
\dimen0=\textheight
\advance\dimen0 by-\textwidth
\hsize=\textheight
\linewidth=\textheight\fi%
\csname pcaption\the\pageloopnum\endcsname
\csname pletteredcaption\the\pageloopnum\endcsname
\csname pcontinuedcaption\the\pageloopnum\endcsname
\csname pletteredcontcaption\the\pageloopnum\endcsname
  \vskip\intextfloatskip
  \unvbox\csname pagebox\the\pageloopnum\endcsname}
  \vskip\intextfloatskip %%
\csname plabel\the\pageloopnum\endcsname
\expandafter\gdef\csname pagefloat\the\pageloopnum\endcsname{}}\fi}
\newif\iffirstfighere
\global\firstfigheretrue
\newif\iffirsttabhere
\global\firsttabheretrue
\def\dofigmessage{\iffirstfighere\global\firstfigherefalse
\typeout{^^J
========================================================================
^^J
C A R E F U L !!!!!^^J
^^J
You have used \string\begin{figure}[h]\space !^^J
If there are any figures that appear earlier on the page^^J
they may be numbered incorrectly.^^J
^^J
Please check this and every other page on which you have used ^^J
\string\begin{figure}[h]\space !^^J
^^J
========================================================================
^^J}
\else
\typeout{^^J^^J
C A R E F U L !! \string\begin{figure}[h] used on this page !^^J^^J}
\fi}
\def\dotabmessage{\iffirsttabhere\global\firsttabherefalse
\typeout{^^J
========================================================================
^^J
C A R E F U L !!!!!^^J
^^J
You have used \string\begin{table}[h]\space !^^J
If there are any tables that appear earlier on the page^^J
they may be numbered incorrectly.^^J
^^J
Please check this and every other page on which you have used ^^J
\string\begin{table}[h]\space !^^J
^^J
========================================================================
^^J}
\else
\typeout{^^J^^J
C A R E F U L !! \string\begin{table}[h] used on this page !^^J^^J}
\fi}
\def\lookforposition[#1]{\defone#1*%
\let\go\startonecolfloat%
\let\endtable\endonecolfloat%
\let\endfigure\endonecolfloat%
\ifappendon\def\one{h}\fi%
\if\one h%
\ifcaptypefig%
\dofigmessage%
\else%
\dotabmessage%
\fi%
\let\go\doonecolfighere%
\let\endtable\endonecolfighere%
\let\endfigure\endonecolfighere%
\else%
\if\one t%
    \gdef\toporbotfloat{topfloat}%
\else%
\if\one b%
    \gdef\toporbotfloat{botfloat}%
\else%
\if\one p%
\let\go\startpagefloat%
\let\endtable\endpagefloat%
\let\endfigure\endpagefloat%
\else%
\if\one e% for either
   \ifdim\pagetotal>.6\pagegoal%
    \gdef\toporbotfloat{botfloat}%
    \else%
    \gdef\toporbotfloat{topfloat}%
    \fi%
\else%
    \doerr%
    \ifdim\pagetotal>.5\pagegoal%
    \gdef\toporbotfloat{botfloat}%
    \else%
    \gdef\toporbotfloat{topfloat}%
    \fi%
\fi\fi\fi\fi\fi\go}
\long\def\saveherecaption#1{\gdef\thesavedcaption{\savecaption{#1}}}
\long\def\saveherecontinuedcaption{\gdef\thesavedcaption{\continuedcaption}}
\long\def\savehereletteredcaption#1#2{\gdef\thesavedcaption{%
\gdef\theletter{#1}\savecaption{#2}}}
\long\def\savehereletteredcontinuedcaption#1{\gdef\thesavedcaption{%
\letteredcontinuedcaption{#1}}}
\long\def\saveherelabel#1{\gdef\thesavedlabel{#1}}
\saveherelabel{\relax}
\def\thesavedcaption{\relax}
\newbox\herebox
%% either figure [h] or table [h]
\newif\ifhere
\def\doonecolfighere{\bgroup
\heretrue
\ifcaptypefig\def\@captype{figure}\else
\def\@captype{table}\centering\fi
\setbox\herebox\vbox\bgroup
\let\label\saveherelabel
\let\caption\saveherecaption
\let\letteredcaption\savehereletteredcaption
\let\continuedcaption\saveherecontinuedcaption
\let\letteredcontinuedcaption\savehereletteredcontinuedcaption
}
\def\endonecolfighere{%
\egroup\vbox{%
\vskip\intextfloatskip
\ifappendon
\let\thefigure\appthefigure
\let\thetable\appthetable
\let\savecaption\appcaption\fi
%
\ifx\thesavedcaption\xrelax
  \unvbox\herebox
\else
  \ifcaptypefig
  \unvbox\herebox
  \thesavedcaption
%
  \else
%
  \thesavedcaption
  \unvbox\herebox
  \fi
\fi
%
\gdef\thesavedcaption{\relax}
\ifx\thesavedlabel\xrelax\else%
   \ifcaptypefig\else\edef\@currentlabel{\thetable}\fi
  \savelabel{\thesavedlabel}
  \gdef\thesavedlabel{\relax}
\fi
\gdef\theletter{\relax}
\vskip\intextfloatskip}\egroup\noindent\global\@ignoretrue}%
\def\doerr{%
\typeout{\space\space\space\space\space\space\space\space\space}
\typeout{Sorry!
Your choices following \string\figure\space or \string\table\space^^J%
are only [h] for `here', [t] for `top', [b] for `bottom', or [p]
for `page'.^^J%
I have ignored the [\one] and inserted the figure or table at this
point^^J%
on the page. See documentation if you need more help.}
\typeout{\space\space\space\space\space\space\space\space\space}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Inserts spanning both columns:
\newbox\spanbox
\def\centertabular{\bgroup\centering\savetabular}
\def\endcentertabular{\saveendtabular\vskip1pt\egroup}
%%%
\newbox\dbltopins
\newbox\dblbotins
\newskip\dblbotskip
\dblbotskip=12pt plus 3pt
\def\twocolfig{\par\global\captypefigtrue
\def\@captype{figure}
\@ifnextchar[\twolookforposition{\twolookforposition[t]}}%
\def\twocoltable{\par\tabletextsize
\global\captypefigfalse
\def\@captype{table}
\@ifnextchar[\twolookforposition{\twolookforposition[t]}}%
\newcount\dblfigandtabnumber
\dblfigandtabnumber=1
\newbox\dblfigandtabbox
\def\starttwocolfloat{\expandafter%
\ifx\csname dblfigandtabbox\the\dblfigandtabnumber\endcsname\relax%
\expandafter%
\newbox\csname dblfigandtabbox\the\dblfigandtabnumber\endcsname\fi%
\global\setbox\csname dblfigandtabbox\the\dblfigandtabnumber\endcsname=
\vbox\bgroup\let\label\dblcatchlabel%
\ifcaptypefig\else
\def\@captype{table}\centering\tabletextsize\fi%
\hsize=\textwidth
\linewidth=\textwidth\relax
%
\let\caption\dblcatchcaption
\let\letteredcaption\dblcatchletteredcaption
\let\continuedcaption\dblcatchcontinuedcaption
\let\letteredcontinuedcaption\dblcatchletteredcontinuedcaption
\let\label\dblcatchlabel
}
\newcount\dblloopnum
\newcount\savedblfigandtabnumber
\newcount\savefigandtabnumber
\def\xdbltopfloat{dbltopfloat}
\def\xdblbotfloat{dblbotfloat}
\def\endtwocolfloat{%
\ifcaptypefig\else
\ifnobottomtabline\global\nobottomtablinefalse\vskip-6.5pt\else
\vskip2pt
\thetablelines\fi\fi
\egroup
%
\ifx\dbltoporbotfloat\xdbltopfloat
\ifcaptypefig
\expandafter\gdef\csname dbltopfloat\the\dblfigandtabnumber\endcsname{%
\vbox{\vskip\PushTwoColTopFig
\copy\csname dblfigandtabbox\the\dblloopnum\endcsname
  \vskip\captionskip
  \csname dblcaption\the\dblloopnum\endcsname
  \csname dblletteredcaption\the\dblloopnum\endcsname
  \csname dblcontinuedcaption\the\dblloopnum\endcsname
  \csname dblletteredcontcaption\the\dblloopnum\endcsname
\ifredefining
\xdef\@currentlabel{\thefigure}%
\csname dbllabel\the\dblloopnum\endcsname\fi}
  \vskip\intextfloatskip %%
\ifredefining
\expandafter\gdef\csname dbltopfloat\the\dblloopnum\endcsname{}\fi}%
\else% table
\expandafter\gdef\csname dbltopfloat\the\dblfigandtabnumber\endcsname{%
\vbox{\vskip\PushTwoColTopTab
  \csname dblcaption\the\dblloopnum\endcsname
  \csname dblletteredcaption\the\dblloopnum\endcsname
  \csname dblcontinuedcaption\the\dblloopnum\endcsname
  \csname dblletteredcontcaption\the\dblloopnum\endcsname
  \vskip\captionskip
  \copy\csname dblfigandtabbox\the\dblloopnum\endcsname}
  \vskip\intextfloatskip %%
\ifredefining
\csname dbllabel\the\dblloopnum\endcsname
\expandafter\gdef\csname dbltopfloat\the\dblloopnum\endcsname{}\fi}
%
\fi
%
\else% bottom float
%
\ifcaptypefig
\expandafter\gdef\csname dblbotfloat\the\dblfigandtabnumber\endcsname{%
  \vskip\intextfloatskip
\vbox{\copy\csname dblfigandtabbox\the\dblloopnum\endcsname
  \vskip\captionskip
  \csname dblcaption\the\dblloopnum\endcsname
  \csname dblletteredcaption\the\dblloopnum\endcsname
  \csname dblcontinuedcaption\the\dblloopnum\endcsname
  \csname dblletteredcontcaption\the\dblloopnum\endcsname
\ifredefining
\csname dbllabel\the\dblloopnum\endcsname\fi
\vskip\PushTwoColBotFig}%
 \ifredefining%
\expandafter\gdef\csname dblbotfloat\the\dblloopnum\endcsname{}\fi}%
\else% table
\expandafter\gdef\csname dblbotfloat\the\dblfigandtabnumber\endcsname{%
  \vskip\intextfloatskip
\vbox{% 
  \csname dblcaption\the\dblloopnum\endcsname
  \csname dblletteredcaption\the\dblloopnum\endcsname
  \csname dblcontinuedcaption\the\dblloopnum\endcsname
  \csname dblletteredcontcaption\the\dblloopnum\endcsname
  \vskip\captionskip
\ifredefining
\csname dbllabel\the\dblloopnum\endcsname\fi
  \copy\csname dblfigandtabbox\the\dblloopnum\endcsname
\vskip\PushTwoColBotTab}%
\ifredefining%
\expandafter\gdef\csname dblbotfloat\the\dblloopnum\endcsname{}\fi}
%
\fi\fi%
%%
\global\advance\dblfigandtabnumber by1 \relax
}
\newbox\dblspanherebox
\def\dodblfigurehere{\global\setbox\dblspanherebox=\vbox\bgroup
\let\label\saveherelabel
\ifcaptypefig\def\@captype{figure}\else
\def\@captype{table}\centering\fi
\vskip\intextfloatskip
\hsize=\textwidth
\linewidth=\textwidth
\let\label\saveherelabel
\let\caption\saveherecaption
\let\letteredcaption\savehereletteredcaption
\let\continuedcaption\saveherecontinuedcaption
\let\letteredcontinuedcaption\savehereletteredcontinuedcaption}
\def\enddodblfigurehere{\vskip\intextfloatskip\egroup%
\aftergroup\endcolsneatly}
%%
\def\endcolsneatly{\ifdim\lastskip=2sp
\vskip-9.5pt\null\fi\endtwocolumns
\vbox{\vskip\intextfloatskip
\ifcaptypefig\def\@captype{figure}\else
\def\@captype{table}\centering\fi
\ifappendon
\let\thefigure\appthefigure
\let\thetable\appthetable
\let\savecaption\appcaption\fi
%
\ifx\thesavedcaption\xrelax
\unvbox\dblspanherebox
\vskip\intextfloatskip
\else
  \ifcaptypefig
\unvbox\dblspanherebox
  \thesavedcaption
\vskip\intextfloatskip
%
  \else
%
\vskip\intextfloatskip
  \thesavedcaption
\unvbox\dblspanherebox
\vskip\intextfloatskip
  \fi
\fi
%
\gdef\thesavedcaption{\relax}
\ifx\thesavedlabel\xrelax\else\savelabel{\thesavedlabel}
\gdef\thesavedlabel{\relax}\fi
\gdef\theletter{\relax}
\vskip\intextfloatskip}
\twocolumns\vskip2sp}
\def\twolookforposition[#1]{\defone#1*
\let\go\starttwocolfloat
\ifappendon\def\one{h}\fi
\if\one h %
\ifcaptypefig
\dofigmessage
\else
\dotabmessage
\fi
\let\go\dodblfigurehere
\@namedef{endfigure*}{\enddodblfigurehere}
\@namedef{endtable*}{\enddodblfigurehere}
\else
\if\one t
  \gdef\dbltoporbotfloat{dbltopfloat}
\else
\if\one b
  \gdef\dbltoporbotfloat{dblbotfloat}
\else
\if\one p
  \let\go\startpagefloat
\@namedef{endfigure*}{\endpagefloat}
\@namedef{endtable*}{\endpagefloat}
\else
\doerr
\fi\fi\fi\fi\go}
\gdef\dblfigure#1#2#3#4#5{\bgroup\def\@captype{figure}
\hbox to\textwidth{\vtop{\hsize=#2\textwidth
\advance\hsize by -1pc
\vskip#1
\caption{#3}
}\hskip2pc
\vtop{\hsize=#4\textwidth
\advance\hsize by -1pc
\vskip#1
\caption{#5}
}\hfill}\egroup}
\gdef\dbltable#1#2#3#4{\bgroup\def\@captype{table}
\hbox to\textwidth{\vtop{\parindent=0pt
\hsize=#1\textwidth
\advance\hsize by -1pc
\footnotesize
#2
}\hskip2pc
\vtop{\parindent=0pt \hsize=#3\textwidth
\advance\hsize by -1pc
\footnotesize
#4
}\hfill}\egroup}
\let\saveenddocument\enddocument
\def\enddocument{%
\ifdim\ht\dbltopins>0pt\vbox{\unvbox\dbltopins}\fi
\ifdim\ht\dblbotins>0pt\vbox{\unvbox\dblbotins}\fi
\ifnum\pagefloatnumber>\pageloopnum
\newpage
\insertpage{\hsize=\textwidth
\linewidth=\textwidth
\csname pagefloat\the\pageloopnum \endcsname}\fi%
\saveenddocument}
\splittopskip=\topskip
\def~{\penalty\@M{ }}
\def\mathleftline{\vskip-\parskip
\hbox to\textwidth{\hrulefill\hskip.5\textwidth}\vskip-\abovedisplayskip}
\def\mathrightline{\hbox to\textwidth{\hskip.5\textwidth\hrulefill}\vskip18pt}
\let\topline\mathleftline
\let\botline\mathrightline
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Specific Style Settings
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Contents:
%% 1) Fonts
%% 2) Font families
%% 3) Set Dimensions
%% 4) Running Head and Folio
%% 5) Article title page dimensions and parameters
%% 6) Abstract
%% 7) Keywords
%% 8) Section heads
%% 9) Splitmath
%% 10) Math, figure, theorem counters.
%% 11) Theorem environments
%% 12) Captions
%% 13) Table settings
%% 14) Lists
%% 15) Footnotes
%% 16) Appendices
%% FIX FOR CATCODE is needed, change ` to \char96 when \catcode`\ is
%% used in file
%%%%%%%%%%%%
%% 1) Fonts
%% PostScript Font information is found in bjourps.sty.
  %% Special Purpose Fonts
  %% Fonts to make bold in super- and sub-scripts be the right size.
  \font\sevbf =cmbx7
  \font\fivbf =cmbx5
  \let\bit\bf
  \def\baselinestretch{0}
\let\bit\bf
\titlesize=18pt %% for boldmath in title
\ifll\else
\def\titleboldmath{\global\font\thirtymib cmmib10 at\space \titlesize%
\global\font\thirtysyb cmbsy10\space at \titlesize %
\global\font\thirtylyb lasyb10\space at \titlesize
\everymath{\mit}
\everydisplay{\mit}
\textfont\@ne\thirtymib
\textfont\tw@\thirtysyb
\textfont\lyfam\thirtylyb
\@boldtrue}
\fi
%%
\let\tenbit\bf
\let\ninebit\bf
\def\cmtimes{cmr10 }%% used to test to see if PS fonts are
%% being used. For sample pages and documentation.
%% Computer Modern Default Fonts:
%%
\def\timesroman{cmr10 }
\def\timesitalic{cmti10 }
\def\timesbold{cmbx10 }
\def\timesbolditalic{cmbx10 }
\def\helvetica{cmss10 }
\def\helveticabold{cmssbx10 }
\def\helveticaboldoblique{cmssbx10 }
\def\universebold{cmbx10 }
\def\smallcaps{cmcsc10 }
\def\courier{cmtt10 }
%%  Fonts For Particular Use:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
\font\tenbit= \timesbolditalic at 10pt
\font\ninebit=\timesbolditalic at 9pt
\font\eightbit=\timesbolditalic at 8pt
%% headline
\font\foliofont= \timesroman at 10pt
\font\headtextfont= \timesroman at 8pt %running head text
\font\cccfont=\timesroman at 7pt
%% footline
\font\footlinefont=\helvetica at 8pt
\font\foliofootfont= \timesroman at 8pt
%% titlepage
\titlesize=18pt %% for boldmath in title
\font\titlefont=\helvetica at 16pt
\font\subtitlefont=\helvetica at 12pt
\font\specialsectionfont=\helvetica at 16pt
\font\authorfont=\timesroman at 11pt
\font\affilfont=\timesitalic at 8pt
\font\emailfont=\timesroman at 8pt
\font\communicatedfont=\timesitalic at 8pt
\font\receivedfont=\timesroman at 8pt
\font\dedicationfont=cmcsc10 at 8pt
\font\titlethanksfont=\helvetica at 10pt
\font\subtitlethanksfont=\helvetica at 8pt
\font\abstractfont=\timesbold at 9pt
\font\abstractcopyrightfont=\timesbold at 7.5pt
\font\abstractnamefont=\helveticaboldoblique at 10pt
\font\keywordnamefont=\timesbolditalic at 9pt
\font\keywordfont=\timesbold at 9pt
%% section heads
\font\sectionfont=\timesbold at 10pt %
\font\subsectionfont=\timesbold at 10pt
\font\subsubsectionfont=\timesitalic at 10pt %
\font\paragraphfont=\timesitalic at 10pt
%% caption fonts
\font\captionnamefont=\timesbold at 8pt
\gdef\captiontextfont{\footnotesize\rm}
\font\tablenamefont=\timesbold at 8pt
\gdef\tabletextfont{\footnotesize\rm}
\gdef\tabletextsize{\footnotesize\rm}
%% theorem type fonts
\let\demofont=\it %??
\font\theoremfont=cmcsc10
\let\theoremtextfont\it
\let\prooffont\it
\def\hfourbfont{\normalsize\relax}
%% appendix
\let\appendixfont\sectionfont
%% documentation
\font\elevenbit=\timesbolditalic at 11pt
%%%%%%%%%
%% 2) Font Families
\def\egnbold{\footnotesize\bf}
\def\tenbold{\normalsize\bf}
\def\xiipt{\sectionfont}%% to accomodate Tapsco change.
%%%%%%%%%%
%% 3) Set dimensions and parameters
\textheight = 56pc %% first page textheight
\newdimen\realtextheight
\realtextheight = 56pc %% following pages textheight
\textwidth = 43pc
\hsize=43pc
\newdimen\savehsize
\savehsize=43pc
\newdimen\mycolumnwidth
\mycolumnwidth=21pc
\parskip 0pt plus .001pt
\parindent 10pt
\newdimen\saveparindent
\saveparindent=\parindent
\newdimen\saveparskip
\saveparskip=\parskip
\clubpenalty=10000
\widowpenalty=10000
\predisplaypenalty=10000
%  PAGE LAYOUT PARAMETERS
%
%   \topmargin      : Extra space added to top of page.
%   @twoside        : boolean.  T if two-sided printing
%   \oddsidemargin  : IF @twoside = T
%                         THEN extra space added to left of odd-numbered
%                              pages.
%                         ELSE extra space added to left of all pages.
%   \evensidemargin : IF @twoside = T
%                         THEN extra space added to left of even-numbered
%                              pages.
%   \headheight     : height of head
%   \headsep        : separation between head and text
%   \footskip       : distance separation between baseline of last
%                     line of text and baseline of foot.
%                     Note difference between \footSKIP and \headSEP.
%   \textheight     : height of text on page, excluding head and foot
%   \textwidth      : width of printing on page
%   \@textbottom    : Command executed at bottom of vbox holding text of page
%                     (including figures).  The \raggedbottom command
%                     almost \let's this to \vfil (actually sets it to
%                     \vskip \z@ plus.0001fil). %expanded 18 Jun 86
%
%   \@texttop       : Command executed at top of vbox holding text of page
%                     (including figures).  Used by letter style; can also
%                     be used to produce centered pages.  Is \let to \relax
%                     by \raggedbottom and \flushbottom.
%
%   Page layout must also initialize \@colht and \@colroom to \textheight.
%
%  PAGE STYLE PARAMETERS:
%
%   \floatsep       : Space left between floats.
%   \textfloatsep   : Space between last top float or first bottom float
%                     and the text.
%   \topfigrule     : Command to place rule (or whatever) between floats
%                     at top of page and text.  Executed in inner vertical
%                     mode right before the \textfloatsep skip separating
%                     the floats from the text.  Must occupy zero vertical
%                     space.  (See \footnoterule.)
%   \botfigrule     : Same as \topfigrule, but put after the \textfloatsep
%                     skip separating text from the floats at bottom of page.
%   \intextsep      : Space left on top and bottom of an in-text float.
%   \@maxsep        : The maximum of \floatsep, \textfloatsep and \intextsep
%   \@fptop         : Glue to go at top of float column -- must be 0pt +
%                     stretch
%   \@fpsep         : Glue to go between floats in a float column.
%   \@fpbot         : Glue to go at bottom of float column -- must be 0pt +
%                     stretch
\topmargin -.75in
  \headheight 1pc
  \headsep   1pc
  \topskip = 10pt
  \footskip 0pt
\lineskip0pt
\normallineskip 1pt
\def\baselinestretch{1}
\oddsidemargin0in
\evensidemargin0in
\marginparwidth .75in
\marginparsep 7pt
\columnsep 1pc
\columnseprule 0pt
\footnotesep4pt
\skip\footins  24pt plus 4pt minus 2pt
\floatsep 12pt plus 2pt minus 2pt
\textfloatsep 20pt plus 2pt minus 4pt
\intextsep 12pt plus 2pt minus 2pt
\ifll\else
\@maxsep 20pt
\@dblmaxsep 20pt
\fi
\dblfloatsep 12pt plus 2pt minus 2pt
\dbltextfloatsep 20pt plus 2pt minus 4pt
\@fptop 0pt plus 1fil
\@fpsep 8pt plus 2fil
\@fpbot 0pt plus 1fil
\@dblfptop 0pt plus 1fil
\@dblfpsep 8pt plus 2fil
\@dblfpbot 0pt plus 1fil
\marginparpush 5pt
\@lowpenalty   51
\@medpenalty  151
\@highpenalty 301
\@beginparpenalty -\@lowpenalty
\@endparpenalty   -\@lowpenalty
\@itempenalty     -\@lowpenalty
%%%%%%%%
%% 4) Running Head and Folio
%% Uppercase on author and title line in running head.
\authorUCtrue
\titleUCtrue
\def\headlinesize{\footnotesize}
\footlinefoliofalse
\footlinehrulefalse
\newif\ifnoheadline
\def\jheadline{\hbox to\textwidth{\iftitle%
\iffinaltypesetters
\ifnoheadline\global\noheadlinefalse
\else
\vtop to0pt{\footnotesize\baselineskip=8pt
\noindent\cccfont\thejourname\\
Vol.~{\thevolume}, \themonth, \thepages
\ \theyear\newline
\cccfont\uppercase{Article no.}\ \thearticlenumber\vskip1pt\vss}%
\fi\else\hfill\fi
\else\ifodd\c@page
{\hfill\headlinesize\headtextfont\thetitle}%
\hfill\llap{\foliofont\the\c@page}
\else\rlap{\foliofont\the\c@page}\hfill%
{\headlinesize\headtextfont\theauthors}\hfill\fi\fi}}
\newif\ifnoapline
\def\jfootline{\iftitle\global\titlefalse%
%%
   \vtop to0pt{\vskip13pt
      \hbox to\textwidth{\hfill\foliofootfont\the\c@page\hfill}
      \vskip8pt
\hbox to\textwidth{\ifodd\c@page\hfill\fi
%  
\iffinaltypesetters
    \footlinefont
    \parindent=0pt
    \ifnoapline
    \else
%
         \ifodd\c@page
         \ifll\else\hbadness10000\fi
%
  \vtop to0pt{\footnotesize\baselineskip=8pt\cccfont
  \ifll\else\vskip12pt\fi
  \noindent\hfill{\thecccline
  \vskip2pt}
  \noindent\hfill Copyright \smcopyright~\theyear\ by Academic Press%
  \vskip1sp
 \noindent\hfill All rights of reproduction in any form reserved.
 \vskip1sp\vss}
%
       \else
 \vtop to0pt{\cccfont\baselineskip8pt
 \noindent{\footnotesize \thecccline
  \vskip2pt}
  \noindent Copyright \smcopyright~\theyear\ by Academic Press%
  \vskip1sp
  \noindent All rights of reproduction in any form reserved.
  \vskip1sp
       \vss}\fi%% ends ifodd
%
\fi% end ifnoapp
\fi% end iffinaltypesetters
\ifodd\c@page\else\hfill\fi}%end hbox to textwidth
\vss}%
%
\else\hfill\fi% end iftitle
}
%%%%%%
%% 5) Article title page dimensions and parameters
\CenterArticleHeadtrue
\def\specialtitleins{\raggedright}
\def\newtitlesize{\LARGE\baselineskip=20pt}%% added by Tapsco
\abovetitleskip=7.5pc %subtract .5pc to get the right dimension
\belowtitleskip=12pt
\abovesubtitleskip=0pt
\belowsubtitleskip=0pt %% since skip is actually above author
\aboveauthorskip=12pt
\belowauthorskip=9pt
\aboveaffilskip=6pt
\belowaffilskip=12pt
\newif\iffirstsection
\def\editor#1{%
\def\theeditor{\vskip\aboveacceptedskip\noindent Recommending editor: #1}}
\def\editors#1{%
\def\theeditor{\vskip\aboveacceptedskip\noindent Recommending editors: #1}}
\let\StartOnNewPage\relax %% can start on even or odd pages
%%%%%%%%%%%%
%% 6) Abstract
\def\abstractsize{\small}
\abstractmargin=0pt
\aboveabstractskip=1sp
\belowabstractskip=1sp
\belowabstractnameskip=8pt
%% 7) Keywords
\def\keywordssize{\small}
\abovekeywordsskip=16pt
%%%%%%
%% 8) Section Heads
\def\thesection       {\arabic{section}}
\def\thesubsection    {\thesection.\arabic{subsection}.}
\def\thesubsubsection {\thesubsection\arabic{subsubsection}.}
\def\xmakecenterlines#1{\vtop{%
\parindent=0pt
\parskip=0pt
\hyphenpenalty=10000
\centering
#1
}}
\sectionwrap=20pc
\newskip\sectskip
\sectskip=12pt plus2pt minus 2pt
\newskip\belowsectionskip
\belowsectionskip=10pt
\newskip\subsectskip
\subsectskip=10pt plus.001pt minus 1pt
\newskip\subsubsectskip
\subsubsectskip=10pt plus1pt minus 1pt
\def\ast{*}
\newif\ifsectionon
\def\section#1{\iffirstsection \global\firstsectionfalse\vskip-12pt\fi
\def\yone{#1}\ifx\yone\ast\let\go\ssection\else%
\let\go\xxsection\fi\go{#1}}
\let\savesection\section
\newif\ifsendcontents
\def\xxsection#1{\vskip\sectskip
\global\sectionontrue
\refstepcounter{section}
\def\@currentlabel{\ifappendon\applett.\fi\the\c@section}
\ifsendcontents\else%
        % hack so contents will only be sent for article with \contents
{\let\\ \
\addcontentsline{toc}{section}{\string\vskip-1pt
\ifappendon\applett.\fi\the\c@section.\string\ \string\ {%
\string\affilfont\space #1.}}}\fi
\xmakecenterlines{\hyphenpenalty=10000
\sectiontitlemath %% makes 10pt bold math
%\everymath{\zmit}
\normalsize\sectionfont\ifappendon\applett.\fi\thesection.\nobreak\hskip4pt%
\uppercase{#1}\vskip\belowsectionskip}%
\nobreak\global\everymath={}%
\everypar={\global\sectiononfalse\everypar={}}\ignorespaces}
\def\ssection#1#2{\vskip\sectskip\global\sectionontrue%
\xmakecenterlines{\hyphenpenalty=10000
\ifdraft\baselineskip=22pt\else\baselineskip=12pt\fi
\sectiontitlemath %% makes 10pt bold math
%\everymath{\zmit}
\sectionfont\uppercase{#2}\vskip\belowsectionskip}%
\nobreak\everypar={\global\sectiononfalse\everypar={}}}
%%%%
\def\subsection#1{\def\one{#1}\ifx\one\ast\let\go\ssubsection\else%
\let\go\xxsubsection\fi\go{#1}}
\let\savesubsection\subsection
\newif\ifsubsectionon
\def\xxsubsection#1{\ifsectionon\global\sectiononfalse\ifvmode\vskip-3pt
\else\vskip\subsectskip\fi%
\else\vskip\subsectskip\fi\global\subsectionontrue
\refstepcounter{subsection}%
\def\@currentlabel{\ifappendon\applett.\fi\the\c@section.\the\c@subsection}
\xmakecenterlines{\hyphenpenalty=10000
\sectiontitlemath %% 10pt boldmath
%\everymath{\zmit}
\subsectionfont\ifappendon\applett.\fi\thesubsection\hskip1em#1\vskip6pt}%
\global\everymath={}%
\everypar={\global\subsectiononfalse\everypar={}}
\nobreak}
\def\ssubsection#1#2{\ifsectionon\global\sectiononfalse
\ifvmode\vskip-\lastskip\fi\vskip\subsectskip\else
\vskip\subsectskip\fi\global\subsectionontrue
\refstepcounter{subsection}%
\xmakecenterlines{\hyphenpenalty=10000
\sectiontitlemath% boldmath at 10pt
\subsectionfont#2
\vskip6pt}%
\everypar={\global\subsectiononfalse\everypar={}}
\nobreak}
\def\subsubsection#1{\def\one{#1}\ifx\one\ast\let\go\ssubsubsection\else%
\let\go\xxsubsubsection\fi\go{#1}
}
\def\xxsubsubsection#1{%
\refstepcounter{subsubsection}%
\def\@currentlabel{\the\c@section.\the\c@subsection.\the\c@subsubsection}
\ifsectionon\global\sectiononfalse\ifvmode\vskip-3pt\else%
\vskip\subsubsectskip\fi\else%
\ifsubsectionon\global\subsectiononfalse
\ifvmode\vskip-3pt\else%
\vskip\subsubsectskip\fi\else\vskip\subsubsectskip\fi\fi%
\vtop{\def\\ {\vskip1sp}
\noindent{\subsubsectionfont\thesubsubsection\hskip1em%
#1}}\global\everymath={}%
\normalsize
\ifdraft \baselineskip=22pt\else \baselineskip=12pt\fi
\vskip4pt\nobreak}
\def\ssubsubsection#1#2{%
\refstepcounter{subsubsection}%
\ifsectionon\global\sectiononfalse\ifvmode\vskip-3pt\else
\vskip\subsubsectskip\fi\else%
\ifsubsectionon\global\subsectiononfalse\ifvmode\vskip-3pt
\else\vskip\subsubsectskip\fi\else\vskip\subsubsectskip\fi\fi%
\noindent\hskip1em{\subsubsectionfont#2}
\ifdraft \baselineskip=22pt\else \baselineskip=12pt\fi
\vskip4pt}
\def\paragraph#1{\vskip6pt
{\footnotesize\paragraphfont #1.\hskip1em\relax}%
\ifdraft \baselineskip=22pt\else \baselineskip=12pt\fi
\ignorespaces}
%%%%%%
%% 9) Split math
%% Set this true when the equation numbers in split math will
%% be vertically centered. Otherwise the equation number will
%% be at the right side of the second line.
\CenterSplitMathEqnotrue
%%%%%%
%% 10) Math, figure, table, theorem counters.
\def\figurename{FIG.}
\def\thefigure{\ifappendon\applett\fi\@arabic\c@figure\theletter}
\def\tablename{TABLE}
\def\thetable{\ifappendon\applett\fi\the\c@table\theletter}
\@addtoreset{equation}{chapter}   % Makes \section reset 'equation' counter.
\def\theequation{\ifappendon\applett.\fi%
\arabic{equation}\themathletter}
\let\savetheequation\theequation
%%%%%%
%% 11) Theorem environments
%% Writing the command \MakePeriodAfterTheorem in the file
%% will make `Theorem 2. xxxx' rather than `Theorem 2 xxxx'
\MakePeriodAfterTheorem
\newdimen\slugwidth
\newdimen\slugheight
\newdimen\slugdepth
\slugwidth=4pt
\slugheight=2pt
\slugdepth=2pt
\def\@thmcountersep{.}
%% do not use section numbers with theorem
\thmsectionnumfalse
%% Proof that will put proof box at the far left if the
%% proof ends with an equation, otherwise will leave 1 em space
%% following text.
\newbox\proofbox
\def\proof{\futurelet\next\lookforbracket}
\def\lookforbracket{\ifx\next[\let\go\usespecialterm
\else\let\go\relax
\ifvmode\vskip-\lastskip\fi
\vskip\abovetheoremskip
\global\setbox\proofbox=\vbox\bgroup%
\indent{\prooffont Proof\@thmcountersep}%
\enskip\relax\fi\ignorespaces\go}
\def\usespecialterm[#1]{\ifvmode\vskip-\lastskip\fi
\vskip\abovetheoremskip
\global\setbox\proofbox=\vbox\bgroup%
\noindent\hskip\parindent%
{\prooffont Proof\ \ {\rm(#1).}}\ \ \relax\ignorespaces}
\def\endproof{\hbadness10000\parfillskip=0pt\egroup
\unvbox\proofbox
\setbox0=\lastbox
\ifdim\ht0>1pt
\vskip-2pt
\noindent\hbox to\mycolumnwidth{\vbox{\parindent=0pt\unhbox0 \qed\hss}\hss}
\else
\vskip-8pt
\noindent
\vrule height\slugheight depth\slugdepth width\slugwidth\relax%
\fi\vskip\belowtheoremskip}
\newif\ifalwaysopenbox
\def\alwaysopenbox{\global\openboxtrue\global\alwaysopenboxtrue}
\def\openbox{\global\openboxtrue}
\newif\ifopenbox
\def\qed{\ifopenbox\ifalwaysopenbox\else\global\openboxfalse\fi
\unskip\nobreak~~~~\llap{%
\vbox{\hrule\hbox{\vrule height\slugheight depth\slugdepth%
\hskip\slugwidth\vrule}\hrule}}%
\else%
\unskip\nobreak~~~~\llap{\vrule height \slugheight width \slugwidth
depth\slugdepth}\fi}
\let\slug\qed
\let\filledbox\slug
\def\@begintheorem#1#2{\trivlist
\item[\hskip\parindent\theoremfont#1\ \/\/%
\ifthmsectionnum\ifappendon\applett\else\thesection\fi.\fi%
#2\ifperiodaftertheorem\@thmcountersep\fi]%
\def\@currentlabel{\ifthmsectionnum\ifappendon\applett\else\thesection\fi%
.\fi#2}\enskip \ \theoremtextfont}
\def\@opargbegintheorem#1#2#3{\trivlist
   \item[\hskip\parindent\theoremfont #1\ \/\/%
\ifthmsectionnum\ifappendon\applett\else\thesection\fi.\fi#2\ \ {(\rm#3).} ]%
\def\@currentlabel{%
\ifthmsectionnum\ifappendon\applett\else\thesection\fi.\fi#2}%
\enskip\ \theoremtextfont}
%% use section numbers with theorem
\thmsectionnumtrue
\newtheorem{lemma}{Lemma}
\newtheorem{property}{Property}
\newtheorem{proposition}{Proposition}
\newtheorem{corollary}{Corollary}
\@addtoreset{theorem}{section}
\@addtoreset{lemma}{section}
\@addtoreset{proposition}{section}
\@addtoreset{property}{section}
\@addtoreset{corollary}{section}
\def\xendtrivlist{\@endtheorem}
\def\case#1{\vskip12pt\indent{Case \uppercase{#1:}}\enskip}
\let\endcase\xendtrivlist
\def\result{\TheoremTypeEnv{\it Result}}
\let\endresult\xendtrivlist
\newcounter{exampnum}[section]
\def\example{\global\advance\c@exampnum by1
\TheoremTypeEnv{\theoremfont Example \ifappendon\applett\else\thesection\fi%
.\the\c@exampnum}%
\def\@currentlabel{\ifappendon\applett\else\thesection\fi.\the\c@exampnum}%
\rm}
\let\endexample\xendtrivlist
\@addtoreset{exampnum}{section}
\newcounter{xalgo}[section]
\def\algo{\global\advance\c@xalgo by1
\TheoremTypeEnv{\theoremfont Algorithm
\ifappendon\applett\else\thesection\fi.\the\c@xalgo}%
\def\@currentlabel{\ifappendon\applett\else\thesection\fi.\the\c@xalgo}%
\rm}
\let\endalgo\xendtrivlist
\@addtoreset{xalgo}{section}
\newcounter{defnum}[section]
\def\definition{\global\advance\c@defnum by1
\TheoremTypeEnv{\theoremfont Definition
\ifappendon\applett\else\thesection\fi.\the\c@defnum}%
\def\@currentlabel{\ifappendon\applett\else\thesection\fi.\the\c@defnum}%
\rm}
\let\enddefinition\xendtrivlist
\@addtoreset{defnum}{section}
\newcounter{connum}[section]
\def\conjecture{\global\advance\c@connum by1
\TheoremTypeEnv{\it Conjecture \the\c@connum}
\def\@currentlabel{\the\c@connum}%
}
\def\endconjecture{~\slug\xendtrivlist}
\def\demo#1{\vskip6pt\indent{\demofont #1.}\quad\relax}
\let\enddemo\@endtheorem
\let\proclaim\TheoremTypeEnv
\let\endproclaim\@endtheorem
\def\claim{\demo{Claim}}
\let\endclaim\enddemo
\def\remark{\demo{Remark}}
\let\endremark\enddemo
%%%%%%
%% 12) Captions
\centermultiplelinestrue
\centersinglelinetrue
\centersingletablinetrue
\let\captionsize\small
\abovefigcaptionskip=2pt
\abovetabcaptionskip=1pt
\abovetableskip=-4pt
%%%%%%
%% 13) Table settings
%% (make no op because they are not applied consistently)
\def\thetablelines{}%\hrule width \hsize\vskip1sp}%
\belowtabcaptionskip=3pt
\let\tablenotesize\footnotesize
\let\tabletextsize\footnotesize
\fullwidthtablefalse
\lineabovetabcaptionfalse
\linebelowtabcaptiontrue
\abovetabularskip=0pt
\belowtabularskip=0pt
%%%%%%
%% 14) Lists
%% Listing defaults:
%% Listing defaults:
\def\labelenumi{\theenumi}
\def\theenumi{\normalsize\rm\arabic{enumi}.}
\def\labelenumii{\theenumii}
\def\theenumii{\hss(\roman{enumii})}
\def\p@enumii{\theenumii}
\def\labelenumiii{\normalsize\theenumiii}
\def\theenumiii{\normalsize\alph{enumiii}.}
\def\p@enumiii{\theenumiii}
\def\labelenumiv{\theenumiv}
\def\theenumiv{\normalsize\Alph{enumiv}}
\def\p@enumiv{\theenumiv}
\def\enumerate{\ifnum \@enumdepth >\thr@@ \@toodeep \else
\advance \@enumdepth \@ne
\edef\@enumctr{enum\romannumeral\the\@enumdepth }%
\expandafter\list\csname label\@enumctr \endcsname
{\usecounter \@enumctr \def\makelabel ##1{%
\ifdim\leftmargin=1sp\relax\hskip\saveparindent\fi
\ifdim\leftmargin=2sp\relax\hskip2\saveparindent\fi
\hskip\@itemdepth\saveparindent\relax%
\hskip\@enumdepth\saveparindent\relax ##1}}
\fi}
\def\itemize{\ifnum \@itemdepth >\thr@@ \@toodeep \else
\advance \@itemdepth \@ne
\edef \@itemitem {labelitem\romannumeral\the\@itemdepth}%
\expandafter\list\csname \@itemitem \endcsname {\def\makelabel ##1{%
\ifdim\leftmargin=1sp\relax\hskip\saveparindent\fi
\ifdim\leftmargin=2sp\relax\hskip2\saveparindent\fi
\hskip\@enumdepth\saveparindent\relax%
\hskip\@itemdepth\saveparindent\relax##1}}\fi}
\newenvironment{description}
{\leftmargini=0pt \leftmarginii=1sp \leftmarginiii=2sp
\list{}{\labelwidth\z@ \itemindent-\leftmargin
                        \let\makelabel\descriptionlabel}}
               {\endlist}
\newcommand*\descriptionlabel[1]{\hspace\labelsep
\ifdim\leftmargin=0pt \hskip\saveparindent
\else \ifdim\leftmargin=1sp \hskip2\saveparindent
\else \ifdim\leftmargin=2sp \hskip3\saveparindent\fi\fi\fi
                                \normalsize\it #1}
\def\labelitemi{\normalsize$\m@th\bullet$}
\def\labelitemii{{\bf --}}
\def\labelitemiii{\normalsize$\m@th\ast$}
\def\labelitemiv{\normalsize$\m@th\cdot$}
\leftmargini 0pt
\leftmarginii 0pt
\leftmarginiii 0pt
\leftmarginiv 0 pt
\leftmarginv 0pt
\leftmarginvi 0pt
\leftmargin\leftmargini
\labelsep 4pt
\labelwidth\leftmargini\advance\labelwidth-\labelsep
\def\@listI{\leftmargin\leftmargini
\rightmargin\leftmargini
\parsep 4\p@ plus2\p@ minus\p@
\topsep 8\p@ plus2\p@ minus4\p@
\itemsep -3pt %4\p@ plus2\p@ minus\p@
}
\let\@listi\@listI
\@listi
\def\@listii{\leftmargin\leftmarginii
   \labelwidth\leftmarginii\advance\labelwidth-\labelsep
   \topsep 4\p@ plus2\p@ minus\p@
   \parsep 2\p@ plus\p@ minus\p@
   \itemsep \parsep}
\def\@listiii{\leftmargin\leftmarginiii
    \labelwidth\leftmarginiii\advance\labelwidth-\labelsep
    \topsep 2\p@ plus\p@ minus\p@
    \parsep \z@ \partopsep \p@ plus\z@ minus\p@
    \itemsep \topsep}
\def\@listiv{\leftmargin\leftmarginiv
     \labelwidth\leftmarginiv\advance\labelwidth-\labelsep}
\def\@listv{\leftmargin\leftmarginv
     \labelwidth\leftmarginv\advance\labelwidth-\labelsep}
\def\@listvi{\leftmargin\leftmarginvi
     \labelwidth\leftmarginvi\advance\labelwidth-\labelsep}
%%%%%%%%%
%% 15) Footnotes
\def\footnoterule{\kern -3\p@ \hrule
width 6pc %%<=== change this dimen to change width of footnote rule line
\kern 2.6\p@}
\let\footnote\endnotes
%%%%%%%%%
%% 16) Appendices
\def\appendixname{APPENDIX}
%%%%
\def\editorial{\vspace*{2.5pt}
\noindent{\titlefont Editorial}
\vskip24pt}
%%%%
\let\@noitemerr\relax
\resetdefaults
\hoffset-14pt
\endinput
Change Log
----------
October 11, 2001
Made change to onecoltable and onecolfigure so that
there isn't an error message if they are called in
vmode.
August 25, 2000
Change to eqnarray to get rid of extra space when mathletter
is used; Change to \@cite to make it use () rather than [];
Change to \@eqnnum to use [] rather than (), change to
\eqsplitmath from () to [].
May 23, 2000
Change to \begin{figure}[t]
... \end{figure} so that if it is found within a paragraph
it will not start a new line, and it will not put in any
extra spaces between words of the paragraph.
May 22, 2000
Change to \begin[figure}[h]... \end{figure} so that
if it is found within a paragraph the line following
will not be indented.
February 25, 2000
Improvement to \eqnarray so that if it follows
an equation with \mathletter, and uses mathletter
itself, it still will advance its counter.
January 19, 2000
Change to xref on no numbered algorithms so that they
use the number of the algorithm rather than the line
number.
Also change appendix so that the second one will be
B, etc.
May 29, 1999
Made endproof put qed symbol at far left is proof ends
with an equation, but following text if proof ends with
text. (tricky!); set section counter to 0 in appendix;
made theorems and similar environments in appendix use
appendix letters.
Changed to avoid noindent after example type environments.
\def\@endtheorem{\if@newlist\@noitemerr\fi
   \if@inlabel\indent\fi
   \ifhmode\unskip \par\fi
\everypar={} <===
\vskip\belowtheoremskip
\indent <===
}
May 15, 1999
Made math in title not be bold, so that they match
the title font more closely.
Got rid of titlemath and captiontitlemath because
they made boldmath malfunction when MathTimes fonts
were used.
Made table text not be bold, since authors might want
to have something stand out in table text, and having
table be all bold seems odd.
Changed definition of \bbf so that it changes sizes
in sub and superscripts.
Improved theorem type environments to make labelling work right.
April 15, 1999
Fixed bug in captiontitlemath and titlemath to keep font set
from being redefined after the first time it is used. Changed
\sectiontitlemath to \boldmath since it is in 10pt. Slightly
improves speed of processing.
April 12, 1999
Made \bibreferences#1 with #1 a no-op so that if author supplies
\thebibliography{10} it will not cause a problem.
Changed xref currentlabel in \@begintheorem and \@opargbegintheorem
Used \@addtoreset to start these items renumbering with
each new section: corollary, lemma, proposition and property.