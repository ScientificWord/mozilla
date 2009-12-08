<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<xsl:template match="html:msiframe">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif
	<!--xsl:if test="@position='inner' || @position='outer'" -->				
\begin{wrapfigure}
<xsl:choose>
  <xsl:when test="@position='inner'">{i}</xsl:when>
  <xsl:when test="@position='outer'">{o}</xsl:when>
</xsl:choose>
[<xsl:value-of select="@overhang"/><xsl:value-of select="@units"/>]
{<xsl:choose><xsl:when test="@rotation='rot0'"><xsl:value-of select="@width"/></xsl:when><xsl:otherwise><xsl:value-of select="@height"/></xsl:otherwise></xsl:choose><xsl:value-of select="@units"/>}
\setlength \intextsep {<xsl:value-of select="@topmargin"/><xsl:value-of select="@units"/>}
\setlength \columnsep {<xsl:value-of select="@sidemargin"/><xsl:value-of select="@units"/>}
\setlength \fboxrule {<xsl:value-of select="@border"/><xsl:value-of select="@units"/>}
\setlength \fboxsep {<xsl:value-of select="@padding"/><xsl:value-of select="@units"/>}
<xsl:if test="@rotation='rot90'">\begin{turn}{90}</xsl:if>
<xsl:if test="@rotation='rot270'">\begin{turn}{-90}</xsl:if>
\begin{boxedminipage}[t]{<xsl:choose><xsl:when test="@rotation='rot0'"><xsl:value-of select="@width"/></xsl:when><xsl:otherwise><xsl:value-of select="@height"/></xsl:otherwise></xsl:choose><xsl:value-of select="@units"/>}
<xsl:choose><xsl:when test="@textalignment='inner'">\filouter </xsl:when>
  <xsl:when test="@textalignment='center'">\centering </xsl:when>
  <xsl:when test="@textalignment='justify'">\justifying </xsl:when>
  <xsl:when test="@textalignment='left'">\filright </xsl:when>
  <xsl:when test="@textalignment='right'">\filleft </xsl:when>
  <xsl:when test="@textalignment='outer'">\filinner </xsl:when></xsl:choose>

<xsl:apply-templates/>
		  
\end{boxedminipage} 
<xsl:if test="@rotation!='rot0'">\end{turn}</xsl:if> 
\end{wrapfigure}
</xsl:template>
		  
</xsl:stylesheet>