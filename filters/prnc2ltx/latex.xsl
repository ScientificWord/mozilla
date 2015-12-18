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

<xsl:variable name="newline">
   <!-- xsl:text>&#xA;</xsl:text -->
   <xsl:text>\MsiNewline</xsl:text>
</xsl:variable>

<xsl:variable name="blankline">
   <!-- xsl:value-of select="$newline"/><xsl:value-of select="$newline"/ -->
   <xsl:text>\MsiBlankline</xsl:text>
</xsl:variable>



<xsl:variable name="char-info.tr">
    <char-table>
       <char unicode="&#x03B1;" latex="\alpha"/>
       <char unicode="&#x03B2;" latex="\beta"/>
              <char unicode="&#x03B3;" latex="\gamma"/>
       <char unicode="&#x03B4;" latex="\delta"/>
       <char unicode="&#x03F5;" latex="\epsilon"/>
       <char unicode="&#x03B5;" latex="\varepsilon"/>
       <char unicode="&#x03B6;" latex="\zeta"/>
       <char unicode="&#x03B7;" latex="\eta"/>
       <char unicode="&#x03B8;" latex="\theta"/>
       <char unicode="&#x03B9;" latex="\iota"/>
       <char unicode="&#x03BA;" latex="\kappa"/>
       <char unicode="&#x03BB;" latex="\lambda"/>
       <char unicode="&#x03BC;" latex="\mu"/>
       <char unicode="&#x03BD;" latex="\nu"/>
       <char unicode="&#x03BE;" latex="\xi"/>
       <!-- char unicode="&#x03BF;" latex="\omicron"/ -->
       <char unicode="&#x03C0;" latex="\pi"/>
       <char unicode="&#x03C1;" latex="\rho"/>
       <char unicode="&#x03C3;" latex="\sigma"/>
       <char unicode="&#x03C4;" latex="\tau"/>
       <char unicode="&#x03C5;" latex="\upsilon"/>
       <char unicode="&#x03D5;" latex="\phi"/>
       <char unicode="&#x03C6;" latex="\varphi"/>
       <char unicode="&#x03C7;" latex="\chi"/>
       <char unicode="&#x03C8;" latex="\psi"/>
       <char unicode="&#x03C9;" latex="\omega"/>

       <char unicode="&#x0393;" latex="\Gamma"/>
       <char unicode="&#x0394;" latex="\Delta"/>
       <char unicode="&#x0398;" latex="\Theta"/>
       <char unicode="&#x039B;" latex="\Lambda"/>
       <char unicode="&#x039E;" latex="\Xi"/>
       <char unicode="&#x03A0;" latex="\Pi"/>
       <char unicode="&#x03A3;" latex="\Sigma"/>
       <char unicode="&#x03A5;" latex="\Upsilon"/>
       <char unicode="&#x03A6;" latex="\Phi"/>
       <char unicode="&#x03A8;" latex="\Psi"/>
       <char unicode="&#x03A9;" latex="\Omega"/>

       <char unicode="&#x2A00;" latex="\bigodot" />
       <char unicode="&#x2A02;" latex="\bigotimes" />
       <char unicode="&#x2A04;" latex="\tbiguplus"/>
       <char unicode="&#x2A06;" latex="\bigsqcup"/>
       <char unicode="&#x2A0C;" latex="\iiiint" />
       <char unicode="&#x22C3;" latex="\tbigcup"/>
    </char-table>
</xsl:variable>

<xsl:variable name="char-info" select="exsl:node-set($char-info.tr)"/>

<!-- if the context is something you want stuffed between [...] call
     this -->

<xsl:template name="optional">

   <xsl:variable name="content">
     <xsl:value-of select="."/>
   </xsl:variable>

   <xsl:if test="string-length($content)!='0'">
     <xsl:text>[</xsl:text>
     <xsl:value-of select="."/>
     <xsl:text>]</xsl:text>
   </xsl:if>
</xsl:template>




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

<xsl:preserve-space elements="pre html:bodyText"/>

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

<xsl:template match="*">
<!--   <xsl:text>%[[[What?? </xsl:text>
  <xsl:value-of select="name()"/>
  <xsl:text>]]]</xsl:text>
  <xsl:value-of select="$newline"/>
 -->  <!-- Matches all elements. Make it visible for now
       so we can better catch lost cases. -->
</xsl:template>

<!-- documentclass and usepackage are handled specially by the
     preamble code. So ignore if you see in the clear -->

