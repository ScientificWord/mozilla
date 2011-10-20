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

<xsl:param name="endnotes" select="count(//html:endnotes[@val='end'])"/>
<xsl:param name="footnotecount" select="count(//html:note[@type='footnote'])"/>
<xsl:param name="indexitems" select="count(//html:indexitem)"/>
<xsl:param name="toclocation">
  <xsl:choose>
    <xsl:when test="//html:maketoc">none</xsl:when>
    <xsl:when test="//html:part">tocpart</xsl:when>
    <xsl:when test="//html:chapter">tocchap</xsl:when>
    <xsl:when test="//html:section">tocsect</xsl:when>
    <xsl:otherwise> none </xsl:otherwise>
  </xsl:choose>
</xsl:param>

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
<xsl:include href="babel.xsl"/>

<xsl:template match="/">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*">%<!-- Matches everything but the root -->
</xsl:template>

<xsl:template match="html:html">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:head">
\documentclass<xsl:if test="//html:colist/@*">[<xsl:for-each select="//html:colist/@*"
    ><xsl:if test="name()!='enabled'"><xsl:value-of select="."/><xsl:if test="(position()>1) and (position()!=last())">, </xsl:if></xsl:if></xsl:for-each>]</xsl:if>{<xsl:value-of select="//html:documentclass/@class"/>}
  <xsl:apply-templates/>
</xsl:template>


<!-- JCS
<xsl:template match="html:body">
<xsl:apply-templates/>
</xsl:template>
-->




<xsl:template match="html:texlogo"><xsl:choose
><xsl:when test="@name='tex'">\TeX{}</xsl:when>
<xsl:when test="@name='latex'">\LaTeX{}</xsl:when>
<xsl:when test="@name='pdftex'">\textsc{pdf}\TeX{}</xsl:when>
<xsl:when test="@name='pdflatex'">\textsc{pdf}\LaTeX{}</xsl:when>
<xsl:when test="@name='xetex'">\XeTeX{}</xsl:when>
<xsl:when test="@name='xelatex'">\XeLaTeX{}</xsl:when></xsl:choose></xsl:template>


<!-- xsl:template match="html:colist/@*">
  <xsl:value-of select="."/><xsl:text> </xsl:text>
</xsl:template -->


<xsl:template match="//html:docformat">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:usepackage">
\usepackage{<xsl:value-of select="@package"/>}</xsl:template>

<xsl:variable name="theoremenvList">
  <xsl:for-each select="//html:newtheorem">
    <xsl:element name="thmenvnode">
      <xsl:attribute name="tagname"><xsl:value-of select="@name"/></xsl:attribute>
      <xsl:attribute name="numbering">
      <xsl:choose>
        <xsl:when test="@counter and string-length(@counter)">
          <xsl:value-of select="@counter"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@name"/>
        </xsl:otherwise>
      </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="texname"><xsl:value-of select="@name"/></xsl:attribute>
      <xsl:attribute name="label"><xsl:value-of select="@label"/></xsl:attribute>
      <xsl:if test="@theoremstyle">
        <xsl:attribute name="thmstyle"><xsl:value-of select="@theoremstyle"/></xsl:attribute>
      </xsl:if>
    </xsl:element>
  </xsl:for-each>
</xsl:variable>

<xsl:variable name="theoremenvNodeList" select ="exsl:node-set($theoremenvList)"/>

