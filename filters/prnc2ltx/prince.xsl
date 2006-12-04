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

<xsl:template match="*"><!-- Matches everything but the root --></xsl:template>

<xsl:template match="html:html"><xsl:apply-templates/></xsl:template>

<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="sw:preamble">
<xsl:apply-templates/>
</xsl:template>


 
<xsl:template match="documentclass">
\documentclass{<xsl:value-of select="@class"/>}</xsl:template>

<xsl:template match="usepackage">
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:template match="newtheorem">
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}</xsl:template>


<xsl:template match="sw:docbody">
\begin{document}
<xsl:apply-templates/>
\end{document}
</xsl:template>


<xsl:template match="title">
\title{<xsl:value-of select="."/>}</xsl:template>

<xsl:template match="author">
\author{<xsl:value-of select="child::text()"/> <xsl:apply-templates select="address"/>}</xsl:template>

<xsl:template match="address">\\<xsl:value-of select="."/></xsl:template>

<xsl:template match="abstract">
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
</xsl:template>

<xsl:template match="maketitle">
\maketitle
</xsl:template>

<xsl:template match="section">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsubsection">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="paragraph">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="subparagraph">
<xsl:apply-templates/>
</xsl:template>




<xsl:template match="sectiontitle">
<xsl:if test="name(..)='section'">
\section{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='subsection'">
\subsection{<xsl:value-of select="."/>}
</xsl:if>
<xsl:if test="name(..)='subsubsection'">
\subsubsection{<xsl:value-of select="."/>}
</xsl:if>

</xsl:template>

<xsl:template match="enumerate">
\begin{enumerate}
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>
<xsl:template match="itemize">							 
\begin{itemize}
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="description">
\begin{description}
<xsl:apply-templates/>
\end{description}
</xsl:template>

<xsl:template match="item">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="cite">
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="ref">\ref{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="pageref">\pageref{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="footnote">\footnote{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="marginpar">\marginpar{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="quote">
\begin{quote}
<xsl:apply-templates/>
\end{quote}
</xsl:template>

<xsl:template match="quotation">
\begin{quotation}
<xsl:apply-templates/>
\end{quotation}
</xsl:template>


<xsl:template match="theorem">
\begin{<xsl:value-of select="@type"/>}
<xsl:apply-templates/>
\end{<xsl:value-of select="@type"/>}
</xsl:template>

<xsl:template match="pre">
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
<xsl:template match="textquotedblleft">\textquotedblleft </xsl:template>
<xsl:template match="textquotedblright">\textquotedblright </xsl:template>
<xsl:template match="textbackslash">\textbackslash </xsl:template>

<!--
<xsl:template match="mml:math">\textbf{Math here}</xsl:template>
-->

</xsl:stylesheet>