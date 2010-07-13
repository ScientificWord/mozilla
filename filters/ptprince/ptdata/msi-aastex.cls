

%% Sectioning

\newsection{part}{1}{Part \the\sectionnumber}{sectiontitle}%
\newsection{section}{2}{Section \the\sectionnumber}{sectiontitle}%
\newsection{subsection}{3}{\the\sectionnumber.\the\subsectionnumber}{sectiontitle}%
\newsection{subsubsection}{4}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber}{sectiontitle}%
\newsection{paragraph}{5}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%
\newsection{subparagraph}{6}{\thesectionnumber.\thesubsectionnumber.\the\subsubsectionnumber.\the\paragraphnumber}{sectiontitle}%

%% Environments


%%

\MSIMathQuote{sun}
\MSIMathQuote{earth}


\MSIEmptyElement{micron}
\MSIEmptyElement{sbond}
\MSIEmptyElement{dbond}
\MSIEmptyElement{tbond}


\MSITextElement{object}
\MSITextElement{objectname}





%% Finish up

\def\classcss{@import url("resource://app/res/css/article.css");}

\makeatother