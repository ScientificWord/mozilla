\def\currvers{February 25, 2000}
%% ajour.cls/sty %% 
%%
%% This file can be renamed to either
%% ajour.cls or ajour.sty depending on whether you are
%% using LaTeX2.09 (ajour.sty) or LaTeX2e (ajour.cls).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% LaTeX Generic Style file for Articles 
%% in `A' Format Journal: 
%% 
%% 6x9, Single Column;
%% Journal of Functional Analysis, and others of the same size
%%
%% Published by Academic Press
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Macros written by
%% Amy Hendrickson
%% TeXnology, Inc.
%% 57 Longwood Avenue
%% Brookline, Massachusetts  02446
%% 617 / 738-8029
%% amyh@TeXnology.com
%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\typeout{^^J^^J
Document Style `A Format Journals' ^^J
for Academic Press,^^J
Version \currvers^^J^^J}

\newif\ifll

\expandafter\ifx\csname LaTeXe\endcsname\relax % LaTeX2.09 is used
\else% LaTeX2e is being used, so set ll true
\global\lltrue\fi

\ifll \let\dooptions\ProcessOptions
\else
\let\dooptions\@options\fi
\dooptions

%% For both 2e and 2.09 ===>>>

\newif\iftypesetters
\def\finaltypesetting{\global\typesetterstrue\global\noaplinefalse}

%% Special Purpose Fonts
%% Fonts to make bold in super- and sub-scripts be the right size.
\font\sevbf =cmbx7
\font\fivbf =cmbx5
\let\bit\bf
\def\baselinestretch{0}

