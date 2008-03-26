


%% Sectioning

\newsection{part}{1}{Part \the\sectionnumber}{sectiontitle}%
\newsection{chapter}{2}{Chapter \the\chapternumber}{sectiontitle}
\newsection{section}{2}{Section \the\sectionnumber}{sectiontitle}%
\newsection{subsection}{3}{\the\sectionnumber.\the\subsectionnumber}{sectiontitle}%
\newsection{subsubsection}{4}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber}{sectiontitle}%
\newsection{paragraph}{5}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%
\newsection{subparagraph}{6}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%

%% Environments



%% Frontmatter commands

\msi@pass@newline{\frontmatter}
\msi@pass@newline{\tableofcontents}
\msi@pass@newline{\maketitle}

%% Miscellaneous commands

\msi@pass{\textquotedblleft}
\msi@pass{\textquotedblright}
\msi@pass{\textbackslash}

\msi@pass{\allowbreak}
\msi@pass{\epsilon}
\msi@pass{\cup}
\msi@pass{\cap}
\msi@pass{\gcd}

\msi@pass{\int}
\msi@pass{\dint}
\msi@pass{\sum}
\msi@pass{\infty}
\msi@pass{\neq}
\msi@pass{\leq}
\msi@pass{\geq}
\msi@pass{\det}
\msi@pass{\lim}


\msi@pass{\lfloor}
\msi@pass{\rfloor}
\msi@pass{\lceil}
\msi@pass{\rceil}

\msi@pass{\sin}
\msi@pass{\cos}

\msi@pass{\alpha}
\msi@pass{\beta}
\msi@pass{\gamma}
\msi@pass{\delta}


\msi@pass{\mu}
\msi@pass{\pi}
\msi@pass{\theta}

\msi@pass{\left}
\msi@pass{\right}
\msi@pass{\in}

\msi@pass{\vert}
\msi@pass{\Vert}


\msi@pass{\blacktriangleright}
\msi@pass{\max}
\msi@pass{\min}
\msi@pass{\prime}
\msi@pass{\circ}


\msi@pass{\Delta}
\msi@pass{\Gamma}

\msi@pass{\nabla}

\msi@pass{\times}
\msi@pass{\rightarrow}




\def\textbf#1{\msiopentag{bold}{<bold>}#1\msiclosetag{bold}{</bold>}}
\def\textit#1{\msiopentag{it}{<it>}#1\msiclosetag{it}{</it>}}
\def\textsf#1{\msiopentag{sf}{<sf>}#1\msiclosetag{sf}{</sf>}}
\def\textsl#1{\msiopentag{sl}{<sl>}#1\msiclosetag{sl}{</sl>}}
\def\texttt#1{\msiopentag{tt}{<tt>}#1\msiclosetag{tt}{</tt>}}
\def\textrm#1{\msiopentag{rm}{<rm>}#1\msiclosetag{rm}{</rm>}}
\def\textsc#1{\msiopentag{sc}{<sc>}#1\msiclosetag{sc}{</sc>}}


\@text@tag{it}
\@text@tag{normalsize}
\@text@tag{tiny}
\@text@tag{small}
\@text@tag{Large}
\@text@tag{large}
\@text@tag{LARGE}
\@text@tag{huge}
\@text@tag{Huge}
\@text@tag{scriptsize}
\@text@tag{footnotesize}

\immediate\openout2={my.css}
\immediate\write2{@import url("resource://app/res/css/book.css");}