<xsl:template match="html:documentclass"/>
<xsl:template match="html:sw-meta"/>
<xsl:template match="html:usepackage"/>
<xsl:template match="html:address"/> <!-- Gobbled by <author> -->
<xsl:template match="html:plot"/>
<xsl:template match="html:cursor"/>







<xsl:template match="html:html">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:head">
  <xsl:call-template name="metadata"/>
  <xsl:value-of select="$blankline"/>
  <xsl:text>\documentclass</xsl:text>
    <xsl:if test="//html:colist/@*">
       <xsl:text>[</xsl:text>
       <xsl:for-each select="//html:colist/@*">
          <xsl:if test="name()!='enabled'">
             <xsl:value-of select="."/>
             <xsl:if test="(position()!=last()) and (string-length(normalize-space(.)) > 0)">
               <xsl:text>, </xsl:text>
             </xsl:if>
          </xsl:if>
       </xsl:for-each>
       <xsl:text>]</xsl:text>
     </xsl:if>
     <xsl:text>{</xsl:text>
     <xsl:value-of select="//html:documentclass/@class"/>
     <xsl:text>}</xsl:text>
  <xsl:apply-templates/>
</xsl:template>




<xsl:template match="html:texlogo"><xsl:choose
><xsl:when test="@name='tex'"> \TeX{}</xsl:when>
<xsl:when test="@name='latex'"> \LaTeX{}</xsl:when>
<xsl:when test="@name='pdftex'"> \textsc{pdf}\TeX{}</xsl:when>
<xsl:when test="@name='pdflatex'"> \textsc{pdf}\LaTeX{}</xsl:when>
<xsl:when test="@name='xetex'"> \XeTeX{}</xsl:when>
<xsl:when test="@name='xelatex'"> \XeLaTeX{}</xsl:when>
<xsl:when test="@name='amstex'"> \AmS-\TeX{}</xsl:when>
<xsl:when test="@name='amslatex'"> \AmS-\LaTeX{}</xsl:when>
</xsl:choose></xsl:template>




<xsl:template match="//html:docformat">
<xsl:apply-templates/>
</xsl:template>

<!-- Usepackage is handled specially by the preamble code. So ignore it here -->
<xsl:template match="html:usepackage"></xsl:template>

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
  <xsl:for-each select="//html:assertion|//html:lemma|//html:conjecture|//html:corollary|//html:criterion|//html:proposition|//html:theorem|//html:algorithm|//html:assumption|//html:axiom|//html:condition|//html:definition|//html:example|//html:exercise|//html:hypothesis|//html:problem|//html:property|//html:question|//html:problem|//html:acknowledgment|//html:acknowledgement|//html:case|//html:claim|//html:conclusion|//html:notation|//html:remark|//html:summary|//html:generictheorem">
    <xsl:variable name="existingThmTranslation">
      <xsl:call-template name="checkTeXNameForEnvironments">
        <xsl:with-param name="theTag" select="name(.)"/>
        <xsl:with-param name="theNumbering">
          <xsl:if test="not(not(@numbering)) and string-length(@numbering)"><xsl:value-of select="@numbering"/></xsl:if>
        </xsl:with-param>
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

  <xsl:variable name="n-msidisplays">
    <xsl:value-of select="count(.//html:msidisplay)" />
  </xsl:variable>

  <xsl:if test="contains($equationNumberingContainers, concat('-',local-name(.),'-'))">
    <xsl:if test="descendant::html:msidisplay[last()][@subEquationNumbers='true']
        and (descendant::html:msidisplay[last()]/ancestor::*[contains($equationNumberingContainers, concat('-',local-name(.),'-'))][1]=current())">
      <xsl:value-of select="$newline"/>
      <xsl:text>\end{subequations}</xsl:text>
    </xsl:if>
  </xsl:if>

</xsl:template>

<xsl:template match="html:body">
   <!--\input tcilatex.tex  should not be done under some conditions -->
   <xsl:if test="count(//html:indexitem) &gt; 0">
     <xsl:value-of select="$newline"/>
     <xsl:text>\makeindex</xsl:text>
   </xsl:if>
   <xsl:value-of select="$newline"/>
   <xsl:text>\begin{document}</xsl:text>
   <xsl:apply-templates/>
   <xsl:if test="($endnotes &gt; 0) and ($footnotecount &gt; 0)">
      <xsl:value-of select="$newline"/>
      <xsl:text>\theendnotes </xsl:text>
   </xsl:if>
   <xsl:if test="$indexitems &gt; 0">
     <xsl:if test="count(//html:printindex) = 0">
       <xsl:value-of select="$newline"/>
       <xsl:text>\printindex </xsl:text>
     </xsl:if>
   </xsl:if>
   <xsl:call-template name="checkEndSubEquationsScope"/>
   <xsl:value-of select="$newline"/>
   <xsl:text>\end{document}</xsl:text>