\ifll
%% 
\expandafter\ifx\csname sqrtsign\endcsname\relax
\def\sqrtsign{\radical "270370}\fi
\expandafter\ifx\csname mathunderscore\endcsname\relax
\def\mathunderscore{\leavevmode\kern.06em\vbox{\hrule width.3em}}
\def\mathellipsis{\mbox {$\m@th{\,.\,.\,.}\,$}}\fi
\global\newdimen\phspace
\else
\gdef\@plus{plus}
\gdef\@minus{minus}
\fi

\ifll\else
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

%%%%%%%
%% LaTeX book.sty, (January 14, 1992 release) 
%% was the original basis for this style, but
%% many changes have been made to it.

\def\@ptsize{0}

\@twosidetrue     %  Defines twoside option.
\@mparswitchtrue  %  Marginpars go on outside of page.

\def\ds@draft{\overfullrule 5pt}  % Causes overfull hboxes to be marked.


%% Special text placement: description, quotation

\def\quotation{\list{}{\listparindent 1.5em
    \itemindent\listparindent
    \rightmargin\leftmargin\parsep \z@ plus\p@}\item[]}
\let\endquotation=\endlist

\def\quote{\list{}{\rightmargin\leftmargin}\item[]}
\let\endquote=\endlist

\newdimen\descriptionmargin

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

%% Irrelevant, but in case someone uses this command:
\@namedef{figure*}{\typeout{<< NOTE: \string\begin{figure*} and
\string\end{figure*} are for multiple columns and are not used in this style.
Please use \string\begin{figure} and \string\end{figure} instead.>> }
\figure}
\@namedef{endfigure*}{\end@float}

%% Table counters and parameters.
\newcounter{table}[chapter]

\def\fps@table{tbp}
\def\ftype@table{2}

\def\fnum@table{\tablename~\thetable}

\let\endtable\end@float

%% Irrelevant, but in case someone uses this command:
\@namedef{table*}{\typeout{<< NOTE: \string\begin{table*} and
\string\end{table*} are for multiple columns and are not used in this style.
Please use \string\begin{table} and \string\end{table} instead.>> }
\table}
\@namedef{endtable*}{\endtable}

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

%% Special equation numbers. i.e., 16'
\def\specialeqnum#1{\def\theequation{#1}}

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
\vbox{\unskip\noindent\hbox to\textwidth{\hss\unhbox0\hss}\vskip3pt%
\noindent\hbox to\textwidth{\hfill\displaywidth\linewidth\llap{\@eqnnum}}}%
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

\let\saveendeq\endequation

\def\@@eqncr{\let\@tempa\relax%
    \ifcase\@eqcnt \def\@tempa{& & &}\or \def\@tempa{& &}%
     \else \def\@tempa{&}\fi% 
     \@tempa% 
\ifx\themathletter\xrelax%
%% make equation letters work ==>
\if@eqnsw%
\stepcounter{equation}\fi%
%% <==
\else\iffirsttime\global\firsttimefalse\stepcounter{equation}%
\fi\fi%
\if@eqnsw\@eqnnum%
\fi%
\ifx\templabel\relax\else\savelabel{\templabel}\fi%%% <<=== make label
\global\let\templabel\relax          % catch current number and letter
\global\let\themathletter\xrelax     % <<===
\gdef\theequation{\savetheequation}%
\global\@eqnswtrue\global\@eqcnt\z@\cr}


\def\eqnarray{\global\firsttimetrue % feb,2000
\let\@currentlabel=\theequation
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
$$\vcenter{\hbox to\textwidth{$\displaystyle#1$\hfil}
\ifCenterSplitMathEqno
\vskip-3pt
\vbox to0pt{\vss\hbox to\textwidth{\hfill(\theequation)}}
\else\vskip2pt\fi
\hbox to\textwidth{\hfil$\displaystyle#2$
\ifCenterSplitMathEqno
\hskip\wd2\qquad\else\qquad\hbox{(\theequation)}\fi}%
}% end vcenter
\let\@currentlabel=\theequation
\ifx\templabel\relax\else\savelabel{\templabel}\fi%
\global\let\templabel\relax
\gdef\themathletter{\relax}%
$$\aftergroup{\end{eqsplitmath}}}

\def\endeqsplitmath{\gdef\theequation{\savetheequation}%
\noindent\hskip-.25em\relax}


\long\def\splitmath#1\\ #2\end#3{$$
\vcenter{\hbox to\textwidth{$\displaystyle#1$\hfil}
\vskip3pt
\hbox to\textwidth{\hfil$\displaystyle#2$}%
}% end vcenter
$$\aftergroup{\end{splitmath}}}

\let\endsplitmath\endeqsplitmath

%%%%

\def\specialeqnum#1{\global\firsttimefalse
\mathletter{xxx}%% mathletter to keep the equation counter
                                    %%from advancing
\gdef\theequation{#1}}

\def\bbf#1{\mathchoice{\hbox{\boldmath$\displaystyle#1$}}
{\hbox{\boldmath$\textstyle#1$}}
{\hbox{\boldmath$\scriptstyle#1$}}
{\hbox{\boldmath$\scriptscriptstyle#1$}}
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% ii)  Lettered captions for tables and figures,
%%      Multiple caption lines may center automatically.


\def\theletter{\relax}
\newif\ifxfirsttime
\xfirsttimetrue
\def\xtable{table}
\def\xrelax{\relax}

\newif\ifcontinued
\newif\ifturnoffdot
\def\continuedcaption{\global\turnoffdottrue
\global\continuedtrue\caption{\relax}}

\newif\ifsidebyside
\def\caption{
\ifx\@captype\xtable
\else%
  \ifx\theletter\xrelax%
   \global\xfirsttimetrue
      \ifcontinued\global\continuedfalse\else\refstepcounter\@captype\fi%
  \else%
      \ifxfirsttime\global\xfirsttimefalse%
%
         \ifcontinued\global\continuedfalse
         \else%
         \refstepcounter\@captype\fi
%
\fi\fi\fi%
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}%
\xdef\currcaptype{\@captype}%
\global\continuedfalse\@dblarg{\@caption\@captype}}

\def\zzzcaption{\textwidth=13pc
\ifx\@captype\xtable
\else%
  \ifx\theletter\xrelax%
   \global\xfirsttimetrue
      \ifcontinued\global\continuedfalse\else\refstepcounter\@captype\fi%
  \else%
      \ifxfirsttime\global\xfirsttimefalse%
%
         \ifcontinued\global\continuedfalse
         \else%
         \refstepcounter\@captype\fi
%
\fi\fi\fi%
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname}%
\xdef\currcaptype{\@captype}%
\global\continuedfalse\@dblarg{\@caption\@captype}}

\def\sptabcaption{\ifx\theletter\xrelax%
\global\xfirsttimetrue\ifcontinued\global\continuedfalse\else%
\refstepcounter\@captype\fi\else%
\ifxfirsttime\global\xfirsttimefalse%
\ifcontinued\global\continuedfalse\else%
\refstepcounter\@captype\fi\fi\fi%
\global\continuedfalse%
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname\theletter}%
\xdef\currcaptype{\@captype}%
\@dblarg{\@caption\@captype}}

\def\letteredcaption#1{\gdef\theletter{#1}\caption}
\def\letteredcontinuedcaption#1{\gdef\theletter{#1}\continuedcaption}

\newskip\abovefigcaptionskip
\newskip\abovetabcaptionskip
\newskip\abovetableskip
\newif\ifcentermultiplelines
\newif\ifcentersingleline
\newif\ifcentersingletabline
\newif\iflineabovetabcaption
\newif\iflinebelowtabcaption
\newdimen\belowtabcaptionskip

\long\def\makecenterlines#1{
\setbox1=\vbox{\parindent=0pt
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
\hbox to\hsize{\hss\unhbox0\hss}}}

\def\tabfontswitch{%
\ifx\@captype\xtable\tablecaptextfont\else\captiontextfont\fi}

\def\tabnameswitch{\ifx\@captype\xtable%
\expandafter\ifx\csname tablecapnamefont\endcsname\relax%
\captionnamefont\else\tablecapnamefont\fi\else\captionnamefont\fi}

\long\def\@makecaption#1#2{%
     \dimen0=\textheight \advance\dimen0 by-\textwidth
     \iflandscape\hsize=\textheight\else \hsize=\textwidth\fi
%
\ifx\@captype\xtable% TABLE CAPTIONS
  \vskip\abovetabcaptionskip%
%
\iflandscape\moveright-.5\dimen0\fi%
    \vbox{\hbox to\textwidth{\hfill\captionsize\tabnameswitch#1%
    \ifturnoffdot---{\it Continued}\hskip4pt\else.\fi\hfill}}
%
\setbox\@tempboxa\hbox{\tabfontswitch#2}%
  \ifturnoffdot\else
\vskip1pt
  \ifdim \wd\@tempboxa >\textwidth
{\hbadness10000
\vskip4pt
\tablecaptextfont \tolerance=50
\hyphenpenalty10000 
\makecenterlines{#2}
}
        \else
\vskip1pt
             \hbox to\textwidth{\hfill\tablecaptextfont#2\hfill}
  \fi
  \fi
\else% FIGURE CAPTIONS
\setbox\@tempboxa\hbox{\captionsize\tabnameswitch#1.~\tabfontswitch#2}%
  \vskip\abovefigcaptionskip
  \ifdim \wd\@tempboxa >.9\textwidth
%
    \iflandscape\moveright-.5\dimen0\fi%
    \vbox{\captionsize\tabnameswitch\hskip\saveparindent\relax#1%
    \ifturnoffdot---{\it Continued}\hskip4pt\else.\fi~\tabfontswitch#2}
%
  \else
    \iflandscape\moveright-.5\dimen0\fi%
    \vbox{\hbox to\textwidth{\hfill\captionsize\tabnameswitch#1%
    \ifturnoffdot---{\it Continued}\else.\fi~\tabfontswitch#2\hfill}}
  \fi
\fi%
%
\global\everymath{}
\edef\@currentlabel{\expandafter\csname p@\@captype\endcsname%
\expandafter\csname the\@captype\endcsname\theletter}%
%
\gdef\theletter{\relax}%	
\ifx\@captype\xtable
\iflinebelowtabcaption\vskip\belowtabcaptionskip\fi
\vskip\abovetableskip\fi
\global\turnoffdotfalse\relax}

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
\ifx\@captype\xtable
\let\endtable\specialendtable\fi
\hbox to27pc{\hsize13pc \textwidth=13pc
\vtop{\ifx\@captype\xtable\startsidebysidetable\fi
\hsize13pc \textwidth=13pc
\parindent=0pt
%\centering

#1\vskip1sp
\ifx\@captype\xtable\endsidebysidetable\fi
}\hfill\vtop{\ifx\@captype\xtable\startsidebysidetable\fi
\hsize13pc
\textwidth=13pc
\columnwidth=\hsize 
\parindent=0pt
%\centering
#2

\ifx\@captype\xtable\endsidebysidetable\fi}}}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% iii) Tables. Ruled lines added to top or bottom of table.
%%      Table content will extend to the left and right of columns,
%%      no extra horizontal space left.

\long\def\@caption#1[#2]#3{\par
  \begingroup
    \@parboxrestore
    \if@minipage \@setminipage \fi
    \normalsize
    \@makecaption{\csname fnum@#1\endcsname}{\ignorespaces\hskip4pt #3}\par
  \endgroup}

\def\catchlabel{}

\def\savetable{\@float{table}}
\let\endsavetable\end@float

%% Made \begin{specialtable}... \end{specialtable} for cases in which there
%% is a tabular inside another tabular, or for when author wants to
%% use \begin{table}...\end{table} with something other than 
%% \begin{tabular}...\end{tabular} in it.

\def\specialtable{\def\@captype{table}%
\let\caption\sptabcaption
\let\tabular\savetabular\let\endtabular\saveendtabular%
\@float{table}}
\def\endspecialtable{\ifnobottomtabline\global\nobottomtablinefalse\else
\vskip6pt
\fi\end@float}


\newif\iffullwidthtable
\def\table{\def\tabular{\tiny\xtabular}
\let\endtabular\endxtabular
\long\def\tablenotes##1{\def\thetabnotes{{\vskip1sp
\leftskip=6pt \parindent=-6pt \rightskip=0pt \parfillskip=0pt plus 1fil
\tablenotesize##1\vskip1sp}}}
\expandafter\def\csname tabular*\endcsname ##1{%
\global\setbox\savecurrtable\hbox\bgroup\def\@halignto{to ##1}\@tabular}
\expandafter \let \csname endtabular*\endcsname = \endxtabular
\def\label##1{\gdef\catchlabel{##1}}%
\def\caption##1{\gdef\catchcaption{##1}}%
\tabletextsize\@float{table}}

\let\saveendtable\endtable

\newif\ifnobottomtabline
\def\nobottomline{\global\nobottomtablinetrue}
\let\nobottomlines\nobottomline

\def\startsidebysidetable{\def\tabular{\tiny\xtabular}
\let\endtabular\endxtabular
\long\def\tablenotes##1{\def\thetabnotes{{\vskip1sp
\leftskip=6pt \parindent=-6pt \rightskip=0pt \parfillskip=0pt plus 1fil
\tablenotesize##1\vskip1sp}}}
\expandafter\def\csname tabular*\endcsname ##1{%
\global\setbox\savecurrtable\hbox\bgroup\def\@halignto{to ##1}\@tabular}
\expandafter \let \csname endtabular*\endcsname = \endxtabular
\def\label##1{\gdef\catchlabel{##1}}%
\def\caption##1{\gdef\catchcaption{##1}}%
\tabletextsize%\@float{table}
}

\def\endsidebysidetable{%
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
\hsize=\textheight\else\hsize=\textwidth\ifll\else\hskip-.5\dimen0 \fi
\fi%
\hbox to\hsize{\hss%
\vbox{\ifdim\wd\savecurrtable>0pt% 
      \iffullwidthtable\hsize=\textwidth \centering\else%
      \hsize=\wd\savecurrtable\fi\fi%
\ifx\catchcaption\empty\else\vskip1sp{\savecaption{\catchcaption}}\fi%
%
\ifdim\wd\savecurrtable>0pt\relax%
\dimen0=\wd\savecurrtable
%% center table:
\hbox to\textwidth{\hss\unhbox\savecurrtable\hss}%
\vskip3pt
\hbox to\textwidth{\hss\vtop{\parindent=0pt
\hsize=\dimen0\thetabnotes}\hss}%
\gdef\thetabnotes{\relax}
\else\vskip1sp\fi
}\hss}%
%
\ifx\catchlabel\empty\else%
\xdef\@currentlabel{\thetable\theletter}%
\savelabel{\catchlabel}\fi%
\gdef\@currentlabel{}%
\gdef\catchlabel{}%
\gdef\catchcaption{}%
%\saveendtable%
\global\wd\savecurrtable=0pt}

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
\hsize=\textheight\else\hsize=\textwidth\ifll\else\hskip-.5\dimen0 \fi
\fi%
\hbox to\hsize{\hss%
\vbox{\ifdim\wd\savecurrtable>0pt% 
      \iffullwidthtable\hsize=\textwidth \centering\else%
      \hsize=\wd\savecurrtable\fi\fi%
\ifx\catchcaption\empty\else\vskip1sp{\savecaption{\catchcaption}}\fi%
%
\ifdim\wd\savecurrtable>0pt\relax%
\dimen0=\wd\savecurrtable
%% center table:
\hbox to\textwidth{\hss\unhbox\savecurrtable\hss}%
\vskip3pt
\hbox to\textwidth{\hss\vtop{\parindent=0pt
\hsize=\dimen0\thetabnotes}\hss}%
\gdef\thetabnotes{\relax}
\else\vskip1sp\fi
}\hss}%
%
\ifx\catchlabel\empty\else%
\xdef\@currentlabel{\thetable\theletter}%
\savelabel{\catchlabel}\fi%
\gdef\@currentlabel{}%
\gdef\catchlabel{}%
\gdef\catchcaption{}%
\saveendtable%
\global\wd\savecurrtable=0pt}


\long\def\tablenotes#1{\ifvmode\ifdim\lastskip>0pt\vskip-\lastskip\fi\fi%
\vskip3pt%
{\leftskip=6pt \parindent=-6pt \rightskip=0pt \parfillskip=0pt plus 1fil%
\tablenotesize#1\vskip1sp}\vskip1sp}

\let\thetabnotes\relax

\let\savecaption\caption
\let\catchcaption\empty
\newbox\savecurrtable

\let\savetabular\tabular
\let\saveendtabular\endtabular

\newskip\abovetabularskip
\newskip\belowtabularskip

\def\tabular{\vskip\abovetabularskip\vbox\bgroup\parindent=0pt\savetabular}
\def\endtabular{\saveendtabular\egroup\vskip\belowtabularskip}

\def\xtabular{\global\setbox\savecurrtable\hbox\bgroup%
\tabletextsize\unskip\savetabular}
\def\endxtabular{\saveendtabular\egroup}

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

\def\hline{\noalign{\vskip3pt\iffullwidthtable \hrule width\textwidth
height .5pt\else\hrule height .5pt\fi%
\vskip3pt}}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% iv)  Landscape figures and tables

\newif\iflandscape
\newif\iflandscapetable

\def\landscapefigure{\clearpage\global\landscapetrue
\thispagestyle{empty}
\figure[h]\vskip\textwidth\relax}

\def\endlandscapefigure{\endfigure\global\landscapefalse\clearpage}

\def\landscapetable{\clearpage\global\landscapetabletrue
\thispagestyle{empty}\table[h]}

\def\endlandscapetable{\endtable
\global\landscapetablefalse
\clearpage}

\def\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}

\newif\ifdraft
\def\draft{\global\drafttrue
\def\@oddfoot{\vtop to 0pt{\parindent=0pt \hsize=\textwidth \everypar{}
\vskip38pt\draftfont
D R A F T\hfill\today, \realtime\hfill D R A F T\global\titlefalse\vss}}
\def\@evenfoot{\@oddfoot}\ifll\normalsize
\else\normalsize\@normalsize\fi}

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
%% 4) Algorithm
%% 5) Footnotes, endnotes
%% 6) Acknowledgments
%% 7) Appendices
%% 8) References
%% 9) Font tools

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
\expandafter\ifx\csname endpage\the\currarticlenumber\endcsname\relax%
 ?? \else
\ifnum\c@page=%
\expandafter\csname endpage\the\currarticlenumber\endcsname\else\mrule%
\csname endpage\the\currarticlenumber\endcsname\fi\fi}

\newif\ifleadingzeros
\def\startingpage{%
\xdef\firstpage{\ifleadingzeros\ifnum\c@page<10 000\the\c@page\else%
\ifnum\c@page<100 00\the\c@page\else%
\ifnum\c@page<1000 0\the\c@page\else\the\c@page\fi\fi\fi
\else\the\c@page\fi}
\def\thepages{\firstpage\relax\lastpage\relax}}

\def\realcopyright{%
\ooalign {\hfil\raise .2ex\hbox{c}\hfil\crcr%
\raise.2ex\hbox{$\scriptstyle\bigcirc$}}}

\def\smcopyright{%
\ooalign {\hfil\raise .3ex\hbox{$\scriptstyle\rm c$}\hfil\crcr%
\raise.2ex\hbox{$\scriptstyle\bigcirc$}}}

\def\yearofpublication#1{\def\theyear{#1}}
\def\volume#1{\def\thevolume{#1}}
\def\issuenumber#1{\def\theissuenumber{#1}}
\def\cccline#1{\def\thecccline{#1}}

\yearofpublication{Year ??}
\volume{Volume ??}
\issuenumber{Number ??}

\def\received#1{\def\thereceived{#1}}
\def\revised#1{\def\therevised{#1}}
\def\correspondence#1{\def\thecorrespondence{#1}}
\def\grantinfo#1{\def\thegrantinfo{Contract grant sponsor: #1}}

\def\thereceived{}
\def\therevised{}
\def\theaccepted{}
\def\thecorrespondence{}
\def\thegrantinfo{}

\cccline{???} %% need to be used once for each journal, do not reset
              %% for each article.


%% To be used in Academic Press part of file:

\newif\ifauthorUC
\newif\iftitleUC

\def\authorrunninghead#1{\def\theauthors{\ifauthorUC\uppercase\fi{#1}}}
\def\titlerunninghead#1{\def\thetitle{\iftitleUC\uppercase\fi{#1}}}

\def\and{\vskip1pt{\centering and \vskip6pt}}

%% Reset default values

\def\resetdefaults{%
\gdef\theauthors{! Please 
write 
{\tt\string\authorrunninghead\string{\string<Author Name(s)\string>\string}}
in file !}
\gdef\thetitle{! Please 
write 
{\tt\string\titlerunninghead\string{\string<(Shortened) Article Title\string>%
\string}} in file !}
\yearofpublication{???}
\volume{???}
\gdef\thereceived{}
\gdef\therevised{}
\gdef\theaccepted{}
\gdef\thededication{}
\gdef\thecorrespondence{}
\gdef\thecommline{}
\gdef\thegrantinfo{}
\gdef\thejourname{Journal Name}
\gdef\thearticlenumber{HA-00000}
}

\resetdefaults

\newdimen\abovetitleskip
\newdimen\belowtitleskip
\newdimen\abovesubtitleskip
\newdimen\belowsubtitleskip
\newdimen\aboveauthorskip
\newdimen\belowauthorskip
\newdimen\aboveaffilskip
\newdimen\belowaffilskip


%% Making \thanks work multiple times in the title and in 
%% authors line.

\let\savefootnotetext\footnotetext
\newcount\thanksnum
\newcount\dothanks
\def\dothankssymbol{\ifcase\thanksnum\or*\or$\dagger$\or
$\ddagger$\or $\mathchar "278$\or $\mathchar "27B$\or $\|$\or $\dagger\dagger$
   \or $\ddagger\ddagger$ \else\@ctrerr\fi\relax}

\newcount\thankscounter
\def\thanks#1{\global\advance\thanksnum by1\relax
\global\advance\dothanks by1
\setbox1=\hbox{X}\vbox to\ht1{\hbox{\dothankssymbol}\vfill}}

\long\def\authorthanks{\futurelet\next\lookforast}
\def\lookforast{\ifx\next* \long\def\go##1##2{\titlethanks{##2}}
\else\let\go\xauthorthanks\fi\go}

\long\def\xauthorthanks#1{\global\advance\thanksnum by1\relax
\global\advance\dothanks by1
\raise.8ex\hbox{\/\footnotesize\dothankssymbol}}

