<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<xsl:template name="HTMLColorToRGB">
  <xsl:param name="colorStr"/>
  <xsl:variable name="digitsStr" select="'0123456789ABCDEF'"/>
  <!-- First ensure string has exactly six characters -->
  <xsl:variable name="fixedStr">
    <xsl:call-template name="fixHTMLColorString">
      <xsl:with-param name="theString" select="$colorStr"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="redVal">
    <xsl:value-of select="16 * string-length(substring-before($digitsStr,substring($fixedStr,1,1))) + string-length(substring-before($digitsStr,substring($fixedStr,2,1)))"/>
  </xsl:variable>
  <xsl:variable name="greenVal">
    <xsl:value-of select="16 * string-length(substring-before($digitsStr,substring($fixedStr,3,1))) + string-length(substring-before($digitsStr,substring($fixedStr,4,1)))"/>
  </xsl:variable>
  <xsl:variable name="blueVal">
    <xsl:value-of select="16 * string-length(substring-before($digitsStr,substring($fixedStr,5,1))) + string-length(substring-before($digitsStr,substring($fixedStr,6,1)))"/>
  </xsl:variable>
  <xsl:value-of select="number($redVal) div 255.0"/><xsl:text>,</xsl:text><xsl:value-of select="number($greenVal) div 255.0"/><xsl:text>,</xsl:text><xsl:value-of select="number($blueVal) div 255.0"/>
</xsl:template>

