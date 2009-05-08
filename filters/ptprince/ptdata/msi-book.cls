


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





%\immediate\openout2={my.css}
%\immediate\write2{@import url("resource://app/res/css/book.css");}

\def\classcss{@import url("resource://app/res/css/book.css");}