\long\def\maketemp{\futurelet\next\lookmaketemp}
\def\lookmaketemp{\ifx\next*\long\def\go##1##2{\titlemaketemp{##2}}
\else\let\go\xmaketemp\fi\go}

\long\def\xmaketemp#1{\global\advance\thankscounter by1\relax
\expandafter\gdef\csname tempthanks\the\thankscounter\endcsname{#1}}

%%%%%%%%%%%%%%%%%%%%
%% Separate \thanks environment. Now used for unnumbered footnotes !

\newcount\titlethanksnum
\newcount\dotitlethanks
\newcount\titlethankscounter

\long\def\titlethanks#1{\global\advance\titlethanksnum by1\relax
\global\advance\dotitlethanks by1}

\long\def\titlethankssymbol{\titlethanksfont\the\thanksnum}

\long\def\titlemaketemp#1{\global\advance\titlethankscounter by1\relax
\expandafter\gdef\csname temptitlethanks\the\titlethankscounter\endcsname{#1}}
%%%%%%%%%%%%%%%%%%%%

\newif\iftitle

\def\StartOnNewPage{\clearpage
\ifodd\c@page\else\null\clearpage\fi}

\newcount\currarticlenumber

\long\def\subtitle#1{\vbox{\vskip\abovesubtitleskip
\let\thanks\authorthanks
\centering\def\\ {\vskip1pt}%
{\large\baselineskip=15pt\subtitlefont#1\vrule depth \belowsubtitleskip
width0pt height 0pt
\vskip1sp}%
}%
\setbox1=\hbox{\let\\ \relax\let\thanks\maketemp #1}}

\long\def\title#1{\global\currarticlenumber=\the\c@page
\startingpage
\global\titletrue
%
%
\vbox{\vskip\abovetitleskip}
{%
\let\thanks\authorthanks
\centering\def\\ {\vskip1pt}%
\large%\boldmath
\titlefont\baselineskip16pt#1\vrule depth\belowtitleskip
width0pt height 0pt
\vskip1sp%
}%
\setbox1=\hbox{\let\\ \relax\let\thanks\maketemp #1}
\normalsize}

\def\specialsection#1{\newpage
\global\currarticlenumber=\the\c@page
\startingpage
\global\titletrue
\global\noheadlinetrue
%
%
\vbox{\vskip\abovetitleskip}
{%
\let\thanks\authorthanks
\centering\def\\ {\vskip1pt}%
\medium\boldmath\titlefont\baselineskip16pt\uppercase{#1}
\vskip3pc
}%
\setbox1=\hbox{\let\\ \relax\let\thanks\maketemp #1}
\normalsize}

\newif\ifCenterArticleHead 

\long\def\email#1{\vskip-2pt\vskip-\belowaffilskip
\hbox to \textwidth{\hss \emailfont E-mail: #1\vrule
depth10pt width0pt\hss}}

\long\def\dedication#1{\def\thededication{\uppercase{#1}}}
\def\xdedication{\vskip6pt\hbox to \textwidth{\hss \dedicationfont
\thededication\hss}}

\long\def\affil#1{%
\hbox to\textwidth{\hss\vbox{\centering\hsize=.9\textwidth
{\let\thanks\authorthanks\frenchspacing\baselineskip=10pt\affilfont#1\vrule
width0pt depth\belowaffilskip\vskip1sp}}\hss}%
\setbox1=\hbox{\let\thanks\maketemp#1}}

\long\def\commline#1{\def\thecommline{%
\vskip3pt\hbox to\textwidth{\let\thanks\authorthanks%
\setbox1=\hbox{\let\thanks\maketemp#1}\hfill
\communicatedfont #1\hfill}\vskip8pt}}

\def\thecommline{}

\newif\ifabstractname
\newdimen\aboveabstractskip
\newdimen\belowabstractskip
\newdimen\belowabstractnameskip

\newdimen\abstractmargin
\abstractmargin=0pt

\newdimen\abovekeywordsskip

\def\keywords#1{{\vskip\abovekeywordsskip
\indent{\keywordnamefont\unskip Key Words\/:\ \ }
\small\keywordfont\unskip#1}}

%%%%%%%%%%%%
%% Begin, End article

\newskip\abovereceivedskip
\newskip\aboverevisedskip
\newskip\aboveacceptedskip

\def\footnotefont{\rm} %% default, may be redefined in particular .sty file.  


%%%%%%%%%%%
%% 3) Theorems, Proofs, Examples, etc.

\newif\ifperiodaftertheorem
\def\MakePeriodAfterTheorem{\global\periodaftertheoremtrue}

%
\newif\ifthmsectionnum

\newtheorem{theorem}{Theorem}

%% for users-- to turn on section numbers in theorem number:
\def\theoremsectionnumbers{\global\thmsectionnumtrue
\@addtoreset{theorem}{section}
\@addtoreset{lemma}{section}
\@addtoreset{proposition}{section}
\@addtoreset{property}{section}
\@addtoreset{corollary}{section}
}


%% for users-- to turn off section numbers in theorem number:
\def\notheoremsectionnumbers{\global\thmsectionnumfalse
\theorem{theorem}{Theorem}}

\theoremsectionnumbers
%\notheoremsectionnumbers

\newskip\abovetheoremskip
\newskip\belowtheoremskip
\abovetheoremskip=6pt plus .1pt
\belowtheoremskip=6pt plus .1pt

\def\@xnthm#1#2[#3]{\expandafter\@ifdefinable\csname #1\endcsname
{\@definecounter{#1}\@newctr{#1}[#3]%
\expandafter\xdef\csname the#1\endcsname{%
       \expandafter\noexpand\csname the#3\endcsname \@thmcountersep
          \@thmcounter{#1}}%
     \global\@namedef{#1}{\@thm{#1}{#2}}%
     \global\@namedef{end#1}{\@endtheorem}}}


%%%% Redefine these two in particular style if necessary.

\def\TheoremTypeEnv#1{\gdef\saveone{#1}\futurelet\next\tlookforbracket}

\def\tlookforbracket{\ifx\next[\let\go\xTheoremWithTitle
\else\let\go\xTheoremTypeEnv\fi\go}

\def\xTheoremTypeEnv{\ifvmode\vskip-\lastskip\fi
\vskip\abovetheoremskip
\indent{\theoremfont\saveone%
\ifperiodaftertheorem\@thmcountersep\fi}\enskip\hfourbfont}

\def\xTheoremWithTitle[#1]{\ifvmode\vskip-\lastskip\fi
\vskip\abovetheoremskip
\noindent{\theoremfont\saveone%
\quad #1\ifperiodaftertheorem\@thmcountersep\fi}\enskip \hfourbfont}

\def\ExampleTypeEnv#1{\trivlist \item[\hskip \labelsep{\it#1:}]}

%%%%

\def\@endtheorem{\if@newlist\@noitemerr\fi 
   \if@inlabel\indent\fi 
   \ifhmode\unskip \par\fi 
\everypar={}
\vskip\belowtheoremskip
\indent
}

%%% Demo and Proclaim

\def\demo#1{\vskip\abovetheoremskip\indent{\demofont #1.}\quad\relax}
\let\enddemo\@endtheorem

\let\proclaim\TheoremTypeEnv
\let\endproclaim\@endtheorem

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
\gdef\@currentlabel{\the\algocount}
\fi
\futurelet\next\lookatalgo}

\def\numberedpars{\global\everypar={\ifnonumline
\ifnevernums\else
\global\nonumlinefalse\fi\else
\global\advance\algoline by1
\hskip\parindent\hbox to 0pt{\hss\rm\the\algoline.\hskip8pt}\fi}}

\def\xalgoinsides[#1]{\noindent%
\hskip\parindent{\theoremfont Algorithm \the\algocount\ (#1).}%
\vskip12pt
\bgroup
\let\\ \doalgoskip
\let\note\algonote
\obeylines\obeyspaces
\numberedpars
\ifnevernums\else
\gdef\@currentlabel{\the\algoline}\fi}

\def\yalgoinsides{\noindent\hskip\parindent{\theoremfont Algorithm \the\algocount.}
\vskip12pt
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

\def\endnote#1{\global\advance\footnum by 1\relax$^{\the\footnum}$%
\long\expandafter\gdef\csname foot\the\footnum\endcsname{%
\vtop{\footnotesize\hsize=\textwidth\leftskip=12pt\relax\parindent=-12pt
\indent\hbox to12pt{\the\footnum.\hfill}{#1\strut\vskip1sp}}\vskip1pt}}

\def\endnotetext#1{\global\advance\footnum by 1\relax
\long\expandafter\gdef\csname foot\the\footnum\endcsname{%
\vtop{\footnotesize\hsize=\textwidth\leftskip=12pt\relax\parindent=-12pt
\indent\hbox to12pt{\the\footnum.\hfill}{#1\strut\vskip1sp}}\vskip1pt}}

\def\notes{\ifnum\footnum=0\else
\savefootnum=\footnum
\footnum=0
\savesection*{Notes}%
\noindent\loop\ifnum\savefootnum>\footnum%
\global\advance\footnum by1\relax%
\csname foot\the\footnum\endcsname\relax%
\expandafter\gdef\csname foot\the\footnum\endcsname{\relax}\relax%
\repeat\global\footnum=0\relax\fi}

%% Write this in a .sty file to have endnotes. Must also alter
%% references to make endnotes print before references.
% \let\footnote\endnote


%%%%%%%%
%% 6) Acknowledgments

\newskip\ackskip

\def\acknowledgment{\goodbreak\savesection*{Acknowledgment}
\bgroup \footnotesize}

\def\acknowledgments{\goodbreak\savesection*{Acknowledgments}
\bgroup \footnotesize}

\def\endacknowledgment{\vskip1sp\egroup}
\def\endacknowledgments{\vskip1sp\egroup}

%%%%%%%%
%% 7) Appendices
% Appendix takes an argument: \appendix{A}, \appendix{B} or \appendix{}
% 

\newif\ifappendon

\def\howmany#1#2*{\def\xtwo{#2}}
\newif\ifupperappend
\newcount\c@appcount

\def\appendix#1{\ifappendon\else\global\c@appcount=0\relax\fi
\global\advance\c@appcount by 1\relax
\global\appendontrue\goodbreak\def\xone{#1}%
\refstepcounter{chapter}%
\refstepcounter{section}%
\global\c@equation=0
\global\c@section=0
\savesection*{\appendixfont\appendixname%
\ifx\xone\empty\lowercase{\xdef\applett{\Alph{appcount}}}\else%
\expandafter\howmany\xone*%
\appendixfont\ifx\xtwo\empty\ #1\lowercase{\xdef\applett{\Alph{appcount}}}
\else%
\unskip:\quad #1\lowercase{\xdef\applett{\Alph{appcount}}}\fi\fi}
}

\def\appendixtitle#1{\vskip-\lastskip\vskip4pt\savesubsection*{#1}}
\let\titledappendix\appendix

%%%%%%%%
%% 8) References

%%%%%% Article Bibliography Using BibTeX

\def\bblname#1{\def\currentfilename{#1}}

\def\include#1{\relax\ifnum\@auxout=\@partaux 
\@latex@error {\string \include \space cannot be nested}\@eha 
\else\gdef\currfile{#1} \@include #1 \fi } 

\def\articlebibliography#1{%
{\let\thebibliography\bibreferences
\let\endthebibliography\endreferences
\@input {\currfile.bbl}}
}

\def\@biblabel#1{#1.}

\def\bibitem{\@ifnextchar[\@lbibitem\@bibitem}

\def\@lbibitem[#1]#2{\item[\@biblabel{#1}\hfill]
\if@filesw
      {\let\protect\noexpand
       \immediate
       \write\@auxout{\string\bibcite{#2}{#1}}}\fi\ignorespaces}

\def\@bibitem#1{\item\if@filesw \immediate\write\@auxout
       {\string\bibcite{#1}{\the\value{\@listctr}}}\fi\ignorespaces}

%%%%%%%%
%% 9) Font Tools:
%%    PostScript Times Roman text fonts, and Bold Math in large sizes


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


\def\endash{{\the\textfont0--}} %% Need these because PostScript fonts do not
\def\emdash{{\the\textfont0---}}%% make these dashes join together.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% specific journal 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% 
%% Contents:
%% 1) Fonts
%% 2) Font families
%% 3) Set Dimensions
%% 4) Running Head and Folio
%% 5) Article title page dimensions and parameters
%% 6) Section heads
%% 7) Splitmath
%% 8) Math, figure, theorem counters.
%% 9) Theorem environments
%% 10) Captions
%% 11) Table settings
%% 12) Lists
%% 13) Footnotes
%% 14) Appendices
%% 15) References
%% 16) Received lines

%%%%%%%%%%%%
%% 1) Fonts

%% Computer Modern Default Fonts:
%%
\def\timesroman{cmr10 }
\def\timesitalic{cmti10 }
\def\timesbold{cmbx10 }
\def\timesbolditalic{cmbx10 }
\def\helvetica{cmss10 } 
\def\smallcaps{cmcsc10 }
\def\courier{cmtt10 }

%%  Fonts For Particular Use:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

\font\fourteenbit= \timesbolditalic  at 12pt
\font\tenbit= \timesbolditalic  at 10pt 
\font\ninebit=  \timesbolditalic  at 9pt

\def\authorfont{\normalsize\rm} 
\def\titlefontsize{\medium}
\font\titlefont=\helvetica at 12pt 
\font\subtitlefont= \helvetica at 10pt
\font\authorfont= \timesroman at 10pt
  \font\titleheadsc=cmcsc10 at 6pt
  \font\titlethanksfont=\helvetica at 8pt 
  \font\cccfont=\timesroman at 7pt
\font\specialsectionfont= \helvetica at 12pt

\newdimen\titlesize
\titlesize=12pt %% to make bold math in title be the right size.  

\font\affilfont=\timesitalic at 8pt  
\font\emailfont=\timesroman at 8pt 
\font\communicatedfont=\timesitalic at 8pt
\def\receivedfont{\footnotesize} 
\font\dedicationfont=\timesroman at 9pt 

\def\abstractfont{\footnotesize}
\def\abstractnamefont{\footnotesize\rm}
\font\abstractcopyrightfont\timesroman at 6pt
\font\keywordnamefont=\timesitalic at 9pt 
\font\keywordfont=\timesroman at 9pt 

\font\foliofont=\timesroman at 10pt  
\font\foliofootfont=\timesroman at 8pt
\def\headtextfont{\footnotesize} % headline text font 
\def\footnotefont{\footnotesize} %
\def\footlinefont{\small} %

\font\sectionfont=\timesbold at 10pt 
\let\subsectionfont\sectionfont
\font\subsubsectionfont=\timesitalic at 10pt
\font\paragraphfont=\timesitalic at 10pt 
\font\subparagraphfont=\timesroman at 10pt %% h4a

\def\captionnamefont{\footnotesize\bf} 
\def\captiontextfont{\footnotesize\rm} 
\def\tablecapnamefont{\footnotesize\bf}
\def\tablecaptextfont{\footnotesize\rm}
\def\tabletextsize{\footnotesize} 

\font\theoremfont=cmcsc10 at 9.5pt
\def\theoremtextfont{\it}
\def\demofont{\it}
\def\prooffont{\it}

\def\hfourbfont{\normalsize\relax}

\def\appendixfont{\bf}
\font\draftfont\courier at 14pt

%%%%%%%%%
%% 2) Font Families

\ifll
\renewcommand{\@ptsize}{}
\renewcommand{\normalsize}{%
\ifdraft
   \@setfontsize\normalsize\@xpt{22}%
\else
   \@setfontsize\normalsize\@xpt{12}%
\fi%
\abovedisplayskip 12pt plus1pt minus4\p@
\belowdisplayskip=\abovedisplayskip
\abovedisplayshortskip 6pt plus3\p@
\belowdisplayshortskip 6pt plus3\p@ minus3\p@
\let\@listi\@listI\let\bit\tenbit}
%
\newcommand{\small}{%
\ifdraft
   \@setfontsize\small\@ixpt{22}%
\else
   \@setfontsize\small\@ixpt{12}%
\fi
\abovedisplayskip 8.5\p@ plus3\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus2\p@
\belowdisplayshortskip 4\p@ plus2\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 4\p@ plus2\p@ minus2\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}\let\bit\ninebit}
%
\newcommand{\footnotesize}{%
\ifdraft
   \@setfontsize\footnotesize\@viiipt{22}%
\else
   \@setfontsize\footnotesize\@viiipt{9.5}%
\fi
\abovedisplayskip 6\p@ plus2\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus\p@
\belowdisplayshortskip 3\p@ plus\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 3\p@ plus\p@ minus\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}
%
\newcommand{\scriptsize}{\@setfontsize\scriptsize\@viipt\@viiipt}
\newcommand{\tiny}{\@setfontsize\tiny\@vpt\@vipt}
\newcommand{\medium}{\@setfontsize\medium\@xipt{12pt}}
\newcommand{\large}{\@setfontsize\large\@xiipt{14}}
\newcommand{\Large}{\@setfontsize\Large\@xivpt{18}}
\newcommand{\LARGE}{\@setfontsize\LARGE\@xviipt{22}}
\newcommand{\huge}{\@setfontsize\huge\@xxpt{25}}
\def\HUGE{\titlemath}
%
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
%
\else %% LaTeX2.09
\def\@normalsize{\@setsize\normalsize{13pt}\xpt\@xpt
\ifdraft\baselineskip22pt\fi
\scriptfont\bffam\sevbf \scriptscriptfont\bffam\fivbf %%<<==== makes
                               % \bf change sizes in sub and superscript
\abovedisplayskip 12pt plus1pt minus4\p@
\belowdisplayskip=\abovedisplayskip
\abovedisplayshortskip 6pt plus3\p@
\belowdisplayshortskip 6pt plus3\p@ minus3\p@
\let\@listi\@listI\let\bit\tenbit}

\def\small{\@setsize\small{12pt}\ixpt\@ixpt
\ifdraft\baselineskip22pt\fi
\abovedisplayskip 8.5\p@ plus3\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus2\p@
\belowdisplayshortskip 4\p@ plus2\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 4\p@ plus2\p@ minus2\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}\let\bit\ninebit}
%
\def\footnotesize{\@setsize\footnotesize{9.5pt}\viiipt\@viiipt
\ifdraft\baselineskip22pt\fi
\abovedisplayskip 6\p@ plus2\p@ minus4\p@
\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus\p@
\belowdisplayshortskip 3\p@ plus\p@ minus2\p@
\def\@listi{\leftmargin\leftmargini
\topsep 3\p@ plus\p@ minus\p@\parsep 2\p@ plus\p@ minus\p@
\itemsep \parsep}}

\def\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt
\ifdraft\baselineskip22pt\fi}
\def\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
\def\medium{\@setsize\medium{12pt}\xipt\@xipt}
\def\large{\@setsize\large{14pt}\xiipt\@xiipt}
\def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
\def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
\def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
\def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
\def\HUGE{\@setsize\HUGE{25pt}\xxxpt\@xxxpt}
\fi

%%%%%%%%%%
%% 3) Setting dimensions and parameters

\textheight = 43pc
\textwidth = 27pc

\parskip 0pt %plus .1pt
\parindent 10pt
\newdimen\saveparindent
\saveparindent\parindent

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

\dimen0=.75in
\advance\dimen0 by 1.5pc

\hoffset=\dimen0
\topmargin 0in
  \headheight 9pt
  \headsep   21pt
  \topskip = 8.5pt
  \footskip 14pt

\lineskip 1pt
\normallineskip 1pt
\def\baselinestretch{1}

\oddsidemargin0in
\evensidemargin0in

\marginparwidth .75in
\marginparsep 7pt

\columnsep 10pt
\columnseprule 0pt

\descriptionmargin=32pt

\footnotesep6pt
\skip\footins  12pt plus .5pt minus 2pt

\floatsep 12pt plus 2pt minus 2pt
\textfloatsep 20pt plus 2pt minus 4pt
\intextsep 12pt plus 2pt minus 2pt

%% might be redundant?
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

\def\headlinesize{\small}

\footlinefoliofalse
\footlinehrulefalse

\newif\ifnoheadline
\def\jheadline{\hbox to\textwidth{\iftitle%
\iftypesetters
\ifnoheadline\global\noheadlinefalse
\else
\vtop to0pt{\vss\footnotesize
\noindent{\titleheadsc\thejourname} {\bf\thevolume},
\the\c@page--%
\expandafter\ifx\csname endpage\the\currarticlenumber\endcsname\relax ??
\else\expandafter\csname endpage\the\currarticlenumber\endcsname\fi 
\ (\theyear)\newline
{\titleheadsc\uppercase{Article no.}\ }\thearticlenumber\vskip4pt\vss}%
\fi\else\hfill\fi
\else\ifodd\c@page
{\hfill\headlinesize\headtextfont\thetitle}%
\hfill\llap{\foliofont\the\c@page}
\else\rlap{\foliofont\the\c@page}\hfill%
{\headlinesize\headtextfont\theauthors}\hfill\fi\fi}}


\def\ccclineinfo{\iftypesetters
 \footlinefont
 \parindent=0pt
 \ifnoapline
 \else
 \ifodd\c@page
 {\ifll\else\hbadness10000\fi
 \vtop to0pt{\scriptsize\cccfont\baselineskip8pt 
 \ifll\else\vskip12pt\fi
 \noindent\hfill{\footnotesize \thecccline
 \vskip2pt}
 \noindent\hfill Copyright \smcopyright~\theyear\ by Academic Press%
 \vskip1sp
 \noindent\hfill All rights of reproduction in any form reserved.
 \vskip1sp\vss}\vskip1sp}
 \else
 \vtop to0pt{\cccfont\baselineskip8pt
 \noindent{\footnotesize \thecccline
  \vskip2pt}
  \noindent Copyright \smcopyright~\theyear\ by Academic Press%
  \vskip1sp
  \noindent All rights of reproduction in any form reserved.
  \vskip1sp
  \vss}
\fi\fi
\fi}

\def\jfootline{\vtop to0pt{\iftitle\global\titlefalse
\hbox to\textwidth{\hfill\foliofootfont\the\c@page\hfill}\else\hfill\fi
\ccclineinfo
\vss}}

%%%%%%
%% 5) Article title page dimensions and parameters

