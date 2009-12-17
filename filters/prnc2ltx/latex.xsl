<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:regexp="http://exslt.org/regular-expressions" 
    xmlns:exsl="http://exslt.org/common"
    extension-element-prefixes="regexp">

<xsl:output method="text" encoding="UTF-8"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="pre"/>
<xsl:import href="regexp.xsl" />


<xsl:include href="mml2ltex.xsl"/>
<xsl:include href="preamble.xsl"/>
<xsl:include href="spaces.xsl"/>
<xsl:include href="frame.xsl"/>

<xsl:template match="/">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*">%<!-- Matches everything but the root -->
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:html">
#ifdef DEBUG
    <xsl:message>Into html:html</xsl:message>
#endif
	<xsl:apply-templates/>
#ifdef DEBUG
    <xsl:message>Out of html:html</xsl:message>
#endif
</xsl:template>

<xsl:template match="html:head">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/></xsl:template>
<!-- JCS
<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>
-->




<xsl:template match="latex">\LaTeX{}</xsl:template>
    <xsl:message><xsl:value-of select="."/></xsl:message>
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif

<xsl:template match="html:hspace">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
  <xsl:choose>
    <xsl:when test="@dim='2em'">\qquad </xsl:when>
	<xsl:otherwise> </xsl:otherwise>
  </xsl:choose>
</xsl:template>
 
<xsl:template match="html:documentclass">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\documentclass{<xsl:value-of select="@class"/>}
</xsl:template>

<xsl:template match="//html:docformat">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:usepackage">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:template match="html:newtheorem">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}</xsl:template>


<xsl:template match="html:body">
#ifdef DEBUG
    <xsl:message>Into html:body</xsl:message>
#endif
<!--\input tcilatex.tex   
should not be done under some conditions -->
\begin{document}
<xsl:apply-templates/>
<xsl:if test="$endnotes &gt; 0">
\theendnotes
</xsl:if>
\end{document}
	
#ifdef DEBUG
    <xsl:message>Out of html:body</xsl:message>
#endif
</xsl:template>

<xsl:template match="html:br[@hard='1']">\\
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:br[@temp]">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:title">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\title{<xsl:apply-templates/>}
</xsl:template>

<xsl:template match="html:author">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\author{<xsl:value-of select="child::text()"/>}</xsl:template>

