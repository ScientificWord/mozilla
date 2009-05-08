\newsection{section}{1}{Section \the\sectionnumber}{sectiontitle}%
\newsection{subsection}{2}{\the\sectionnumber.\the\subsectionnumber}{sectiontitle}%
\newsection{subsubsection}{3}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber}{sectiontitle}%
\newsection{paragraph}{4}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%
\newsection{subparagraph}{5}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%







%%\immediate\openout2={my.css}
%%\immediate\write2{@import url("resource://app/res/css/report.css");}

\def\classcss{@import url("resource://app/res/css/report.css");}
