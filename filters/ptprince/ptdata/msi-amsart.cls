
\newcount\c@MaxMatrixCols

\def\frontmatter{\msitag{<frontmatter/>}}

\def\title{%
  \@ifnextchar[{\@titlea}{\@titleb}}

\def\@titlea[#1]#2{%
  \msitag{<title><short>#1</short><long>#2</long></title>}%
}

\def\@titleb#1{\@titlea[]{#1}}

\def\author{%
  \@ifnextchar[{\@authora}{\@authorb}}

\def\@authora[#1]#2{%
  \msitag{<author><authorid>#1</authorid>#2</author>}%
}

\def\@authorb#1{\@authora[]{#1}}

\def\address{\@ifnextchar[{\@addressa}{\@addressb}}

\def\@addressa[#1]#2{%
  \msitag{^^0a}%
  \msiopentag{address}{<address>}%
     \msiopentag{authorid}{<authorid>}%
     #1
     \msiclosetag{authorid}{</authorid>}%
  #2
  \msiclosetag{address}{</address>}%
}

\def\@addressb#1{\@addressa[]{#1}}


\def\curraddr{\@ifnextchar[{\@curraddra}{\@curraddrb}}

\def\@curraddra[#1]#2{%
  \msitag{<curraddr><authorid>#1</authorid>}#2\msitag{</curraddr>}%
}

\def\@curraddrb#1{\@curraddra[]{#1}}


\def\email{\@ifnextchar[{\@emaila}{\@emailb}}

\def\@emaila[#1]#2{%
  \msitag{<email><authorid>#1</authorid>}#2\msitag{</email>}%
}

\def\@emailb#1{\@emaila[]{#1}}



\def\urladdr{\@ifnextchar[{\@urladdra}{\@urladdrb}}

\def\@urladdra[#1]#2{%
  \msitag{<urladdr><authorid>#1</authorid>}#2\msitag{</urladdr>}%
}

\def\@urladdrb#1{\@urladdra[]{#1}}

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
%\immediate\write2{@import url("resource://app/res/css/article.css");}

\def\classcss{@import url("resource://app/res/css/amsart.css");}