<xsl:variable name="neededNewTheorems">
  <xsl:for-each select="//html:assertion|//html:conjecture|//html:corollary|//html:criterion|//html:proposition|//html:theorem|//html:algorithm|//html:assumption|//html:axiom|//html:condition|//html:definition|//html:example|//html:exercise|//html:hypothesis|//html:problem|//html:property|//html:question|//html:acknowledgment|//html:case|//html:claim|//html:conclusion|//html:notation|//html:remark|//html:summary|//html:generictheorem">
    <xsl:variable name="existingThmTranslation">
      <xsl:call-template name="checkTeXNameForEnvironments">
        <xsl:with-param name="theTag" select="name(.)"/>
        <xsl:if test="not(not(@numbering)) and string-length(@numbering)">
          <xsl:with-param name="theNumbering" select="@numbering"/>
        </xsl:if>
      </xsl:call-template>
    </xsl:variable>
    <xsl:if test="not($existingThmTranslation) or not(string-length($existingThmTranslation))">
      <xsl:element name="thmenvnode">
      <xsl:attribute name="tagname"><xsl:value-of select="name(.)"/></xsl:attribute>
      <xsl:attribute name="texname"><xsl:value-of select="name(.)"/></xsl:attribute>
      <xsl:choose>
        <xsl:when test="not(not(@numbering))">
          <xsl:attribute name="numbering"><xsl:value-of select="@numbering"/></xsl:attribute>
        </xsl:when>
        <xsl:otherwise>
          <xsl:attribute name="numbering"><xsl:value-of select="name(.)"/></xsl:attribute>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:attribute name="label"><xsl:value-of select="translate(substring(name(.),1,1),'abcdefghijklmnopqrstuvwxyz','ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/><xsl:value-of select="substring(name(.),2)"/></xsl:attribute>
      </xsl:element>
    </xsl:if>
  </xsl:for-each>
</xsl:variable>

<xsl:variable name="neededNewTheoremsNodeList" select = "exsl:node-set($neededNewTheorems)"/>

<xsl:variable name="sortedNeededTheorems">
  <xsl:for-each select="$neededNewTheoremsNodeList/thmenvnode">
    <xsl:sort select="@texname" />
    <xsl:copy-of select="."/>
  </xsl:for-each>
</xsl:variable>

<xsl:variable name="sortedNeededNewTheoremsNodeList" select = "exsl:node-set($sortedNeededTheorems)"/>

<xsl:template name="writeOutANewTheorem">
  <xsl:choose>
    <xsl:when test="@numbering='none'">
      <xsl:text xml:space="preserve">
\newtheorem*{</xsl:text>
      <xsl:value-of select="@texname"/>}{<xsl:value-of select="@label"/>}
    </xsl:when>
    <xsl:when test="not(@numbering) or @numbering=@texname">
      <xsl:text xml:space="preserve">
\newtheorem{</xsl:text>
      <xsl:value-of select="@texname"/>}{<xsl:value-of select="@label"/>}
    </xsl:when>
    <xsl:otherwise>
      <xsl:text xml:space="preserve">
\newtheorem{</xsl:text>
      <xsl:value-of select="@texname"/>}[<xsl:value-of select="@numbering"/>]{<xsl:value-of select="@label"/>}
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- xsl:template match="html:newtheorem">
\newtheorem{<xsl:value-of select="@name"/>}<xsl:if test="not(not(@counter))">[<xsl:value-of select="@counter"/>]</xsl:if>{<xsl:value-of select="@label"/>}</xsl:template -->

<xsl:template match="html:newtheorem">
</xsl:template>

<xsl:template name="generateMissingNewTheorems">
  <xsl:for-each select="$sortedNeededNewTheoremsNodeList/*[@texname and string-length(@texname)]">
    <xsl:variable name="pos" select="position()"/>
    <!-- xsl:variable name="currtexname" select="@texname"/ -->
    <xsl:if test="($pos=1) or not(@texname=$sortedNeededNewTheoremsNodeList/thmenvnode[$pos - 1]/@texname)">
      <xsl:choose>
        <xsl:when test="@numbering='none'">
\newtheorem*{<xsl:value-of select="@texname"/>}{<xsl:value-of select="@label"/>}
        </xsl:when>
        <xsl:when test="not(@numbering) or @numbering=@texname">
\newtheorem{<xsl:value-of select="@texname"/>}{<xsl:value-of select="@label"/>}
        </xsl:when>
        <xsl:otherwise>
\newtheorem{<xsl:value-of select="@texname"/>}[<xsl:value-of select="@numbering"/>]{<xsl:value-of select="@label"/>}
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

