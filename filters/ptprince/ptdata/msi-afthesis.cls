
\def\address#1{\out@address{#1}}
\def\rank#1{\out@rank{#1}}
\def\designator#1{\out@designator{#1}}
\def\previousdegrees#1{\out@previousdegrees{#1}}
\def\degree#1{\out@degree{#1}}
\def\school#1{\out@school{#1}}
\def\committee#1{\out@committee{#1}}
\def\abbreviation#1{\out@abbreviation{#1}}
%\def\numberbychapter{<numberbychapter/>}
%\def\flyleaf{<flyleaf/>}
%\def\disclaimerpage{<disclaimerpage/>}
%\def\titlepage{<titlepage/>}
%\def\approvalpage{<approvalpage/>}
%\def\listoffigures{<listoffigures/>}
%\def\listoftables{<listoftables/>}
%\def\listofsymbols{<listofsymbols/>}
%\def\listofabbreviations{<listofabbreviations/>}


\def\out@address#1{%
   \msitag{^^0a}\msiopentag{address}{<address>}#1\msiclosetag{address}{</address>}}

\def\out@rank#1{%
   \msitag{^^0a}\msiopentag{rank}{<rank>}#1\msiclosetag{rank}{</rank>}}

\def\out@designator#1{%
   \msitag{^^0a}\msiopentag{designator}{<designator>}#1\msiclosetag{designator}{</designator>}}

\def\out@previousdegrees#1{%
   \msitag{^^0a}\msiopentag{previousdegrees}{<previousdegrees>}#1\msiclosetag{previousdegrees}{</previousdegrees>}}

\def\out@degree#1{%
   \msitag{^^0a}\msiopentag{degree}{<degree>}#1\msiclosetag{degree}{</degree>}}

\def\out@school#1{%
   \msitag{^^0a}\msiopentag{school}{<school>}#1\msiclosetag{school}{</school>}}

\def\out@committee#1{%
   \msitag{^^0a}\msiopentag{committee}{<committee>}#1\msiclosetag{committee}{</committee>}}

\def\out@abbreviation#1{%
   \msitag{^^0a}\msiopentag{abbreviation}{<abbreviation>}#1\msiclosetag{abbreviation}{</abbreviation>}}