<xsl:template name="fixHTMLColorString">
  <xsl:param name="theString"/>
  <xsl:choose>
    <xsl:when test="string-length($theString) &gt; 6"><xsl:value-of select="substring($theString, 1, 6)"/></xsl:when>
    <xsl:when test="string-length($theString) &lt; 6">
      <xsl:call-template name="fixHTMLColorString">
        <xsl:with-param name="theString" select="concat('0',$theString)"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise><xsl:value-of select="$theString"/></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:msiframe">
  <xsl:variable name="width">
    <xsl:choose>
      <xsl:when test="@frametype='image'">
        <xsl:call-template name="getObjectWidth">
          <xsl:with-param name="objNode" select="./html:object[1]"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise><xsl:value-of select="@width"/><xsl:value-of select="@units"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="height">
    <xsl:choose>
      <xsl:when test="@frametype='image'">0pt</xsl:when>
      <xsl:otherwise><xsl:value-of select="@height"/><xsl:value-of select="@units"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
	<xsl:if test="@topmargin or @sidemargin or @border or @padding">{</xsl:if>				
  <xsl:if test="@topmargin">\setlength\intextsep{<xsl:value-of select="@topmargin"/><xsl:value-of select="@units"/>}      
  </xsl:if>
  <xsl:if test="@sidemargin">\setlength\columnsep{<xsl:value-of select="@sidemargin"/><xsl:value-of select="@units"/>} </xsl:if>
  <xsl:if test="@border">\setlength\fboxrule{<xsl:value-of select="@border"/><xsl:value-of select="@units"/>} </xsl:if>
  <xsl:if test="@padding">\setlength\fboxsep{<xsl:value-of select="@padding"/><xsl:value-of select="@units"/>} </xsl:if>
<xsl:choose>
  <xsl:when test="@pos='float' and (@placeLocation='h' or @placeLocation='H') and (@placement='L' or @placement='R' or @placement='I' or @placement='O')">\begin{wrapfigure}
    <xsl:if test="@nlines">[<xsl:value-of select="@nlines"/>]</xsl:if>
    <xsl:choose>
      <xsl:when test="@placement='I'">{i}</xsl:when>
      <xsl:when test="@placement='O'">{o}</xsl:when>
      <xsl:when test="@placement='L'">{l}</xsl:when>
      <xsl:when test="@placement='R'">{r}</xsl:when>
      <xsl:otherwise>{r}</xsl:otherwise>
    </xsl:choose>
    <xsl:if test="@overhang">[<xsl:value-of select="@overhang"/><xsl:value-of select="@units"/>]</xsl:if>
    {<xsl:choose><xsl:when test="not(@rotation) or (@rotation='rot0')"><xsl:value-of select="$width"/></xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="@overhang"><xsl:value-of select="$height"/></xsl:when>
          <xsl:otherwise>0pt</xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>}</xsl:when>
  <xsl:when test="@pos='float' and ((@placement='full') or (@frametype='image') or ((@placeLocation !='h') and (@placeLocation !='H')))">\begin{figure}[<xsl:value-of select="@placeLocation"></xsl:value-of>]<xsl:if test="@pos='float' and  (not(@placement) or (@placement='full'))">\begin{center}</xsl:if></xsl:when>
  <xsl:when test="@pos='display'">\begin{center}</xsl:when>
  <xsl:when test="(@pos='inline') and (@frametype='image')">
    {\parbox[t]{<xsl:value-of select="$width"/>}{ %
\begin{center}</xsl:when>
  </xsl:choose>
<xsl:choose>
  <xsl:when test="@frametype='image'">
    <xsl:if test="(./html:imagecaption) and (@captionloc='above')">
      <xsl:if test="@pos='float'">\caption{</xsl:if>
      <xsl:apply-templates select="html:imagecaption[1]"/>
      <xsl:choose>
        <xsl:when test="@pos='float'">}</xsl:when>
        <xsl:otherwise>\\
</xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:apply-templates mode="contents" select="html:object[1]"/>
    <xsl:if test="@captionloc='below'">
      <xsl:choose>
        <xsl:when test="@pos='float'">\caption{</xsl:when>
        <xsl:otherwise>\\
</xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="html:imagecaption[1]"/>
      <xsl:if test="@pos='float'">}</xsl:if>
    </xsl:if>
  </xsl:when>
<xsl:otherwise>
<xsl:if test="@rotation='rot90'">\begin{turn}{-90}</xsl:if>
<xsl:if test="@rotation='rot270'">\begin{turn}{90}</xsl:if>
\fcolorbox<xsl:if test="@border-color"><xsl:choose
 	><xsl:when test="substring(./@border-color,1,1)='#'">[rgb]{<xsl:call-template name="HTMLColorToRGB"><xsl:with-param name="colorStr" select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"/></xsl:call-template
	>}</xsl:when
	><xsl:otherwise>{<xsl:value-of select="./@border-color"/></xsl:otherwise
  ></xsl:choose
  >}</xsl:if><xsl:if test="not(@border-color)">{white}</xsl:if>
<xsl:if test="@background-color">{<xsl:choose
 	><xsl:when test="substring(./@background-color,1,1)='#'">[rgb]{<xsl:call-template name="HTMLColorToRGB"><xsl:with-param name="colorStr" select="translate(substring(./@background-color,2,8),'abcdef','ABCDEF')"/></xsl:call-template
	>}</xsl:when
	><xsl:otherwise><xsl:value-of select="./@background-color"/></xsl:otherwise
  ></xsl:choose
  >}</xsl:if><xsl:if test="not(@background-color)">{white}</xsl:if
>{\begin{minipage}[t]<xsl:if test="@height and not(@height='0')">[<xsl:value-of select="@height"/><xsl:value-of select="@units"/>]</xsl:if
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
>\end{minipage}}<xsl:if test="@rotation!='rot0'">\end{turn}</xsl:if
></xsl:otherwise>
</xsl:choose>
<xsl:if test="@pos='float' and (@placeLocation='h' or @placeLocation='H') and (@placement='L' or @placement='R' or @placement='I' or @placement='O')">\end{wrapfigure}</xsl:if
><xsl:if test="@pos='float' and  (not(@placement) or (@placement='full'))">\end{center}</xsl:if
><xsl:if test="@pos='float' and (@frametype='image' or @placement='full' or (@placeLocation!='h' and @placeLocation!='H'))">\end{figure}</xsl:if
><xsl:if test="@pos='display'">\end{center}</xsl:if
><xsl:if test="@pos='inline' and @frametype='image'">\end{center}}}</xsl:if>
<xsl:if test="@topmargin or @sidemargin or @border or @padding">}</xsl:if>				
</xsl:template>
		  
</xsl:stylesheet>