<xsl:template name="writeNewTheoremList">
  <xsl:choose>
    <xsl:when test="$theoremenvNodeList/thmenvnode[@thmstyle and (@thmstyle!='plain')] |$sortedNeededNewTheoremsNodeList/thmenvnode[@thmstyle and (@thmstyle!='plain')]"> 
      <xsl:for-each select="$theoremenvNodeList/thmenvnode[not(@thmstyle) or (@thmstyle='plain')] | $sortedNeededNewTheoremsNodeList/thmenvnode[not(@thmstyle) or (@thmstyle='plain')]">
        <xsl:if test="position()=1">
          <xsl:text xml:space="preserve">
\theoremstyle{plain}</xsl:text>
        </xsl:if>
        <xsl:call-template name="writeOutANewTheorem" />
      </xsl:for-each>
      <xsl:for-each select="$theoremenvNodeList/thmenvnode[@thmstyle='definition'] | $sortedNeededNewTheoremsNodeList/thmenvnode[@thmstyle='definition']">
        <xsl:if test="position()=1">
          <xsl:text xml:space="preserve">
\theoremstyle{definition}</xsl:text>
        </xsl:if>
        <xsl:call-template name="writeOutANewTheorem" />
      </xsl:for-each>
      <xsl:for-each select="$theoremenvNodeList/thmenvnode[@thmstyle='remark'] | $sortedNeededNewTheoremsNodeList/thmenvnode[@thmstyle='remark']">
        <xsl:if test="position()=1">
          <xsl:text xml:space="preserve">
\theoremstyle{remark}</xsl:text>
        </xsl:if>
        <xsl:call-template name="writeOutANewTheorem" />
      </xsl:for-each>
    </xsl:when>
    <xsl:otherwise>
      <xsl:for-each select="$theoremenvNodeList/thmenvnode | $sortedNeededNewTheoremsNodeList/thmenvnode">
        <xsl:call-template name="writeOutANewTheorem" />
      </xsl:for-each>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:variable name="equationNumberingContainers">
  <xsl:choose>
    <xsl:when test=".//html:documentclass[@class='article']">
      <xsl:text>-section-chapter-part-body-</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>-chapter-part-body-</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:variable>

