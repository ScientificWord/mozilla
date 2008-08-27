<!-- Topic.xsl -->
<xsl:stylesheet version="2.0" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>

<xsl:include href="mml2ltex.xsl"/>

<xsl:output method="text"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="pre"/>



<xsl:template match="/"><xsl:apply-templates/></xsl:template>

<xsl:template match="*"><xsl:apply-templates/></xsl:template>
<xsl:template match="html:html"><xsl:apply-templates/></xsl:template>

<xsl:template match="html:body">
  <!-- \begin{document} -->
<xsl:apply-templates/>
\end{document}
</xsl:template>

<xsl:template match="sw:preamble">
<xsl:apply-templates/>
<!--\input tcilatex.tex   
should not be done under some conditions -->
\begin{document}
</xsl:template>


 
<xsl:template match="sw:documentclass">
\documentclass{<xsl:value-of select="@class"/>}
</xsl:template>

<xsl:template match="sw:usepackage">
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:template match="sw:newtheorem">
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}</xsl:template>


<!-- 
<xsl:template match="docbody">
\begin{document}
<xsl:apply-templates/>
\end{document}
</xsl:template>
-->

<xsl:template match="sw:title">
\title{<xsl:value-of select="."/>}</xsl:template>

<xsl:template match="sw:author">
\author{<xsl:value-of select="child::text()"/> <xsl:apply-templates select="address"/>}</xsl:template>

<xsl:template match="sw:address">\\<xsl:value-of select="."/></xsl:template>

<xsl:template match="sw:abstract">
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
</xsl:template>

<xsl:template match="sw:maketitle">
\maketitle
</xsl:template>

<xsl:template match="sw:chapter">
<xsl:apply-templates/>
</xsl:template>


<xsl:template match="sw:section">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:subsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:subsubsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:paragraph">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:subparagraph">
<xsl:apply-templates/>
</xsl:template>




<xsl:template match="sw:sectiontitle">
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

</xsl:template>

<xsl:template match="sw:enumerate">
\begin{enumerate}
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>
<xsl:template match="sw:itemize">							 
\begin{itemize}
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="sw:description">
\begin{description}
<xsl:apply-templates/>
\end{description}
</xsl:template>

<xsl:template match="sw:item">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:explicit-item">[<xsl:apply-templates/>]</xsl:template>


<xsl:template match="sw:cite">
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="sw:index">\index{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="sw:ref">\ref{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sw:pageref">\pageref{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="sw:footnote">\footnote{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sw:marginpar">\marginpar{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="sw:quote">
\begin{quote}
<xsl:apply-templates/>
\end{quote}
</xsl:template>

<xsl:template match="sw:quotation">
\begin{quotation}
<xsl:apply-templates/>
\end{quotation}
</xsl:template>


<xsl:template match="sw:theorem">
\begin{<xsl:value-of select="@type"/>}
<xsl:apply-templates/>
\end{<xsl:value-of select="@type"/>}
</xsl:template>

<xsl:template match="sw:pre">
\begin{verbatim}
<xsl:value-of select="."/>
\end{verbatim}
</xsl:template>

<xsl:template match="p">
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

<xsl:template match="LaTeX">\LaTeX\ </xsl:template>
<xsl:template match="TeX">\TeX\ </xsl:template>
<xsl:template match="textquotedblleft">\textquotedblleft </xsl:template>
<xsl:template match="textquotedblright">\textquotedblright </xsl:template>
<xsl:template match="textbackslash">\textbackslash </xsl:template>

<xsl:template match="sw:TeXButton">
%TCIMACRO{\TeXButton<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:TBLabel">{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sw:TBTeX">{<xsl:apply-templates/>}%
%BeginExpansion
<xsl:apply-templates/>
%EndExpansion
</xsl:template>

<xsl:template match="sw:vspace">
<xsl:if test="@type='customSpace'">
\vspace{<xsl:value-of select="@dim"/>}
</xsl:if>
</xsl:template>

<xsl:template match="sw:hspace">
<xsl:if test="@type='customSpace'">
\hspace{<xsl:value-of select="@dim"/>}
</xsl:if>
</xsl:template>


<xsl:template match="sw:GrayBox">
\begin{GrayBox}
<xsl:apply-templates/>
\end{GrayBox}
</xsl:template>

<xsl:template match="sw:QTR">\QTR{<xsl:value-of select="@type"/>}{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="sw:QTP">\QTP{<xsl:value-of select="@type"/>}
<xsl:apply-templates/>
\par

</xsl:template>

<xsl:template match="sw:marker">\label{<xsl:value-of select="@id"/>}</xsl:template>

<xsl:template match="a">\ref{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="sw:requestimplementation">[ NEED TO IMPLEMENT: \verb+<xsl:apply-templates/>+] </xsl:template>

<xsl:template match="sw:Note">
\begin{Note}
<xsl:apply-templates/>
\end{Note}
</xsl:template>


</xsl:stylesheet>