<!-- Topic.xsl -->

<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>


<xsl:include href="mml2ltex.xsl"/>


<xsl:output method="text" encoding="UTF-8"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="pre"/>


<xsl:template match="/"><xsl:apply-templates/></xsl:template>

<xsl:template match="*"><!-- Matches everything but the root --></xsl:template>

<xsl:template match="html:html"><xsl:apply-templates/></xsl:template>
<xsl:template match="html:head"><xsl:apply-templates/></xsl:template>
<!-- JCS
<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>
-->

<xsl:template match="html:preamble">
<xsl:apply-templates/>
</xsl:template>

<!-- use docformat information to call the geometry package -->
<xsl:template match="html:pagelayout[@latex='true']">
<xsl:variable name="unit"><xsl:value-of select="@unit"/></xsl:variable>

\usepackage[ <xsl:apply-templates/>
]{geometry}
</xsl:template>

<xsl:template match="html:page">
  paper=<xsl:value-of select="@paper"/>paper,
  twoside=<xsl:value-of select="@twoside"/>,
  landscape=<xsl:value-of select="@landscape"/>,</xsl:template>

<xsl:template match="html:page[@paper='screen']">
  paper=screen,
  twoside=false,
  landscape=false,</xsl:template>

<xsl:template match="html:page[@paper='other']">
  paperwidth=<xsl:value-of select="@width"/>,
  paperheight=<xsl:value-of select="@height"/>, </xsl:template>

<xsl:template match="html:textregion">
  textwidth=<xsl:value-of select="@width"/>,		    
  textheight=<xsl:value-of select="@height"/>, </xsl:template>

<xsl:template match="html:margin">
  left=<xsl:value-of select="@left"/>,
  top=<xsl:value-of select="@top"/>, </xsl:template>

<xsl:template match="html:header">
  headheight=<xsl:value-of select="@height"/>,
  headsep=<xsl:value-of select="@sep"/>, </xsl:template>

<xsl:template match="html:columns[@count='2']">
  twocolumn=true,
  columnsep=<xsl:value-of select="@sep"/>, </xsl:template>

<xsl:template match="html:marginnote[@hidden='false']">
  marginparwidth=<xsl:value-of select="@width"/>,
  marginparsep=<xsl:value-of select="@sep"/>,	</xsl:template>

<xsl:template match="html:footer">
  footskip=<xsl:value-of 
  select="concat(number(substring(@height,1,string-length(@height)-2))+number(substring(@sep,1,string-length(@sep)-2)),substring(@sep,string-length(@sep)-2))"/></xsl:template>


<xsl:template match="html:fontchoices">
\usepackage{fontspec}
\usepackage{xunicode}
\usepackage{xltxtra}
\TeXXeTstate=1
\defaultfontfeatures{Scale=MatchLowercase,Mapping=tex-text}
<xsl:apply-templates/></xsl:template>  

<xsl:template match="html:mainfont">
\setmainfont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:mainfont[@name='Default']"></xsl:template>

<xsl:template match="html:sansfont">
\setsansfont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:sansfont[@name='']"></xsl:template>

<xsl:template match="html:fixedfont">
\setmonofont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:fixedfont[@name='']"></xsl:template>

<xsl:template match="html:x1font">
\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:x2font">
\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:x3font">
\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:latex">\LaTeX</xsl:template>
<!-- xsl:template match="mml:math">[Math Here]</xsl:template -->

<xsl:template match="html:hspace">
  <xsl:choose>
    <xsl:when test="@dim='2em'">\qquad </xsl:when>
	<xsl:otherwise> </xsl:otherwise>
  </xsl:choose>
</xsl:template>
 
<xsl:template match="html:documentclass">
\documentclass{<xsl:value-of select="@class"/>}</xsl:template>

<xsl:template match="//html:docformat">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:usepackage">
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:template match="html:newtheorem">
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}
</xsl:template>
						  

<xsl:template match="html:body">
\begin{document}
<xsl:apply-templates/>
\end{document}
</xsl:template>


<xsl:template match="html:title">
\title{<xsl:apply-templates/>}
</xsl:template>

<xsl:template match="html:author">
\author{<xsl:value-of select="child::text()"/>}</xsl:template>

<xsl:template match="html:address">
\address{<xsl:value-of select="."/>}</xsl:template>

<xsl:template match="html:abstract">
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
\maketitle
</xsl:template>

<xsl:template match="html:maketitle">
\maketitle
</xsl:template>

<xsl:template match="html:section">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsubsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:paragraph">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subparagraph">
<xsl:apply-templates/>
</xsl:template>


<xsl:template match="html:para">
<xsl:apply-templates/>\par </xsl:template>



<xsl:template match="html:sectiontitle">
<xsl:if test="name(..)='section'">
\section{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='subsection'">
\subsection{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='subsubsection'">
\subsubsection{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='paragraph'">
\paragraph{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='subparagraph'">
\subparagraph{<xsl:value-of select="."/>}
</xsl:if>

</xsl:template>

<xsl:template match="html:enumerate">
\begin{enumerate}
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>

<xsl:template match="html:itemize">
\begin{itemize}
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="html:description">
\begin{description}
<xsl:apply-templates/>
\end{description}
</xsl:template>

<xsl:template match="html:item">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:cite">
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:ref">\ref{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:pageref">\pageref{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:footnote">\footnote{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:marginpar">\marginpar{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:quote">
\begin{quote}
<xsl:apply-templates/>
\end{quote}
</xsl:template>

<xsl:template match="html:quotation">
\begin{quotation}
<xsl:apply-templates/>
\end{quotation}
</xsl:template>


<xsl:template match="html:theorem">
\begin{<xsl:value-of select="@type"/>}
<xsl:apply-templates/>
\end{<xsl:value-of select="@type"/>}
</xsl:template>

<xsl:template match="html:pre">
\begin{verbatim}
<xsl:value-of select="."/>
\end{verbatim}
</xsl:template>

<xsl:template match="html:p">
<xsl:text>

</xsl:text>
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="bold">\textbf{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="it">\textit{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="rm">\textrm{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sf">\textsf{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sl">\textsl{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sc">\textsc{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="tt">\texttt{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="em">\emph{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="hebrew">{\hebrew\beginR <xsl:apply-templates/> \endR} </xsl:template>

<xsl:template match="tiny">{\tiny <xsl:apply-templates/>}</xsl:template>
<xsl:template match="scriptsize">{\scriptsize <xsl:apply-templates/>}</xsl:template>
<xsl:template match="footnotesize">{\footnotesize <xsl:apply-templates/>}</xsl:template>
<xsl:template match="small">{\small <xsl:apply-templates/>}</xsl:template>
<xsl:template match="normalsize">{\normalsize <xsl:apply-templates/>}</xsl:template>

<xsl:template match="large">{\large <xsl:apply-templates/>}</xsl:template>
<xsl:template match="Large">{\Large <xsl:apply-templates/>}</xsl:template>
<xsl:template match="LARGE">{\LARGE <xsl:apply-templates/>}</xsl:template>
<xsl:template match="huge">{\huge <xsl:apply-templates/>}</xsl:template>
<xsl:template match="Huge">{\Huge <xsl:apply-templates/>}</xsl:template>

<xsl:template match="tex">\TeX{}</xsl:template>
<xsl:template match="textquotedblleft">\textquotedblleft </xsl:template>
<xsl:template match="textquotedblright">\textquotedblright </xsl:template>
<xsl:template match="textbackslash">\textbackslash </xsl:template>

</xsl:stylesheet>