<xsl:template name="checkEndSubEquationsScope">
  <xsl:if test="contains($equationNumberingContainers, concat('-',local-name(.),'-'))">
    <xsl:if test=".//html:msidisplay[last()][@subEquationNumbers='true'] 
        and (.//html:msidisplay[last()]/ancestor::*[contains($equationNumberingContainers, concat('-',local-name(.),'-'))]=current())">
      <xsl:text xml:space="preserve">
\end{subequations}
</xsl:text>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template match="html:body">
<!--\input tcilatex.tex  
should not be done under some conditions -->
<xsl:if test="count(//html:indexitem) &gt; 0"
  >\makeindex</xsl:if>
\begin{document}
<xsl:apply-templates/>
<xsl:if test="($endnotes &gt; 0) and ($footnotecount &gt; 0)">
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

<xsl:template match="html:br[@hard='1']">~\\
</xsl:template>

<xsl:template match="html:msibr">~\\
</xsl:template>

<xsl:template match="html:br">
</xsl:template>

<xsl:template match="html:br"
></xsl:template>

<xsl:template match="html:title">
\title{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:author">
\author{<xsl:value-of select="normalize-space(child::text())"/><xsl:text/>
<xsl:if test="../html:address">~\\<xsl:value-of select="normalize-space(../html:address)"/></xsl:if>
}</xsl:template>               

          
<xsl:template match="html:abstract">
\begin{abstract}
<xsl:apply-templates/>
\end{abstract}
</xsl:template>

<xsl:template match="html:maketitle">
\maketitle
</xsl:template>

<xsl:template name="maketables">
  \tableofcontents <xsl:text/>
  <xsl:if test="//html:lof">\listoffigures</xsl:if>
  <xsl:if test="//html:lot">\listoftables</xsl:if>
</xsl:template>

<xsl:template match="html:maketoc">
\tableofcontents <xsl:text/>
</xsl:template>

<xsl:template match="html:makelof">
\listoffigures <xsl:text/>
</xsl:template>

<!--xsl:template match="html:makelot">
\listoftables <xsl:text/>
</xsl:template -->

<xsl:template match="html:date">
\date{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="//html:part[1]">
  <xsl:if test="starts-with($toclocation,'tocpart')">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:apply-templates/>
  <xsl:call-template name="checkEndSubEquationsScope"/>
</xsl:template>

<xsl:template match="html:part">
<xsl:apply-templates/>
<xsl:call-template name="checkEndSubEquationsScope"/>
</xsl:template>

<xsl:template match="//html:chapter[1]">
  <xsl:if test="starts-with($toclocation,'tocchap')">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:apply-templates/>
  <xsl:call-template name="checkEndSubEquationsScope"/>
</xsl:template>

<xsl:template match="html:chapter">
  <xsl:apply-templates/>
  <xsl:call-template name="checkEndSubEquationsScope"/>
</xsl:template>

<xsl:template match="//html:section[1]">
  <xsl:if test="starts-with($toclocation,'tocsect')">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:apply-templates/>
  <xsl:call-template name="checkEndSubEquationsScope"/>
</xsl:template>

<xsl:template match="html:section">
<xsl:apply-templates/>
<xsl:call-template name="checkEndSubEquationsScope"/>
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


<xsl:template match="html:bodyText">
  <xsl:if test="(position() = 1) and (starts-with($toclocation,'tocpara'))">
    <xsl:call-template name="maketables" />
  </xsl:if>
<xsl:apply-templates/>\par 
</xsl:template>



<xsl:template match="html:sectiontitle">
<xsl:if test="name(..)='chapter'">
\chapter<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='section'">
\section<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsection'">
\subsection<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subsubsection'">
\subsubsection<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='paragraph'">
\paragraph<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
<xsl:if test="name(..)='subparagraph'">
\subparagraph<xsl:if test="../@nonum='true'">*</xsl:if><xsl:apply-templates mode="shortTitle"/>{<xsl:apply-templates/>}
</xsl:if>
</xsl:template>


<xsl:template match="html:sectiontitle/html:shortTitle"></xsl:template>
<xsl:template match="html:sectiontitle//text()" mode="shortTitle"></xsl:template>
<xsl:template match="html:shortTitle" mode="shortTitle">[<xsl:apply-templates/>]</xsl:template>


<xsl:template match="html:assertion|html:conjecture|html:corollary|html:criterion|html:proposition|html:theorem|html:algorithm|html:assumption|html:axiom|html:condition|html:definition|html:example|html:exercise|html:hypothesis|html:problem|html:property|html:question|html:acknowledgment|html:case|html:claim|html:conclusion|html:notation|html:remark|html:summary">
<xsl:call-template name="processThmEnvironment">
  <xsl:with-param name="theTag" select="name()"/>
  <xsl:with-param name="theNumbering" select="@numbering"/>
</xsl:call-template>
</xsl:template>

<xsl:template name="checkTeXNameForEnvironments">
  <xsl:param name="theTag"/>
  <xsl:param name="theNumbering"/>
  <xsl:choose>
    <xsl:when test="string-length(normalize-space($theNumbering))">
      <xsl:variable name="theDefinition" select="$theoremenvNodeList/*[@tagname=$theTag][@numbering=$theNumbering]" />
      <xsl:if test="$theDefinition"><xsl:value-of select="$theDefinition/@texname"/></xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="theDefinition2" select="$theoremenvNodeList/*[@tagname=$theTag]" />
      <xsl:if test="$theDefinition2"><xsl:value-of select="$theDefinition2/@texname"/></xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="getTeXNameForEnvironment">
  <xsl:param name="theTag"/>
  <xsl:param name="theNumbering"/>
  <xsl:choose>
    <xsl:when test="string-length(normalize-space($theNumbering))">
      <xsl:variable name="theDefinition" select="$theoremenvNodeList/*[@tagname=$theTag][@numbering=$theNumbering]" />
      <xsl:variable name="theDefinitionNew" select="$sortedNeededNewTheoremsNodeList/*[@tagname=$theTag][@numbering=$theNumbering]" />
      <xsl:choose>
        <xsl:when test="$theDefinition"><xsl:value-of select="$theDefinition/@texname"/></xsl:when>
        <xsl:when test="$theDefinitionNew"><xsl:value-of select="$theDefinitionNew/@texname"/></xsl:when>
        <xsl:otherwise></xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="theDefinition2" select="$theoremenvNodeList/*[@tagname=$theTag]" />
      <xsl:variable name="theDefinition2New" select="$sortedNeededNewTheoremsNodeList/*[@tagname=$theTag]" />
      <xsl:choose>
        <xsl:when test="$theDefinition2"><xsl:value-of select="$theDefinition2/@texname"/></xsl:when>
        <xsl:when test="$theDefinition2New"><xsl:value-of select="$theDefinition2New/@texname"/></xsl:when>
        <xsl:otherwise></xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="processThmEnvironment">
  <xsl:param name="theTag"/>
  <xsl:param name="theNumbering"/>
  <xsl:variable name="definedTagnameToUse">
    <xsl:call-template name="getTeXNameForEnvironment">
      <xsl:with-param name="theTag" select="$theTag"/>
      <xsl:with-param name="theNumbering" select="$theNumbering"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="tagnameToUse">
    <xsl:choose>
      <xsl:when test="$definedTagnameToUse and string-length(normalize-space($definedTagnameToUse))">
        <xsl:value-of select="$definedTagnameToUse"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$theTag"/><xsl:if test="$theNumbering='none'">*</xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
\begin{<xsl:value-of select="$tagnameToUse"/>}<xsl:apply-templates mode="envleadin"/>
<xsl:apply-templates/>
\end{<xsl:value-of select="$tagnameToUse"/>}
</xsl:template>

<xsl:template match="html:proof">
\begin{proof}<xsl:apply-templates mode="envleadin"/>
<xsl:apply-templates/>
\end{proof}</xsl:template>

<xsl:template match="html:envLeadIn"></xsl:template>
<xsl:template match="*" mode="envleadin"></xsl:template>
<xsl:template match="html:envLeadIn" mode="envleadin">[<xsl:apply-templates/>]</xsl:template>

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

<xsl:template match="html:numberedListItem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:bulletListItem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:descriptionListItem">
\item <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:citation">
<xsl:choose>
  <xsl:when test="@nocite='true'">\nocite</xsl:when>
  <xsl:otherwise>\cite</xsl:otherwise>
</xsl:choose>
<xsl:if test="@hasRemark='true'">[<xsl:apply-templates select="html:biblabel"/>]</xsl:if
>{<xsl:value-of select="@citekey"/>}
</xsl:template>

<xsl:template match="html:biblabel"><xsl:apply-templates/></xsl:template>

<xsl:template match="html:bibtexbibliography">\bibliographystyle{<xsl:value-of select="@styleFile"/>}
\bibliography{<xsl:value-of select="@databaseFile"/>}
</xsl:template>

<xsl:template match="html:xref">
<xsl:choose>
  <xsl:when test="@reftype='page'">\vpageref</xsl:when>
  <xsl:otherwise>\vref</xsl:otherwise>
</xsl:choose>
{<xsl:value-of select="@key"/>}\xspace%%
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
  
<xsl:variable name="markOrText" select="parent::html:notewrapper/@markOrText" />
<xsl:variable name="overrideNumber" select="parent::html:notewrapper/@footnoteNumber" />
<xsl:choose>
  <xsl:when test="$endnotes &gt; 0">\protect\endnote</xsl:when>
  <xsl:when test="$markOrText='textOnly'">\protect\footnotetext</xsl:when>
  <xsl:when test="$markOrText='markOnly'">\protect\footnotemark</xsl:when>
  <xsl:otherwise>\protect\footnote</xsl:otherwise>
</xsl:choose>
<xsl:if test="$overrideNumber"><xsl:text>[</xsl:text><xsl:value-of select="$overrideNumber"/><xsl:text>]</xsl:text></xsl:if>
<xsl:if test="not($markOrText='markOnly')">
<xsl:text>{</xsl:text>
<xsl:apply-templates/>
<xsl:text xml:space="preserve">}
</xsl:text>
</xsl:if>
</xsl:template>

<xsl:template match="html:note">\marginpar{<xsl:apply-templates/>}
  
</xsl:template>

<xsl:template match="html:note//html:bodyText[position()=last()]">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:shortQuote"> 
\begin{quote}
<xsl:apply-templates/>
\end{quote}
</xsl:template>


<xsl:template match="html:longQuotation">
  
\begin{quotation}
<xsl:apply-templates/>
\end{quotation}
</xsl:template>

<xsl:template match="html:centeredEnv">
  
\begin{center}
<xsl:apply-templates/>
\end{center}
</xsl:template>

<xsl:template match="html:hebrew">
  
\begin{hebrew}
<xsl:apply-templates/>
\end{hebrew}
</xsl:template>

<xsl:template match="html:arabic">
  
\begin{arabic}
<xsl:apply-templates/>
\end{arabic}
</xsl:template>

<xsl:template match="html:centered">
  
\begin{center}
\par
<xsl:apply-templates/>
\end{center}
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
<xsl:template match="html:bold">\begin{bfseries}<xsl:apply-templates
  />\end{bfseries}</xsl:template>
<xsl:template match="html:italics">\begin{itshape}<xsl:apply-templates
  />\end{itshape}</xsl:template>
<xsl:template match="html:smallbox">\fbox{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:roman">\textrm{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sansSerif">\textsf{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:slanted">\textsl{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:smallCaps">\textsc{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:typewriter">\texttt{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:emphasized">\emph{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:upper">\uppercase{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:lower">\lowercase{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:texthebrew">\texthebrew{<xsl:apply-templates
  />} </xsl:template>
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
<xsl:if test="not(@pre)">
%TCIMACRO{\TeXButton<xsl:apply-templates mode="tex"/>
</xsl:if>
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

<xsl:template match="html:a">\ref{<xsl:apply-templates/>}</xsl:template>


<xsl:template match="html:marker">\label{<xsl:value-of select="@id"/>}</xsl:template>

<xsl:template match="a">\ref{<xsl:apply-templates/>}</xsl:template>

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


<xsl:template match="html:epigraph">
\begin{epigraph}
<xsl:apply-templates/>
\end{epigraph}
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
<xsl:template match="html:a[@name]">\label{<xsl:value-of select="@name"/>}</xsl:template> 

<xsl:template match="html:texb">
  <xsl:if test="not(@pre)" >
    <xsl:if test="@enc='1'">
%TCIMACRO{\TeXButton{<xsl:value-of select="@name"/>}{<xsl:apply-templates/>}}%
%Package required: [<xsl:value-of select="@opt"/>]{<xsl:value-of select="@req"/>}
%BeginExpansion
    </xsl:if>
    <xsl:apply-templates mode="tex"/>
    <xsl:if test="@enc='1'">
      <xsl:text>
%EndExpansion
      </xsl:text>
</xsl:if>
</xsl:if>
</xsl:template>

<xsl:template match="html:bibliography">
\begin{thebibliography}
<xsl:apply-templates select="html:bibitem"/>
\end{thebibliography}
</xsl:template>

<xsl:template match="html:bibitem">
\bibitem<xsl:if test="@hasLabel='true'">[<xsl:apply-templates select="html:biblabel"/>]</xsl:if
>{<xsl:value-of select="@bibitemkey"/>} <xsl:apply-templates select="*[local-name()!='biblabel']"/>
</xsl:template>

<xsl:template match="html:msidisplay">
  <xsl:variable name="subEquationsInScope">
    <xsl:if test="preceding::html:msidisplay[1][@subEquationNumbers='true']">
      <xsl:variable name="ourSection" select="generate-id(ancestor::*[contains($equationNumberingContainers,concat('-',local-name(.),'-'))][1])"/>
      <xsl:variable name="otherSection" select="generate-id(preceding::html:msidisplay[1]/ancestor::*[contains($equationNumberingContainers,concat('-',local-name(.),'-'))][1])"/>
      <xsl:if test="$ourSection=$otherSection">
        <xsl:text>true</xsl:text>
      </xsl:if>
    </xsl:if>
  </xsl:variable>
  <xsl:if test="($subEquationsInScope='true') and not((@subEquationNumbers='true') and (@subEquationContinuation='true'))">
        <xsl:text xml:space="preserve">\end{subequations}
</xsl:text>
  </xsl:if>
  <xsl:if test="(@subEquationNumbers='true') and not((@subEquationContinuation='true') and ($subEquationsInScope='true'))">
    <xsl:text xml:space="preserve">\begin{subequations}
</xsl:text>
  </xsl:if>
  <xsl:choose>
    <xsl:when test=".//mml:mtable[@type='eqnarray']">
    <xsl:for-each select=".//mml:mtable[@type='eqnarray'][1]">
      <xsl:call-template name="eqnarray">
        <xsl:with-param name="n-rows" select="count(./mml:mtr)" />
        <xsl:with-param name="n-labeledrows">
          <xsl:choose>
            <xsl:when test="@numbering='none'">
              <xsl:text>0</xsl:text>
            </xsl:when>
            <xsl:when test="@alignment='alignSingleEqn'">
              <xsl:choose>
              <xsl:when test="./mml:mtr/mml:mtd[not(@numbering='none')]">
                <xsl:text>1</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>0</xsl:text>
              </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="count(./mml:mtr/mml:mtd[not(@numbering='none')])" />
            </xsl:otherwise>
          </xsl:choose>
        </xsl:with-param>
        <xsl:with-param name="n-aligns">
          <xsl:choose>
            <xsl:when test="(@alignment='alignCentered') or (@alignment='alignSingleEqn')">
              <xsl:text>0</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>1</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:with-param>
        <xsl:with-param name="theAlignment" select="@alignment"/>
      </xsl:call-template>
    </xsl:for-each>
    </xsl:when>
    <xsl:otherwise>  <!-- Single-line display - either \equation or \equation* -->
      <xsl:text xml:space="preserve">
\begin{equation</xsl:text>
      <xsl:if test="@numbering='none'">
        <xsl:text>*</xsl:text>
      </xsl:if>
      <xsl:text xml:space="preserve">}
</xsl:text>
      <xsl:apply-templates select="mml:math/*" />
      <xsl:if test="@customLabel and string-length(@customLabel)">
        <xsl:text xml:space="preserve"> \tag{</xsl:text>
        <xsl:value-of select="@customLabel" />
        <xsl:text>}</xsl:text>
      </xsl:if>
      <xsl:choose>
        <xsl:when test="@key and string-length(@key)">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="@key"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
        <xsl:when test="@marker and string-length(@marker)">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="@marker"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
      </xsl:choose>
      <xsl:text xml:space="preserve">
\end{equation</xsl:text>
      <xsl:if test="@numbering='none'">
        <xsl:text>*</xsl:text>
      </xsl:if>
      <xsl:text xml:space="preserve">}
</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:frontmatter"><xsl:apply-templates/></xsl:template>

<xsl:template match="html:rawTeX">
  <xsl:value-of select="@tex"/>
    <xsl:if test="not(string-length(translate(@tex,'}','')) &gt; string-length(translate(@tex,'{','')))">{</xsl:if>
    <xsl:apply-templates/>}</xsl:template>



</xsl:stylesheet>
