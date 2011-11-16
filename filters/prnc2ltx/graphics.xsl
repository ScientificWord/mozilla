<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>  

<xsl:template name="unit">
  <xsl:choose>
    <xsl:when test="@units='px'">pt</xsl:when>
    <xsl:otherwise><xsl:value-of select="@units"/></xsl:otherwise>
  </xsl:choose>
</xsl:template>
  
<xsl:template name="buildincludegraphics">
  <xsl:variable name="theUnit"><xsl:call-template name="unit"/></xsl:variable>
  <xsl:variable name="imageWidth">
    <xsl:call-template name="getImageWidth">
      <xsl:with-param name="objNode" select="."/>
    </xsl:call-template>
  </xsl:variable>
  \includegraphics[<xsl:if test="@rot">angle=<xsl:value-of select="@rot"/>,</xsl:if>
  <xsl:if test="number($imageWidth) != 0"> width=<xsl:value-of select="$imageWidth"/><xsl:value-of select="$theUnit"/>,</xsl:if>
  <xsl:if test="@imageHeight and (number(@imageHeight) != 0)"> totalheight=<xsl:value-of select="@imageHeight"/><xsl:value-of select="$theUnit"/>,</xsl:if>
  <xsl:if test="@naturalWidth and @naturalHeight and (number(@naturalWidth) != 0) and (number(@naturalHeight) != 0)"> natwidth=<xsl:value-of select="@naturalWidth"/><xsl:value-of select="$theUnit"/>, natheight=<xsl:value-of select="@naturalHeight"/><xsl:value-of select="$theUnit"/></xsl:if
>]{<xsl:call-template name="getSourceName"/>}
</xsl:template>    

<xsl:template name="getSourceName">
  <xsl:variable name="rawName">
    <xsl:choose>
      <xsl:when test="@typesetSource"><xsl:value-of select="@typesetSource"/></xsl:when>
      <xsl:when test="@data"><xsl:value-of select="@data"/></xsl:when>
      <xsl:when test="@src"><xsl:value-of select="@src"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="''"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="modifiedName">
    <xsl:choose>
      <xsl:when test="starts-with($rawName,'../')"><xsl:value-of select="substring-after($rawName, '../')"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="$rawName"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="starts-with($modifiedName, 'graphics/')"><xsl:value-of select="substring-after($modifiedName, 'graphics/')"/></xsl:when>
    <xsl:when test="starts-with($modifiedName, 'tcache/')"><xsl:value-of select="substring-after($modifiedName, 'tcache/')"/></xsl:when>
    <xsl:when test="starts-with($modifiedName, 'gcache/')"><xsl:value-of select="substring-after($modifiedName, 'gcache/')"/></xsl:when>
    <xsl:otherwise><xsl:value-of select="$modifiedName"/></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:object" mode="contents">
  <xsl:variable name="theUnit"><xsl:call-template name="unit"/></xsl:variable>
  <xsl:choose>
    <xsl:when test="@pos='float'">
      <xsl:if test="@border-width">
        \setlength\fboxrule{<xsl:value-of select="@border-width"/><xsl:value-of select="$theUnit"/>}
        <xsl:if test="@padding"> \setlength\fboxsep{<xsl:value-of select="@padding"/><xsl:value-of select="$theUnit"/>}</xsl:if>
        <xsl:if test="@border-color">{\color{    
          <xsl:choose>
 	          <xsl:when test="substring(./@border-color,1,1)='#'">[rgb]{<xsl:call-template name="HTMLColorToRGB"><xsl:with-param name="colorStr" select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"/></xsl:call-template>}</xsl:when
 	          ><xsl:otherwise>black} <!--<xsl:value-of select="./@border-color"/>} --></xsl:otherwise>
          </xsl:choose>}</xsl:if>
        \framebox{</xsl:if>
      <xsl:call-template name="buildincludegraphics"/>
      <xsl:if test="@border-color">}</xsl:if>
      <xsl:if test="@border-width">}</xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="buildincludegraphics"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:object">
  <xsl:choose>
    <xsl:when test="@pos='inline'">
      <xsl:apply-templates select="." mode="contents"/>
    </xsl:when>
    <xsl:when test="@pos='display'">
      \begin{center} <xsl:apply-templates select="." mode="contents"/> \end{center}      
    </xsl:when>
    <xsl:when test="@pos='float'">
      <xsl:choose>
        <xsl:when test="@placement='full'">
          \begin{figure}\begin{center}
        </xsl:when>
        <xsl:otherwise>
          <xsl:if test="@sidemargin &gt; 0">
            \columnsep=<xsl:value-of select="@sidemargin"/><xsl:call-template name="unit"/>
          </xsl:if>
          <xsl:if test="@topmargin &gt; 0">
            \intextsep=<xsl:value-of select="@topmargin"/><xsl:call-template name="unit"/><xsl:text> </xsl:text>
          </xsl:if>
            \begin{wrapfigure}{<xsl:choose>
              <xsl:when test="not(substring(@placement,1,1))">O</xsl:when>
              <xsl:otherwise><xsl:value-of select="substring(@placement,1,1)"/></xsl:otherwise>
            </xsl:choose>}<xsl:if test="@overhang &gt; 0">[<xsl:value-of select="@overhang"/><xsl:call-template name="unit"/>]</xsl:if>{0pt}
        </xsl:otherwise>
      </xsl:choose>
      <!-- xsl:if test="@captionabove"><xsl:apply-templates/></xsl:if -->
      <xsl:apply-templates select="." mode="contents"/>
      <xsl:choose>
        <xsl:when test="@placement='full'">
  \end{centerfigure}\begin{figure}
        </xsl:when>
        <xsl:otherwise>
  \end{wrapfigure}
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="." mode="contents"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
		  		  