<xsl:template match="html:address">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\address{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:abstract">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
\maketitle</xsl:template>

<xsl:template match="html:maketitle">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\maketitle
</xsl:template>

<xsl:template match="html:chapter">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:section">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsection">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsubsection">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:paragraph">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subparagraph">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
</xsl:template>


<xsl:template match="html:para">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>\par 
</xsl:template>



<xsl:template match="html:sectiontitle">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:if test="name(..)='chapter'">
\chapter{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='section'">
\section{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsection'">
\subsection{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsubsection'">
\subsubsection{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='paragraph'">
\paragraph{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subparagraph'">
\subparagraph{<xsl:apply-templates/>}
</xsl:if>
</xsl:template>


<xsl:template match="html:drop">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\lettrine[lhang=.2]{\textbf{<xsl:apply-templates/>}}{}
</xsl:template>


<xsl:template match="html:enumerate">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{enumerate}
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>

<xsl:template match="html:itemize">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{itemize}
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="html:description">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{description}
<xsl:apply-templates/>
\end{description}
</xsl:template>

<xsl:template match="html:item">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:cite">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:ref">\ref{<xsl:apply-templates/>}</xsl:template>
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:template match="html:pageref">\pageref{<xsl:apply-templates/>}</xsl:template>
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif

<xsl:template match="html:notewrapper"><xsl:apply-templates/></xsl:template>
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif

<xsl:template match="html:note[@type='footnote']">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:choose>
  <xsl:when test="$endnotes &gt; 0">\endnote{</xsl:when>
  <xsl:otherwise>\footnote{</xsl:otherwise>
</xsl:choose>
<xsl:apply-templates/>
}
</xsl:template>

<xsl:template match="html:note">\marginpar{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>


<xsl:template match="html:quote">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{quote}
<xsl:apply-templates/>
\end{quote}
</xsl:template>


<xsl:template match="html:quotation">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{quotation}
<xsl:apply-templates/>
\end{quotation}
</xsl:template>


<xsl:template match="html:theorem">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{<xsl:value-of select="@type"/>}
<xsl:apply-templates/>
\end{<xsl:value-of select="@type"/>}
</xsl:template>

<xsl:template match="html:pre">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{verbatim}
<xsl:value-of select="."/>
\end{verbatim}
</xsl:template>

<xsl:template match="html:p">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:text>

</xsl:text>
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:alt">{\addfontfeatures{RawFeature=+salt}<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:bold">\textbf{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:it">\textit{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:rm">\textrm{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:sf">\textsf{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:sl">\textsl{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:sc">\textsc{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:tt">\texttt{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:em">\emph{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:upper">\uppercase{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:lower">\lowercase{<xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:hebrew">{\hebrew\beginR <xsl:apply-templates/> \endR} 
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:tiny">{\tiny <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:scriptsize">{\scriptsize <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:footnotesize">{\footnotesize <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:small">{\small <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:normalsize">{\normalsize <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:large">{\large <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:Large">{\Large <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:LARGE">{\LARGE <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:huge">{\huge <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:Huge">{\Huge <xsl:apply-templates/>}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:fontsize">
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
  <xsl:variable name="fontsize" select="@size"/>%
  <xsl:variable name="units" select="substring-after($fontsize,' ')"/>%
  <xsl:if test="number(substring-before($fontsize,'/'))>0">
    {\fontsize{<xsl:value-of select="concat(substring-before($fontsize,'/'),$units)"/>}{<xsl:choose><xsl:when test="number(substring-before(substring-after($fontsize,'/'),' '))>0"><xsl:value-of select="concat(substring-before(substring-after($fontsize,'/'),' '),$units)"/></xsl:when><xsl:otherwise><xsl:value-of select="concat(substring-before($fontsize,'/'),$units)"/></xsl:otherwise></xsl:choose>}\selectfont </xsl:if>
    <xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:fontcolor">\textcolor[HTML]{<xsl:value-of select="substring(./@color,2,8)"/>}{<xsl:apply-templates/>}</xsl:template>
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:template match="html:otfont">{\fontspec{<xsl:value-of select="@fontname"/>}<xsl:apply-templates/>}
</xsl:template>

<xsl:template match="html:tex">\TeX{}
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:textquotedblleft">\textquotedblleft% 
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:textquotedblright">\textquotedblright% 
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:textbackslash">\textbackslash% 
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:TeXButton">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
%TCIMACRO{\TeXButton<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:TBLabel">{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:TBTeX">{<xsl:apply-templates/>}%
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
%BeginExpansion
<xsl:apply-templates/>
%EndExpansion
</xsl:template>

<xsl:template match="html:explicit-item">[<xsl:apply-templates/>]
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:a">\ref{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:cite">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}
</xsl:template>

<xsl:template match="html:index">\index{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:ref">\ref{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:pageref">\pageref{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>


<xsl:template match="html:marker">\label{<xsl:value-of select="@id"/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="a">\ref{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:requestimplementation">[ NEED TO IMPLEMENT: \verb+<xsl:apply-templates/>+] 
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>

<xsl:template match="html:Note">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{Note}
<xsl:apply-templates/>
\end{Note}
</xsl:template>


<xsl:template match="html:GrayBox">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{GrayBox}
<xsl:apply-templates/>
\end{GrayBox}
</xsl:template>


<xsl:template match="html:proof">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
\begin{proof}
<xsl:apply-templates/>
\end{proof}
</xsl:template>

<xsl:template match="html:QTR">\QTR{<xsl:value-of select="@type"/>}{<xsl:apply-templates/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
</xsl:template>
<xsl:template match="html:QTP">\QTP{<xsl:value-of select="@type"/>}
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
<xsl:apply-templates/>
\par

</xsl:template>






</xsl:stylesheet>


