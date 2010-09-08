<?xml version="1.0"?>

<!-- NOTE:

  The funny spacing and line breaks come from the need to keep the XSL from putting blank lines in the
  generated TeX. This could be done by having each template definition all on one line, but that would be
  unreadable. The compromise is to make all of most of the line breaks in this document fall *inside* the 
  xsl tags. This is why many lines start with the ">" character. -->

<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:param name="endnotes" select="count(//html:endnotes)"/>
<xsl:param name="indexitems" select="count(//html:indexitem)"/>

<xsl:output method="text" encoding="UTF-8"/>
<xsl:strip-space elements="*"/>
<xsl:preserve-space elements="pre"/>

<xsl:include href="table.xsl"/>
<xsl:include href="graphics.xsl"/>
<xsl:include href="MML2LTeX.xsl"/>
<xsl:include href="preamble.xsl"/>
<xsl:include href="spaces.xsl"/>
<xsl:include href="frame.xsl"/>
<xsl:include href="texescape.xsl"/>

<xsl:template match="/">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*">%<!-- Matches everything but the root -->
</xsl:template>

<xsl:template match="html:html">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:head">
<xsl:apply-templates/></xsl:template>
<!-- JCS
<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>
-->




<xsl:template match="html:texlogo">
  <xsl:choose
  > <xsl:when test="@name='tex'">\TeX{}</xsl:when>
    <xsl:when test="@name='latex'">\LaTeX{}</xsl:when>
    <xsl:when test="@name='pdftex'">\textsc{pdf}\TeX{}</xsl:when>
    <xsl:when test="@name='pdflatex'">\textsc{pdf}\LaTeX{}</xsl:when>
    <xsl:when test="@name='xetex'">\XeTeX{}</xsl:when>
    <xsl:when test="@name='xelatex'">\XeLaTeX{}</xsl:when>
  </xsl:choose>
</xsl:template>


<xsl:template match="html:documentclass">

\documentclass[<xsl:apply-templates/>]{<xsl:value-of select="@class"/>}
</xsl:template>

<xsl:template match="html:colist/@*">
  <xsl:value-of select="."/><xsl:text> </xsl:text>
</xsl:template>


<xsl:template match="//html:docformat">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:usepackage">
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:template match="html:newtheorem">
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}</xsl:template>

<xsl:template match="html:body">
<!--\input tcilatex.tex  
should not be done under some conditions -->
\begin{document}
<xsl:apply-templates/>
<xsl:if test="$endnotes &gt; 0">
\theendnotes
</xsl:if>
<xsl:if test="$indexitems &gt; 0"
><xsl:if test="count(//html:printindex) = 0"
>\printindex</xsl:if></xsl:if>
\end{document}
</xsl:template>

<xsl:template match="html:printindex">
\printindex
</xsl:template>

<xsl:template match="html:br[@hard='1']">\\
</xsl:template>

<xsl:template match="html:br[@temp]"
></xsl:template>

