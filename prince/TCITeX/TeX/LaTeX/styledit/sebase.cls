% SEBASE.STY  
% Copyright 2004 MacKichan Software, Inc.
% NOTICE: This macro file is NOT proprietary and may be freely copied and distributed.

% Base macros for MacKichan Software Style Editor

\NeedsTeXFormat{LaTeX2e}[1997/12/01]
\ProvidesPackage{sebase}[2004/10/21]


\newif\ifse@usehyper
\DeclareOption{hyper}{\global\se@usehypertrue}
\ProcessOptions

\RequirePackage{theorem}
\RequirePackage{amssymb}
\RequirePackage{amsfonts}
\RequirePackage{amsmath}
\RequirePackage{afterpage}
\RequirePackage{multicol}


\ifx\pdfoutput\relax\let\pdfoutput=\undefined\fi
\newcount\se@pdfoutput
\newcount\c@hitemcount
\ifx\pdfoutput\undefined
\else
  \ifcase\pdfoutput
  \else
    \se@pdfoutput=1
    \RequirePackage[pdftex]{color}%
	\ifse@usehyper
	  \RequirePackage[debug]{hyperref}%
	  \c@tocdepth=4%
	  \def\unnumberedapp#1{Unnum\arabic{#1}}
	\fi
  \fi  
\fi

\catcode`\@=11\relax% @ letter

\newcounter{se@tempcnt}

\def\Def@PUniqueName{%
   \addtocounter{se@tempcnt}{1}
   \edef\@tempname{se@name@\arabic{se@tempcnt}}
   \DefPN{\@tempname}%
}


\def\multilimits@{\bgroup
  \Let@
  \restore@math@cr
  \default@tag
 \baselineskip\fontdimen10 \scriptfont\tw@
 \advance\baselineskip\fontdimen12 \scriptfont\tw@
 \lineskip\thr@@\fontdimen8 \scriptfont\thr@@
 \lineskiplimit\lineskip
 \vbox\bgroup\ialign\bgroup\hfil$\m@th\scriptstyle{##}$\hfil\crcr}

\def\Sb{_\multilimits@}%
\def\endSb{\crcr\egroup\egroup\egroup}%
\def\Sp{^\multilimits@}%
\let\endSp\endSb


\iftagsleft@
   \def\df@tag{\hbox to1sp{}\rlap{%
                  \normalfont\normalcolor
                  \hskip -\displaywidth\tagform@\p@equation}%
               }
\else
   \def\@eqnnum{{\normalfont\normalcolor \tagform@\p@equation}}%
\fi
\let\df@label\@empty

\def\endmathdisplay@a{%
  \if@eqnsw \gdef\df@tag{\@eqnnum}\fi
  \if@fleqn \@xp\endmathdisplay@fleqn
  \else \ifx\df@tag\@empty \else \veqno \alt@tag \df@tag \fi
    \ifx\df@label\@empty \else \@xp\ltx@label\@xp{\df@label}\fi
  \fi
  \ifnum\dspbrk@lvl>\m@ne
    \postdisplaypenalty -\@getpen\dspbrk@lvl
    \global\dspbrk@lvl\m@ne
  \fi
}


% Had to remove a reference to p@equation
\def\make@df@tag@@@#1{\gdef\df@tag{\tagform@{#1}%
  \toks@\@xp{{#1}}\edef\@currentlabel{\the\toks@}}}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEGeneral}
\let\date\relax
\def\protect{\noexpand\protect\noexpand}

%
% DEBUGGING MACROS
%

\newif\ifSEDebugging
\SEDebuggingfalse
%\SEDebuggingtrue

\ifSEDebugging
   \def\DEBUG#1{\expandafter\show\csname #1\endcsname}
   \def\DEBUGMSG#1{\typeout{DBG - #1}}
\else
   \def\DEBUG#1{\relax}
   \def\DEBUGMSG#1{\relax}
\fi

%
% Some Conditionals
%

\def\Not#1{TT\fi\csname if#1\endcsname\else}
\def\NotX#1{TT\fi\if #1\else}

\def\MathMode{TT\fi\ifmmode}

\def\EqualStrings#1#2{%
   TT\fi
   \edef\@tempa{#1}%
   \edef\@tempb{#2}%
   \ifx\@tempa\@tempb
}

\def\EmptyArg#1{%
   TT\fi
   \edef\@tempa{#1}%
   \edef\@tempb{}%
   \ifx\@tempa\@tempb
}


\def\Empty#1{%
   TT\fi
   \edef\@tempa{#1}%
   \edef\@tempb{}%
   \ifx\@tempa\@tempb
}


% #1 should be a control sequence. Determine if it is defined or not
\def\Undefined#1{%
   TT\fi
   \ifx#1\undefined
}


% #1 should be a name. Determine if the corresponding control
% sequence is defined or not
\def\UndefinedN#1{%
   TT\fi
   \expandafter\ifx\csname#1\endcsname\relax
}


%
% ERROR MESSAGES
%


\def\SEDECLAREERROR#1#2#3{%
    \if\Empty{#3}
       \expandafter\def\csname SEERR#1\endcsname{\errmessage{#2}}%
    \else
       \expandafter\def\csname SEERR#1\endcsname##1{\errmessage{#2}}%
    \fi
}


\def\SEWARNING#1{%
   \typeout{Style Editor Warning: #1}%
}

\def\SEDECLAREWARNING#1#2#3{%
    \if\Empty{#3}
       \expandafter\def\csname SEERR#1\endcsname{\SEWARNING{#2}}%
    \else
       \expandafter\def\csname SEERR#1\endcsname##1{\SEWARNING{#2}}%
    \fi
}


\SEDECLAREWARNING{REDEFINE}{Control sequence already defined:}{}%



\newtoks\@temptoksa
\newtoks\@temptoksb
\newtoks\@temptoksc

%
% DefN#1#2 - define name
%

\def\DefN#1{%
   \expandafter\def\csname #1\endcsname
}

\def\EDefN#1{%
    \expandafter\edef\csname #1\endcsname
}

\def\GDefN#1{%
    \expandafter\gdef\csname #1\endcsname
}

\def\XDefN#1{%
    \expandafter\xdef\csname #1\endcsname
}

\def\NameUse#1{%
    \csname#1\endcsname
}

\def\DefPN#1{%
   \EDefN{#1}{\expandafter \protect \csname p@#1\endcsname}%
   \DefN{p@#1}%... picks up next arg
}



\def\IDefPN#1{%
   \noexpand\DefPN{#1}
}

\def\EDefPN#1{%
   \EDefN{#1}{\expandafter \protect \csname p@#1\endcsname}
   \EDefN{p@#1}%... picks up next arg
}

% Define a protected counter name
\def\EDefPCN#1{%
   \EDefN{#1}{\expandafter \counter@protect \csname p@#1\endcsname}
   \EDefN{p@#1}%... picks up next arg
}

\def\counter@protect{\noexpand\counter@protect\noexpand}
\let\counter@@protect\counter@protect


\def\IDef{\def\noexpand}% inner define use in edef to redefine a cs
\def\INoBreak{\protect\nobreak}


% In the following macros \x@x@ is a macro used to 
% communicate an argument to \doAppend

\def\EAppDefPN#1{%
   \edef\assignee{\expandafter\noexpand\csname p@#1\endcsname}%
   \afterassignment\doAppend
   \edef\x@x@x%... pick up next arg
}


\def\AppDefPN#1{%
   \edef\assignee{\expandafter\noexpand\csname p@#1\endcsname}%
   \afterassignment\doAppend
   \def\x@x@x%... pick up next arg
}




\def\EPrependDefN#1{%
   \edef\assignee{\expandafter\noexpand\csname#1\endcsname}
   \afterassignment\doPrepend
   \edef\x@x@x%... pick up next arg
}

\def\EAppendDefN#1{%
   \edef\assignee{\expandafter\noexpand\csname#1\endcsname}
   \afterassignment\doAppend
   \edef\x@x@x%... pick up next arg
}



\def\doPrepend{%
   \@temptoksa=\expandafter{\x@x@x}
   \@temptoksb=\expandafter\expandafter\expandafter{\assignee}%
   \expandafter\edef\assignee{\the\@temptoksa\the\@temptoksb}%
}


\def\doAppend{%
   \@temptoksa=\expandafter{\x@x@x}
   \@temptoksb=\expandafter\expandafter\expandafter{\assignee}% 
   \expandafter\edef\assignee{\the\@temptoksb\the\@temptoksa}%
}




% There is no protect in this macro - assume #1 protects itself	
% if need be.
\def\CallName#1{\expandafter\csname#1\endcsname}


\def\Variable#1#2#3{
    \EDefN{Set#1}##1{\expandafter\def\expandafter\noexpand\csname @#1\endcsname{##1}}%
    \EDefN{Get#1}{\expandafter\protect\csname @#1\endcsname}%
    \if\UndefinedN{#1}%
    \else
      \SEERRREDEFINE
	  \typeout{\space\space\space #1}
    \fi
    \EDefN{#1}{\expandafter\protect\csname @#1\endcsname}%
    \edef\@temp{\expandafter\noexpand\csname Set#1\endcsname{#2}}%
    \@temp
}

\def\MakeParam#1{%
    \if\UndefinedN{the#1}%
       \EDefN{#1}##1{\expandafter\def\csname the#1\endcsname{##1}}%
    \else
       \SEERRREDEFINE
	   \typeout{\space\space\space the#1}
    \fi   
}


\def\MakeTF#1{%
   \DefN{TF@#1}{\csname if#1\endcsname T\else F\fi}
}

\def\MakeIf#1{%
    \expandafter\newif\csname if#1\endcsname
    \expandafter\def\csname #1\endcsname##1{\csname #1##1\endcsname}
    \MakeTF{#1}
}




%
% List processing macros
%
\def\@emptylist{}

\def\EmptyList#1{%
   TT\fi
   \ifx#1\@emptylist
}

\def\Car#1#2{%
   \def\@Cartemp \<##1##2\@endcar{\def#2{##1}}
   \expandafter\@Cartemp #1\@endcar
}

\def\Cdr#1#2{%
   \def\@Cdrtemp \<##1##2\@endcdr{\def#2{##2}}
   \expandafter\@Cdrtemp #1\@endcdr
}

\def\RemoveFront#1{%
   \if\EmptyList{#1}
   \else
      \def\@removefront \<##1##2\@endcdr{\gdef#1{##2}}%
      \expandafter\@removefront #1\@endcdr
   \fi
}


\def\ForEach#1#2{%
   \let\@fetemp=#1%
   \let\se@macro=#2%
   \@foreach
}

\def\@foreach{%
   \if\EmptyList\@fetemp
      \let\se@next\relax
   \else
      \Car{\@fetemp}{\se@element}%
      \se@macro{\se@element}%
      \RemoveFront{\@fetemp}%
      \let\se@next\@foreach
   \fi
   \se@next
}


\newtoks\@aeelt
\def\AssignElement#1{\@aeelt\expandafter{#1}}


\def\LastOf#1#2{%
   \bgroup
      \ForEach{#1}{\AssignElement}%
      \xdef#2{\the\@aeelt}%
   \egroup
}

\def\AppendRear#1#2{%
   \edef\@artemp{\noexpand\<{#2}}%
   \@temptoksa=\expandafter{\@artemp}%
   \@temptoksb=\expandafter{#1}%
   \xdef#1{\the\@temptoksb \the\@temptoksa}%
}

%
% Ubiquitous parameters
%

\MakeParam{Name}
\MakeParam{Mode}

% 
% Spacing macros
%
\DefPN{Space}#1{%
   \ifHorizontalSpace
       \hspace{#1}%
   \else
       \vspace{#1}%
   \fi
}

%\EDefN{#1}{\expandafter \space@protect \csname p@#1\endcsname}%
%\def\space@protect {\noexpand\space@protect\noexpand



\MakeIf{HorizontalSpace}

\DefPN{SpaceVertically}{\HorizontalSpacefalse\ifvmode\else\par\fi}
\DefPN{SpaceHorizontally}{\HorizontalSpacetrue}
\SpaceHorizontally

%

\def\thepage{\arabic{page}}


% These macros handle everypar events.  Right now there is just one - suppress indent
% but later there may be others. Follow same pattern. 


\def\ResetEveryPar{\gdef\EveryParIndent{}}

\def\InitEveryPar{%
   \everypar = {%
        \EveryParIndent
        \ResetEveryPar}%
}
 

\DefPN{SuppressNextIndent}{%
    \InitEveryPar
    \gdef\EveryParIndent{\hskip -\parindent}%
}

% Page switching macros

\DefPN{StartCurrentPage}{}
\DefPN{StartNextPage}{\clearpage}
\DefPN{StartNextOddPage}{%
   \clearemptydoublepage
}



\def\ToksAppendLeft#1#2{%
   \expandafter#1\expandafter{\expandafter#2\the#1}%
}

\def\ToksAppendRight#1#2{%
   \expandafter#1\expandafter{\the#1#2}%
}

%
% Append To Macro Definition
%

\long\def\DefAppendRight#1#2{%
    \if\Undefined{#1}
       \let#1\@empty
    \fi
    \@temptoksa=\expandafter{#1}%
    \@temptoksb={#2}%
    \edef#1{\the\@temptoksa\the\@temptoksb}%
}

\long\def\XDefAppendRight#1#2{%
    \if\Undefined{#1}
       \let#1\@empty
    \fi
    \@temptoksa=\expandafter{#1}%
    \@temptoksb={#2}%
    \xdef#1{\the\@temptoksa\the\@temptoksb}%
}

\long\def\DefAppendLeft#1#2{%
    \if\Undefined{#1}
       \let#1\@empty
    \fi
    \@temptoksa=\expandafter{#1}%
    \@temptoksb={#2}%
 	\edef#1{\the\@temptoksb\the\@temptoksa}%
}


\let\se@footnotetext\footnotetext

\DefPN{SaveFootnotes}{%
   \def\\{\par}
   \xdef\fnotes{\relax}%
   \def\footnote##1{\footnotemark\XDefAppendRight\fnotes{\footnotetext{##1}}}
   \def\thanks##1{\footnotemark\XDefAppendRight\fnotes{\footnotetext{##1}}}%
   \def\footnotetext{\protect\se@footnotetext}%
   \aftergroup\fnotes
}

\def\IAddVSpace#1{%
    \noexpand\ifvmode\noexpand\else\noexpand\par\noexpand\fi
    \protect\addvspace{#1}%
}

\def\IAddPenalty#1{\noexpand\addpenalty{#1}}


\renewcommand\footnoterule{%
   \kern-3\p@
   \hrule\@width.4\columnwidth
   \kern2.6\p@}

\newcommand\@makefntext[1]{%
   \parindent 1em%
   \noindent
   \hb@xt@1.8em{\hss\@makefnmark}#1}



%\catcode`\@=12\relax % at other


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEFonts}