\CenterArticleHeadtrue

\long\def\author#1{%
\vskip2pt
\hbox to\textwidth{\hss\let\thanks\authorthanks
\let\\ =\ %
\frenchspacing\authorfont#1\vrule width0pt depth 
\belowauthorskip\hss}
\setbox1=\hbox{\let\thanks\maketemp #1}}

\let\authors\author

\newdimen\savetextheight
\def\changetextheight{\global\savetextheight\textheight
\global\advance\textheight by-4pc
}


\def\authorsize{\Large}

\def\specialtitleins{\hsize=\textwidth
\raggedright}


\abovetitleskip=4pc
\belowtitleskip=12pt
\abovesubtitleskip=0pt
\belowsubtitleskip=6pt
\aboveauthorskip=0pt
\belowauthorskip=12pt
\aboveaffilskip=0pt
\belowaffilskip=8pt

%% ABSTRACT


%% If ABSTRACT should be printed
\abstractnametrue

\aboveabstractskip=36pt
\belowabstractskip=0pt
\belowabstractnameskip=12pt

\abstractmargin=18pt

%% Keywords

\def\keywordssize{\small}
\abovekeywordsskip=16pt


%%%%%%
%% 6) Section Heads


\def\thesection       {\arabic{section}}
\def\thesubsection    {\thesection.\arabic{subsection}.}
\def\thesubsubsection {\thesubsection\arabic{subsubsection}.}