</xsl:template>

<xsl:template match="html:printindex">
\printindex
</xsl:template>

<xsl:template match="html:br[@hard='1']">~\\
</xsl:template>

<!-- use version in spaces.xsl
<xsl:template match="html:msibr">~\\
</xsl:template>
-->

<xsl:template match="html:br|mml:br">
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="html:br" mode="verb">
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="html:author">
  <xsl:if test="not (preceding-sibling::html:author)">
     <xsl:value-of select="$newline"/>
     <xsl:text>\author{</xsl:text>
     <xsl:apply-templates/>
     <xsl:if test="name(following-sibling::*[1])='address'">
       <xsl:text>~\\</xsl:text>
       <xsl:value-of select="$newline"/>
       <xsl:apply-templates select="following-sibling::*[1]" mode="frontmatter" />
     </xsl:if>
     <xsl:apply-templates select="following-sibling::html:author" mode="building-author" />
     <xsl:text>}</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="html:address" mode="frontmatter">
   <xsl:apply-templates/>
</xsl:template>


<!-- for the sake of the above template, -->
<xsl:template match="html:msibr" mode="frontmatter">
  <xsl:text>~\\</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="html:author" mode="building-author">
 \and <xsl:apply-templates/>
  <xsl:if test="name(following-sibling::*[1])='address'">~\\
    <xsl:apply-templates select="following-sibling::*[1]"/>
  </xsl:if>
</xsl:template>



<!-- Special handling for footnotes in front matter tags
 -->

