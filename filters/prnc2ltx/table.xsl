<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>


<xsl:template name="buildtable">
\begin{tabulary}
  <xsl:choose>
    <xsl:when test="@width &gt; 0">{<xsl:value-of select="@width"/>pt}</xsl:when>
    <xsl:otherwise>{500pt}</xsl:otherwise>
  </xsl:choose>
  <xsl:apply-templates/>
\end{tabulary}
</xsl:template>    

<xsl:template match="html:table">
  <xsl:choose>
    <xsl:when test="@pos='display'">
      \begin{center}
      <xsl:call-template name="buildtable"/>
      \end{center}
    </xsl:when>
    <xsl:when test="@pos='float'"> 
      \begin{wraptable}{
      <xsl:choose>
        <xsl:when test="not(substring(@placement,1,1))">O</xsl:when>
        <xsl:otherwise><xsl:value-of select="substring(@placement,1,1)"/></xsl:otherwise>
      </xsl:choose>}{0pt}
      <xsl:call-template name="buildtable"/>
      \end{wraptable}
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="buildtable"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
		  
<xsl:template match="html:tbody">
  <xsl:apply-templates mode="definecols"/>
  <xsl:apply-templates/>
</xsl:template>
		  

<xsl:template match = "html:tr">
  <xsl:apply-templates/>\\
</xsl:template>

<xsl:template match = "html:tbody/html:tr[1]" mode="definecols">
  {<xsl:apply-templates mode="definecols"/>}
</xsl:template>

<xsl:template match = "html:tbody/html:tr[1]">
  <xsl:apply-templates/>\\\hline
</xsl:template>


<xsl:template match = "html:tbody/html:tr[position()>1]" mode="definecols">
</xsl:template>


<xsl:template match = "html:td"
  ><xsl:if test="position()>1"> &amp; </xsl:if><xsl:apply-templates/>
</xsl:template>

<xsl:template match = "html:td//html:br"> <!-- don't allow \\ in table data-->
</xsl:template>

<xsl:template match = "html:tr[1]/html:td" mode="definecols"
  ><xsl:text> l </xsl:text>
</xsl:template>


</xsl:stylesheet>
