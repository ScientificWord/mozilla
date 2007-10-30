<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exslt="http://exslt.org/common"
>
<xsl:output method="text" encoding="UTF-8"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="pre"/>


<xsl:include href="mml2ltex.xsl"/>
<xsl:include href="preamble.xsl"/>
	

<xsl:template match="/"><xsl:apply-templates/></xsl:template>

<xsl:template match="*"><!-- Matches everything but the root --></xsl:template>

<xsl:template match="html:html"><xsl:apply-templates/></xsl:template>
<xsl:template match="html:head"><xsl:apply-templates/></xsl:template>
<!-- JCS
<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>
-->




<xsl:template match="html:latex">\LaTeX</xsl:template>
<!-- xsl:template match="mml:math">[Math Here]</xsl:template -->

<xsl:template match="html:hspace">
  <xsl:choose>
    <xsl:when test="@dim='2em'">\qquad </xsl:when>
	<xsl:otherwise> </xsl:otherwise>
  </xsl:choose>
</xsl:template>
 
<xsl:template match="html:documentclass">
\documentclass{<xsl:value-of select="@class"/>}
</xsl:template>

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

<xsl:template match="html:bold">\textbf{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:it">\textit{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:rm">\textrm{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:sf">\textsf{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:sl">\textsl{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:sc">\textsc{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:tt">\texttt{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:em">\emph{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:hebrew">{\hebrew\beginR <xsl:apply-templates/> \endR} </xsl:template>

<xsl:template match="html:tiny">{\tiny <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:scriptsize">{\scriptsize <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:footnotesize">{\footnotesize <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:small">{\small <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:normalsize">{\normalsize <xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:large">{\large <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:Large">{\Large <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:LARGE">{\LARGE <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:huge">{\huge <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:Huge">{\Huge <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:fontsize">{\fontsize{<xsl:value-of select="@size"/>}{<xsl:value-of select="@size"/>}\selectfont <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:fontcolor">\textcolor[HTML]{<xsl:value-of select="substring(./@color,2,8)"/>}{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:otfont">{\fontspec{<xsl:value-of select="@fontname"/>}<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:tex">\TeX{}</xsl:template>
<xsl:template match="html:textquotedblleft">\textquotedblleft </xsl:template>
<xsl:template match="html:textquotedblright">\textquotedblright </xsl:template>
<xsl:template match="html:textbackslash">\textbackslash </xsl:template>

</xsl:stylesheet>


