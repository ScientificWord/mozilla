PK    �F3�               css/PK    ���>���  �  
   css/my.css@import url(resource://app/res/css/latex_internal.css);
@import url(resource://app/res/css/baselatex.css);
@import url(resource://app/res/css/latex.css);
@import url(resource://app/res/css/article.css);

/*  The body selector is where you can change the base font of the
    entire document */
body{}
  
/*  The following group determines the formatting of the titles of 
    the various document parts, and whether they are preceded by numbers */
    
part>sectiontitle{}  
part>sectiontitle:before{}
chapter>sectiontitle{}  
chapter>sectiontitle:before{}
section>sectiontitle{}  
section>sectiontitle:before{}
subsection>sectiontitle{}  
subsection>sectiontitle:before{}
subsubsection>sectiontitle{}  
subsubsection>sectiontitle:before{}
paragraph>sectiontitle{}  
paragraph>sectiontitle:before{}

/* The following let you change fonts, styles, margins, etc. for
   the standard paragraph-like items */
bodyMath{}
bodyText{}
caption{}
epigraph{}
flushleft{}
flushright{}
frontmatter{}
longQuotation{}
plotcaption{}  
preface{}
rtlBodyText{}
shortQuote{}
table{}
verbatim{}
  
/* The following are the text tags. You can change font, size, color,
   and other text attributes like (horrors!) making the text blink. */
blackboardBold{}
bold{}
calligraphic{}
emphasized{}
footnotesize{}
fraktur{}
Huge{}
huge{}
italics{}
LARGE{}
Large{}
large{}
phantom{}
pre{}
roman{}
rtl{}
sansSerif{}
scriptsize{}
slanted{}
smallCaps{}
small{}
tiny{}
typewriter{}

/*  The following selectors allow you to modify the appearance of list
    items depending on the depth of the nested lists */  
bulletlist bulletlistItem{}
bulletlist bulletlist bulletlistItem{}
bulletlist bulletlist bulletlist bulletlistItem{}
bulletlist bulletlist bulletlist bulletlist bulletlistItem{}
numberedlist numberedlistItem{}
numberedlist numberedlist numberedlistItem{}
numberedlist numberedlist numberedlist numberedlistItem{}
numberedlist numberedlist numberedlist numberedlist bulletlistItem{}
numberedlist numberedlistItem:before{}
numberedlist numberedlist numberedlistItem:before{}
numberedlist numberedlist numberedlist numberedlistItem:before{}
numberedlist numberedlist numberedlist numberedlist numberedlistItem:before{}

/*  Selectors for standard document parts */    
address{}
author{}
date{}
title{}
  
/*  Selectors for theorem-like environments */
acknowledgement{}  
algorithm{}
assertion{}
assumption{}
axiom{}
case{}
claim{}
conclusion{}
condition{}
conjecture{}
corollary{}
criterion{}
definition{}
example{}
exercise{}
hypothesis{}
lemma{}
notation{}
problem{}
proof{}
property{}
proposition{}
question{}
remark{}
summary{}
theorem{}
  
/*  Selectors that let you choose the lead-ins for these environments */
  
acknowledgement>*:first-child:before{}  
algorithm>*:first-child:before{}
assertion>*:first-child:before{}
assumption>*:first-child:before{}
axiom>*:first-child:before{}
case>*:first-child:before{}
claim>*:first-child:before{}
conclusion>*:first-child:before{}
condition>*:first-child:before{}
conjecture>*:first-child:before{}
corollary>*:first-child:before{}
criterion>*:first-child:before{}
definition>*:first-child:before{}
example>*:first-child:before{}
exercise>*:first-child:before{}
hypothesis>*:first-child:before{}
lemma>*:first-child:before{}
notation>*:first-child:before{}
problem>*:first-child:before{}
proof>*:first-child:before{}
property>*:first-child:before{}
proposition>*:first-child:before{}
question>*:first-child:before{}
remark>*:first-child:before{}
summary>*:first-child:before{}
theorem>*:first-child:before{}
PK    skQ�75VE  E     css/msi_Tags.cssHuge:before {
  content: 'Huge';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


LARGE:before {
  content: 'LARGE';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


Large:before {
  content: 'Large';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


alt:before {
  content: 'alt';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


blackboardBold:before {
  content: 'blackboardBold';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


bold:before {
  content: 'bold';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


boldsymbol:before {
  content: 'boldsymbol';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


calligraphic:before {
  content: 'calligraphic';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


descriptionLabel:before {
  content: 'descriptionLabel';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


drop:before {
  content: 'drop';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


emphasized:before {
  content: 'emphasized';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


footnotesize:before {
  content: 'footnotesize';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


fraktur:before {
  content: 'fraktur';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


huge:before {
  content: 'huge';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


italics:before {
  content: 'italics';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


large:before {
  content: 'large';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


lower:before {
  content: 'lower';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


ltxframe:before {
  content: 'ltxframe';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


normalsize:before {
  content: 'normalsize';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


phantom:before {
  content: 'phantom';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


roman:before {
  content: 'roman';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


rtl:before {
  content: 'rtl';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


sansSerif:before {
  content: 'sansSerif';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


scriptsize:before {
  content: 'scriptsize';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


slanted:before {
  content: 'slanted';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


small:before {
  content: 'small';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


smallCaps:before {
  content: 'smallCaps';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


smallbox:before {
  content: 'smallbox';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


sub:before {
  content: 'sub';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


sup:before {
  content: 'sup';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


tiny:before {
  content: 'tiny';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


typewriter:before {
  content: 'typewriter';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


underline:before {
  content: 'underline';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


upper:before {
  content: 'upper';
  font: bold small sans-serif;
  background-color: yellow;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


bodyMath:before {
  content: 'bodyMath';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


bodyText:before {
  content: 'bodyText';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


centered:before {
  content: 'centered';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


flushleft:before {
  content: 'flushleft';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


flushright:before {
  content: 'flushright';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


p:before {
  content: 'p';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


rtlBodyText:before {
  content: 'rtlBodyText';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


sectiontitle:before {
  content: 'sectiontitle';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


shortQuote:before {
  content: 'shortQuote';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


verbatim:before {
  content: 'verbatim';
  background-color: rgb(255,200,200);
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


bibitem:before {
  content: 'bibitem';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


bulletListItem:before {
  content: 'bulletListItem';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


descriptionListItem:before {
  content: 'descriptionListItem';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


numberedListItem:before {
  content: 'numberedListItem';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


chapter:before {
  content: 'chapter';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


paragraph:before {
  content: 'paragraph';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


part:before {
  content: 'part';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


section:before {
  content: 'section';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


subparagraph:before {
  content: 'subparagraph';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


subsection:before {
  content: 'subsection';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


subsubsection:before {
  content: 'subsubsection';
  color: green;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


algorithm:before {
  content: 'algorithm';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


assertion:before {
  content: 'assertion';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


assumption:before {
  content: 'assumption';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


axiom:before {
  content: 'axiom';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


case:before {
  content: 'case';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


centeredEnv:before {
  content: 'centeredEnv';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


claim:before {
  content: 'claim';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


conclusion:before {
  content: 'conclusion';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


condition:before {
  content: 'condition';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


conjecture:before {
  content: 'conjecture';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


corollary:before {
  content: 'corollary';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


criterion:before {
  content: 'criterion';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


dedication:before {
  content: 'dedication';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


definition:before {
  content: 'definition';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


example:before {
  content: 'example';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


exercise:before {
  content: 'exercise';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


glossary:before {
  content: 'glossary';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


hypothesis:before {
  content: 'hypothesis';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


lemma:before {
  content: 'lemma';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


longQuotation:before {
  content: 'longQuotation';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


msiframe:before {
  content: 'msiframe';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


notation:before {
  content: 'notation';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


note:before {
  content: 'note';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


preface:before {
  content: 'preface';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


problem:before {
  content: 'problem';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


proof:before {
  content: 'proof';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


property:before {
  content: 'property';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


proposition:before {
  content: 'proposition';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


question:before {
  content: 'question';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


remark:before {
  content: 'remark';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


solution:before {
  content: 'solution';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


summary:before {
  content: 'summary';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


theorem:before {
  content: 'theorem';
  color: teal;
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


abstract:before {
  content: 'abstract';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


author:before {
  content: 'author';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


date:before {
  content: 'date';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


title:before {
  content: 'title';
  background-color: silver;
  font: bold small sans-serif;
  border: thin solid black;
  padding: 2px;
  -moz-border-radius: 4px;
}


PK    skQ8��̬  �  
   main.xhtml<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet href="css/my.css" type="text/css"?>
<?sw-tagdefs href="resource://app/res/tagdefs/latexdefs.xml" type="text/xml" ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:mml="http://www.w3.org/1998/Math/MathML">
  <head><sw-meta id="sw-meta" product="Scientific WorkPlace" version="2020090317" created="Wed Mar 01 2017 11:41:53 GMT-0700 (MST)" lastrevised="Wed Nov 11 2020 14:24:56 GMT-0500 (EST)"/>
    <preamble hide="true">
      <documentclass class="article"/>
      <requirespackage package="amsfonts" pri="010"/>
      <requirespackage req="amsmath" pri="010"/>
      <preambleTeX><![CDATA[\newtheorem {theorem}{Theorem}
\newtheorem{acknowledgment}[theorem]{Acknowledgment}
\newtheorem{algorithm}[theorem]{Algorithm}
\newtheorem{assertion}[theorem]{Assertion}
\newtheorem{assumption}[theorem]{Assumption}
\newtheorem{axiom}[theorem]{Axiom}
\newtheorem{case}[theorem]{Case}
\newtheorem{claim}[theorem]{Claim}
\newtheorem{conclusion}[theorem]{Conclusion}
\newtheorem{condition}[theorem]{Condition}
\newtheorem{conjecture}[theorem]{Conjecture}
\newtheorem{corollary}[theorem]{Corollary}
\newtheorem{criterion}[theorem]{Criterion}
\newtheorem{definition}[theorem]{Definition}
\newtheorem{example}[theorem]{Example}
\newtheorem{exercise}[theorem]{Exercise}
\newtheorem{hypothesis}[theorem]{Hypothesis}
\newtheorem{lemma}[theorem]{Lemma}
\newtheorem{notation}[theorem]{Notation}
\newtheorem{note}[theorem]{Note}
\newtheorem{problem}[theorem]{Problem}
\newtheorem{property}[theorem]{Property}
\newtheorem{proposition}[theorem]{Proposition}
\newtheorem{question}[theorem]{Question}
\newtheorem{remark}[theorem]{Remark}
\newtheorem{summary}[theorem]{Summary}
\newenvironment {proof}[1][Proof]{\noindent \textbf {#1.} }{\ \rule {0.5em}{0.5em}}]]></preambleTeX>
    </preamble>
  </head>
  <body showexpanders="true" showfmbuttons="true" showshort="true">
    <title xmlns="http://www.w3.org/1999/xhtml">Standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> Article</title>
    <author>A. U. Thor<msibr invisDisplay="&amp;#x21b5;" type="newLine"/>University of Stewart Island</author>
    <maketitle/><br/>
    <section xmlns="http://www.w3.org/1999/xhtml" id="tsid_3141594">
      <sectiontitle>Standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> Article</sectiontitle>
      <bodyText>This document illustrates the appearance of an article created with the shell <bold>
Standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> Article</bold>  or the shell <bold>
Blank - Standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> Article</bold>. Both shells produce
documents with centered title information, left-justified headings, theorem environments, and appendices.</bodyText>
      <bodyText>The standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> shells provide the most general and portable set of document features. You can achieve almost any typesetting effect by beginning with a standard shell and adding 	<texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> packages as necessary.</bodyText>
      <bodyText>The document class base file for this shell is <typewriter>article.cls</typewriter>.</bodyText>
    </section>
  </body>
</html>PK     �F3�                            css/PK     ���>���  �  
             "   css/my.cssPK     skQ�75VE  E                 css/msi_Tags.cssPK     skQ8��̬  �  
             MS  main.xhtmlPK      �   !a    