\setcounter{secnumdepth}{2}

% \@startsection {NAME}{LEVEL}{INDENT}{BEFORESKIP}{AFTERSKIP}{STYLE} 
%            optional * [ALTHEADING]{HEADING}
%    Generic command to start a section.  
%    NAME       : e.g., 'subsection'
%    LEVEL      : a number, denoting depth of section -- e.g., chapter=1,
%                 section = 2, etc.  A section number will be printed if
%                 and only if LEVEL < or = the value of the secnumdepth
%                 counter.
%    INDENT     : Indentation of heading from left margin
%    BEFORESKIP : Absolute value = skip to leave above the heading.  
%                 If negative, then paragraph indent of text following 
%                 heading is suppressed.
%    AFTERSKIP  : if positive, then skip to leave below heading,
%                       else - skip to leave to right of run-in heading.
%    STYLE      : commands to set style
%  If '*' missing, then increments the counter.  If it is present, then
%  there should be no [ALTHEADING] argument.  A sectioning command
%  is normally defined to \@startsection + its first six arguments.

\def\xmakecenterlines#1{\vbox{%
\parindent=0pt
\parskip=0pt
\hyphenpenalty=10000
\centering
#1
}}


\sectionwrap=20pc

\newskip\sectskip
\sectskip=20pt %plus2pt minus 6pt

