<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<xsl:template match="html:table">
\begin{tabular}<xsl:apply-templates/>\end{tabular}
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

<xsl:template match = "html:tbody/html:tr[position()>1]" mode="definecols">
</xsl:template>


<xsl:template match = "html:td"
  ><xsl:if test="position()>1"> &amp; </xsl:if><xsl:apply-templates/>
</xsl:template>

<xsl:template match = "html:tr[1]/html:td" mode="definecols"
  ><xsl:text> l </xsl:text>
</xsl:template>


</xsl:stylesheet>