\MakeParam{Family}
\MakeParam{Shape}
\MakeParam{Series}
\MakeParam{Size}



\def\FontNFSS#1{#1\DefineFont}

\def\DefineFont{%
      \EDefPN{\theName}{}%
      
      \if\NotX{\EqualStrings{\theFamily}{Inherit}}
         \EAppDefPN{\theName}{\noexpand\fontencoding{\csname \theFamily @enc\endcsname}}%
         \EAppDefPN{\theName}{\noexpand\fontfamily{\theFamily}}%
      \fi
      
      \if\NotX{\EqualStrings{\theSeries}{Inherit}}
         \EAppDefPN{\theName}{\noexpand\fontseries{\theSeries}}%
      \fi
      
      \if\NotX{\EqualStrings{\theShape}{Inherit}}
         \EAppDefPN{\theName}{\noexpand\fontshape{\theShape}}%
      \fi
      
      \if\NotX{\EqualStrings{\theSize}{Inherit}}
         \EAppDefPN{\theName}{%
             \noexpand\fontsize{\csname \theSize @fsize\endcsname}%
                               {\csname \theSize @lead\endcsname}}%
      \fi
      
      \EAppDefPN{\theName}{\noexpand\selectfont}
      
      %\DEBUG{\theName}
      %\DEBUG{p@\theName}
}


\def\SwitchFont#1{%
   \CallName{#1}%
}

\def\SwitchParagraph#1{%
   \CallName{#1}%
}



%
% Some default settings
%

\def\tiny{\fontsize{5}{6}\selectfont}
\def\footnotesize{\fontsize{8}{9}\selectfont}
\def\scriptsize{\fontsize{7}{8}\selectfont}
\def\small{\fontsize{9}{11}\selectfont}
\def\normalsize{\fontsize{10}{12}\selectfont}
\def\large{\fontsize{12}{14}\selectfont}
\def\Large{\fontsize{14}{18}\selectfont}
\def\LARGE{\fontsize{17}{22}\selectfont}
\def\huge{\fontsize{20}{25}\selectfont}
\def\Huge{\fontsize{30}{35}\selectfont}

\def\itdefault{italic}
\def\bfdefault{bold}
\def\sldefault{slant}
\def\scdefault{smallcaps}
\def\sfdefault{SansSerif}
\def\ttdefault{Typewriter}
\def\rmdefault{Serif}
\def\seriesdefault{medium}
\def\shapedefault{upright}


\def\normalsize{\@setfontsize\normalsize\@xpt\@xiipt}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEColor}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                %
%        Color switching macros                  %
%                                                %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Color}

\def\CurrentColor{R:0 G:0 B:0}

\def\RestoreColor{%
   \expandafter\MakeSpecial\CurrentColor XXX%
}

\DefPN{SwitchColor}#1{%
     \def\CurrentColor{#1}%
     \aftergroup\RestoreColor
     \expandafter\MakeSpecial #1XXX%
}