\newskip\belowsectionskip
\belowsectionskip=10pt

\newskip\subsectskip
\subsectskip=8pt %plus2pt minus 1pt

\newskip\subsubsectskip
\subsubsectskip=10pt %plus2pt minus 1pt

\def\ast{*}
\newif\ifsectionon
\def\section#1{\def\yone{#1}\ifx\yone\ast\let\go\ssection\else%
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
\the\c@section.\string\ \string\ {\string\affilfont\space #1.}}}\fi
\xmakecenterlines{\hyphenpenalty=10000 
\ifdraft \baselineskip=22pt\else \baselineskip=13pt\fi
\sectiontitlemath %% makes 10pt bold math
%\everymath{\zmit}
\normalsize\sectionfont\ifappendon\applett.\fi\thesection.\nobreak\hskip4pt%+++
\uppercase{#1\vrule width0pt depth\belowsectionskip}}%
\nobreak\global\everymath={}%
\everypar={\global\sectiononfalse\everypar={}}\ignorespaces}

\def\ssection#1#2{\vskip\sectskip\global\sectionontrue%
\xmakecenterlines{\hyphenpenalty=10000 
\ifdraft\baselineskip=22pt\else\baselineskip=13pt\fi
\sectiontitlemath %% makes 10pt bold math
%\everymath{\zmit}
\sectionfont\uppercase{#2}}%
\nobreak\vskip4pt\everypar={\global\sectiononfalse\everypar={}}}


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
%\everymath{\zmit}
\sectiontitlemath %% 10pt boldmath
\ifdraft\baselineskip22pt\else\baselineskip=13pt\fi
\subsectionfont\ifappendon\applett.\fi\thesubsection\hskip1em#1}%
\global\everymath={}%
\everypar={\global\subsectiononfalse\everypar={}}
\nobreak\vskip2pt}

\def\ssubsection#1#2{\ifsectionon\global\sectiononfalse
\ifvmode\vskip-3pt\else\vskip\subsectskip\fi\else
\vskip\subsectskip\fi\global\subsectionontrue
\refstepcounter{subsection}%
\xmakecenterlines{\hyphenpenalty=10000 
\sectiontitlemath %% 10pt boldmath
\ifdraft\baselineskip22pt\else\baselineskip=13pt\fi\subsectionfont#2}%
\everypar={\global\subsectiononfalse\everypar={}}
\nobreak\vskip2pt}

\def\subsubsection#1{\def\one{#1}\ifx\one\ast\let\go\ssubsubsection\else%
\let\go\xxsubsubsection\fi\go{#1}
}

\def\xxsubsubsection#1{%
\refstepcounter{subsubsection}%
\def\@currentlabel{\ifappendon\applett.\fi\the\c@section.\the\c@subsection.\the\c@subsubsection}
\ifsectionon\global\sectiononfalse\ifvmode\vskip-3pt\else%
\vskip\subsubsectskip\fi\else%
\ifsubsectionon\global\subsectiononfalse
\ifvmode\vskip-3pt\else%
\vskip\subsubsectskip\fi\else\vskip\subsubsectskip\fi\fi%
\vtop{\def\\ {\vskip1sp}
\noindent{\subsubsectionfont\ifappendon\applett.\fi\thesubsubsection\hskip1em%
#1}}\global\everymath={}%
\normalsize
\ifdraft \baselineskip=22pt\else \baselineskip=13pt\fi
\vskip4pt\nobreak}

\def\ssubsubsection#1#2{%
\refstepcounter{subsubsection}%
\ifsectionon\global\sectiononfalse\ifvmode\vskip-3pt\else
\vskip\subsubsectskip\fi\else%
\ifsubsectionon\global\subsectiononfalse\ifvmode\vskip-3pt 
\else\vskip\subsubsectskip\fi\else\vskip\subsubsectskip\fi\fi%
\noindent\hskip1em{\subsubsectionfont#2}
\ifdraft \baselineskip=22pt\else \baselineskip=13pt\fi
\vskip4pt}

\def\paragraph#1{\vskip6pt
{\footnotesize\paragraphfont #1.\hskip1em\relax}%
\ifdraft \baselineskip=22pt\else \baselineskip=13pt\fi
\ignorespaces}


%%%%%%
%% 7) Split math

%% Set this true when the equation numbers in split math will
%% be vertically centered. Otherwise the equation number will
%% be at the right side of the second line.

%%\CenterSplitMathEqnotrue



%%%%%%
%% 8) Math, figure, table, theorem counters.

\def\figurename{FIG.}
\def\thefigure{\@arabic\c@figure\theletter}

\def\tablename{TABLE}
\def\thetable{\@arabic\c@table\theletter}

%\@addtoreset{equation}{section}  % Makes \section reset 'equation' counter.

\def\theequation{\ifappendon\applett.\fi\arabic{equation}\themathletter}
\let\savetheequation\theequation

%%%%%%
%% 9) Theorem environments

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
\noindent\hbox to\textwidth{\vbox{\parindent=0pt\unhbox0 \qed\hss}\hss}
\else
\vskip-8pt
\noindent
\vrule height\slugheight depth\slugdepth width\slugwidth\relax%
\fi\vskip\belowtheoremskip}

