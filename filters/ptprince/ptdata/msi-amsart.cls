

% The AMS logo

\def\AmS{<ams>AmS</ams>}



%% Frontmatter commands

\MSIEmptyElement{frontmatter}
\MSIEmptyElement{tableofcontents}
\MSIEmptyElement{maketitle}

% Use default \title 

\def\author{\@ifnextchar[{\@authora}{\@authorb}}

\def\@authora[#1]#2{%
  \msiopentag{author}{<author>}
    \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
    #2%
  \msiclosetag{author}{</author>}}

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


\input amsmath.tex

\msi@extrapi{%
  %\msitag{<?sw-tagdefs href="resource://app/res/tagdefs/revtex4-1defs.xml" type="text/xml" ?>^^0a}
  \msitag{<?sw-xslt href="amscls.xsl" type="text/xml" ?>^^0a}
}

\def\classcss{@import url("resource://app/res/css/amsart.css");}
\makeatother
