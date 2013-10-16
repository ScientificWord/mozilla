
%\title{Manuscript Title:\\with Forced Linebreak}% Force line breaks with \\
%(gp) Use underlying LaTeX definition
%Looks like optional parameter (like AMS styles) to title has been removed with REVTeX4-1.

%\date{\today}
%(gp) Use underlying LaTeX definition

%\author{\firstname{Ann} \surname{Author}}
\def\author#1{\out@author{#1}}
\def\out@author#1{%
   {\everypar={}\msitag{^^0a}\msiopentag{author}{<author>}#1\msiclosetag{author}{</author>}}}

\def\firstname#1{\out@firstname{#1}}
\def\out@firstname#1{%
   {\everypar={}\msitag{^^0a}\msiopentag{firstname}{<firstname>}#1\msiclosetag{firstname}{</firstname>}}}

\def\surname#1{\out@surname{#1}}
\def\out@surname#1{%
   {\everypar={}\msitag{^^0a}\msiopentag{surname}{<surname>}#1\msiclosetag{surname}{</surname>}}}

%\affiliation{%
% Authors' institution and/or address\\
% This line break forced with \textbackslash\textbackslash
%}%
\def\affiliation#1{\out@affiliation{#1}}
\def\out@affiliation#1{%
   \msitag{^^0a}\msiopentag{affiliation}{<affiliation>}#1\msiclosetag{affiliation}{</affiliation>}}

%\altaffiliation[Also at ]{Physics Department, XYZ University.}%Lines break automatically or can be forced with \\
\def\altaffiliation{%
  \@ifnextchar[{\@altaffiliationa}{\@altaffiliationb}}
\def\@altaffiliationa[#1]#2{%
  \msitag{<altaffiliation alt="#1">#2</altaffiliation>}%
}
\def\@altaffiliationb#1{\@altaffiliationa[]{#1}}

%\collaboration{MUSO Collaboration}%\noaffiliation
\def\collaboration#1{\out@collaboration{#1}}
\def\out@collaboration#1{%
   \msitag{^^0a}\msiopentag{collaboration}{<collaboration>}#1\msiclosetag{collaboration}{</collaboration>}}

%\noaffiliation
\MSIEmptyElement{noaffiliation}

%\email[optional text]{Second.Author@institution.edu}
\def\email{%
  \@ifnextchar[{\@emaila}{\@emailb}}
\def\@emaila[#1]#2{%
  \msitag{<email alt="#1">#2</email>}%
}
\def\@emailb#1{\@emaila[]{#1}}

%\homepage[optional text]{Second.Author@institution.edu}
\def\homepage{%
  \@ifnextchar[{\@homepagea}{\@homepageb}}
\def\@homepagea[#1]#2{%
  \msitag{<homepage alt="#1">#2</homepage>}%
}
\def\@homepageb#1{\@homepagea[]{#1}}

%\thanks{A footnote to the article title}%
%(gp) Use underlying LaTeX definition

%\begin{abstract}...\end{abstract}
%(gp) Use underlying LaTeX definition

%\pacs{PACS number}
\def\pacs#1{\out@pacs{#1}}
\def\out@pacs#1{%
   \msitag{^^0a}\msiopentag{pacs}{<pacs>}#1\msiclosetag{pacs}{</pacs>}}

%\keywords{one two three}
\def\keywords#1{\out@keywords{#1}}
\def\out@keywords#1{%
   \msitag{^^0a}\msiopentag{keywords}{<keywords>}#1\msiclosetag{keywords}{</keywords>}}

%\preprint{HEP/123-qed}
\def\preprint#1{\out@preprint{#1}}
\def\out@preprint#1{%
   {\everypar={}\msitag{^^0a}\msiopentag{preprint}{<preprint>}#1\msiclosetag{preprint}{</preprint>}}}

%\volumeyear{year}
\def\volumeyear#1{\out@volumeyear{#1}}
\def\out@volumeyear#1{%
   \msitag{^^0a}\msiopentag{volumeyear}{<volumeyear>}#1\msiclosetag{volumeyear}{</volumeyear>}}

%\volumenumber{number}
\def\volumenumber#1{\out@volumenumber{#1}}
\def\out@volumenumber#1{%
   \msitag{^^0a}\msiopentag{volumenumber}{<volumenumber>}#1\msiclosetag{volumenumber}{</volumenumber>}}

%\issuenumber{number}
\def\issuenumber#1{\out@issuenumber{#1}}
\def\out@issuenumber#1{%
   \msitag{^^0a}\msiopentag{issuenumber}{<issuenumber>}#1\msiclosetag{issuenumber}{</issuenumber>}}

%\eid{identifier}
\def\eid#1{\out@eid{#1}}
\def\out@eid#1{%
   \msitag{^^0a}\msiopentag{eid}{<eid>}#1\msiclosetag{eid}{</eid>}}

%\startpage{page #}
\def\startpage#1{\out@startpage{#1}}
\def\out@startpage#1{%
   \msitag{^^0a}\msiopentag{startpage}{<startpage>}#1\msiclosetag{startpage}{</startpage>}}

%\endpage{page #}
\def\endpage#1{\out@endpage{#1}}
\def\out@endpage#1{%
   \msitag{^^0a}\msiopentag{endpage}{<endpage>}#1\msiclosetag{endpage}{</endpage>}}

%%\accepted[Accepted text]{date}
%%Alternate version, but Jon says the one used is better
%\def\accepted{%
%  \@ifnextchar[{\@accepteda}{\@acceptedb}}
%\def\@accepteda[#1]#2{%
%  \msitag{<accepted alt="#1">#2</accepted>}}%
%\def\@acceptedb#1{\msitag{<accepted>#1</accepted>}}%

%\accepted[Accepted text]{date}
\def\accepted{%
  \@ifnextchar[{\@accepteda}{\@acceptedb}}
\def\@accepteda[#1]#2{%
  \msiopentag{accepted}{<accepted xalt="#1">}#2\msiclosetag{accepted}{</accepted>^^0a}}%
\def\@acceptedb#1{\msiopentag{accepted}{<accepted>}#1\msiclosetag{accepted}{</accepted>^^0a}}%

%\published[Published text]{date}
\def\published{%
  \@ifnextchar[{\@publisheda}{\@publishedb}}
\def\@publisheda[#1]#2{%
  \msiopentag{published}{<published xalt="#1">}#2\msiclosetag{published}{</published>^^0a}}%
\def\@publishedb#1{\msiopentag{published}{<published>}#1\msiclosetag{published}{</published>^^0a}}%

%\received[Received text]{date}
\def\received{%
  \@ifnextchar[{\@receiveda}{\@receivedb}}
\def\@receiveda[#1]#2{%
  \msiopentag{received}{<received xalt="#1">}#2\msiclosetag{received}{</received>^^0a}}%
\def\@receivedb#1{\msiopentag{received}{<received>}#1\msiclosetag{received}{</received>^^0a}}%

%\revised[Revised text]{date}
\def\revised{%
  \@ifnextchar[{\@reviseda}{\@revisedb}}
\def\@reviseda[#1]#2{%
  \msiopentag{revised}{<revised xalt="#1">}#2\msiclosetag{revised}{</revised>^^0a}}%
\def\@revisedb#1{\msiopentag{revised}{<revised>}#1\msiclosetag{revised}{</revised>^^0a}}%

% Handle the REVTeX widetext environment
\MSISimpleEnvironment{widetext}

% Handle the REVTeX acknowledgments environment
\MSISimpleEnvironment{acknowledgments}

\msi@extrapi{%
  \msitag{<?sw-tagdefs href="resource://app/res/tagdefs/revtex4-1defs.xml" type="text/xml" ?>^^0a}
  \msitag{<?sw-xslt href="revtex4-1.xsl" type="text/xml" ?>^^0a}
}

\def\classcss{@import url("resource://app/res/css/revtex4-1.css");}

\endinput
