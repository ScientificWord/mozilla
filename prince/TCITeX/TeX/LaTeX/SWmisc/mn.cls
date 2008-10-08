% mn.cls
%
%
\newdimen\@maxsep       % for 2.09 compatability
\newdimen\@dblmaxsep
\newdimen\footheight
% The following section was added 8/26/96.
% Previously, we had added a line to mn.sty.
% This section avoids either changing mn.sty
% or getting the error message  \normalsize not defined
% upon Typeset Preview.
\typeout{mn.cls - GP}

  \def\@normalsize{\@setsize\normalsize{12pt}\ixpt\@ixpt
   \abovedisplayskip 4pt plus 1pt minus 1pt
   \belowdisplayskip \abovedisplayskip
   \abovedisplayshortskip 4pt plus 1pt
   \belowdisplayshortskip \abovedisplayshortskip
   \let\@listi\@listI}
\let\normalsize=\@normalsize
% JLW
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}

\input mn.sty