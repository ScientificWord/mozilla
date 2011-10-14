<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<xsl:template match="html:msiframe">
	<!--xsl:if test="@position='inner' || @position='outer'" -->				
  <xsl:if test="@topmargin">\setlength\intextsep{<xsl:value-of select="@topmargin"/><xsl:value-of select="@units"/>} </xsl:if>
  <xsl:if test="@sidemargin">\setlength\columnsep{<xsl:value-of select="@sidemargin"/><xsl:value-of select="@units"/>} </xsl:if>
  <xsl:if test="@border">\setlength\fboxrule{<xsl:value-of select="@border"/><xsl:value-of select="@units"/>} </xsl:if>
  <xsl:if test="@padding">\setlength\fboxsep{<xsl:value-of select="@padding"/><xsl:value-of select="@units"/>} </xsl:if>
<xsl:choose>
  <xsl:when test="@pos='float' and (@placelocation='h' or @placelocation='H') and (@placement='L' or @placement='R' or @placement='I' or @placement='O')">\begin{wrapfigure}             
    <xsl:if test="@nlines">[<xsl:value-of select="@nlines"/>]</xsl:if>
    <xsl:choose>
      <xsl:when test="@placement='I'">{i}</xsl:when>
      <xsl:when test="@placement='O'">{o}</xsl:when>
      <xsl:when test="@placement='L'">{l}</xsl:when>
      <xsl:when test="@placement='R'">{r}</xsl:when>
      <xsl:otherwise>{r}</xsl:otherwise>
    </xsl:choose>
    <xsl:if test="@overhang">[<xsl:value-of select="@overhang"/><xsl:value-of select="@units"/>]</xsl:if>
    {<xsl:choose><xsl:when test="not(@rotation) or (@rotation='rot0')"><xsl:value-of select="@width"/><xsl:value-of select="@units"/></xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="@overhang"><xsl:value-of select="@height"/><xsl:value-of select="@units"/></xsl:when>
          <xsl:otherwise>0pt</xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>}</xsl:when>
    <xsl:when test="@pos='float' and (@placement='full' or (@placelocation!='h' and @placelocation!='H'))"></xsl:when>
    <xsl:when test="@pos='display'"></xsl:when></xsl:choose>
<xsl:if test="@rotation='rot90'">\begin{turn}{-90}</xsl:if>
<xsl:if test="@rotation='rot270'">\begin{turn}{90}</xsl:if>
\fcolorbox<xsl:if test="@border-color"><xsl:choose
 	><xsl:when test="substring(./@border-color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"
	/></xsl:when
	><xsl:otherwise>{<xsl:value-of select="./@border-color"/></xsl:otherwise
  ></xsl:choose
  >}</xsl:if><xsl:if test="not(@border-color)">{white}</xsl:if>
<xsl:if test="@background-color">{<xsl:choose
 	><xsl:when test="substring(./@background-color,1,1)='#'"><xsl:value-of select="translate(substring(./@background-color,2,8),'abcdef','ABCDEF')"
	/></xsl:when
	><xsl:otherwise><xsl:value-of select="./@background-color"/></xsl:otherwise
  ></xsl:choose
  >}</xsl:if><xsl:if test="not(@background-color)">{white}</xsl:if
>{\begin{minipage}[t]<xsl:if test="@height">[<xsl:value-of select="@height"/><xsl:value-of select="@units"/>]</xsl:if
>{<xsl:choose><xsl:when test="not(@rotation) or (@rotation='rot0')"><xsl:value-of select="@width - 2*@padding"/></xsl:when><xsl:otherwise><xsl:value-of select="@width - 2*@padding"/></xsl:otherwise></xsl:choose><xsl:value-of select="@units"/>} %
<xsl:choose><xsl:when test="@textalignment='center'">\begin{Centering}</xsl:when>
  <xsl:when test="@textalignment='justify'"></xsl:when>
  <xsl:when test="@textalignment='left'">\begin{FlushLeft}</xsl:when>
  <xsl:when test="@textalignment='right'">\begin{FlushRight}</xsl:when></xsl:choose>
<xsl:apply-templates/>
<xsl:choose><xsl:when test="@textalignment='center'">\end{Centering}</xsl:when>
  <xsl:when test="@textalignment='justify'"></xsl:when>
  <xsl:when test="@textalignment='left'">\end{FlushLeft}</xsl:when>
  <xsl:when test="@textalignment='right'">\end{FlushRight}</xsl:when></xsl:choose
>\end{minipage}}<xsl:if test="@rotation!='rot0'">\end{turn}</xsl:if>\end{wrapfigure}
</xsl:template>
		  
</xsl:stylesheet>