%% Writing the command \MakePeriodAfterTheorem in the file
%% will make `Theorem 2. xxxx' rather than `Theorem 2 xxxx'

\MakePeriodAfterTheorem

% For square boxes for Q.E.D. symbol

\newdimen\slugwidth
\newdimen\slugheight
\newdimen\slugdepth

\slugwidth=3pt
\slugheight=7pt
\slugdepth=2pt

\newif\ifopenbox
\newif\ifalwaysopenbox
\def\alwaysopenbox{\global\openboxtrue\global\alwaysopenboxtrue}
\def\openbox{\global\openboxtrue}


\def\qed{\ifopenbox\ifalwaysopenbox\else\global\openboxfalse\fi
\unskip\nobreak~~~{%
\vbox{\hrule\hbox{\vrule height\slugheight depth\slugdepth%
\hskip\slugwidth\vrule}\hrule}}%
\else%
\unskip\nobreak~~~\hbox{\vrule height\slugheight depth\slugdepth
width \slugwidth}\fi}

\let\slug\qed
\let\filledbox\slug

\def\xendtrivlist{\@endtheorem}

\def\@thmcountersep{.}

\def\@begintheorem#1#2{\trivlist 
\labelsep=0pt
\item[\hskip\parindent\theoremfont#1\ \/\/%
\ifthmsectionnum\ifappendon\applett.\fi\thesection.\fi#2%
\ifperiodaftertheorem\@thmcountersep\fi]%
\def\@currentlabel{\ifthmsectionnum\ifappendon\applett.\fi\thesection.\fi#2}%
\enskip\ \theoremtextfont}

\def\@opargbegintheorem#1#2#3{\trivlist
\labelsep=0pt
   \item[\hskip\parindent\theoremfont #1\ \/\/%
\ifthmsectionnum\ifappendon\applett.\fi\thesection.\fi#2\ \ {\rm(#3).} ]%
\def\@currentlabel{\ifthmsectionnum\ifappendon\applett.\fi\thesection.\fi#2}%
\enskip\ \theoremtextfont}


%% use section numbers with theorem
\thmsectionnumtrue

\newtheorem{lemma}{Lemma}
\newtheorem{proposition}{Proposition}
\newtheorem{property}{Property}
\newtheorem{corollary}{Corollary}

\def\case#1{\vskip12pt\indent{Case \uppercase{#1:}}\enskip}
\let\endcase\xendtrivlist

\def\result{\TheoremTypeEnv{\it Result}}
\let\endresult\xendtrivlist

\newcounter{exampnum}[section]
\def\example{\global\advance\c@exampnum by1
\TheoremTypeEnv{\theoremfont Example 
\ifappendon\applett.\fi\thesection.\the\c@exampnum}
\def\@currentlabel{\ifthmsectionnum\ifappendon\applett.\fi\thesection.%
\fi\the\c@exampnum}%
\rm}
\let\endexample\xendtrivlist
\@addtoreset{exampnum}{section}

\newcounter{xalgo}[section]
\def\algo{\global\advance\c@xalgo by1
\TheoremTypeEnv{\theoremfont Algorithm 
\ifappendon\applett.\fi\thesection.\the\c@xalgo}
\def\@currentlabel{\ifthmsectionnum\ifappendon\applett.\fi\thesection.%
\fi\the\c@xalgo}%
\rm}
\let\endalgo\xendtrivlist
\@addtoreset{xalgo}{section}

\newcounter{defnum}[section]
\def\definition{\global\advance\c@defnum by1
\TheoremTypeEnv{\theoremfont Definition 
\ifappendon\applett.\fi\thesection.\the\c@defnum}
\def\@currentlabel{%
\ifthmsectionnum\ifappendon\applett.\fi\thesection.\fi\the\c@defnum}%
\rm}
\let\enddefinition\xendtrivlist
\@addtoreset{defnum}{section}

\newcounter{connum}[section]
\def\conjecture{\global\advance\c@connum by1
\TheoremTypeEnv{\it Conjecture \ifappendon\applett.\fi\thesection.%
\the\c@connum}\rm%
\def\@currentlabel{\ifappendon\applett.\fi\thesection.\the\c@connum}}
\def\endconjecture{\xendtrivlist}
\@addtoreset{connum}{section}

\newcounter{remarknum}[section]
\def\remark{\global\advance\c@remarknum by1
\TheoremTypeEnv{\it Remark \ifappendon\applett.\fi\thesection.%
\the\c@remarknum}%
\def\@currentlabel{\ifappendon\applett.\fi\thesection.\the\c@remarknum}%
\rm}
\let\endremark\xendtrivlist
\@addtoreset{remarknum}{section}

\newcounter{claimnum}[section]
\def\claim{\global\advance\c@claimnum by1
\TheoremTypeEnv{\it Claim \ifappendon\applett.\fi\thesection.%
\the\c@claimnum}%
\def\@currentlabel{\ifappendon\applett.\fi\thesection.\the\c@claimnum}%
\rm}
\let\endclaim\xendtrivlist
\@addtoreset{claimnum}{section}



%%%%%%
%% 10) Captions

\centermultiplelinestrue
\centersinglelinetrue
\centersingletablinetrue

\let\captionsize\footnotesize
\abovefigcaptionskip=2pt
\abovetabcaptionskip=1pt

\abovetableskip=2pt

%%%%%%
%% 11) Table settings

\abovetabularskip=12pt
\belowtabularskip=12pt

\belowtabcaptionskip=3pt
\let\tablenotesize\footnotesize
\let\tabletextsize\small
\fullwidthtablefalse
\lineabovetabcaptionfalse
\linebelowtabcaptionfalse

%%%%%%
%% 12) Lists
%% Listing defaults:

