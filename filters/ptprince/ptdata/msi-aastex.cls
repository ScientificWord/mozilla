

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



%% Finish up

\def\classcss{@import url("resource://app/res/css/article.css");}

\makeatother