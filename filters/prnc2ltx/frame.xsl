<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>
<xsl:template match="html:graph"><xsl:apply-templates /></xsl:template>
<xsl:template match="html:msiframe">
  <xsl:variable name="width">
    <xsl:choose>
      <xsl:when test="@frametype='image'">
        <xsl:call-template name="getObjectWidth">
          <xsl:with-param name="objNode" select="(./html:object|./html:embed)[1]"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@imageWidth"><xsl:value-of select="@imageWidth"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="@width"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="units">
    <xsl:choose>
      <xsl:when test="@frametype='image'"><xsl:call-template name="unit"/></xsl:when>
      <xsl:when test="@units"><xsl:value-of select="@units"/></xsl:when>
      <xsl:otherwise><xsl:text>pt</xsl:text></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="height">
    <xsl:choose>
      <xsl:when test="@frametype='image'">0pt</xsl:when>
      <xsl:when test="@imageHeight"><xsl:value-of select="@imageHeight"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="@height"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="limitframemetrics">
    <xsl:choose>
      <xsl:when test="@topmargin or @sidemargin or @border or @padding">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="isdisplay">
    <xsl:choose>
      <xsl:when test="@pos='display'">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="inlineOffset">
    <xsl:choose>
      <xsl:when test="(@pos='inline') and (@inlineOffset) and (number(@inlineOffset)!=0)">
        <xsl:value-of select="-number(@inlineOffset)"/><xsl:value-of select="$units"/>
      </xsl:when>
      <xsl:otherwise></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="floatsonside">
    <xsl:choose>
      <xsl:when test="@pos='float' and (@placeLocation='h' or @placeLocation='H') and (@placement='L' or @placement='R' or @placement='I' or @placement='O')">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="floatcenter">
    <xsl:choose>
      <xsl:when test="@pos='float' and ((@placement='full') or (@placement='f') or ((@placeLocation !='h') and (@placeLocation !='H')))">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="captionloc">
    <xsl:choose>
      <xsl:when test="(html:imagecaption[1]) and (@captionloc='above')">1</xsl:when>
      <xsl:when test="(html:imagecaption[1]) and (@captionloc='below')">2</xsl:when>
      <!-- <xsl:otherwise></xsl:otherwise> -->
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="usecolor">
    <xsl:choose>
      <xsl:when test="@border-color or @background-color">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="needminipage" select="0"/>
  <xsl:if test="$inlineOffset and string-length($inlineOffset)">\raisebox{<xsl:value-of select="$inlineOffset"/>}{</xsl:if>
	<xsl:if test="$limitframemetrics=1"><xsl:if test="not($inlineOffset) or not(string-length($inlineOffset))">{</xsl:if>
    <xsl:if test="@sidemargin">\setlength\columnsep{<xsl:value-of select="@sidemargin"/>
      <xsl:value-of select="$units"/>}
    </xsl:if>
    <xsl:choose>   
      <xsl:when test="@border">\setlength\fboxrule{<xsl:value-of select="@border"/><xsl:value-of select="$units"/>} 
      </xsl:when>
      <xsl:otherwise>\setlength\fboxrule{0pt} </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="@padding">\setlength\fboxsep{<xsl:value-of select="@padding"/><xsl:value-of select="$units"/>} 
    </xsl:if>
  </xsl:if>
  <xsl:choose>
    <xsl:when test="$floatsonside=1">\begin{wrapfigure}
      <xsl:if test="@nlines">[<xsl:value-of select="@nlines"/>]</xsl:if>
      <xsl:choose>
        <xsl:when test="@placement='I'">{I}</xsl:when>
        <xsl:when test="@placement='O'">{O}</xsl:when>
        <xsl:when test="@placement='L'">{L}</xsl:when>
        <xsl:when test="@placement='R'">{R}</xsl:when>
        <xsl:otherwise>{R}</xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@overhang">[<xsl:value-of select="@overhang"/><xsl:value-of select="$units"/>]</xsl:if>
      {<xsl:choose>
        <xsl:when test="not(@rotation) or (@rotation='rot0')"><xsl:value-of select="$width"/><xsl:value-of select="$units"/></xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="@overhang"><xsl:value-of select="$height"/></xsl:when>
            <xsl:otherwise>0</xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>}
    </xsl:when>
  <!-- <xsl:when test="@pos='float' and ((@placement='full') or (@frametype='image') or ((@placeLocation !='h') and (@placeLocation !='H')))">\begin{figure}[<xsl:value-of select="@placeLocation"></xsl:value-of>]<xsl:if test="@pos='float' and  (not(@placement) or (@placement='full'))">\begin{center}</xsl:if></xsl:when> -->
    <xsl:when test="$floatcenter=1">\begin{figure}[<xsl:value-of select="@placeLocation"/>]</xsl:when>
    <xsl:when test="$isdisplay=1">\begin{center}</xsl:when>
    <xsl:when test="(@pos='inline') and (@frametype='image')">
    {\parbox[b]{<xsl:value-of select="$width"/><xsl:value-of select="$units"/>}{ %
\begin{center}
    </xsl:when>
  </xsl:choose>
  <xsl:if test="$captionloc=1">
    <xsl:if test="@pos='float'">\caption{</xsl:if>
    <xsl:apply-templates select="html:imagecaption[1]" mode="caption"/>
    <xsl:choose>
      <xsl:when test="@pos='float'">}</xsl:when>
      <xsl:otherwise>\\</xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@rotation='rot90'">\begin{turn}{-90}</xsl:if>
  <xsl:if test="@rotation='rot270'">\begin{turn}{90}</xsl:if>{ 
  <xsl:if test="$usecolor=1">\fcolorbox
    <xsl:if test="@border-color">
      <xsl:choose>
        <xsl:when test="substring(./@border-color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"/>
        </xsl:when>
        <xsl:otherwise>{<xsl:value-of select="./@border-color"/></xsl:otherwise>
      </xsl:choose>}
    </xsl:if>
    <xsl:if test="not(@border-color)">{#FFFFFF}</xsl:if>
    <xsl:if test="@background-color">{
      <xsl:choose>
        <xsl:when test="substring(./@background-color,1,1)='#'"><xsl:value-of select="translate(substring(./@background-color,2,8),'abcdef','ABCDEF')"/></xsl:when>
        <xsl:otherwise><xsl:value-of select="./@background-color"/></xsl:otherwise>
      </xsl:choose>}
    </xsl:if>
    <xsl:if test="not(@background-color)">{#FFFFFF}</xsl:if>{
  </xsl:if>
  <xsl:if test="$needminipage=1">
    {\begin{minipage}[t]{
    <xsl:choose>
      <xsl:when test="not(@rotation) or (@rotation='rot0')"><xsl:value-of select="$width"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="$width"/></xsl:otherwise>
    </xsl:choose><xsl:value-of select="$units"/>} %
  </xsl:if>
  <xsl:choose>
    <xsl:when test="@textalignment='center'">\centering </xsl:when>
    <xsl:when test="@textalignment='left'">\begin{FlushLeft}</xsl:when>
    <xsl:when test="@textalignment='right'">\begin{FlushRight}</xsl:when>
  </xsl:choose>
  <xsl:choose>
    <xsl:when test="@frametype='image'">
      <xsl:apply-templates mode="contents" select="(html:object|html:embed)[1]"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates/>
    </xsl:otherwise>
  </xsl:choose>
  <!-- Now back out putting in \end{environment} or } as necessary -->
  <xsl:choose>
    <xsl:when test="@textalignment='left'">\end{FlushLeft}</xsl:when>
    <xsl:when test="@textalignment='right'">\end{FlushRight}</xsl:when>
  </xsl:choose>
  <xsl:if test="$captionloc=2">
    <xsl:choose>
      <xsl:when test="@pos='float'">\caption{</xsl:when>
      <xsl:otherwise>\\ </xsl:otherwise>
    </xsl:choose>
    <xsl:apply-templates select="html:imagecaption[1]" mode="caption"/>
    <xsl:choose>
      <xsl:when test="@pos='float'">}</xsl:when>
      <xsl:otherwise>\\</xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="$needminipage=1">
  \end{minipage}}
  </xsl:if>  
  <xsl:if test="$usecolor=1">}</xsl:if>
  }
  <xsl:choose>
    <xsl:when test="(@pos='inline') and (@frametype='image')">\end{center}}}</xsl:when>
    <xsl:when test="$isdisplay=1">\end{center}</xsl:when>
    <xsl:when test="$floatcenter=1">\end{figure}</xsl:when>
    <xsl:when test="$floatsonside=1">\end{wrapfigure} </xsl:when>   
  </xsl:choose>
  <xsl:if test="($limitframemetrics=1) or ($inlineOffset and string-length($inlineOffset))">}</xsl:if> 
</xsl:template>
		  
</xsl:stylesheet>