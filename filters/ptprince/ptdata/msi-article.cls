

%% Sectioning

\newsection{part}{1}{Part \the\sectionnumber}{sectiontitle}%
\newsection{section}{2}{Section \the\sectionnumber}{sectiontitle}%
\newsection{subsection}{3}{\the\sectionnumber.\the\subsectionnumber}{sectiontitle}%
\newsection{subsubsection}{4}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber}{sectiontitle}%
\newsection{paragraph}{5}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%
\newsection{subparagraph}{6}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%

%% Environments

%\newenvironment{center}
%\newenvironment{flushleft}
%\newenvironment{flushright}
%\newenvironment{quote}
%\newenvironment{quotation}
%% Handled separately? (gp) {verbatim}
%% Handled separately? (gp) {verbatim*}
%\newenvironment{enumerate}
%\newenvironment{itemize}
%\newenvironment{description}
%\newenvironment{verse}
%\newenvironment{figure}
%\newenvironment{figure*}
%\newenvironment{table}
%\newenvironment{table*}
%\newenvironment{titlepage}
%\newenvironment{thebibliography}
%\newenvironment{theindex}

%% Text tags

\def\textbf#1{\msiopentag{bold}{<bold>}#1\msiclosetag{bold}{</bold>}}
\def\textit#1{\msiopentag{it}{<it>}#1\msiclosetag{it}{</it>}}
\def\textsf#1{\msiopentag{sf}{<sf>}#1\msiclosetag{sf}{</sf>}}
\def\textsl#1{\msiopentag{sl}{<sl>}#1\msiclosetag{sl}{</sl>}}
\def\texttt#1{\msiopentag{tt}{<tt>}#1\msiclosetag{tt}{</tt>}}
\def\textrm#1{\msiopentag{rm}{<rm>}#1\msiclosetag{rm}{</rm>}}
\def\textsc#1{\msiopentag{sc}{<sc>}#1\msiclosetag{sc}{</sc>}}

\@text@tag{tiny}
\@text@tag{scriptsize}
\@text@tag{footnotesize}
\@text@tag{small}
\@text@tag{normalsize}
\@text@tag{large}
\@text@tag{Large}
\@text@tag{LARGE}
\@text@tag{huge}
\@text@tag{Huge}


%% Finish up

\immediate\openout2={css/doc.css}
\immediate\write2{@import url("resource://app/res/css/article.css");}

\makeatother