<xsl:template match="html:note[@type='footnote']" mode="frontmatter">\thanks{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:abstract">
    <xsl:value-of select="$newline"/>
    <xsl:text>\begin{abstract}</xsl:text>
    <xsl:apply-templates/>
    <xsl:value-of select="$newline"/>
    <xsl:text>\end{abstract}</xsl:text>
</xsl:template>

<xsl:template match="html:maketitle">
   <xsl:value-of select="$newline"/>
   <xsl:text>\maketitle</xsl:text>
   <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template name="maketables">
<!--  \tableofcontents <xsl:text/>
   <xsl:if test="//html:lof">
      <xsl:value-of select="$newline"/>
      <xsl:text>\listoffigures</xsl:text>
   </xsl:if>
   <xsl:if test="//html:lot">
      <xsl:value-of select="$newline"/>
      <xsl:text>\listoftables</xsl:text>
   </xsl:if> -->
</xsl:template>

<xsl:template match="html:maketoc">
  <xsl:value-of select="$newline"/>
  <xsl:text>\tableofcontents</xsl:text>
</xsl:template>

<xsl:template match="html:makelof">
  <xsl:value-of select="$newline"/>
  <xsl:text>\listoffigures</xsl:text>
</xsl:template>

<xsl:template match="html:makelot">
  <xsl:value-of select="$newline"/>
  <xsl:text>\listoftables</xsl:text>
</xsl:template>

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

<xsl:template match="html:bodyText|mml:bodyText" mode="verb">
   <xsl:apply-templates mode="verb"/>
</xsl:template>


<xsl:template match="html:bodyText|mml:bodyText">
   <xsl:variable name="content">
     <!-- xsl:value-of select="."/ -->
     <xsl:apply-templates />
   </xsl:variable>
  <xsl:if test="(position() = 1) and (starts-with($toclocation,'tocpara'))">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:if test="position()!=last or string-length($content) != '0'">
    <xsl:apply-templates/>
  </xsl:if>
  <xsl:if test="position()!=last()">
     <xsl:value-of select="$blankline"/>
  </xsl:if>
</xsl:template>

<xsl:template match="html:rtlBodyText|mml:rtlBodyText">
  <xsl:text>\begin{RTL}</xsl:text>
  <xsl:variable name="content">
     <!-- xsl:value-of select="."/ -->
     <xsl:apply-templates />
   </xsl:variable>
  <xsl:if test="(position() = 1) and (starts-with($toclocation,'tocpara'))">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:if test="position()!=last or string-length($content) != '0'">
    <xsl:apply-templates/>
  </xsl:if>
  <xsl:if test="position()!=last()">
     <xsl:value-of select="$blankline"/>
  </xsl:if>
  <xsl:text>\end{RTL}</xsl:text>
</xsl:template>

<xsl:template match="html:bodyText" mode="inner">
  <xsl:if test="(position() = 1) and (starts-with($toclocation,'tocpara'))">
    <xsl:call-template name="maketables" />
  </xsl:if>
  <xsl:apply-templates/>
  <xsl:if test="position()!=last()">
     <xsl:text>\par </xsl:text>
   </xsl:if>
</xsl:template>

<xsl:template match="html:bodyText" mode="frontmatter">
  <xsl:apply-templates/>
  <xsl:if test="position()!=last()">
     <xsl:text>\\</xsl:text>
     <xsl:value-of select="$newline"/>
  </xsl:if>
</xsl:template>


<xsl:template match="html:bodyMath">
  <xsl:if test="(position() = 1) and (starts-with($toclocation,'tocpara'))">
    <xsl:call-template name="maketables" />
  </xsl:if>
<xsl:apply-templates/>
<xsl:if test="position()!=last()">\par </xsl:if>
</xsl:template>



<xsl:template match="html:sectiontitle">
   <xsl:value-of select="$blankline"/>
   <xsl:choose>
      <xsl:when test="name(..)='part'">
         <xsl:text>\part</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='chapter'">
         <xsl:text>\chapter</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='section'">
         <xsl:text>\section</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='subsection'">
         <xsl:text>\subsection</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='subsubsection'">
         <xsl:text>\subsubsection</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='paragraph'">
         <xsl:text>\paragraph</xsl:text>
      </xsl:when>

      <xsl:when test="name(..)='subparagraph'">
         <xsl:text>\subparagraph</xsl:text>
      </xsl:when>

    </xsl:choose>
    <xsl:if test="../@nonum='true'">
       <xsl:text>*</xsl:text>
    </xsl:if>
    <xsl:apply-templates mode="shortTitle"/>
    <xsl:text>{</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="html:title">
  <xsl:value-of select="$newline"/>
  <xsl:text>\title</xsl:text>
  <xsl:apply-templates mode="shortTitle"/>
  <xsl:text>{</xsl:text>
  <xsl:apply-templates/>
  <xsl:text>}</xsl:text>
</xsl:template>


<xsl:template match="*" mode="shortTitle"></xsl:template>
<xsl:template match="*/text()" mode="shortTitle"></xsl:template>
<xsl:template match="html:shortTitle"></xsl:template>
<xsl:template match="html:shortTitle" mode="shortTitle">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:assertion|html:lemma|html:conjecture|html:corollary|html:criterion|html:proposition|html:theorem|html:algorithm|html:assumption|html:axiom|html:condition|html:definition|html:example|html:exercise|html:hypothesis|html:problem|html:property|html:question|html:acknowledgment|html:acknowledgement|html:case|html:claim|html:conclusion|html:notation|html:remark|html:summary">
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
  <xsl:value-of select="$newline"/>
  <xsl:text>\begin{</xsl:text><xsl:value-of select="$tagnameToUse"/><xsl:text>}</xsl:text>
  <xsl:apply-templates mode="envleadin"/>
     <xsl:value-of select="$newline"/>
     <xsl:apply-templates/>
  <xsl:value-of select="$newline"/>
  <xsl:text>\end{</xsl:text><xsl:value-of select="$tagnameToUse"/><xsl:text>}</xsl:text>
  <xsl:value-of select="$blankline"/>
</xsl:template>

<xsl:template match="html:environment">
  <xsl:value-of select="$newline"/>
  <xsl:text>\begin{</xsl:text>
  <xsl:value-of select="@type"/>
  <xsl:text>}</xsl:text>
  <xsl:if test="@param">
    <xsl:text>{</xsl:text>
      <xsl:value-of select="@param"/>
    <xsl:text>}</xsl:text>
  </xsl:if>
  <xsl:apply-templates mode="envleadin"/>
  <xsl:apply-templates/>
  <xsl:value-of select="$newline"/>
  <xsl:text>\end{</xsl:text>
  <xsl:value-of select="@type"/>
  <xsl:text>}</xsl:text>
  <xsl:value-of select="$newline"/>
</xsl:template>


<xsl:template match="html:proof">
  <xsl:value-of select="$newline"/>
  <xsl:text>\begin{proof}</xsl:text>
  <xsl:value-of select="$newline"/>
  <xsl:apply-templates mode="envleadin"/>
  <xsl:apply-templates/>
  <xsl:value-of select="$newline"/>
  <xsl:text>\end{proof}</xsl:text>
  <xsl:value-of select="$blankline"/>
</xsl:template>

<xsl:template match="html:envLeadIn"></xsl:template>
<xsl:template match="@*|node()" mode="envleadin"></xsl:template>
<xsl:template match="html:envLeadIn" mode="envleadin">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:drop">
\lettrine[lhang=.2]{\textbf{<xsl:apply-templates/>}}{}
</xsl:template>


<xsl:template match="html:numberedlist">
  <xsl:if test="*">
    <xsl:text>\begin{enumerate}</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:apply-templates/>
    <xsl:text>\end{enumerate}</xsl:text>
    <xsl:value-of select="$newline"/>
  </xsl:if>
</xsl:template>

<xsl:template match="html:bulletlist">
  <xsl:if test="*">
    <xsl:text>\begin{itemize}</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:apply-templates/>
    <xsl:text>\end{itemize}</xsl:text>
    <xsl:value-of select="$newline"/>
  </xsl:if>
</xsl:template>

<xsl:template match="html:descriptionlist">
   <xsl:if test="*">
     <xsl:text>\begin{description}</xsl:text>
     <xsl:value-of select="$newline"/>
     <xsl:apply-templates/>
     <xsl:text>\end{description}</xsl:text>
   </xsl:if>
</xsl:template>


<xsl:template match="html:numberedListItem">

   <xsl:text>\item </xsl:text>
   <xsl:apply-templates/>
   <xsl:if test="position() != last()">
     <xsl:value-of select="$blankline"/>
   </xsl:if>
</xsl:template>

<!-- If we have an optional argument that contains a ] then we add {...} to make sure that
     we don't accidently close the opening [ of the optional argument -->

<xsl:template match="html:numberedLabel">
  <xsl:variable name="theLabel">
    <xsl:apply-templates/>
  </xsl:variable>
  <xsl:variable name="theText">
    <xsl:value-of select="$theLabel"/>
  </xsl:variable>
  <xsl:text>[</xsl:text>
  <xsl:choose>
     <xsl:when test="contains($theText, ']')" >
        <xsl:text>{</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
     </xsl:when>
     <xsl:otherwise>
        <xsl:apply-templates/>
     </xsl:otherwise>
  </xsl:choose>
  <xsl:text>]</xsl:text>
</xsl:template>

<xsl:template match="html:bulletListItem">
  <xsl:text>\item </xsl:text>
   <xsl:apply-templates/>
   <xsl:if test="position() != last()">
     <xsl:value-of select="$blankline"/>
   </xsl:if>
</xsl:template>

<xsl:template match="html:bulletLabel">
  <xsl:call-template name="optional"/>
</xsl:template>

<xsl:template match="html:descriptionListItem">
  <xsl:text>\item </xsl:text>
   <xsl:apply-templates/>
   <xsl:if test="position() != last()">
     <xsl:value-of select="$blankline"/>
   </xsl:if>
</xsl:template>

<xsl:template match="html:descriptionLabel">
  <xsl:call-template name="optional"/>
  <!-- [<xsl:apply-templates/>] -->
</xsl:template>

<xsl:template match="html:citation">
<xsl:choose>
  <xsl:when test="@nocite='true'">\nocite</xsl:when>
  <xsl:otherwise>\cite</xsl:otherwise></xsl:choose>
<xsl:if test="@hasRemark='true'"><xsl:apply-templates select="html:biblabel"/></xsl:if>{<xsl:value-of select="@citekey"/>}</xsl:template>

<!-- biblabe defined below
<xsl:template match="html:biblabel"><xsl:apply-templates/></xsl:template>
-->

<xsl:template match="html:bibtexbibliography">\bibliographystyle{<xsl:value-of select="@styleFile"/>}
\bibliography{<xsl:value-of select="@databaseFile"/>}
</xsl:template>

<xsl:template match="html:xref|mml:xref">
  <xsl:choose>
    <xsl:when test="@req='varioref'">
      <xsl:choose>
        <xsl:when test="@reftype='page'">\vpageref{<xsl:value-of select="@key"/>}</xsl:when>
        <xsl:otherwise>\vref{<xsl:value-of select="@key"/>}</xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="@reftype='page'">\pageref{<xsl:value-of select="@key"/>}</xsl:when>
        <xsl:otherwise>\ref{<xsl:value-of select="@key"/>}</xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="html:prispec">@<xsl:apply-templates/></xsl:template>
<xsl:template match="html:secspec">@<xsl:apply-templates/></xsl:template>
<xsl:template match="html:terspec">@<xsl:apply-templates/></xsl:template>

<xsl:template match="html:indexitem">
  <xsl:text>\index</xsl:text>
  <xsl:text>{</xsl:text>
  <xsl:if test="@pri">
    <xsl:value-of select="@pri"/>
    <xsl:apply-templates select="html:prispec"/>
  </xsl:if>
  <xsl:if test="@sec">
    <xsl:text>!</xsl:text>
    <xsl:value-of select="@sec"/>
    <xsl:apply-templates select="html:secspec"/>
  </xsl:if>
  <xsl:if test="@ter">
    <xsl:text>!</xsl:text>
    <xsl:value-of select="@ter"/>
    <xsl:apply-templates select="html:terspec"/>
  </xsl:if>
  <xsl:if test="@enc">
    <xsl:text>|</xsl:text>
    <xsl:value-of select="@enc"/>
  </xsl:if>
  <xsl:text>}</xsl:text>
</xsl:template>

<xsl:template match="html:notewrapper">
  <xsl:apply-templates/>
</xsl:template>


<xsl:template match="html:note[@type='footnote']">

  <xsl:variable name="markOrText" select="parent::html:notewrapper/@markOrText" />

  <xsl:variable name="overrideNumber" select="parent::html:notewrapper/@footnoteNumber" />

  <xsl:choose>
    <xsl:when test="$endnotes &gt; 0">\protect\endnote</xsl:when>
    <xsl:when test="$markOrText='textOnly'">\protect\footnotetext</xsl:when>
    <xsl:when test="$markOrText='markOnly'">\protect\footnotemark</xsl:when>
    <xsl:otherwise>
      <xsl:text>\protect\footnote</xsl:text>
    </xsl:otherwise>
  </xsl:choose>

  <xsl:if test="$overrideNumber">
    <xsl:text>[</xsl:text>
    <xsl:value-of select="$overrideNumber"/>
    <xsl:text>]</xsl:text>
  </xsl:if>

  <xsl:if test="not($markOrText='markOnly')">
    <xsl:text>{</xsl:text>
    <xsl:apply-templates mode="inner"/>
    <xsl:text >}</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="html:note">
   <xsl:text>\marginpar{</xsl:text>
   <xsl:apply-templates/>
   <xsl:text>}</xsl:text>
</xsl:template>

<xsl:template match="html:note//html:bodyText[position()=last()]">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:note//html:bodyMath[position()=last()]">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:verbatim">
  <xsl:value-of select="$newline"/>
  <xsl:text>\begin{verbatim}</xsl:text>
  <xsl:apply-templates mode="verb"/>
  <xsl:text>\end{verbatim}</xsl:text>
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
\end{center}\par
</xsl:template>

<xsl:template match="html:hebrew">

\begin{hebrew}
<xsl:apply-templates/>
\end{hebrew}
</xsl:template>

<xsl:template match="html:Arabic">
\begin{Arabic}
<xsl:apply-templates/>
\end{Arabic}
</xsl:template>

<xsl:template match="html:centered">
\begin{center}<xsl:apply-templates/>\end{center}\par
</xsl:template>

<xsl:template match="html:flushleft">
\begin{flushleft}<xsl:apply-templates/>\end{flushleft}\par
</xsl:template>

<xsl:template match="html:flushright">
\begin{flushright}<xsl:apply-templates/>\end{flushright}\par
</xsl:template>

<xsl:template match="html:pre">
\begin{verbatim}
<xsl:value-of select="."/>
\end{verbatim}
</xsl:template>

<xsl:template match="html:p">\par<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:alt">{\addfontfeatures{RawFeature=+salt}<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:bold | mml:bold">\textbf{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:italics | mml:italics">\textit{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:smallbox">\fbox{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:roman | mml:roman">\textrm{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sansSerif | mml:sansSerif">\textsf{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:slanted | mml:slanted">\textsl{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:smallCaps | mml:smallCaps">\textsc{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:typewriter | mml:typewriter">\texttt{<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:emphasized | mml:emphasized">\emph{<xsl:apply-templates
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
<xsl:template match="html:underline">\underline {<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sub">\textsubscript {<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="html:sup">\textsuperscript {<xsl:apply-templates
  />}</xsl:template>
<xsl:template match="mml:mi[@msimathname='true']">
	<xsl:choose>
		<xsl:when test=".='arccos'">\arccos </xsl:when>
    <xsl:when test=".='cos'">\cos </xsl:when>
    <xsl:when test=".='csc'">\csc </xsl:when>
    <xsl:when test=".='exp'">\exp </xsl:when>
    <xsl:when test=".='ker'">\ker </xsl:when>
    <xsl:when test=".='limsup'">\limsup </xsl:when>
    <xsl:when test=".='min'">\min </xsl:when>
    <xsl:when test=".='sinh'">\sinh </xsl:when>
    <xsl:when test=".='arcsin'">\arcsin </xsl:when>
    <xsl:when test=".='cosh'">\cosh </xsl:when>
    <xsl:when test=".='deg'">\deg </xsl:when>
    <xsl:when test=".='gcd'">\gcd </xsl:when>
    <xsl:when test=".='lg'">\lg </xsl:when>
    <xsl:when test=".='ln'">\ln </xsl:when>
    <xsl:when test=".='Pr'">\Pr </xsl:when>
    <xsl:when test=".='sup'">\sup </xsl:when>
    <xsl:when test=".='arctan'">\arctan </xsl:when>
    <xsl:when test=".='cot'">\cot </xsl:when>
    <xsl:when test=".='det'">\det </xsl:when>
    <xsl:when test=".='hom'">\hom </xsl:when>
    <xsl:when test=".='lim'">\lim </xsl:when>
    <xsl:when test=".='log'">\log </xsl:when>
    <xsl:when test=".='sec'">\sec </xsl:when>
    <xsl:when test=".='tan'">\tan </xsl:when>
    <xsl:when test=".='arg'">\arg </xsl:when>
    <xsl:when test=".='coth'">\coth </xsl:when>
    <xsl:when test=".='dim'">\dim </xsl:when>
    <xsl:when test=".='inf'">\inf </xsl:when>
    <xsl:when test=".='liminf'">\liminf </xsl:when>
    <xsl:when test=".='max'">\max </xsl:when>
    <xsl:when test=".='sin'">\sin </xsl:when>
    <xsl:when test=".='tanh'">\tanh </xsl:when>
    <xsl:otherwise>\ensuremath{\operatorname*{<xsl:apply-templates/>}}</xsl:otherwise>
	</xsl:choose>
</xsl:template>
<xsl:template match="mml:mi[@msiunit='true']">
  <xsl:apply-templates/>
</xsl:template>
<!-- <xsl:template match="mml:mi[@msiunit='true']">
  \ensuremath{\operatorname{<xsl:apply-templates/>}}
</xsl:template> -->
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

<xsl:template match="html:ltxframe">\frame{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:tex"
  >\TeX{}</xsl:template>
<xsl:template match="html:textquotedblleft"
  >\textquotedblleft </xsl:template>
<xsl:template match="html:textquotedblright"
  >\textquotedblright </xsl:template>
<xsl:template match="html:textbackslash"
  >\textbackslash </xsl:template>

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

<xsl:template match="html:explicit-item">
  <xsl:call-template name="optional"/>
 <!--  [<xsl:apply-templates/>]  -->
</xsl:template>

<xsl:template match="html:a">\href{<xsl:value-of select="@href"/>}
{<xsl:apply-templates/>} </xsl:template>


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

<!-- Just send Body Math through without additional markup -->
<xsl:template match="html:QTP[@type='Body Math']"><xsl:value-of select="@type"/>
<xsl:apply-templates/>
\par
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
  <xsl:if test="not(@pre) or (@pre='0')" >
    <xsl:if test="@enc='1'">
      <!-- add newline if needed -->
      <xsl:variable name="next">
         <xsl:value-of select="preceding-sibling::*[last()]" />
      </xsl:variable>
      <xsl:variable name="has-newline" >
         <xsl:value-of select="substring($next, string-length($next),1)='&#xA;'" />
      </xsl:variable>
      <xsl:if test="$has-newline='false'">
         <xsl:value-of select="$newline"/>
      </xsl:if>
      <xsl:text>%TCIMACRO{\TeXButton{</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>}{</xsl:text>
      <xsl:apply-templates mode="texcomment"/>
      <xsl:text>}}%</xsl:text>
      <!-- %Package required: [<xsl:value-of select="@opt"/>]{<xsl:value-of select="@req"/>} -->
      <xsl:value-of select="$newline"/>
      <xsl:text>%BeginExpansion</xsl:text>
      <xsl:value-of select="$newline"/>
    </xsl:if>
    <xsl:apply-templates mode="tex"/>
    <xsl:if test="@enc='1'">
      <xsl:value-of select="$newline"/>
      <xsl:text>%EndExpansion</xsl:text>
      <!-- add newline if needed -->
      <xsl:variable name="next">
         <xsl:value-of select="following-sibling::text()[1]" />
      </xsl:variable>
      <xsl:variable name="has-newline" >
        <xsl:value-of select="substring($next,1,1)='&#xA;'" />
      </xsl:variable>
      <xsl:if test="$has-newline='false'">
         <xsl:value-of select="$newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template match="html:bibliography">
<xsl:if test="*">
\begin{thebibliography}{99}
<xsl:apply-templates select="html:bibitem"/>
\end{thebibliography}</xsl:if>
</xsl:template>

<xsl:template match="html:bibitem">
   <xsl:text>\bibitem </xsl:text>
   <xsl:apply-templates/>
   <xsl:if test="position() != last()">
     <xsl:value-of select="$blankline"/>
   </xsl:if>
</xsl:template>

<xsl:template match="html:bibkey">
  <xsl:text>{</xsl:text>
  <xsl:value-of select="."/>
  <xsl:text>}</xsl:text>
</xsl:template>

<xsl:template match="html:biblabel">
  <xsl:if test = "string-length(normalize-space(.)) > 0">
    <xsl:call-template name="optional"/>
  </xsl:if>
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
            <xsl:when test="@numbering='none' or ancestor::html:msidisplay[@numbering='none']">
              <xsl:text>0</xsl:text>
            </xsl:when>
            <xsl:when test="@alignment='alignSingleEqn'">
              <xsl:choose>
              <xsl:when test="./mml:mtr[not(@numbering='none')]">
                <xsl:text>1</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>0</xsl:text>
              </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="count(./mml:mtr[not(@numbering='none')])" />
            </xsl:otherwise>
          </xsl:choose>
        </xsl:with-param>
        <xsl:with-param name="n-aligns">
        <!-- jcs I don't understand this. It seems we want to count the number of alignments.
             <xsl:choose>
               <xsl:when test="(@alignment='alignCentered') or (@alignment='alignSingleEqn')">
                 <xsl:text>0</xsl:text>
               </xsl:when>
               <xsl:otherwise>
                 <xsl:text>1</xsl:text>
               </xsl:otherwise>
             </xsl:choose>
          -->
          <xsl:value-of select="count(./mml:mtr[1]/mml:mtd/mml:maligngroup)"/>

        </xsl:with-param>
        <xsl:with-param name="theAlignment" select="@alignment"/>
      </xsl:call-template>
    </xsl:for-each>
    </xsl:when>
    <xsl:otherwise>  <!-- Single-line display - either \equation or \equation* -->
      <xsl:text>\begin{equation</xsl:text>
      <xsl:if test="@numbering='none'">
        <xsl:text>*</xsl:text>
      </xsl:if>
      <xsl:text>}</xsl:text>
      <xsl:apply-templates select="mml:math/*" />
      <xsl:if test="@customLabel and string-length(@customLabel)">
        <xsl:text>\tag</xsl:text>
          <xsl:if test="@suppressAnnotation='true'">
             <xsl:text>*</xsl:text>
          </xsl:if>
        <xsl:text>{</xsl:text>
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
      <xsl:value-of select="$newline"/>
      <xsl:text>\end{equation</xsl:text>
      <xsl:if test="@numbering='none'">
        <xsl:text>*</xsl:text>
      </xsl:if>
      <xsl:text>}</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:frontmatter">
   <xsl:value-of select="$newline"/>
   <xsl:text>\frontmatter</xsl:text>
</xsl:template>
<xsl:template match="html:mainmatter">\mainmatter<xsl:apply-templates/></xsl:template>
<xsl:template match="html:backmatter">\backmatter<xsl:apply-templates/></xsl:template>
<xsl:template match="html:appendix">\appendix</xsl:template>
<xsl:template match="html:printindex">\printindex</xsl:template>

<xsl:template match="html:rawTeX">
  <xsl:value-of select="@tex"/>
  <xsl:choose>
    <xsl:when test="@post">
      <xsl:apply-templates/><xsl:value-of select="@post"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:if test="not(string-length(translate(@tex,'}','')) &gt; string-length(translate(@tex,'{','')))">{</xsl:if>
      <xsl:apply-templates/>}
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:graphSpec">
</xsl:template>

<!-- Start definitions for ulem package -->
<xsl:template match="html:QTR[@type='QQQuline']">
\uline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQuuline']">
\uuline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQuwave']">
\uwave{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQsout']">
\sout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQxout']">
\xout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uline">
\uline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uuline">
\uuline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uwave">
\uwave{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:sout">
\sout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:xout">
\xout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:dashuline">
\dashuline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:dotuline">
\dotuline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>
<!-- End definitions for ulem package -->

<!-- Start definitions for beamer package -->
<xsl:template match="html:QTR[@type='frametitle']">
\frametitle{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='framesubtitle']">
\framesubtitle{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>
<!-- End definitions for beamer package -->
</xsl:stylesheet>

