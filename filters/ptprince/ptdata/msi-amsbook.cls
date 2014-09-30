
% The AMS logo

\def\AmS{<ams>AmS</ams>}



%% Frontmatter commands

\MSIEmptyElement{frontmatter}{T}
\MSIEmptyElement{tableofcontents}{T}
\MSIEmptyElement{maketitle}{T}

% Use default \title 

\def\author{\@ifnextchar[{\@authora}{\@authorb}}

\def\@authora[#1]#2{%
  \msiopentag{author}{<author>}
    \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
    #2%
  \msiclosetag{author}{</author>}}

\def\@authorb#1{%
  \msiopentag{author}{<author>}#1\msiclosetag{author}{</author>}}

\def\address{\@ifnextchar[{\@addressa}{\@addressb}}

\def\@addressa[#1]#2{%
  \msiopentag{address}{<address>}%
     \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
  #2%
  \msiclosetag{address}{</address>}%
}

\def\@addressb#1{
  \msiopentag{address}{<address>}#1\msiclosetag{address}{</address>}}

\def\curraddr{\@ifnextchar[{\@curraddra}{\@curraddrb}}

\def\@curraddra[#1]#2{%
  \msiopentag{curraddr}{<curraddr>}%
     \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
  #2%
  \msiclosetag{curraddr}{</curraddr>}%
}

\def\@curraddrb#1{
  \msiopentag{curraddr}{<curraddr>}#1\msiclosetag{curraddr}{</curraddr>}}


\def\email{\@ifnextchar[{\@emaila}{\@emailb}}

\def\@emaila[#1]#2{%
  \msiopentag{email}{<email>}%
     \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
  #2%
  \msiclosetag{email}{</email>}%
}

\def\@emailb#1{
  \msiopentag{email}{<email>}#1\msiclosetag{email}{</email>}}

\def\urladdr{\@ifnextchar[{\@urladdra}{\@urladdrb}}

\def\@urladdra[#1]#2{%
  \msiopentag{urladdr}{<urladdr>}%
     \msiopentag{authorid}{<authorid>}#1\msiclosetag{authorid}{</authorid>}%
  #2%
  \msiclosetag{urladdr}{</urladdr>}%
}

\def\@urladdrb#1{
  \msiopentag{urladdr}{<urladdr>}#1\msiclosetag{urladdr}{</urladdr>}}


\def\thanks#1{\msitag{<thanks>}#1\msitag{</thanks>}}

\def\dedicatory#1{
  \msiopentag{dedicatory}{<dedicatory>}#1\msiclosetag{dedicatory}{</dedicatory>}}

\def\translator#1{
  \msiopentag{translator}{<translator>}#1\msiclosetag{translator}{</translator>}}

\def\keywords#1{
  \msiopentag{keywords}{<keywords>}#1\msiclosetag{keywords}{</keywords>}}

\def\subjclass{\@ifnextchar[{\@subjclassa}{\@subjclassb}}

\def\@subjclassa[#1]#2{%
  \msiopentag{subjclass}{<subjclass>}%
     \msiopentag{subjclassyear}{<subjclassyear>}#1\msiclosetag{subjclassyear}{</subjclassyear>}%
  #2%
  \msiclosetag{subjclass}{</subjclass>}%
}

\def\@subjclassb#1{
  \msiopentag{subjclass}{<subjclass>}#1\msiclosetag{subjclass}{</subjclass>}}

\input amsmath.tex

\msi@extrapi{%
  \msitag{<?sw-tagdefs href="resource://app/res/tagdefs/amsclsdefs.xml" type="text/xml" ?>^^0a}
  \msitag{<?sw-xslt href="amscls.xsl" type="text/xml" ?>^^0a}
}

\def\classcss{@import url("resource://app/res/css/amsbook.css");}
\makeatother