\def\labelenumi{\normalsize\theenumi}
\def\theenumi{\arabic{enumi}.}

\def\labelenumii{\normalsize\theenumii}
\def\theenumii{\hss(\roman{enumii})}
\def\p@enumii{\theenumii}

\def\labelenumiii{\normalsize\theenumiii}
\def\theenumiii{\alph{enumiii}.}
\def\p@enumiii{\theenumiii}

\def\labelenumiv{\normalsize\theenumiv}
\def\theenumiv{\Alph{enumiv}}
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
\ifdim\leftmargin=0pt \hspace\saveparindent 
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
%% 13) Footnotes

\def\footnoterule{\iftitle\else\kern -3\p@ \hrule 
width 3pc %%<=== change this dimen to change width of footnote rule line
\kern 2.6\p@\fi}

\long\def\@makefntext#1{\noindent\hskip1em$\m@th^{\@thefnmark}$\footnotefont#1}

%% End notes, if we want them.
% \newcount\footnum
% \newcount\savefootnum
% \let\footnote\endnote
% \let\footnotetext\endnotetext


%%%%%%%%%
%% 14) Appendices

\def\appendixname{APPENDIX}

%%%%%%%%%
%% 15) References
\def\referencesize{\footnotesize}

\def\references{\savesection*{References}
\referencesize\itemsep=2pt
\list{\@biblabel{\arabic{enumiv}}}
{\settowidth\labelwidth{\@biblabel{10.}}%
    \leftmargin\labelwidth
\labelsep=4pt
%    \advance\leftmargin\labelsep
    \usecounter{enumiv}%
    \let\p@enumiv\@empty
    \def\theenumiv{\arabic{enumiv}}}%
    \def\newblock{\hskip .11em plus.33em minus.07em}%
    \sloppy\clubpenalty4000\widowpenalty4000 \frenchspacing
    \sfcode`\.=\@m}

%% same as references, but with unused #1 in case \thebibliography{1} is used.
\def\bibreferences#1{\savesection*{References}
\referencesize\itemsep=2pt
\list{\@biblabel{\arabic{enumiv}}}
{\settowidth\labelwidth{\@biblabel{10.}}%
    \leftmargin\labelwidth
\labelsep=4pt
%    \advance\leftmargin\labelsep
    \usecounter{enumiv}%
    \let\p@enumiv\@empty
    \def\theenumiv{\arabic{enumiv}}}%
    \def\newblock{\hskip .11em plus.33em minus.07em}%
    \sloppy\clubpenalty4000\widowpenalty4000 \frenchspacing
    \sfcode`\.=\@m}

\def\endreferences{\endlist}

\let\thebibliography\bibreferences
\let\endthebibliography\endreferences

%%%%%%%%%
%% 16) Received lines

\abovereceivedskip=0pt
\aboverevisedskip=1sp
\aboveacceptedskip=1sp

%%%%%%%%%



%%%%%%%%%

\ifll\normalsize\else
\normalsize\@normalsize\fi

%%----------------------------------------
\newif\ifturnoffskip

%% \refstepcounter{chapter} 
%% will reset footnote num, section, theorem, table and figure

\newif\ifnoapline
\global\noaplinetrue

\def\article{\global\appendonfalse
\gdef\applett{}
\global\sendcontentsfalse
\global\c@equation=0
\global\algoline=0
\parskip=0pt
\refstepcounter{chapter}
%%% Use \thanks for title here; number instead of symbol
%% reset for next article
 \ifnum\titlethanksnum>0 \global\titlethanksnum=0 \fi
%%
 {\long\def\@makefntext##1{\ifnum\titlethanksnum=1
 \global\turnoffskiptrue
% \vskip-10pt
\fi
\parindent=1em
\footnotesize\footnotefont##1}
%%
 \loop\ifnum\titlethanksnum<\titlethankscounter
 \global\advance\titlethanksnum by1\relax
 \savefootnotetext{\csname temptitlethanks\the\titlethanksnum\endcsname}
 \repeat
 \global\titlethanksnum=0 \global\titlethankscounter=0}
 \ifnum\thanksnum>0 \global\thanksnum=0 \fi %<<==
 {\long\def\@makefntext##1{\ifnum\thanksnum=1
 \ifturnoffskip\global\turnoffskipfalse\else
 %\vskip-10pt
\fi\fi
\parindent=1em
\indent\raise.5ex\hbox to 3pt{\dothankssymbol\hss}
 \footnotesize\footnotefont##1}
 \loop\ifnum\thanksnum<\thankscounter
 \global\advance\thanksnum by1\relax
 \savefootnotetext{\csname tempthanks\the\thanksnum\endcsname}
 \repeat
 \global\thanksnum=0 \global\thankscounter=0}
%%% ok to here
%
\vskip12pt}

\def\endarticle{\write\@auxout{\string\expandafter%
\string\gdef\string\csname\space 
endpage\the\currarticlenumber\endcsname{\the\c@page}}
\vskip1pt
\newpage\resetdefaults}


\long\def\abstract#1{\ifx\thecommline\empty\else\thecommline\fi
\noindent\hbox to\textwidth{\receivedfont
\hfill\ifx\thereceived\empty\else\vrule height \abovereceivedskip
width 0pt\thereceived\fi
\ifx\therevised\empty\else\therevised\fi
\ifx\theaccepted\empty\else\theaccepted\fi\hfill}
\vskip1sp
\ifx\thededication\empty\else
\vskip-2pt
\xdedication\vskip2pt\fi
\hbox to\textwidth{\hfill\vtop{\hsize=24pc
\leftskip=0pt \rightskip\leftskip
\abstractfont
\indent\vrule height27pt width0pt #1\iftypesetters\quad {\abstractcopyrightfont%
\realcopyright~\theyear\ Academic Press}\fi
\vskip1sp}\hfill}\vskip2pt
}

\def\received#1{%
\def\thereceived{Received #1}}
\def\revised#1{\def\therevised{;\ \ revised #1}}
\def\accepted#1{%
\def\theaccepted{; accepted #1}}
\def\correspondence#1{\def\thecorrespondence{#1}}
\def\grantinfo#1{\def\thegrantinfo{Contract grant sponsor: #1}}
\def\journame#1{\def\thejourname{\uppercase{#1}}}
\def\articlenumber#1{\def\thearticlenumber{#1}}

\def\zerosection#1{\c@section=-1 \section{#1}}


\def\extract{\bgroup\leftskip16pt\rightskip16pt
\vskip6pt\footnotesize}
\def\endextract{\vskip6pt\egroup}

\let\quotation\extract
\let\endquotation\endextract

\ifll\else
\def\mathcal#1{{\protect\cal #1}}
\def\mathrm#1{{\protect\rm #1}}
\def\mathbf#1{{\protect\bf #1}}
\fi


\let\@noitemerr\relax

\newif\ifmtimes
\endinput

List of Changes:
February 25, 2000
Improvement to \eqnarray so that if it follows
an equation with \mathletter, and uses mathletter
itself, it still will advance its counter.

January 19, 2000
Change to xref on no numbered algorithms so that they
use the number of the algorithm rather than the line
number.

November 29, 1999
Made changes to theorem and example environments so that
they use A.1.1 for theorem in first section of appendix,
etc. Also changed appendix definition so that new letter
is used for each new appendix.

May 28, 1999
Made endproof put qed symbol at far left is proof ends
with an equation, but following text if proof ends with
text. (tricky!)

May 27, 1999
Made theorems and theorem type environments use appendix letter
instead of section number in appendixes.

Changed to avoid noindent after example type environments.
\def\@endtheorem{\if@newlist\@noitemerr\fi 
   \if@inlabel\indent\fi 
   \ifhmode\unskip \par\fi 
\everypar={} <===
\vskip\belowtheoremskip
\indent <=== 
}

May 14, 1999
Made math in title not be bold, so that they match
the title font more closely.

Got rid of titlemath and captiontitlemath because
they made boldmath malfunction when MathTimes fonts
were used.

Changed definition of \bbf so that it changes sizes
in sub and superscripts.

April 21, 1999
Fixed bug in label in definition, similar environments.

April 15, 1999
Fixed bug in captiontitlemath and titlemath to keep font set
from being redefined after the first time it is used. Changed
\sectiontitlemath to \boldmath since it is in 10pt. Substantially
improves speed of processing.

April 12, 1999
Made \bibreferences#1 which doesn't use the #1 in case user
supplies a .bbl file with \thebibliography{1} or other number.

Changed xref currentlabel in \@begintheorem and \@opargbegintheorem

Used \@addtoreset to start these items renumbering with
each new section: corollary, lemma, proposition and property.