\def\MakeSpecial R:#1 G:#2 B:#3XXX{%    
    \ifcase\se@pdfoutput
       \special{textcolor: #1 #2 #3}%
	\else
	   \color[RGB]{#1,#2,#3}%
	\fi
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SECounter}
%
\newcounter{Document}

\MakeParam{Appearance}
\MakeParam{RefPrefix}
\MakeParam{RefFont}
\MakeParam{ResetOn}
\MakeParam{InitialValue}

\def\@MakeNewCounter#1#2#3{
   \if\Empty{#2}
      \if\UndefinedN{c@#1}
         \newcounter{#1}
         \setcounter{#1}{#3}
         \EDefN{ini@#1}{#3}
      \fi
   \else 
      \if\UndefinedN{c@#1}
         \newcounter{#1}[#2]
         \setcounter{#1}{#3}
         \EDefN{ini@#1}{#3}
      \else
         \@addtoreset{#1}{#2}
         \setcounter{#1}{#3}
         \EDefN{ini@#1}{#3}
      \fi
   \fi
}

\def\MakeNewCounter#1#2#3{%
   % expand the arguments first, then do @makenewcounter
   \edef\doit{\noexpand\@MakeNewCounter{#1}{#2}{#3}}
   \doit
}


% Note - because of LaTeX tradition I don't use p@ to 
% prefix a protected form of the counter name (as I
% do with many macros). Instead p@counter is the reference
% prefix.

\def\DefineCounter{
    \MakeNewCounter{\theName}{\theResetOn}{\theInitialValue}
    \EDefPCN{the\theName}{\noexpand\CallName{\theAppearance}{\theName}}
    
    % This is a style editor thing - if foo is a counter then 
    % \foo is the same as \thefoo. 
    \EDefN{\theName}{\CallName{the\theName}}

    \@temptoksa=\expandafter{\theRefPrefix}
    \EDefN{p@\theName}{{\CallName{\theRefFont}\the\@temptoksa}}
	\EDefN{theH\theName}{\the\@temptoksa}
 
    %\DEBUG{\theName}
    %\DEBUG{p@\theName}
    %\DEBUG{the\theName}
}

\def\Counter#1{#1\DefineCounter}



% \refstepcounter is redefined because I don't want to 
% use the form {\csname p@#1\endcsname\csname the#1\endcsname}
% for the label. Instead I let p@counter be a more general thing that
% contains the entire label.
\def\refstepcounter#1{%
    \stepcounter{#1}%
	{%
	   \def\protect{\noexpand\protect\noexpand}
	   %\let\qprotect\relax
	   \def\qprotect{}
	   \def\counter@protect{}
       \xdef\@currentlabel{\csname p@#1\endcsname}%
    }%
}

\def\hrefstepcounter#1{%
    \stepcounter{#1}%
	{%
	   \def\protect{\noexpand\protect\noexpand}
	   %\let\qprotect\relax
	   \def\qprotect{}
	   \def\counter@protect{}
       \xdef\@currentlabel{\csname p@#1\endcsname}%
    }%
	\ifcase\se@pdfoutput
    \else
	   \ifse@usehyper
	      \def\counter@protect##1{##1}%
		  \def\qprotect{}%
	      \edef\@setempa{
	        \noexpand\hyper@refstepcounter{#1}%
	      }%
	      \@setempa
	   \fi
	\fi
}


% Here I change the hook that clears the reset counters
% when a counter increments. The reason is that the style
% editor allows one to specify an initial value that has
% to be taken into account here. 
\def\stepcounter#1{%
  \addtocounter{#1}\@ne
  \begingroup% do the reset list
     \let\@elt\se@stpelt
     \csname cl@#1\endcsname
  \endgroup
}

\def\se@stpelt#1{%
    \global\csname c@#1\endcsname \csname ini@#1\endcsname
}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEVariables}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Document Variables						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   \DocumentVariable{
%      \Name{Author}
%      \Mode{0}
%      \Default{A. U. Thor}
%      \Frontmatter{true}
%      \Leadin{Author:}
%   }

\def\DocumentVariable#1{#1\DefineDocVar}

\MakeIf{Frontmatter} 
\MakeParam{Default}
\MakeParam{Leadin}

\def\DefineDocVar{%
 	\ifFrontmatter
        \Variable{\theName}{\theDefault}{}   
    \else
        % Newfield case
        %\edef\DoNewField{\noexpand\newfield{NF\theName}{\theLeadin}}
        %\DoNewField
        \EDefN{@setempa}{NF\theName}%
        \expandafter\newfield\expandafter{\@setempa}{\theLeadin}%
        \EDefN{@do@NF\theName}{\relax\DefPN{\theName}{\expandafter\noexpand\csname the@NF\theName\endcsname}}
        \EDefN{\theName}{%
           \noexpand\noexpand\CallName{the@NF\theName}}
    \fi
}
 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Style Variables  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\MakeParam{Value}

\def\DefineStyleVar{%
    \EDefN{\theName}{\theValue}
    %\DEBUG{\theName}
} 

\def\StyleVariable#1{#1\DefineStyleVar}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SESimple}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Rules			  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Height}
\MakeParam{Length}
\MakeParam{Shift}

\def\Rule#1{#1\DefineRule}


\def\DefineRule{%
   \EDefPN{\theName}{%
        \bgroup
           \SwitchColor{\theColor}%
           \noexpand\rule[\theShift]{\theLength}{\theHeight}
        \egroup
   }
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Inline Elements	  						%
%											    %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Font}
\MakeParam{Components}
\MakeParam{PrintColor}
\MakeIf{FixedWidth}
\MakeParam{Width}
\MakeIf{RightJustified}


\def\InlineElement#1{#1\DefineInlineElt}

\def\DefineInlineElt{%
    \@temptoksa=\expandafter{\theComponents}
    \ifFixedWidth
        \EDefPN{\theName}{%
            \setbox0=\hbox\bgroup
                \SpaceHorizontally 
   	            \SwitchFont{\theFont}%
                \SwitchColor{\thePrintColor}%                
                \the\@temptoksa
            \egroup
            \@tempdima=\theWidth
            \advance\@tempdima by -\wd0%
            \ifRightJustified
              \hskip\@tempdima
              \unhbox0%
            \else
              \unhbox0%
              \hskip\@tempdima
            \fi
         }
    \else
         \EDefPN{\theName}{%
            \bgroup
   	          \SpaceHorizontally 
   	          \SwitchFont{\theFont}%
              \SwitchColor{\thePrintColor}%
              \the\@temptoksa
            \egroup
         }
   	\fi
    %\DEBUG{\theName}
    %\DEBUG{p@\theName}
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Text Tags         						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%



\MakeParam{ScreenFace}
\MakeParam{ScreenColor}

\def\TextTag#1{#1\DefineTextTag}

\def\DefineTextTag{%
   \EDefPN{\theName}{%
      \leavevmode
      \CallName{\theFont}%
      \SwitchColor{\thePrintColor}%
   }
   %\DEBUG{\theName}
   %\DEBUG{p@\theName}
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												% 
% 		Vertical Material  						% 
%												% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Material}
\MakeParam{Leadin}

\def\Vertical#1{#1\DefineVertical}

\def\DefineVertical{%
    %\EDefPN{\theName}{{\SpaceVertically\theLeadin\theMaterial}}
	\EDefPN{\theName}{{\SpaceVertically\theMaterial}}
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Shell Descriptor  						%  
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% This is not used for tex. Style Editor uses it
% write its own data into the style file
 
\def\ShellDescriptor#1{\relax}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Style Info 								%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% This is not used for tex. Style Editor uses it
% write its own data into the style file


\MakeParam{Version}
\MakeParam{DocumentType}
\MakeParam{StyleDescription}

\def\StyleEditorStyle#1{\relax}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SENewfield}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                           %
%     The Newfield Environment              %
%                                           %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\long\def\@readenvir#1#2 \end{%
   \DefAppendRight\envirsofar{#2}%
   \@ifnextchar\bgroup{\@checkenvir{#1}}{\@envirenderr}%
}


\def\@envirenderr{%
   \errmessage{\string\end\space not followed by environment name}
}


\long\def\firstblank#1{#1}
\long\def\lastblank#1\xxx{#1}
\long\def\debl#1{\lastblank\firstblank#1\xxx}
\long\def\deblank#1{%
     \expandafter\def\expandafter#1\expandafter{\expandafter\debl\expandafter{#1}}}


\def\@blankspace{ }% SciWord's nil field

\def\@checkenvir#1#2{%
   \def\@setempa{#1}%
   \def\@tempb{#2}%
   \ifx\@setempa\@tempb
      \ifx\envirsofar\@blankspace
          \let\envirsofar\@empty
      \fi
      \deblank\envirsofar
      \expandafter\let\csname the@#1\endcsname\envirsofar
      \def\@setempa{\csname @do@#1\endcsname}%
   \else
      \DefAppendRight\envirsofar{\end{#2}}%
      \def\@setempa{\@readenvir{#1}}%
   \fi
   \@setempa
}

\def\newfield#1#2{%
     \expandafter\@ifdefinable\csname#1\endcsname{%
     \DefN{#1}{%
         \endgroup
         \let\envirsofar\@empty
         \@readenvir{#1}%
     }%
     \expandafter\let\csname the@#1\endcsname\@empty
     \expandafter\ifx\csname @do@#1\endcsname\relax
         \DefN{@do@#1}{{\csname the@#1\endcsname}}%
     \fi
 }%
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SEParagraph
%

\def\DefaultParagraphSetup{\relax}

%
% Parameters that have meaning to 
% the style editor only (not TeX) 
%
\MakeParam{Justification}
\MakeIf{AllowHyphens}
\MakeIf{KeepLines}


%
% Current parameters
%
\MakeParam{LabelComponents}
\MakeParam{LabelSep}
\MakeParam{LabelWidth}
\MakeParam{LabelFont}
\MakeIf{FixedWidth}
\MakeParam{Spacing}
\MakeIf{UseLeading}
\UseLeadingfalse
\MakeParam{Font}
\MakeParam{LineSpacing}
\MakeParam{ParSkip}
\MakeParam{ParIndent}
\MakeParam{LeftIndent}
\MakeParam{LeftRag}
\MakeParam{RightIndent}
\MakeParam{RightRag}
\MakeParam{ParFill}
\MakeParam{HyphenPen}
\MakeParam{LinePenalty}
\MakeParam{DblHyphenPen}
\MakeParam{ExHyphenPen}
\MakeParam{FinalHyphenPen}
\MakeParam{PostHyphenMin}
\MakeParam{PreHyphenMin}
\MakeParam{Tolerance}
\MakeParam{PreTolerance}
\MakeParam{EmergencyStretch}
\MakeParam{IncompatabilityPen}
\MakeParam{LineSkipLimit}
\MakeParam{LineSkipGlue}
\MakeParam{PageBreakPen}
\MakeParam{ClubPen}
\MakeParam{WidowPen}
\MakeParam{DisplayPen}
\MakeParam{BrokenPen}
\MakeParam{JustZone}
\MakeParam{MinLineLength}
\MakeIf{HasLabel}
\HasLabelfalse
\MakeIf{RightJust}
\MakeParam{LabelParIndent}
\MakeParam{LabelParSep}



\def\Paragraph#1{#1\DefineParSetup}

\DefPN{DoParLabelFont}{}
\DefPN{DoParLabelComponents}{}

\DefPN{CalculateParfillskip}#1#2#3{%
     \@tempdima  = #1% Justification Zone 
     \@tempdimb  = #2% Minimum line length
     \ifdim\@tempdima=0pt\relax
         \ifdim\@tempdimb=0pt\relax
            \parfillskip=#3\relax
         \else
            \@tempdima=\linewidth
            \advance\@tempdima by -\@tempdimb
            \parfillskip = 0pt plus \@tempdima
         \fi
     \else
         \ifdim\@tempdimb=0pt\relax
            \parfillskip=#3%
            \advance\parfillskip by #1%
         \else
            \@tempdima=\linewidth
            \advance\@tempdima by -#2%
            \advance\@tempdima by -#1%
            \parfillskip=#1 plus \@tempdima
         \fi
     \fi
} 



\def\DefineParSetup{
    \@temptoksa = \expandafter{\theLabelComponents}
   	\EDefPN{\theName}{%
        \DEBUGMSG{Paragraph: \theName}%
        \parskip       = \theParSkip
      	\parindent     = \theParIndent
        \lineskip      = \theLineSkipGlue
        \lineskiplimit = \theLineSkipLimit
        \ifUseLeading
            \baselineskip=\theLineSpacing pt%
        \else
            \baselineskip=\noexpand\f@baselineskip
            \baselineskip=\theLineSpacing\baselineskip
        \fi
        \advance\leftskip by \theLeftIndent plus \theLeftRag
        \leftmargin=\theLeftIndent
        \advance\rightskip by  \theRightIndent plus \theRightRag
        \advance\linewidth by -\leftskip
        \advance\linewidth by -\rightskip
        \CalculateParfillskip{\theJustZone}{\theMinLineLength}{\theParFill}%
        \hyphenpenalty=\theHyphenPen
        \linepenalty=\theLinePenalty
        \interlinepenalty=\thePageBreakPen
        \doublehyphendemerits=\theDblHyphenPen
        \exhyphenpenalty=\theExHyphenPen
        \finalhyphendemerits=\theFinalHyphenPen
        \tolerance=\theTolerance
        \pretolerance=\thePreTolerance
        \emergencystretch=\theEmergencyStretch
        \adjdemerits=\theIncompatabilityPen
        \brokenpenalty=\theBrokenPen
        \clubpenalty=\theClubPen
        \widowpenalty=\theWidowPen
        \displaywidowpenalty=\theDisplayPen
        \labelsep=\theLabelSep
        \labelwidth=\theLabelWidth
        \itemindent=\theLabelParIndent
        \if\Not{FixedWidth}%
           \labelwidth = 0pt
        \fi
        \IDef\p@DoParLabelComponents{%
		   \bgroup
		     \SpaceHorizontally
             \ifFixedWidth
                \hbox to\theLabelWidth {%
                    \ifRightJust\hfil\fi
                    \the\@temptoksa
                    \ifRightJust\else\hfil\fi
                }%
             \else
                \the\@temptoksa
             \fi
             %\hskip\labelsep%
		   \egroup
        }%
        \IDef\p@DoParLabelFont{\SwitchFont{\theLabelFont}}%
   }%
   \EDefN{end\theName}{\endgraf}
   %\DEBUG{\theName}
   %\DEBUG{p@\theName}
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEMath}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Math			  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{EZLabelAppearance}
\MakeParam{EZReset}  % This should be taken out 
\MakeParam{EZLabelPosition}
\def\Left{Left}
\def\Right{Right}
  
\MakeParam{EZEquationPosition}
\def\Center{Center}

\MakeParam{UseCounter}
\MakeParam{Label}
\MakeParam{LabelFont}
\MakeIf{EquationsFlushLeft}
\MakeIf{LabelsOnRight}
\MakeParam{MathIndent}
\MakeParam{Jot}
\MakeParam{ArrayStretch}
\MakeParam{BeforeSkip}
\MakeParam{BeforeShortSkip}
\MakeParam{AfterSkip}
\MakeParam{AfterShortSkip}
\MakeParam{ArrayColSep}
\MakeParam{ArrayRuleWidth}
\MakeParam{EqnCountApp}
\MakeParam{EqnLabelComps}
\MakeParam{EqnXrefComps}
\MakeParam{EqnXrefFont}

% Displayed equations

\newdimen\mathindent
\mathindent = \leftmargini

\def\Math#1{#1\DefineMathSetup}
%\def\DoEquationSetup{\DefineMathSetup}


\def\DefineMathSetup{
    \def\equationCount{\counter@protect\theequation}%
    \def\MathLabelFont{\CallName{\theLabelFont}}%
    {%
       \@temptoksa=\expandafter{\csname\theEqnCountApp\endcsname}
       \xdef\theequation{\the\@temptoksa{equation}}%
       %\show\theequation
    }% 

    \@temptoksa=\expandafter{\theEqnXrefComps}
    \edef\p@equation{{\CallName{\theEqnXrefFont}\the\@temptoksa}}
    %\DEBUG{p@equation}
    \@temptoksa=\expandafter{\theLabel}
    \xdef\theequationlabel{{\text{\MathLabelFont\the\@temptoksa}}}
	
    \ifEquationsFlushLeft
        \input fleqn.clo
        \mathindent = \theMathIndent
    \fi
    \ifLabelsOnRight
    	\gdef\@eqnnum{\theequationlabel}
    \else
    	\gdef\@eqnnum{\hbox to 0.1pt{}\rlap{\hskip -\displaywidth\theequationlabel}}
    \fi
    \abovedisplayskip = \theBeforeSkip
    \abovedisplayshortskip = \theBeforeShortSkip
    \belowdisplayskip = \theAfterSkip
    \belowdisplayshortskip = \theAfterShortSkip
    \arraycolsep = \theArrayColSep
    \arrayrulewidth = \theArrayRuleWidth
}


\def\equationCount{\counter@protect\theequation}

\DefAppendRight{\eqnarray}{\gdef\@currentlabel{\p@equation}}

\newcounter{equationnumber}  
\def\mathletters{%
   \addtocounter{equation}{1}
   \edef\@setemp{\theequation}
   \edef\@currentlabel{\p@equation}%
   \setcounter{equationnumber}{\c@equation}
   \setcounter{equation}{0}%
   \edef\theequation{\@setemp\noexpand\alph{equation}}%
}

\def\endmathletters{%
   \setcounter{equation}{\value{equationnumber}}%
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SEOutput

% Output routine related macros

\newtoks\OldOutput
\OldOutput=\expandafter{\the\output}%

\DefN{ChangeOutput}{%
     \DefAppendLeft{\@outputpage}{%
          \expandafter\def\expandafter\@temptop\expandafter{\topmark}%
          \expandafter\def\expandafter\@tempfirst\expandafter{\firstmark}%
          \expandafter\def\expandafter\@tempbot\expandafter{\botmark}%
          %\PrintMarks{\@temptop}{TOP: }%
          %\PrintMarks{\@tempfirst}{FIRST: }%
          %\PrintMarks{\@tempbot}{BOT: }%
	 }%
	 \DefAppendRight{\@outputpage}{%
         \LastOf{\@tempbot}{\se@temp}%
		 \if\EmptyList\se@temp
		 \else
            \edef\@tempn{\expandafter\TagComp\se@temp}%
            \Clean{\MarkList}{\@tempn}%
		 \fi
     }%
}

\let\@@outputpage\@outputpage


\DefPN{RestoreOutput}{%
      \let\@outputpage\@@outputpage
      \csname\OldPageSetup\endcsname}

\def\crop#1#2#3#4{%
   \newdimen\vpadjust
   \newdimen\hpadjust
   \newdimen\pagewidth
   \newdimen\pageheight
   \pagewidth #1
   \pageheight #2
   \hpadjust #3
   \vpadjust #4
   \advance\pagewidth by -\hpadjust
   \advance\pagewidth by -1in
   \advance\pageheight by -\vpadjust
   \advance\pageheight by -\vpadjust
   \newbox\vbartop
   \newbox\vbarbot
   \newbox\hbarleft
   \newbox\hbarright

   \setbox\vbartop=\hbox{\vrule height 1cm depth -.1cm}
   \dp\vbartop=0pt
   \ht\vbartop=0pt
   \wd\vbartop=0pt

   \setbox\vbarbot=\hbox{\vrule height -.1cm depth 1cm}
   \dp\vbarbot=0pt
   \ht\vbarbot=0pt
   \wd\vbarbot=0pt

   \setbox\hbarleft=\hbox{\hskip -1cm\vbox{\hrule width .9cm}}
   \dp\hbarleft=0pt
   \ht\hbarleft=0pt
   \wd\hbarleft=0pt

   \setbox\hbarright=\hbox{\hskip .1cm\vbox{\hrule width .9cm}}
   \dp\hbarright=0pt
   \ht\hbarright=0pt
   \wd\hbarright=0pt

   \newbox\topmarks
   \newbox\botmarks

   \setbox\topmarks=\hbox to \pagewidth{%
              \hskip -1in
              \hskip\hpadjust
              \vbox{\copy\vbartop\vskip -\baselineskip \copy\hbarleft}  
              \hfil
              \vbox{\copy\vbartop\vskip -\baselineskip \copy\hbarright}}

   \setbox\botmarks=\hbox to \pagewidth{%
              \hskip -1in
              \hskip\hpadjust
              \vbox{\copy\vbarbot\vskip -\baselineskip \copy\hbarleft}  
              \hfil
              \vbox{\copy\vbarbot\vskip -\baselineskip \copy\hbarright}}

   \let\@@shipout\shipout

   \def\shipout \vbox ##1{%
	    \@@shipout\vbox{%
		     	\vskip -1in 
				\vskip \vpadjust
				\copy\topmarks
				\vskip -\baselineskip
				\vskip\pageheight
				\copy\botmarks
				\vskip -\pageheight
				\vskip 1in
				\vskip -\vpadjust
			    ##1
             }%
	}%

}% end of crop macro

      


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEGlobal}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Global Parameters						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Mode}
\MakeParam{FontScheme}
\MakeParam{PointSize}
\MakeParam{ThmHeadingFont}
\MakeParam{GenericPageSetup}
\MakeParam{GenericParagraph}
\MakeParam{GenericFont}
\MakeParam{PageSize}
\MakeParam{PageHeight} 
\MakeParam{PageWidth} 
\MakeParam{TrimHeight} 
\MakeParam{TrimWidth}
\MakeParam{HorizontalOffset}
\MakeParam{VerticalOffset} 
\MakeParam{PrimaryLevel}
\MakeParam{SecondaryLevel}
\MakeIf{CropMarks}
\MakeParam{ExternalFile}
\MakeParam{ExtMacroNames}
\MakeParam{TopNum}
\MakeParam{BottomNum}
\MakeParam{TotalNum}
\MakeParam{TopFrac}
\MakeParam{TextFrac}
\MakeParam{BottomFrac}
\MakeParam{FloatPgFrac}
\MakeParam{DblTopNum}
\MakeParam{DblTopFrac}
\MakeParam{DblFloatPgFrac}
\MakeParam{FloatSep}
\MakeParam{TextFloatSep}
\MakeParam{InTextSep}
\MakeParam{DblFloatSep}
\MakeParam{DblTextFloat}
\MakeParam{TopFigRule}
\MakeParam{BottomFigRule}
\MakeParam{DblFigRule}


\newdimen\trimmargin
\newdimen\verttrimmargin
\newdimen\trimwidth



\def\GlobalParameterSetup{
    \input \theFontScheme
    \if\EqualStrings{\theExternalFile}{}
    \else
       \usepackage{\theExternalFile}
    \fi
	%
    \trimmargin=\thePageWidth
    \advance\trimmargin by -\theTrimWidth
    \divide\trimmargin by 2
    \verttrimmargin=\thePageHeight
    \advance\verttrimmargin by -\theTrimHeight
    \divide\verttrimmargin by 2
    \trimwidth=\theTrimWidth
    \ifCropMarks
       \crop{\thePageWidth}{\thePageHeight}{\trimmargin}{\verttrimmargin}%
    \fi
    \ifcase\se@pdfoutput
    \else
	  \paperwidth\thePageWidth
	  \paperheight\thePageHeight
      \pdfpageheight\thePageHeight
      \pdfpagewidth\thePageWidth
    \fi
    
	\ChangeOutput
    \let\@@outputpage\@outputpage
    \voffset=\theVerticalOffset
    \hoffset=\theHorizontalOffset
    \edef\theorem@headerfont{\csname\theThmHeadingFont\endcsname}
    \setcounter{topnumber}{\theTopNum}
    \setcounter{bottomnumber}{\theBottomNum}
    \setcounter{totalnumber}{\theTotalNum}
    \edef\topfraction{\theTopFrac}
    \edef\bottomfraction{\theBottomFrac}
    \edef\textfraction{\theTextFrac}
    \edef\floatpagefraction{\theFloatPgFrac}
    \setcounter{dbltopnumber}{\theDblTopNum}
    \edef\dbltopfraction{\theDblTopFrac}
    \edef\floatsep{\theFloatSep}
    \edef\textfloatsep{\theTextFloatSep}
    \intextsep=\theInTextSep
    \edef\dblfloatsep{\theDblFloatSep}
    \edef\dbltextfloatsep{\theDblTextFloat}
}

\def\GlobalParams#1{%
    #1\GlobalParameterSetup
}
    



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEPageSetup}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Page Setup		  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%
% Three part Header and footer stuff
%

%
% Some defaults
%
\def\oddlefthead{\relax}
\def\oddmiddlehead{\relax}
\def\oddrighthead{\relax}
\def\oddleftfoot{\relax}
\def\oddmiddlefoot{\relax}
\def\oddrightfoot{\relax}
\def\evenlefthead{\relax}
\def\evenmiddlehead{\relax}
\def\evenrighthead{\relax}
\def\evenleftfoot{\relax}
\def\evenmiddlefoot{\relax}
\def\evenrightfoot{\relax}

\def\@threepart#1#2#3{\SpaceHorizontally\rlap{#1} \hfil {#2} \hfil \llap{#3}}
\def\ps@normal{
   \def\@oddhead{\@threepart{\oddlefthead}{\oddmiddlehead}{\oddrighthead}}
   \def\@oddfoot{\@threepart{\oddleftfoot}{\oddmiddlefoot}{\oddrightfoot}}
   \def\@evenhead{\@threepart{\evenlefthead}{\evenmiddlehead}{\evenrighthead}}
   \def\@evenfoot{\@threepart{\evenleftfoot}{\evenmiddlefoot}{\evenrightfoot}} 
}

\ps@normal

%
% Quick mode parameters
%
\MakeParam{EZMargUnit}
\MakeParam{EZPageSize}

    \def\AFourSize{AFour}
    \def\USLetter{USLetter}
    \def\USLegal{USLegal}
        
\MakeParam{EZTopMargin}
 
\def\EZBottomMargin#1{\def\theEZBottomMargin{#1}}
\def\EZOddLeftMargin#1{\def\theEZOddLeftMargin{#1}}
\def\EZOddRightMargin#1{\def\theEZOddRightMargin{#1}}
      
\def\EZEvenMargins#1{\def\theEZEvenMargins{#1}}

    \def\Same{Same}
    \def\Mirror{Mirror}

\def\EZTwoColumn#1{\csname @twocolumn#1\endcsname} 

\def\QQEmpty{\noexpand\relax}
\def\QQPgNum{\noexpand\thepage}
\def\QQAuthor{\protect\@Author}
\def\QQTitle{\protect\@Title}
\def\QQPriDiv{\noexpand\leftmark}
\def\QQSecDiv{\noexpand\rightmark}   
\def\Custom{\csname QQCustom\endcsname}
    
%
% Custom mode parameters
%

\def\TwoColumn#1{\csname @twocolumn#1\endcsname}


\MakeParam{TypeHeight}
\MakeParam{TypeWidth}
\MakeParam{TextWidth}
\MakeParam{TextHeight} 
\MakeParam{OddLeftMargin} 
\MakeParam{OddRightMargin}
\MakeIf{FacingPages} 
\MakeParam{TopMargin} 
\MakeParam{BottomMargin}
\MakeParam{HeadHeight} 
\MakeParam{HeadSep} 
\MakeParam{TopSkip} 
\MakeParam{FootSkip} 
\MakeParam{FootSep}
\MakeParam{ColumnSep} 
\MakeParam{ColumnRuleWidth} 

\def\OddHeaderLeft#1{\def\theOddHeaderLeft{#1}}
\def\OddHeaderRight#1{\def\theOddHeaderRight{#1}}   
\def\OddHeaderMiddle#1{\def\theOddHeaderMiddle{#1}}
\def\OddFooterLeft#1{\def\theOddFooterLeft{#1}}
\def\OddFooterRight#1{\def\theOddFooterRight{#1}}
\def\OddFooterMiddle#1{\def\theOddFooterMiddle{#1}}
\def\EvenHeaderLeft#1{\def\theEvenHeaderLeft{#1}}
\def\EvenHeaderRight#1{\def\theEvenHeaderRight{#1}}
\def\EvenHeaderMiddle#1{\def\theEvenHeaderMiddle{#1}}
\def\EvenFooterLeft#1{\def\theEvenFooterLeft{#1}}
\def\EvenFooterRight#1{\def\theEvenFooterRight{#1}}
\def\EvenFooterMiddle#1{\def\theEvenFooterMiddle{#1}}

\MakeParam{AutoVert}
\MakeParam{AutoHoriz}
\MakeParam{AutoHead}
\MakeParam{FootnoteBodyFont}
\MakeParam{FootnotePar}
\MakeParam{FootnoteRuleLength}
\MakeParam{FootnoteRuleWidth}
\MakeParam{FootnoteSeparation}
\MakeParam{FootnoteSymbol}
\MakeIf{MarginColumn}
\MakeParam{MarginParFont}
\MakeParam{MarginParPar}
\MakeParam{MarginParPush}
\MakeParam{MarginParSep}
\MakeParam{MarginParWidth}
\MakeParam{DefaultParagraph}
\MakeParam{FootnoteHSkip}
\MakeIf{ReverseMarginPar}


\long\def\PageSetup#1{#1\DefinePageSetup}

\def\DefinePageSetup{
  \long\EDefPN{\theName}{%
    \DEBUGMSG{Page Setup \theName}
    \global\IDef\CurrentPageSetup{\theName}%
	\def\noexpand\@temp{multicols}%
	\noexpand\ifx\noexpand\@currenvir\noexpand\@temp
	\noexpand\else
       \global\oddsidemargin=\theOddLeftMargin
       \textwidth=\trimwidth
       \global\advance\textwidth by -\oddsidemargin     % left margin
       \global\advance\textwidth by -\theOddRightMargin % right margin
 	   \ifMarginColumn
          \global\advance\textwidth by -\theMarginParWidth
          \global\advance\textwidth by -\theMarginParSep
       \fi
 	   \ifFacingPages
          \global\evensidemargin = \theOddRightMargin
          \global\noexpand\@mparswitchtrue
 	   \else
          \global\evensidemargin = \theOddLeftMargin
          \global\noexpand\@mparswitchfalse
 	   \fi
       \ifMarginColumn
          \ifReverseMarginPar
             \DEBUGMSG{Reverse marginpar}
             \global\noexpand\@reversemargintrue
             \global\advance\evensidemargin by \theMarginParWidth
             \global\advance\evensidemargin by \theMarginParSep
             \if\Not{FacingPages}
                \global\advance\oddsidemargin by \theMarginParWidth
                \global\advance\oddsidemargin by \theMarginParSep
             \fi
          \else
             \DEBUGMSG{Normal marginpar}
             \global\noexpand\@reversemarginfalse
             \global\advance\oddsidemargin by \theMarginParWidth
             \global\advance\oddsidemargin by \theMarginParSep
             \if\Not{FacingPages}
                \global\advance\evensidemargin by \theMarginParWidth
                \global\advance\evensidemargin by \theMarginParSep
             \fi
          \fi
       \fi
       \global\columnwidth=\textwidth
       \global\columnsep=\theColumnSep
       \if@twocolumn
          \advance\columnwidth -\columnsep
          \divide\columnwidth\tw@
          \noexpand\@firstcolumntrue 
	   \fi
       \global\hsize=\columnwidth
       \global\linewidth\hsize
       \global\advance\oddsidemargin by \trimmargin
       \global\advance\oddsidemargin by -1in% because 1 inch is automatic
   	   \global\advance\evensidemargin by \trimmargin
       \global\advance\evensidemargin by -1in%
 	   \global\topmargin = \theTopMargin
       \global\topskip = \theTopSkip
       \global\headheight = \theHeadHeight
       \global\headsep = \theHeadSep
       \global\footskip = \theFootSkip
       \global\textheight = \theTextHeight
%	   \global\vsize=\textheight
       \global\@colht=\textheight
       \global\@colroom\textheight
       \global\advance\topmargin by \trimmargin
       \global\advance\topmargin by -1in
      \global\columnseprule = \theColumnRuleWidth
      \global\footnotesep = \theFootnoteSeparation
      \IDef\footnoterule{%
          \kern-3\p@
          \hrule width \theFootnoteRuleLength\space height \theFootnoteRuleWidth
          \kern 2.6\p@
      }%
      \long\IDef\@makefntext####1{%
 	      {%
	    \leftskip = 0pt	% because this may be set inside a list paragraph
           \CallName{\theFootnoteBodyFont}%
           \CallName{\theFootnotePar}%
		\leavevmode
           %%\noexpand\llap{$\noexpand\m@th^{\noexpand\@thefnmark}$\hskip\theFootnoteHSkip}{####1}%
           \noexpand\llap{\textsuperscript{\noexpand\@thefnmark}\hskip\theFootnoteHSkip}{####1}%
           \endgraf
         }%
      }%
      \long\IDef\@ympar####1{%
          \noexpand\@savemarbox\noexpand\@marbox
              {%
                 \CallName{\theMarginParFont}%
                 \CallName{\theMarginParPar}%
                 ####1%
              }%
          \global\setbox\noexpand\@currbox\copy\noexpand\@marbox\noexpand\@xympar
      }%
      \marginparsep =  \theMarginParSep
  	   \marginparpush = \theMarginParPush
      \marginparwidth = \theMarginParWidth
      %
   \noexpand\fi
   \global\IDef\oddlefthead{\theOddHeaderLeft}%
   \global\IDef\oddmiddlehead{\theOddHeaderMiddle}%
   \global\IDef\oddrighthead{\theOddHeaderRight}%
   \global\IDef\oddleftfoot{\theOddFooterLeft}%
   \global\IDef\oddmiddlefoot{\theOddFooterMiddle}%
   \global\IDef\oddrightfoot{\theOddFooterRight}% 
   %
   \global\IDef\evenlefthead{\theEvenHeaderLeft}%
   \global\IDef\evenmiddlehead{\theEvenHeaderMiddle}%
   \global\IDef\evenrighthead{\theEvenHeaderRight}%
   \global\IDef\evenleftfoot{\theEvenFooterLeft}%
   \global\IDef\evenmiddlefoot{\theEvenFooterMiddle}%
   \global\IDef\evenrightfoot{\theEvenFooterRight}% 
   \global\oddsidemargin\oddsidemargin
   \global\evensidemargin\evensidemargin 
  }
  %\DEBUG{ps@\theName}
  %\DEBUG{\theName}
}


  \long\def\@footnotetext#1{%
    \insert\footins{%
      \interlinepenalty\interfootnotelinepenalty
      \splittopskip\footnotesep
      \splitmaxdepth \dp\strutbox \floatingpenalty \@MM
      \let\@setempa\protect
      \def\protect{\noexpand\protect\noexpand}%
      \edef\@currentlabel{\csname p@footnote\endcsname\@thefnmark}%
      \let\protect\@setempa
      \@makefntext{\rule\z@\footnotesep\ignorespaces#1\@finalstrut\strutbox}%
    }%
  }


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEDisplayElts}
%
\MakeParam{Components}
\MakeParam{ParagraphType}
\MakeParam{Font}
\MakeParam{Color}
\MakeParam{SkipBefore}
\MakeParam{SkipAfter}


\def\DisplayElement#1{#1\DefineDisplayElt}


\def\DefineDisplayElt{%
    \@temptoksa=\expandafter{\theComponents}
    \EDefPN{\theName}{%
       \endgraf
       \vbox\bgroup
          \SaveFootnotes
          \SpaceHorizontally
          \SwitchFont{\theFont}%
          \SwitchColor{\theColor}%
          \SwitchParagraph{\theParagraphType}%
          \the\@temptoksa
 	   \egroup
    }%
    %\DEBUG{\theName}
    %\DEBUG{p@\theName}
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%SEMark
%


\newcount\@markCount

\def\primarymark{}
\def\secondarymark{}
\def\MarkList{}

\DefPN{IPrimaryMark}#1{%
   {%
      \def\protect{\noexpand\protect\noexpand}%
	  \let\counter@protect\relax
      \let\label\@gobble
      \let\index\@gobble
      \let\glossary\@gobble
	  \def\FRAME##1##2##3##4##5##6##7##8{}%
	  \def\p@Space{\noexpand\p@Space}%
      \xdef\primarymark{#1}%
      \xdef\secondarymark{}%
   }%
   \DoMark{\primarymark}{\secondarymark}% 
}

\DefPN{ISecondaryMark}#1{%
   {%
      %\let\protect\relax
      \def\protect{\noexpand\protect\noexpand}%
	  \let\counter@protect\relax
      \let\label\@gobble
      \let\index\@gobble
      \let\glossary\@gobble
	  \def\FRAME##1##2##3##4##5##6##7##8{}%
	  \def\p@Space{\noexpand\p@Space}%
      \xdef\secondarymark{#1}%
   }
   \DoMark{\primarymark}{\secondarymark}%
}

\def\DoMark#1#2{%
   {%
      \def\Space##1{\space}%
      \def\protect{\noexpand\protect\noexpand}%
	  \let\counter@protect\relax
      \let\label\@gobble
      \let\index\@gobble
      \let\glossary\@gobble
	  \def\p@Space{\noexpand\p@Space}%
      \global\advance\@markCount by 1
      \edef\@tempmac{\noexpand\AppendRear{\noexpand\MarkList}{{#1}{#2}{\the\@markCount}}}%
      \@tempmac
      \@temptoksa=\expandafter{\MarkList}%
      \mark{\the\@temptoksa}%
    }
}

\def\PriComp#1#2#3{#1}
\def\SecondComp#1#2#3{#2}
\def\TagComp#1#2#3{#3}

\def\PrintMarks#1#2{%
   \begingroup
      \def\p@Space##1{}
      \message{^^J #2}%
      \ForEach{#1}{\Printmark}
   \endgroup
}


\def\Printmark#1{%
   \edef\@tempA{\expandafter\PriComp #1}%
   \edef\@tempB{\expandafter\SecondComp #1}%
   \edef\@tempC{\expandafter\TagComp #1}%
   \message{(\@tempA) \space (\@tempB) \space (\@tempC)}
}


\def\Clean#1#2{%
   \if \EmptyList #1
   \else
      \Car{#1}{\@tempA}%
      \edef\@temp{\expandafter\TagComp\@tempA}%
      \ifnum \@temp > #2
	      \let\se@next\relax
      \else
      	  \RemoveFront{#1}%
      	  \def\se@next{\Clean{#1}{#2}}%
      \fi
      \se@next
   \fi
}


\def\FirstPrimaryMark{%
   \if\EmptyList{\@tempfirst}
   \else
      \Car{\@tempfirst}{\@tempmark}
      \expandafter\PriComp\@tempmark
   \fi
}

\def\LastPrimaryMark{%
   \if\EmptyList{\@tempbot}
   \else
      \LastOf{\@tempbot}{\@tempmark}%
      \expandafter\PriComp\@tempmark
   \fi
}

\def\FirstSecondaryMark{%
   \if\EmptyList{\@tempfirst}
   \else
      \Car{\@tempfirst}{\@tempmark}
      \expandafter\SecondComp\@tempmark
   \fi
}

\def\LastSecondaryMark{%
   \if\EmptyList{\@tempbot}
   \else
      \LastOf{\@tempbot}{\@tempmark}%
      \expandafter\SecondComp\@tempmark
   \fi
}
   

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SELists
%
\MakeParam{LevelUp}	 
\MakeParam{Level}
\MakeParam{Components}
\MakeParam{LabelType}
\MakeParam{LabelWidth}
\MakeParam{LabelSep}
\MakeParam{TopSep}
\MakeParam{BotSep}
\MakeParam{ItemPar}
\MakeParam{ParTopSep}
\MakeParam{ItemSep}
\MakeParam{ItemFont}
\MakeParam{LabelScheme}
\MakeParam{BulletType}

\def\BulletA{$\protect\m@th\bullet$}
\def\BulletB{{\protect\bfseries --}}
\def\BulletC{$\protect\m@th\ast$}
\def\BulletD{$\protect\m@th\cdot$}
\def\BulletE{$\protect\blacktriangleright$}


%\def\ENUMERATED{enumerated}
%\def\DESCRIPTION{custom}

%\def\ListType{NoList}

\DefPN{CheckDepth}{%
    \ifnum\@listdepth>5\relax 
       \@toodeep
    \else 
       \global\advance\@listdepth\@ne 
    \fi
}

\DefPN{SetDefaults}{%
    \ifnum\@listdepth>1\relax 
    \else 
   	   \rightmargin=0pt%
   	   \leftmargin=0pt% 
       \listparindent=0pt% 
       \itemindent=0pt%
    \fi
}


\DefPN{StartListLevel}#1{%
    \CheckDepth
    %\SetDefaults
    \if@inlabel 
       \@noparitemtrue 
       \@noparlisttrue
    \else 
       \@noparlistfalse 
       \@topsep\@topsepadd 
    \fi
    \parindent\listparindent
    \parfillskip 0pt plus 1fil%
    \CallName{#1\@Alph{\@listdepth}}%
    \ignorespaces
}


\DefPN{EndListLevel}#1{% 
   \CallName{end#1\@Alph{\@listdepth}}%
   \endlist
}


\DefPN{CheckInLabel}#1{%
   \if@inlabel 
       \@noparitemtrue 
       \@noparlisttrue
   \else 
       \@noparlistfalse
       %\ifvmode\else\endgraf\fi 
       \@@par\null
       \vskip -\baselineskip
       \vskip #1% 
   \fi
}

% There has been some inconsistency as to whether 
% this is capitalized or not so both defs are included.

\def\ItemCount{\protect\@ItemCount}
\def\itemCount{\protect\@ItemCount}

\DefPN{SetupItemCounter}#1#2#3{%
   \if\EqualStrings{#2}{enumerated}
      \@nmbrlisttrue
      \usecounter{#1count}%
      \def\@ItemCount{\CallName{the#1count}}%
   \else
      \@nmbrlistfalse
      \def\@ItemCount{\CallName{#3}}%
   \fi
}


\DefPN{SetupMakelabel}#1{%
   \if\EqualStrings{#1}{custom}
      \def\makelabel##1{{\DoParLabelFont ##1}\hspace\labelsep}%
   \else
      \def\makelabel##1{\hss\llap{##1}}%
   \fi
}


\DefPN{SetupItemlabel}{%
   \def\@itemlabel{{\DoParLabelFont\DoParLabelComponents}}%
}

\def\DefineListLevel{%
   \EDefN{\theName}{%
      \protect\@newlisttrue
      \SetupItemCounter{\theName}{\theLabelScheme}{\theBulletType}%
      \CheckInLabel{\theTopSep}%
      \SetupMakelabel{\theLabelScheme}%
      \CallName{\theItemFont}%
      \CallName{\theItemPar}%
      \listparindent=\parindent
      \itemsep=\theItemSep
      \SetupItemlabel
   }
   \EDefPN{end\theName}{%
      \endgraf\IAddVSpace{\theBotSep}%
   }
   %\DEBUG{\theName}  
}

    
\def\ListLevel#1{%
   \def\Name##1{\def\theName{##1}}%
   #1%
   \DefineListLevel
}


\def\List#1{%
   \def\Name##1{\def\@theName{##1}}% save the top level name
   #1%
   \EDefPN{\@theName}{\StartListLevel{\@theName}}
   \EDefPN{end\@theName}{\EndListLevel{\@theName}}
   %\DEBUG{end\@theName}
   %\DEBUG{\@theName}
}


\def\@donoparitem{%
   \@noparitemfalse
   \global\setbox\@labels\hbox{%
      \hskip -\leftmargin
      \unhbox\@labels
      \hskip \leftmargin
   }%
   \if\Not{@minipage}%
      \@tempskipa\lastskip
      \vskip -\lastskip
      %\advance\@tempskipa\parskip%\@outerparskip
      %\advance\@tempskipa -\parskip 
      \vskip\@tempskipa
   \fi
}

\def\@item[#1]{%
   \if@noparitem
      \@donoparitem
   \else 
      \if@inlabel\indent\par\fi
      \ifhmode\unskip\unskip\par\fi
      \if@newlist 
         \if@nobreak 
            \@nbitem 
         \else
            \addpenalty\@beginparpenalty
            \addvspace\@topsep 
            \addvspace{-\parskip}%
         \fi
      \else 
         \addpenalty\@itempenalty 
         \addvspace\itemsep
      \fi
      \global\@inlabeltrue
   \fi
   \everypar{%
          \global\@minipagefalse
          \global\@newlistfalse
          \if@inlabel
             \global\@inlabelfalse
             \kern -\parindent
             \box\@labels
             \penalty\z@
          \fi
          \everypar{}}%
   \global\@nobreakfalse
   \if@noitemarg 
       \@noitemargfalse 
       \if@nmbrlist
           \refstepcounter{\@listctr}%
       \fi
   \fi
   \sbox\@tempboxa{\makelabel{#1}}%
   \global\setbox\@labels\hbox{%
            \unhbox\@labels
            \hskip \itemindent
            \hskip -\labelwidth 
            \hskip -\labelsep
            \ifdim \wd\@tempboxa >\labelwidth
                \box\@tempboxa
            \else 
                \hbox to\labelwidth {\unhbox\@tempboxa}%
            \fi
            \hskip \labelsep}%
   \ignorespaces
}


\DefPN{ISimpleItem}[#1]{%
     \leavevmode
     \bgroup
         \hskip \itemindent
         \hskip -\labelwidth 
         \hskip -\labelsep
		 \setbox\@tempboxa\hbox{#1}%
         \ifdim \wd\@tempboxa > \labelwidth
            \box\@tempboxa
         \else 
            \hbox to\labelwidth {\unhbox\@tempboxa}%
         \fi
         \hskip\labelsep
     \egroup
     \ignorespaces
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\ SEToc
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Making TOC Entries 						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\def\StartTOC{\ifvmode\else\endgraf\fi\@starttoc{toc}}

\def\@starttoc#1{%
    \begingroup
        \makeatletter
        \@input{\jobname.#1}%
        \if@filesw 
           \expandafter\newwrite\csname tf@#1\endcsname
              \immediate\openout \csname tf@#1\endcsname \jobname.#1\relax
        \fi 
        \global\@nobreakfalse
        \makeatother 
    \endgroup}
 
 


% AddContentsLine:
% 
%     #1 = filename e.g. toc, lof, lot
%     #2 = level
%     #3 = number
%     #4 = title
%

\edef\thefmtversion{1995/06/01}


 
\DefPN{AddContentsLine}#1#2#3#4{%
   \if@filesw 
      \begingroup
	     \SpaceHorizontally
         \let\label\@gobble 
         \let\index\@gobble 
         \let\glossary\@gobble
	     \def\FRAME##1##2##3##4##5##6##7##8{}%
         {\def\protect{}%
		  \def\hspace{\string\hspace}
		  \def\counter@protect{}%
          \xdef\@tempb{#3}%
         }%
         \def\protect##1{\string##1\space}%
		 \def\counter@protect##1{\string##1\space}%
         \@temptoksa{\thepage}%
		 \if\Undefined{\Hy@Warning}
             \edef\@setempa{\write\@auxout{%
                 \string\@writefile{#1}%
                       {\protect\ContentsEntry{#2}{\@tempb}{#4}{\the\@temptoksa}}%
                 }%
             }%
		 \else
		     \edef\@setempa{\write\@auxout{%
                 \string\@writefile{#1}%
                       {\protect\ContentsEntry{#2}{\@tempb}{#4}{{\the\@temptoksa}{\@currentHref}}}%
                 }%
             }%             
		 \fi
		 \@setempa
		 \if\Undefined{\Hy@Warning}
		 \else
		     \def\counter@protect##1{##1}%
		     %\def\protect{}%			   
			 %\def\@mathbf{}%
			 \@temptoksa={#4}%
             \edef\@setempa{%           	    
                \noexpand\Hy@writebookmark{#3}%
                  {\the\@temptoksa}%
                  {\@currentHref}%
                  {\csname#2@lev\endcsname}%
                  {#1}%
		        }%
			 \def\protect{}%
		     \@setempa
             %%\ifHy@verbose
             %%  \typeout{pdftex: bookmark at \the\inputlineno:
             %%    {#3}
             %%    {\the\@temptoksa}
             %%    {\@currentHref}%
             %%    {\csname#2@lev\endcsname}%
             %%    {#1}%
             %%  }%
             %%\fi
	     \fi
         \if@nobreak 
             \ifvmode
                \nobreak
             \fi
         \fi
     \endgroup
  \fi}

\long\def\AddToContents#1#2{%
   \if@filesw 
      \begingroup
         \let\label\@gobble 
         \let\index\@gobble 
         \let\glossary\@gobble
	     \def\FRAME##1##2##3##4##5##6##7##8{}%
         \def\protect##1{\string##1\space}%
		 \def\counter@protect##1{\string##1\space}%
         \edef\@setempa{%
             \write \@auxout{%
                 \string\@writefile{#1}%
                      {\protect\ContentsEntry{specialtocentry@toc}{}{#2}{}}%
             }%
         }%
         \@setempa
         \if@nobreak 
           \ifvmode\nobreak\fi
         \fi
      \endgroup
   \fi}


% this is for old styles imported without specialtocentry defined.
 
\def\specialtocentry@toc#1#2#3#4{\relax}  
\def\contentsline#1{\csname l@#1\endcsname}





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 	    Contents 		  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\def\EZTOCSetup#1{\relax}

\def\Contents#1{#1}

\MakeIf{MakeTOCEntry}

\MakeParam{Level}
\MakeParam{TOCTitleFont}
\MakeParam{PgNoFont}
\MakeParam{InterItemPenalty}
\MakeParam{InterItemSkip}
\MakeParam{LeftMarginToNumber}
\MakeParam{LeftMarginToText}
\MakeParam{ContinuationIndent}
\MakeParam{RightMarginToNumber}
\MakeParam{RightMarginToText}
\MakeIf{UseLeaderDots}
\MakeParam{DotSeparation}
\MakeParam{MinDotLength}
\MakeIf{UpperCase}

\def\ContentsLevel#1{#1\DefineContentsLevel}


% These macros are essentially taken from Bechtolsheims's Tex in Practice
% See vol III p 121...

\newcount\@GenTocEntryLevel

\DefPN{GenTocEntry}#1#2#3#4{%
   \par
   \bgroup
      \global\@GenTocEntryLevel = #1
      \leftskip = #4
      \parindent = #2
      \advance\parindent by -#4
      \dimen0 = #3
      \advance\dimen0 by -#2
      \@GenTocEntry
}

\def\@GenTocEntry#1#2#3{%
      \rightskip = #1 plus 1in
      \parfillskip = #2
      \advance\parfillskip by -#1
      \ifdim #3 > 0pt
          \setbox0 = \hbox to #3{\hfil.\hfil}%  
      \else 
          \setbox0 = \box\voidb@x
      \fi
      \@@GenTocEntry
}

\def\@@GenTocEntry#1#2#3#4#5#6#7{%
      \leavevmode
      \baselineskip=\f@baselineskip 
      \hbox to \dimen0 {\csname #4\endcsname #2\hfil}%
      {\csname #6\endcsname #3\unskip}%
      \ifvoid 0 
          %\hfill
          % This was changed to keep some space between 
          % the text and the page number
          \hskip 15pt plus 1fill
      \else
          \nobreak\leaders\copy0\hskip #1 plus 1fil
      \fi
      {\csname #7\endcsname#5}%
      \par
   \egroup
}

\def\stripHname#1Count.{\relax}
\def\firstof#1#2{#1}
\def\secondof#1#2{#2}
\def\ContentsEntry#1#2#3#4{%
  \if\Undefined{\Hy@Warning}
     \csname #1\endcsname{#1}{#2}{#3}{#4}%
  \else     
     \csname #1\endcsname{#1}{#2}{\hyper@linkstart{link}{\secondof #4}{#3}\hyper@linkend}{\firstof #4}%
  \fi
}

%\def\ContentsEntry#1#2#3#4{
%   \csname #1\endcsname{#1}{#2}{#3}{#4}%
%}

\newcounter{Temp}

\def\DefineContentsLevel{%
   \ifUseLeaderDots
   \else
      \def\theDotSeparation{0pt}%
   \fi
   \ifMakeTOCEntry
      \EDefN{\theName}##1##2##3##4{%
		  \IAddVSpace{\theInterItemSkip}%	     
          \GenTocEntry{\theLevel}%
              {\theLeftMarginToNumber}%
              {\theLeftMarginToText}%
              {\theContinuationIndent}%
              {\theRightMarginToText}%
              {\theRightMarginToNumber}%
              {\theDotSeparation}%
              {\theMinDotLength}%
              {##2}%
              {\ifUpperCase\uppercase{##3}\else ##3\fi}%
              {\theLabelFont}%
              {##4}%
              {\theTOCTitleFont}%
              {\thePgNoFont}%
		   \IAddPenalty{\theInterItemPenalty}%
          }%
          
   \else
      \EDefN{\theName}##1##2##3##4{\relax}%
   \fi
   %\DEBUG{\theName}
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEDivision}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Division		  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\Division#1{#1\DefineDivision}


%\DefPN{CurrentHeading}{\@CurrentHeading}
%\DefPN{CurrentShortHeading}{\@CurrentShortHeading}

\def\qprotect{\noexpand\qprotect\noexpand}
\def\CurrentHeading{\qprotect\p@CurrentHeading}
\def\p@CurrentHeading{\@CurrentHeading}
\def\CurrentShortHeading{\qprotect\p@CurrentShortHeading}
\def\p@CurrentShortHeading{\@CurrentShortHeading}




\MakeParam{Level}
\MakeIf{OverrideTitle}
\MakeIf{OverrideRest}
\MakeIf{RevertTitle}
\MakeIf{RevertRest}
\MakeParam{SETitlePage}
\MakeParam{RestPages}
\MakeIf{EnterInTOC}
\MakeIf{SetRightMark}
\MakeIf{SetLeftMark}
\MakeIf{DisplayedHeading}
\MakeIf{IndentHeading}
\MakeIf{IndentFirstPar}
\MakeParam{Position}
\MakeParam{TextAfterHead}
\MakeParam{HeadType}
\MakeParam{HeadComponents}
\MakeParam{HeadParagraph}
\MakeParam{HeadFont}
\MakeParam{HeadColor}
\MakeParam{SkipAfter}
\MakeParam{SkipBefore} 
\MakeParam{UseCounter}
\MakeParam{Components}
\MakeParam{TOCLabelComponents}
\MakeIf{OverrideEqnLabel}
\MakeIf{ResetEqnCnt}
\MakeParam{EqnAppearance}
\MakeParam{EqnLabelComponents}
\MakeParam{EqnXRefComponents}
\MakeIf{OverrideFigLabel}
\MakeIf{ResetFigCnt}
\MakeParam{FigAppearance}
\MakeParam{FigLabelComponents}
\MakeParam{FigXRefComponents}
\MakeIf{OverrideTabLabel}
\MakeIf{ResetTabCnt}
\MakeParam{TabAppearance}
\MakeParam{TabLabelComponents}
\MakeParam{TabXRefComponents}
\MakeIf{OverridePageNo}
\MakeParam{PageNoApp}
\MakeIf{PageNoReset}
\MakeIf{GenLeftMark}
\MakeIf{GenRightMark}
\MakeParam{Mark}
 
\def\CurrentPage{CurrentPage}
\def\NextPage{NextPage}
\def\NextOddPage{NextOddPage}
 	 
\def\DisplayType{Display}
\def\InlineType{Inline}
\def\MarginType{Margin}

\def\DefineDivision{%
  \ifx\theHeadType\DisplayType
    \DefineDisplayDivision		
  \else\ifx\theHeadType\InlineType
    \DefineInlineDivision
  \else\ifx\theHeadType\MarginType
    \DefineMarginDivision
  \fi\fi\fi
}

\def\OldPageSetup{\theGenericPageSetup}
\def\theOldPageNoApp{arabic}

\def\StepCounter#1#2{%
	 \hrefstepcounter{#1}
     \ifnum#2=1\relax  % These counters are used for math reset
        \stepcounter{LevelOneDiv}%
        \def\p@theLevelOneDiv{\csname the#1\endcsname}
     \else\ifnum#2=2
        \stepcounter{LevelTwoDiv}%
        \edef\p@theLevelTwoDiv{\csname the#1\endcsname}
     \else\ifnum#2=3
        \stepcounter{LevelThreeDiv}%
        \edef\p@theLevelThreeDiv{\csname the#1\endcsname}
     \else\ifnum#2=4
        \stepcounter{LevelFourDiv}%
        \edef\p@theLevelFourDiv{\csname the#1\endcsname}
     \else\ifnum#2=5
        \stepcounter{LevelFiveDiv}%
        \edef\p@theLevelFiveDiv{\csname the#1\endcsname}
     \else
     \fi\fi\fi\fi\fi
}

\def\theRunLeadin{}


% GenDiv
%   #1 - current short heading
%   #2 - current heading
%   #3 - the counter
%   #4 - the level

\DefPN{GenDiv}#1#2#3#4{
    \def\@CurrentShortHeading{#1}%
    \def\@CurrentHeading{#2}%
	\ifstarred
	\else
       \StepCounter{#3}{#4}
	\fi
}


% make adjustments of page numbering scheme.
%  #1 - new page numbering scheme
%  #2 - force reset?
\DefN{AdjustPageNumbering}#1#2{%
   \ifOverridePageNo
      \@AdjustPageNumbering{#1}{#2}
   \fi
}


\DefPN{@AdjustPageNumbering}#1#2{%
    \if\EqualStrings{#1}{\theOldPageNoApp}
       % continue same numbering - don't reset unless asked.
       \if\EqualStrings{#2}{T}
          \setcounter{page}{1}%
       \fi
    \else
       % changing numbering appearance
       \gdef\thepage{\CallName{#1}{page}}%
       \def\theOldPageNoApp{#1}%
       \setcounter{page}{1}%
    \fi
}



\def\CheckpageOverrides{%
    \ifOverrideRest
       \IDef\OldPageSetup{\theRestPages}%
    \else\ifRevertRest
       \IDef\OldPageSetup{\theGenericPageSetup}
    \fi\fi
    \ifOverrideTitle
       \global\IDef\CurrentPageSetup{\theSETitlePage}%
       \CallName{\theSETitlePage}%
    \else\ifRevertTitle
       \global\IDef\CurrentPageSetup{\noexpand\theGenericPageSetup}%
    \fi\fi
}


\def\CheckCounterOverrides{%
    \let\noexpand\counter@protect\noexpand\counter@@protect    
    \ifOverrideEqnLabel
       \ifResetEqnCnt\noexpand\setcounter{equation}{0}\fi
       \IDef\theequation{\noexpand\CallName{\theEqnAppearance}{equation}}%
       \IDef\theequationlabel{\text{{\noexpand\MathLabelFont\the\@temptoksb}}}%
       \IDef\p@equation{\the\@temptoksc}%
    \fi
    \ifOverrideFigLabel
       \ifResetFigCnt\noexpand\setcounter{figure}{0}\fi
	   \EDefN{figurelabel}{\theFigLabelComponents}
	   \EDefN{figurexref}{\theFigXRefComponents}
       \IDef\p@thefigureCount{\noexpand\CallName{\theFigAppearance}{figure}}%
    \fi
    \ifOverrideTabLabel
       \ifResetTabCnt\noexpand\setcounter{table}{0}\fi
	   \EDefN{tablelabel}{\theTabLabelComponents}
	   \EDefN{tablexref}{\theTabXRefComponents}
       \IDef\p@thetableCount{\noexpand\CallName{\theTabAppearance}{table}}%
    \fi
	\let\noexpand\counter@protect\relax
}

\def\CheckReverts{%
       \ifOverrideTitle
           \noexpand\afterpage{\noexpand\csname\noexpand\OldPageSetup\endcsname}%
       \else\ifRevertTitle
           \noexpand\afterpage{\noexpand\csname\noexpand\OldPageSetup\endcsname}%
       \else\ifOverrideRest
           \noexpand\afterpage{\noexpand\csname\noexpand\OldPageSetup\endcsname}%
       \else\ifRevertRest
           \noexpand\afterpage{\noexpand\csname\noexpand\OldPageSetup\endcsname}%
       \fi\fi\fi\fi
}


\newif\ifstarred

\DefPN{CheckStar}#1{%
     \if\EqualStrings{#1}{*}
          \starredtrue
     \else
          \starredfalse
     \fi
}


\def\DefineDisplayDivision{
   \@temptoksa=\expandafter{\theHeadComponents}
   \@temptoksb=\expandafter{\theEqnLabelComponents}
   \@temptoksc=\expandafter{\theEqnXRefComponents}
   \EDefN{\theName}{%
       \noexpand\@ifnextchar*%
          {\csname @x\theName\endcsname}%
          {\csname @x\theName\endcsname!}%
   }
   \EDefN{@x\theName}##1{%
       \CheckStar{##1}%
       \noexpand\@dblarg\CallName{@\theName}%
   }
   \DefN{\theName @toc}##1##2##3##4{\relax}
   \EDefN{\theName @toc@lev}{\theLevel}
   \EDefN{@\theName}[##1]##2{%
       \DEBUGMSG{Enter Display Division \theName (##2)}
       \CallName{Start\thePosition}%
       \GenDiv{##1}{##2}{\theUseCounter}{\theLevel}%
       \AdjustPageNumbering{\thePageNoApp}{\TF@PageNoReset}
	   \CheckpageOverrides
       \noexpand\@@par
       \penalty\@secpenalty
       \IAddVSpace{\theSkipBefore}%
       \begingroup
	      \SpaceHorizontally	      
          \CallName{\theHeadFont}%
          \CallName{\theHeadParagraph}%
          \SwitchColor{\theHeadColor}%
          \noexpand\ifstarred
              \ISimpleItem[]\the\@temptoksa
          \noexpand\else
              \ISimpleItem[\DoParLabelFont\DoParLabelComponents]\the\@temptoksa
          \noexpand\fi
          \ifGenLeftMark
			 \IPrimaryMark{\theMark}%
          \else\ifGenRightMark
		     \ISecondaryMark{\theMark}%
          \fi\fi
          \endgraf
       \endgroup
       \INoBreak
	   \CheckReverts
       \IAddVSpace{\theSkipAfter}%
       \noexpand\if\noexpand\Not{starred}
          \AddContentsLine%
               {toc}%
               {\theName @toc}%
          	   {\theTOCLabelComponents}%
               {##1}%
       \noexpand\fi
       \if\Not{IndentFirstPar}
          \SuppressNextIndent
       \fi
	   \CheckCounterOverrides
       \INoBreak
       %\DEBUGMSG{Leave Division \theName (##2)}
   }
   %\DEBUG{\theName}
   %\DEBUG{@x\theName}
   %\DEBUG{@\theName}
}


\def\DefineMarginDivision{
   \@temptoksa=\expandafter{\theHeadComponents}
   \@temptoksb=\expandafter{\theEqnLabelComponents}

   \EDefN{\theName}{%
       \noexpand\@ifnextchar*%
          {\csname @x\theName\endcsname}%
          {\csname @x\theName\endcsname!}%
   }

   \EDefN{@x\theName}##1{%
       \CheckStar{##1}%
       \noexpand\@dblarg\CallName{@\theName}%
   }

   \DefN{\theName @toc}##1##2##3##4{\relax}
   \EDefN{\theName @toc@lev}{\theLevel}
    
   \EDefN{@\theName}[##1]##2{%
       \DEBUGMSG{Enter Margin Division \theName (##2)}
       \CallName{Start\thePosition}%
       \GenDiv{##1}{##2}{\theUseCounter}{\theLevel}%
       \AdjustPageNumbering{\thePageNoApp}{\TF@PageNoReset}
	   \CheckpageOverrides
       \penalty\@secpenalty 
       \IAddVSpace{\theSkipBefore}%
       \bgroup
          \hsize=\noexpand\marginparwidth
		  \SpaceHorizontally
          \CallName{\theHeadFont}%
          \CallName{\theHeadParagraph}%
          \parskip=0pt% 
          \global\setbox0=\vtop{%
             \ISimpleItem[\noexpand\DoParLabelFont\noexpand\DoParLabelComponents]%
                    \theHeadComponents
          }% 
          \ifGenLeftMark
			 \IPrimaryMark{\theMark}%
          \else\ifGenRightMark
		     \ISecondaryMark{\theMark}%
          \fi\fi
          \leavevmode
          \hskip -\noexpand\marginparwidth
          \hskip -\noexpand\marginparsep
          \wd0=0pt\dp0=0pt%
          {\SwitchColor{\theHeadColor}%
              \box0%
          }%
          \hskip \noexpand\marginparwidth
          \hskip \noexpand\marginparsep
	   \egroup
       \noexpand\if\noexpand\Not{starred}
          \AddContentsLine%
               {toc}%
               {\theName @toc}%
               {\theTOCLabelComponents}%
               {##1}%
       \noexpand\fi
	   \CheckCounterOverrides
	   \CheckReverts
       \INoBreak
       \if\Not{IndentFirstPar}
          \SuppressNextIndent
       \fi
       \DEBUGMSG{Leave Division \theName (##2)}
       \ignorespaces
   }
   %\typeout{DEFINING A SECTIONING COMMAND:}
   %\DEBUG{@\theName}
}




% INLINE DIVISIONS

\def\EatPar{%				 
   \def\par{\let\par\@@par}\ignorespaces
}


\def\DefineInlineDivision{%
   \@temptoksa=\expandafter{\theHeadComponents}
   \EDefN{\theName}{%
       \noexpand\@ifnextchar*%
          {\csname @x\theName\endcsname}%
          {\csname @x\theName\endcsname!}%
   }
   \EDefN{@x\theName}##1{%
       \CheckStar{##1}%
       \noexpand\@dblarg\CallName{@\theName}%
   }

   \DefN{\theName @toc}##1##2##3##4{\relax}%
   \EDefN{\theName @toc@lev}{\theLevel}

   \EDefN{@\theName}[##1]##2{%
       \DEBUGMSG{Enter Inline Division \theName (##2)}
       \CallName{Start\thePosition}%
       \GenDiv{##1}{##2}{\theUseCounter}{\theLevel}%
       \AdjustPageNumbering{\thePageNoApp}{\TF@PageNoReset}
	   \CheckpageOverrides
       \penalty\@secpenalty
       \IAddVSpace{\theSkipBefore}%
       \ifIndentFirstPar\@@par\else\noindent\fi
       \begingroup%
	      \SpaceHorizontally
          \CallName{\theHeadFont}%
          \SwitchColor{\theHeadColor}% 
          \the\@temptoksa
       \endgroup
       \ifGenLeftMark
	      \IPrimaryMark{\theMark}%
       \else\ifGenRightMark
	      \ISecondaryMark{\theMark}%
       \fi\fi
       \noexpand\if\noexpand\Not{starred}
           \AddContentsLine%
                   {toc}%
                   {\theName @toc}%
                   {\theTOCLabelComponents}%
                   {##2}%
       \noexpand\fi
	   \CheckCounterOverrides
	   \CheckReverts
 	   \INoBreak
       \DEBUGMSG{Leave Division \theName (##2)}
       \noexpand\EatPar     
     }%
     %\typeout{DEFINING A SECTIONING COMMAND:}
     %\DEBUG{@\theName}
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEFloat}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												% 
% 		Floats  					            % 
%												% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\newenvironment{figure}{\@float{figure}}{\end@float}
\newenvironment{figure*}{\@dblfloat{figure}}{\end@dblfloat}
\newenvironment{table}{\@float{table}}{\end@float}
\newenvironment{table*}{\@dblfloat{table}}{\end@dblfloat}
\newcounter{table}%
\newcounter{figure}%

\let\c@tableCount\c@table
\let\c@figureCount\c@figure

\newlength\abovecaptionskip
\newlength\belowcaptionskip
\setlength\abovecaptionskip{10pt}
\setlength\belowcaptionskip{10pt}
\MakeParam{FloatCountApp}
\MakeParam{FloatLabelComps}
\MakeParam{FloatXrefComps}
\MakeParam{FloatXrefFont}
\def\FIGURENAME{figure}
\def\CaptionText{\noexpand\CaptionText}% used in component lists
\def\Float#1{#1\DefineFloat}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fps@figure{tbp}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\figuretoclabel{}
\def\tabletoclabel{}
\def\figure@makecaption#1{Figure \arabic{figure}. #1}
\def\table@makecaption#1{Table \arabic{table}. #1}
\def\p@figure{\counter@protect \p@thefigure}
\def\p@table{\counter@protect \p@thetable}


\def\DefineFloat{%
    \EDefN{\theName label}{{\SpaceHorizontally\theComponents}}
	\EDefN{\theName xref}{{\SpaceHorizontally\theFloatXrefComps}}
    \@temptoksa = \expandafter{\theTOCLabelComponents}
	%
    \EDefPN{\theName toclabel}{\the\@temptoksa}
	\EDefN{p@\theName}{\expandafter\noexpand\csname\theName xref\endcsname}
    %\EDefPN{the\theName}{%
    %   \noexpand\CallName{\theFloatCountApp}{\theName}%
    %}%
	\EDefPCN{the\theName Count}{\noexpand\CallName{\theFloatCountApp}{\theName}}
    \EDefN{\theName @makecaption}##1{%
         \vskip\abovecaptionskip
         \def\CaptionText{##1}%
         %\sbox\@tempboxa{%
		 \setbox\@tempboxa\hbox{%
            \SwitchFont{\theFont}%
            \SwitchParagraph{\theParagraphType}% Defines \DoParLabelFont and \DoParLabelComponents
            \ISimpleItem[\DoParLabelFont\DoParLabelComponents]\expandafter\noexpand\csname \theName label\endcsname
         }%
         \@tempdima=\wd\@tempboxa
         \noexpand\ifdim \@tempdima>\hsize
            {\unhbox\@tempboxa}\par
         \noexpand\else
            \hbox to\hsize{\hfil\box\@tempboxa\hfil}%
         \noexpand\fi
         \vskip\belowcaptionskip
    }%
    %\show\figure@makecaption
}

\long\def\@caption#1[#2]#3{%
  \AddContentsLine
     {\csname ext@#1\endcsname}%
     {#1@toc}%
     {\csname #1toclabel\endcsname}%
     {\ignorespaces #2}%
  \par
  \begingroup
     \@parboxrestore
     \normalsize
     \csname #1@makecaption\endcsname{\ignorespaces #3}\par
  \endgroup
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEIndex}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												% 
% 		Index  					            	% 
%												% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{ItemIndent}
\MakeParam{SubitemIndent}
\MakeIf{Initials}
\MakeParam{NumColumns}
\MakeParam{InitialFont}

\def\Index#1{#1\DefineIndex}

\def\DefineIndex{
  \edef\theindex{%
            \columnseprule \z@
            \columnsep 35\p@
            %\parindent\z@
            \parskip\z@ \@plus .3\p@
            \ifnum\theNumColumns>1
               \output{\the\OldOutput}%
               \noexpand\begin{multicols}{\theNumColumns}%
               \let\noexpand\AP@\relax
               \AP@output{\the\output}%
            \fi
            \let\noexpand\item\noexpand\@idxitem
            \CallName{\theFont}%
            \noexpand\EveryParIndent
  }%
  \ifnum\theNumColumns>1
      \def\endtheindex{\end{multicols}}%
  \else
      \def\endtheindex{\relax}%
  \fi
  \newbox\argbox
  \newcommand\@idxitem{\par\hangindent 40\p@}
  \newcommand\subitem{\par\hangindent 32\p@ \hspace*{10\p@}}
  \newcommand\subsubitem{\par\hangindent 40\p@ \hspace*{30\p@}}
  \newcommand\indexspace{\par \vskip 10\p@ \@plus5\p@ \@minus3\p@\relax}
  \edef\secondary##1##2{\hangindent=\theSubitemIndent\hangafter=0%
      \noexpand\raggedright##1, ##2\par}
  \def\seesecondary##1##2{\hangindent=1em\hangafter=0%
      \raggedright##1\par}
  \def\primary##1{\entry{##1}{}}
  \ifInitials
    \edef\initial##1{\noexpand\bigbreak{\CallName{\theInitialFont}##1}\par}%
  \else
    \def\initial##1{\relax}
  \fi
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SETheorem}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Theorem Statements 						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeIf{UseWithin}
\MakeParam{CounterRef}
\MakeParam{Style}
\MakeParam{BodyFont}
\MakeParam{SkipBefore}
\MakeParam{SkipAfter}
\MakeParam{LeadIn}

\def\Theorem#1{#1\DefineTheorem}
\def\it{\itshape}
\def\TheDocument{Document}

\def\DefLeadin#1{\expandafter\def\csname\theName @leadin\endcsname{#1}}
\def\DefineTheorem{%
   \expandafter\theoremstyle\expandafter{\theStyle}
   \expandafter\theorembodyfont\expandafter{\csname\theBodyFont\endcsname}
   \expandafter\DefLeadin\expandafter{\theLeadIn}
   \ifUseWithin
       \ifx\theCounterRef\TheDocument
           \edef\defineit{\noexpand\newtheorem{\theName}{\expandafter\noexpand\csname\theName @leadin\endcsname}}
       \else
           \edef\defineit{\noexpand\newtheorem{\theName}{\expandafter\noexpand\csname\theName @leadin\endcsname}[\theCounterRef Count]}
       \fi
   \else
       \edef\defineit{\noexpand\newtheorem{\theName}[\theCounterRef]{\theLeadIn}}
   \fi
   \defineit
   \EPrependDefN{\theName}{\vspace{\theSkipBefore}}
   \EAppendDefN{end\theName}{\vspace{\theSkipAfter}}
   \DefN{ini@\theName}{0\relax}% some default initialization for theorem counter
   %\DEBUG{\theName}
   \expandafter\edef\csname p@\theName\endcsname{\expandafter\noexpand\csname the\theName\endcsname}% 
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEBibliog}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Bibliography	  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\MakeParam{EZBibIndent}
   \def\Block{Block}
   \def\Indent{Indent}
   \def\Outdent{Outdent}


\MakeParam{CiteComponents}
\MakeParam{CiteFont}
\MakeParam{LabelComponents}
\MakeParam{LabelFont}
\MakeParam{LabelSep}
\MakeParam{LabelWidth}
\MakeParam{EntryFont}
\MakeParam{ItemIndent}
\MakeParam{ItemSep}
\MakeParam{LeftMar}
\MakeParam{RightMar}
\MakeParam{LineSpacing}
\MakeParam{EntryParagraph}
\MakeIf{Superscript}
\MakeIf{Ranges}
\Superscriptfalse
\Rangesfalse



\def\Bibliography#1{#1\DefineBibliography}
\def\SetSfcode{\sfcode`\.=1000\relax}
\def\NormalLabelForm{\def\makelabel##1{\hss\llap{##1}}}
\def\Remark{\noexpand\if@tempswa , ##2\noexpand\fi}% Used in CiteComponents

\def\DefineBibliography{%
     \edef\thebibliography##1{%
        \noexpand \usecounter{bibitemCount}
        \noexpand \NormalLabelForm    % Use the LabelType parameter to adjust this at some point
        \CallName{\theEntryFont}%
        \CallName{\theEntryParagraph}%
        \itemsep=\theItemSep
        \def\noexpand\newblock{\hskip .11em plus.33em minus.07em}%
        \noexpand\SetSfcode
        \def\noexpand\@biblabel####1{%              Used for explicit label
           {\def\noexpand\p@thebibitemCount{####1}%
            \noexpand\DoParLabelFont\noexpand\DoParLabelComponents}}%
        \def\noexpand\@itemlabel{%                   Used for generated label
           {\noexpand\DoParLabelFont\noexpand\DoParLabelComponents}}%
        \ignorespaces
     }
     \def\endthebibliography{%
        \def\@noitemerr{\@warning{Empty `thebibliography' environment}}%
            \endlist
     }
     %\DEBUG{thebibliography}	
     \def\CiteText{\noexpand\CiteText}
     \ifRanges
       \usepackage{cite}
     \fi
     \edef\@cite##1##2{%
          \def\noexpand\CiteText{##1}% This is used inside CiteComponents
          \ifSuperscript
             $^{\hbox{\CallName{\theCiteFont}\theCiteComponents}}$
          \else
             {\CallName{\theCiteFont}\theCiteComponents}%
          \fi
       }
     \def\@bibitem##1{%
         %\@skiphyperreftrue\H@item\@skiphyperreffalse
		 \if\Undefined{\Hy@Warning}
		 \else
            \hyper@anchorstart{cite.##1}\relax\hyper@anchorend
		 \fi
         %\setbox0=\hbox{##1}%
         %\labelsep=\wd0
         \item
         \if@filesw
		    {%
		       \def\counter@protect{}
               \immediate\write\@auxout
                   {\string\bibcite{##1}{\thebibitemCount}}%
			}%
         \fi
         \ignorespaces
     }
}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEEnviron}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% 		Environments	  						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\MakeParam{Prefix}
\MakeParam{RunIn}
\MakeParam{ParagraphType}
\MakeParam{Font}
\MakeParam{Suffix}
\MakeParam{RunOut}


\def\Environment#1{%
   #1%
   \DefineEnvironment
}

\def\DefineEnvironment{
   \EDefN{\theName}%
        {%
           \SpaceVertically
           \thePrefix%
           \CallName{\theFont}%
           \nobreak
           \CallName{\theParagraphType}%
           \nobreak\par\theRunIn\ignorespaces
        }%
   \EDefN{end\theName}%
        {%
           \theRunOut\endgraf\nobreak\theSuffix\nobreak
        }
  %\DEBUG{\theName}
  %\DEBUG{end\theName}
}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\RequirePackage{SEExceptional}
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%											    %
% 		Exceptional pages 						%
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\MakeParam{PageLayout}
\MakeParam{VerticalMaterial}
\MakeIf{AllowText}
\MakeIf{OverridePgNum}
\MakeParam{NumberType}
\MakeParam{Start}
    \def\NextPage{NextPage}
    \def\NextOddPage{NextOddPage}

\def\Exception#1{#1\DefineException}

\def\DefineException{%
   \EDefN{\theName}{%
 	   \ifx\theStart\NextOddPage
          \noexpand\clearemptydoublepage
       \else
          \noexpand\clearonepage
       \fi
       \ifOverridePgNum
		  \@AdjustPageNumbering{\theNumberType}{T}
       \fi
       \xdef\noexpand\OldPageSetup{\noexpand\CurrentPageSetup}%
       %\noexpand\ChangeOutput
	   \CallName{\thePageLayout}%
	   \noexpand\afterpage{\noexpand\CallName{\noexpand\OldPageSetup}}%
       \begingroup  
            \global\@topnum=0%            
            \if@twocolumn
               \noexpand\twocolumn[\SpaceVertically\null\theVerticalMaterial]%
            \else
               \SpaceVertically
               \theVerticalMaterial
            \fi
       \endgroup
       \ifAllowText
       \else
          \noexpand\clearpage
       \fi
   }%
}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%\usepackage{cite} is used in some cases below

% Some global registers
\newdimen\trimmargin
\newdimen\verttrimmargin
                                       
%
% Error messages
%
\SEDECLAREERROR{NOGENERICFONT}{Missing GenericFont.}{}
\SEDECLAREERROR{NOGENERICPARAGRAPH}{Missing GenericParagraph.}{}
\SEDECLAREERROR{DEPRECATED}{#1 is an obsolete macro.}{WITHARG}

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                  %
% 			Utility Macros	    				   %
%												   %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\today{\ifcase\month\or
  January\or February\or March\or April\or May\or June\or
  July\or August\or September\or October\or November\or December\fi
  \space\number\day, \number\year}
\def\Today{\protect \today}



\def\LineBreak{}% used only as a flag by the style editor

\def\QTP#1{\expandafter\QTPARGS\csname#1\endcsname}
\def\QTPARGS#1{\@ifnextchar[{\QTPOPT#1}{\QTPNOOPT#1}}
\def\QTPOPT#1[#2]#3\par{#1[#2]{#3}}
\def\QTPNOOPT#1#2\par{#1{#2}}

% The TCITag construct
\def\TCItag{\@ifnextchar*{\@TCItagstar}{\@TCItag}}
\def\@TCItag#1{%
    \global\tag@true
    \global\def\@taggnum{(#1)}}
\def\@TCItagstar*#1{%
    \global\tag@true
    \global\def\@taggnum{#1}}


 

% This is to fix a problem where a break occurs after a section head followed
% by a \label.

\newcount\@tempcount
\let\@@label=\label
\def\label#1{\@tempcount=\lastpenalty\@@label{#1}\penalty\@tempcount}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                              %
% Prepare \begin{document} macro               %
%                                              %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\DefAppendLeft{\document}{\CheckStandardMacros}

\DefAppendRight{\document}{
     \if\Undefined{\theGenericFont}
        \SEERRNOGENERICFONT
     \else
        \csname\theGenericFont\endcsname
     \fi
     \if\Undefined{\theGenericParagraph}
        \SEERRNOGENERICPARAGRAPH
     \else
        \csname\theGenericParagraph\endcsname
     \fi
     \if\Undefined{\DefaultPageSetup}
        \SEERRORNOPAGESETUP
     \else
        \DefaultPageSetup
     \fi
     \let\p@TEXTsymbol\TEXTsymbol
     \def\TEXTsymbol#1{\protect\p@TEXTsymbol{#1}}
     \let\p@backslash\backslash
     \def\backslash{\protect\p@backslash}
	 \clearonepage
	 \ifEquationsFlushLeft
	    \advance\mathindent by \theMathIndent
	 \fi	
}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%												%
% Built-in names for use in component lists	    %
%												%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


\def\PageNum{\protect\thepage}
\def\LastLeftMark{\LastPrimaryMark}
\def\FirstRightMark{\FirstSecondaryMark}
\def\LeftBracket{[}
\def\RightBracket{]}
\def\PoundSign{\#}
\def\Percent{\%}
\def\Ampersand{\&}
\def\Underscore{\_}
\def\LeftBrace{\{}
\def\RightBrace{\}}
\def\Backslash{\mbox{$\backslash$}}
\def\Bullet{$\bullet$}
\def\Dash{--}
\def\Star{$\ast$}
\def\Dot{$\cdot$}
\def\LBracket{[}
\def\RBracket{]}
\def\BulletA{$\protect\m@th\bullet$}
\def\BulletB{{\protect\bfseries --}}
\def\BulletC{$\protect\m@th\ast$}
\def\BulletD{$\protect\m@th\cdot$}
\def\BulletE{$\protect\blacktriangleright$}
\def\Tilde{\protect\symbol{126}}
\def\DollarSign{\protect\$}



\def\clearonepage{%
    \if@twocolumn
    \else
       \clearpage
    \fi
}

\def\cleardoublepage{%
    \clearpage
    \ifodd\c@page
    \else
       \if@twocolumn
       \else
         \newpage
       \fi
    \fi
}

\def\clearemptydoublepage{%
     \newpage
     \ifodd\c@page
     \else
        \if@twocolumn
        \else
		   \null
           \thispagestyle{empty}
           \newpage
        \fi
     \fi
}


% This clear page doesn't work with the 
% afterpage package.
%\def\clearpage{%
%  \ifvmode
%    \ifnum \@dbltopnum =\m@ne
%      \ifdim \pagetotal <\topskip
%        %\hbox{}%
%      \fi
%    \fi
%  \fi
%  \newpage
%  %\write\m@ne{}% This causes blank lines in the log file
%  %\vbox to 0pt{}% This causes overfull boxes
%  \penalty -\@Mi
%}




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%										%
%    Defs of names that Sciword uses    %
%                                       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\TableOfContents{\leavevmode\@starttoc{toc}}
\def\tableofcontents{\leavevmode\@starttoc{toc}}
\def\ListOfFigures{\leavevmode\@starttoc{lof}}
\def\ListOfTables{\leavevmode\@starttoc{lot}}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                       %
%    "Standard macros" that	may appear  %
%    in SWP documents but frequently    %
%  	 aren't defined in the style        %
%                                       %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\def\CheckStandardMacros{%
   \DefaultTheorem{acknowledgement}
   \DefaultTheorem{algorithm}
   \DefaultTheorem{axiom}
   \DefaultTheorem{case}
   \DefaultTheorem{claim}
   \DefaultTheorem{conclusion}
   \DefaultTheorem{condition}
   \DefaultTheorem{conjecture}
   \DefaultTheorem{corollary}
   \DefaultTheorem{criterion}
   \DefaultTheorem{definition}
   \DefaultTheorem{example}
   \DefaultTheorem{exercise}
   \DefaultTheorem{lemma}
   \DefaultTheorem{notation}
   \DefaultTheorem{problem}
   \DefaultTheorem{proposition}
   \DefaultTheorem{remark}
   \DefaultTheorem{summary}
   \DefaultTheorem{theorem}
   %\@ifundefined{proof}{\newenvironment{proof}{}{}}{}
}


\def\DefaultTheorem#1{%
   %\expandafter\show\csname #1\endcsname
   \if\UndefinedN{#1}{\newtheorem{#1}{}{}}{}\fi
}
   

  

% Trailer is the last thing called in the style

\def\Trailer#1{%
  \def\theExternalFile{}
  #1%
  \if\Empty{\theExternalFile}
  \else
     \usepackage{\theExternalFile}
  \fi
  \@ifundefined{c@chapterCount}{}{\let\c@chapter=\c@chapterCount}
    \csname\theGenericPageSetup\endcsname
  \let\protect\relax
  \let\qprotect\relax
  \let\counter@protect\relax
}


% More hard-wired default stuff
\def\baselinestretch{1}
\tabcolsep=6pt
\labelsep=6pt % For theorem statements
\doublerulesep=2pt
\def\BodyTextFont{\relax}
\@twocolumnfalse
\@twosidetrue
%\@mparswitchtrue
%\markboth{}{}

% Style editor style needs special endproof

\def\endproof{\mbox{\ \rule{.1in}{.1in}}\if\Not{mmode}\par\fi}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% DEPRECATED MACROS
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%   \def\hangon#1{%
%       \setbox0=\hbox{#1}%
%   	  \hangindent = \wd0%
%   	  \hangafter = 1\relax%
%   }

\def\hangon#1{\SEERRHANGONDEPRECATED}

% \def\SetLineSpace#1{\def\baselinestretch{#1}\huge\normalsize}   % be sure to set font after this.

\def\SetLineSpace#1{\SEERRDEPRECATED{\string\SetLineSpace}}

% \newtoks\thepar 
% \edef\CatchPar{%
%   \def\par{\endgraf\noexpand\CheckLastLine}%
%   %\thepar={\iffalse}\fi%
% }

\def\CatchPar{\SEERRDEPRECATED{\string\CatchPar}}

% \def\CheckLastLine{%
%      \setbox1\lastbox
%      \unskip
%      \box1%
%      \setbox2\hbox{\unhcopy1}%
%      \ifdim\wd2<3em%
%        \setbox3\hbox{}%
%        \CollectPar
%        \global\setbox4\hbox{\unhbox3}%
%        \egroup
%        \noindent
%        \looseness=-1%
%        \unhbox4%
%        \endgraf
%      \else
%        \egroup
%        \noindent
%        \unvbox0%
%      \fi
% }

\def\CheckLastLine{\SEERRDEPRECATED{\string\CheckLastLine}}

% \def\CollectPar{%
%   {\setbox2\lastbox
%    \ifvoid2\else
%      \unskip
%      \unpenalty
%      \CollectPar%
%      \ifvoid3%
%        \global\setbox3\box2%
%      \else
%        \global\setbox3\hbox{\unhbox3\ \unhbox2}%
%      \fi
%    \fi
%   }%
% }

\def\CollectPar{\SEERRDEPRECATED{\string\CollectPar}}

%\newif\ifEZNumbered
%\def\EZNumbered#1{\csname EZNumbered#1\endcsname}
\def\EZNumbered#1{\SEERRDEPRECATED{\string\EZNumbered}}

%\def\NOEXP{\noexpand\noexpand}
\def\NOEXP{\SEERRDEPRECATED{\string\NOEXP}}

%\def\SEEXPAND{\expandafter\eatnoexpand}
\def\SEEXPAND{\SEERRDEPRECATED{\string\SEEXPAND}}

%\def\eatnoexpand#1{}
\def\eatnoexpand#1{\SEERRDEPRECATED{\string\eatnoexpand}}


%\def\SEEXPANDINLINE{\expandafter\eatnoexpandinline}
\def\SEEXPANDINLINE{\SEERRDEPRECATED{\string\SEEXPANDINLINE}}

%\def\eatnoexpandinline#1{{\eatnoexpand#1}}
\def\eatnoexpandinline#1{\SEERRDEPRECATED{\string\eatnoexpandinline}}

%\def\eatblanks#1{#1}
\def\eatblanks#1{\SEERRDEPRECATED{\string\eatblanks}}

\def\protect{\noexpand\protect\noexpand}

% These counters are defined for every style


\Counter{
   \Name{LevelFiveDiv}
   \Mode{0}
   \ResetOn{Document}
   \Appearance{Roman}
   \RefPrefix{}
   \RefFont{BodyTextFont}
   \InitialValue{0}
}
\Counter{
   \Name{LevelFourDiv}
   \Mode{0}
   \ResetOn{Document}
   \Appearance{Roman}
   \RefPrefix{}
   \RefFont{BodyTextFont}
   \InitialValue{0}
}
\Counter{
   \Name{LevelOneDiv}
   \Mode{0}
   \ResetOn{Document}
   \Appearance{Roman}
   \RefPrefix{}
   \RefFont{BodyTextFont}
   \InitialValue{0}
}
\Counter{
   \Name{LevelThreeDiv}
   \Mode{0}
   \ResetOn{Document}
   \Appearance{Roman}
   \RefPrefix{}
   \RefFont{BodyTextFont}
   \InitialValue{0}
}
\Counter{
   \Name{LevelTwoDiv}
   \Mode{0}
   \ResetOn{Document}
   \Appearance{Roman}
   \RefPrefix{}
   \RefFont{BodyTextFont}
   \InitialValue{0}
}

%Define Style Editor macros to correspond to Babel macros (gp)
%First, the standard Babel macros using English
\def\prefacename{Preface}%
\def\refname{References}%
\def\abstractname{Abstract}%
\def\bibname{Bibliography}%
\def\chaptername{Chapter}%
\def\appendixname{Appendix}%
\def\contentsname{Contents}%
\def\listfigurename{List of Figures}%
\def\listtablename{List of Tables}%
\def\indexname{Index}%
\def\figurename{Figure}%
\def\tablename{Table}%
\def\partname{Part}%
\def\enclname{encl}%
\def\ccname{cc}%
\def\headtoname{To}%
\def\pagename{Page}%
\def\seename{see}%
\def\alsoname{see also}%
\def\proofname{Proof}%
\def\glossaryname{Glossary}%
%Now, the SE Babel macros
\def\prefacenameSE{\protect\prefacename}%
\def\refnameSE{\protect\refname}%
\def\abstractnameSE{\protect\abstractname}%
\def\bibnameSE{\protect\bibname}%
\def\chapternameSE{\protect\chaptername}%
\def\appendixnameSE{\protect\appendixname}%
\def\contentsnameSE{\protect\contentsname}%
\def\listfigurenameSE{\protect\listfigurename}%
\def\listtablenameSE{\protect\listtablename}%
\def\indexnameSE{\protect\indexname}%
\def\figurenameSE{\protect\figurename}%
\def\tablenameSE{\protect\tablename}%
\def\partnameSE{\protect\partname}%
\def\enclnameSE{\protect\enclname}%
\def\ccnameSE{\protect\ccname}%
\def\headtonameSE{\protect\headtoname}%
\def\pagenameSE{\protect\pagename}%
\def\seenameSE{\protect\seename}%
\def\alsonameSE{\protect\alsoname}%
\def\proofnameSE{\protect\proofname}%
\def\glossarynameSE{\protect\glossaryname}%

\def\sampleparA{%
   This is some sample text for use in style editor previews.
   This is some sample text for use in style editor previews.
}

\def\sampleparB{%
Why, there, there, there, there! A diamond gone, cost me two thousand ducats
in Frankfort! The curse never fell upon our nation till now; I never felt it
till now. Two thousand ducats in that, and other precious, precious jewels.
I would my daughter were dead at my foot, and the jewels in her ear! Would
she were hearsed at my foot, and the jewels in her coffin! No news of them?
Why, thou loss upon loss! The thief gone with so much, and so much to find
the thief, and no satisfaction, no revenge! Nor no ill luck stirring but
what lights o' my shedding.
}

\ResetEveryPar
\InitEveryPar

\endinput