<xsl:template match="html:title">
\title{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:author">
\author{<xsl:value-of select="normalize-space(child::text())"/><xsl:text/>
<xsl:if test="../html:address">\\<xsl:value-of select="normalize-space(../html:address)"/></xsl:if>
}</xsl:template>			   

		  
<xsl:template match="html:abstract">
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
</xsl:template>

<xsl:template match="html:maketitle">
\maketitle
</xsl:template>

<xsl:template match="html:maketoc">
\tableofcontents <xsl:text/>
</xsl:template>

<xsl:template match="html:makelof">
\listoffigures <xsl:text/>
</xsl:template>

<xsl:template match="html:makelot">
\listoftables <xsl:text/>
</xsl:template>

<xsl:template match="html:date">
\date{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:part">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:chapter">
<xsl:apply-templates/>
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
<xsl:apply-templates/>\par 
</xsl:template>



<xsl:template match="html:sectiontitle">
<xsl:if test="name(..)='chapter'">
\chapter<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='section'">
\section<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsection'">
\subsection<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsubsection'">
\subsubsection<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='paragraph'">
\paragraph<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subparagraph'">
\subparagraph<xsl:if test="../@nonum='true'">*</xsl:if>{<xsl:apply-templates/>}
</xsl:if>
</xsl:template>


<xsl:template match="html:drop">
\lettrine[lhang=.2]{\textbf{<xsl:apply-templates/>}}{}
</xsl:template>


<xsl:template match="html:numberedlist">
\begin{enumerate}
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>

<xsl:template match="html:bulletlist">
\begin{itemize}
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="html:descriptionlist">
\begin{description}
<xsl:apply-templates/>
\end{description}
</xsl:template>

<xsl:template match="html:numberedlistitem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:bulletlistitem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:descriptionlistitem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:cite">
  
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:xref">
<xsl:choose>
  <xsl:when test="@reftype='page'">\vpageref%%</xsl:when>
  <xsl:otherwise>\vref</xsl:otherwise>
</xsl:choose>
{<xsl:value-of select="@key"/>}{}%%
</xsl:template>


<xsl:template match="html:prispec">@<xsl:value-of select="."/></xsl:template>
<xsl:template match="html:secspec">@<xsl:value-of select="."/></xsl:template>
<xsl:template match="html:terspec">@<xsl:value-of select="."/></xsl:template>

<xsl:template match="html:indexitem">
\index<xsl:if test="@pri"
  >{<xsl:value-of select="@pri"/><xsl:apply-templates select="html:prispec"/></xsl:if
  ><xsl:if test="@sec">!<xsl:value-of select="@sec"/><xsl:apply-templates select="html:secspec"/></xsl:if
  ><xsl:if test="@ter">!<xsl:value-of select="@ter"/><xsl:apply-templates select="html:terspec"/></xsl:if
  ><xsl:if test="@xreftext">|see {<xsl:value-of select="@reftext"/>}</xsl:if
  ><xsl:if test="@pnformat='bold'">|textbf</xsl:if
  ><xsl:if test="@pnformat='italics'">|textit</xsl:if>}
</xsl:template>
  

<xsl:template match="html:notewrapper"><xsl:apply-templates/></xsl:template>
  

<xsl:template match="html:note[@type='footnote']">
  
<xsl:choose>
  <xsl:when test="$endnotes &gt; 0">\endnote{</xsl:when>
  <xsl:otherwise>\footnote{</xsl:otherwise>
</xsl:choose>
<xsl:apply-templates/>
<xsl:text>}</xsl:text>
</xsl:template>

<xsl:template match="html:note">\marginpar{<xsl:apply-templates/>}
  
</xsl:template>


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

<xsl:template match="html:centered">
  
\begin{center}
<xsl:apply-templates/>
\end{center}
</xsl:template>

<xsl:template match="html:center">
  
\begin{center}
\par
<xsl:apply-templates/>
\end{center}
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

<xsl:template match="html:p">\par<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:alt">{\addfontfeatures{RawFeature=+salt}<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:bold">\textbf{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:it">\textit{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:rm">\textrm{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sf">\textsf{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sl">\textsl{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sc">\textsc{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:tt">\texttt{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:em">\emph{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:upper">\uppercase{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:lower">\lowercase{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:hebrew">{\hebrew\beginR <xsl:apply-templates
  /> \endR} </xsl:template>
<xsl:template match="html:tiny">{\tiny <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:scriptsize">{\scriptsize <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:footnotesize">{\footnotesize <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:small">{\small <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:normalsize">{\normalsize <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:phantom">\phantom {<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:large">{\large <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:Large">{\Large <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:LARGE">{\LARGE <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:huge">{\huge <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:Huge">{\Huge <xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:fontsize"
  ><xsl:variable name="fontsize" select="@size"
    /><xsl:variable name="units" select="substring-after($fontsize,' ')"
    /><xsl:if test="number(substring-before($fontsize,'/'))>0"
    >{\fontsize{<xsl:value-of select="concat(substring-before($fontsize,'/'),$units)"/>}{<xsl:choose
    ><xsl:when test="number(substring-before(substring-after($fontsize,'/'),' '))>0"
    ><xsl:value-of select="concat(substring-before(substring-after($fontsize,'/'),' '),$units)"
    /></xsl:when><xsl:otherwise><xsl:value-of select="concat(substring-before($fontsize,'/'),$units)"
    /></xsl:otherwise></xsl:choose>}\selectfont </xsl:if
    ><xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:fontcolor">\textcolor<xsl:choose
   ><xsl:when test="substring(./@color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"
   /></xsl:when
   ><xsl:otherwise>{<xsl:value-of select="./@color"/></xsl:otherwise
   ></xsl:choose
   >}{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:otfont">{\fontspec{<xsl:value-of select="@fontname"
  />}<xsl:apply-templates
  />}</xsl:template>

<xsl:template match="html:tex"
  >\TeX{}</xsl:template>
<xsl:template match="html:textquotedblleft"
  >\textquotedblleft</xsl:template>
<xsl:template match="html:textquotedblright"
  >\textquotedblright</xsl:template>
<xsl:template match="html:textbackslash"
  >\textbackslash</xsl:template>

<xsl:template match="html:TeXButton">
%TCIMACRO{\TeXButton<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:TBLabel">{<xsl:apply-templates/>}
  
</xsl:template>
<xsl:template match="html:TBTeX">{<xsl:apply-templates/>}%
  
%BeginExpansion
<xsl:apply-templates/>
%EndExpansion
</xsl:template>

<xsl:template match="html:explicit-item">[<xsl:apply-templates/>]
  
</xsl:template>

<xsl:template match="html:a">\ref{<xsl:apply-templates/>}
  
</xsl:template>
<xsl:template match="html:cite">
  
\cite<xsl:if test="@label">[<xsl:value-of select="@label"/>]</xsl:if>{<xsl:apply-templates/>}
</xsl:template>


<xsl:template match="html:marker">\label{<xsl:value-of select="@id"/>}
  
</xsl:template>

<xsl:template match="a">\ref{<xsl:apply-templates/>}
</xsl:template>

<xsl:template match="html:requestimplementation">[ NEED TO IMPLEMENT: \verb+<xsl:apply-templates/>+] 
</xsl:template>

<xsl:template match="html:Note">
  
\begin{Note}
<xsl:apply-templates/>
\end{Note}
</xsl:template>


<xsl:template match="html:GrayBox">
  
\begin{GrayBox}
<xsl:apply-templates/>
\end{GrayBox}
</xsl:template>


<xsl:template match="html:proof">
  
\begin{proof}
<xsl:apply-templates/>
\end{proof}
</xsl:template>

<xsl:template match="html:QTR">\QTR{<xsl:value-of select="@type"/>}{<xsl:apply-templates/>}
  
</xsl:template>
<xsl:template match="html:QTP">\QTP{<xsl:value-of select="@type"/>}
  
<xsl:apply-templates/>
\par

</xsl:template>

<xsl:template match="html:minpasses">
%% minpasses=<xsl:value-of select="@num"/>
</xsl:template>

<!-- labels -->
<xsl:template match="html:a[@name]"
 >\label{<xsl:value-of select="@name"
 />}</xsl:template> 

<xsl:template match="html:texb"
  ><xsl:if test="@pre!='1'"
  ><xsl:if test="@enc='1'">
%TCIMACRO{\TeXButton{<xsl:value-of select="@name"/>}{<xsl:apply-templates/>}}%
%Package required: [<xsl:value-of select="@opt"/>]{<xsl:value-of select="@req"/>}
%BeginExpansion
</xsl:if
  ><xsl:apply-templates/><xsl:if test="@enc='1'"><xsl:text>
%EndExpansion
</xsl:text></xsl:if></xsl:if></xsl:template>

<xsl:template match="html:msidisplay"><xsl:apply-templates/></xsl:template>


<xsl:template match="html:frontmatter"><xsl:apply-templates/></xsl:template>
</xsl:stylesheet>


