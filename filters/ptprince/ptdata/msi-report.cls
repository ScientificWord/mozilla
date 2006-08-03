\newsection{section}{1}{Section \the\sectionnumber}{sectiontitle}%
\newsection{subsection}{2}{\the\sectionnumber.\the\subsectionnumber}{sectiontitle}%
\newsection{subsubsection}{3}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber}{sectiontitle}%
\newsection{paragraph}{4}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%
\newsection{subparagraph}{5}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%

%% Environments

\msi@environment{quotation}
\msi@environment{quote}
\msi@environment{enumerate}
\msi@environment{itemize}
\msi@environment{description}
\msi@environment{verse}
\msi@environment{figure}
\msi@environment{figure*}
\msi@environment{table}
\msi@environment{table*}



\def\textbf#1{\msiopentag{bold}{<bold>}#1\msiclosetag{bold}{</bold>}}
\def\textit#1{\msiopentag{it}{<it>}#1\msiclosetag{it}{</it>}}
\def\textsf#1{\msiopentag{sf}{<sf>}#1\msiclosetag{sf}{</sf>}}
\def\textsl#1{\msiopentag{sl}{<sl>}#1\msiclosetag{sl}{</sl>}}
\def\texttt#1{\msiopentag{tt}{<tt>}#1\msiclosetag{tt}{</tt>}}
\def\textrm#1{\msiopentag{rm}{<rm>}#1\msiclosetag{rm}{</rm>}}
\def\textsc#1{\msiopentag{sc}{<sc>}#1\msiclosetag{sc}{</sc>}}


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
\immediate\write2{@import url("resource://app/res/css/report.css");}