<xsl:template match="html:caption">\caption{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:imagecaption"><xsl:apply-templates/></xsl:template>

<xsl:template name="getImageWidth">
  <xsl:param name="objNode"/>
  <xsl:param name="noZero" select="false"/>
  <xsl:choose>
    <xsl:when test="$objNode/@imageWidth and (number($objNode/@imageWidth) != 0)"><xsl:value-of select="number($objNode/@imageWidth)"/></xsl:when>
    <xsl:when test="$objNode/@imageHeight and (number($objNode/@imageHeight) != 0) and $objNode/@naturalHeight and (number($objNode/@naturalHeight) != 0) and $objNode/@naturalWidth">
      <xsl:value-of select="(number($objNode/@naturalWidth) * number($objNode/@imageHeight)) div number($objNode/@naturalHeight)"/>
    </xsl:when>
    <xsl:when test="$objNode/@width and (number($objNode/@width) != 0)"><xsl:value-of select="number($objNode/@width)"/></xsl:when>
    <xsl:when test="$objNode/@height and (number($objNode/@height) != 0) and $objNode/@naturalHeight and (number($objNode/@naturalHeight) != 0) and $objNode/@naturalWidth">
      <xsl:value-of select="(number($objNode/@naturalWidth) * number($objNode/@height)) div number($objNode/@naturalHeight)"/>
    </xsl:when>
    <xsl:when test="$noZero and $objNode/@naturalWidth"><xsl:value-of select="number($objNode/@naturalWidth)"/></xsl:when>
    <xsl:otherwise>0</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="getObjectWidth">
  <xsl:param name="objNode"/>
  <xsl:variable name="baseWidth">
    <xsl:call-template name="getImageWidth">
      <xsl:with-param name="objNode" select="$objNode"/>
      <xsl:with-param name="noZero" select="true"/>
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="padding">
    <xsl:choose>
      <xsl:when test="$objNode/@padding"><xsl:value-of select="2*number($objNode/@padding)"/></xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="border">
    <xsl:choose>
      <xsl:when test="$objNode/@border-width"><xsl:value-of select="2*number($objNode/@border-width)"/></xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:value-of select="number($baseWidth) + number($padding) + number($border)" /><xsl:call-template name="unit"/>
</xsl:template>

</xsl:stylesheet>
