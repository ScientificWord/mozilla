<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>  

<xsl:template match="html:object">
  <xsl:choose>
    <xsl:when test="@pos='float'">
      <xsl:choose>
        <xsl:when test="@placement='full'">
          \begin{figure}\begin{center}
        </xsl:when>
        <xsl:otherwise>
          <xsl:if test="@sidemargin &gt; 0">
            \columnsep=<xsl:value-of select="@sidemargin"/><xsl:value-of select="@units"/>
          </xsl:if>
          <xsl:if test="@topmargin &gt; 0">
            \intextsep=<xsl:value-of select="@topmargin"/><xsl:value-of select="@units"/><xsl:text> </xsl:text>
          </xsl:if>
            \begin{wrapfigure}{<xsl:choose>
              <xsl:when test="not(substring(@placement,1,1))">O</xsl:when>
              <xsl:otherwise><xsl:value-of select="substring(@placement,1,1)"/></xsl:otherwise>
            </xsl:choose>}<xsl:if test="@overhang &gt; 0">[<xsl:value-of select="@overhang"/><xsl:value-of select="@units"/>]</xsl:if>{0pt}
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:if test="@pos='display'">
        \begin{center}
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="@captionabove"><xsl:apply-templates/></xsl:if>
  <xsl:if test="@border-width">
    \fboxrule=<xsl:value-of select="@border-width"/><xsl:value-of select="@units"/>
    <xsl:if test="@padding">\fboxsep=<xsl:value-of select="@padding"/><xsl:value-of select="@units"/></xsl:if>
    <xsl:if test="@border-color">{\color{    
    <xsl:choose>
      <xsl:when test="substring(./@border-color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"/>}</xsl:when>
      <xsl:otherwise>black} <!--<xsl:value-of select="./@border-color"/>} --></xsl:otherwise>
    </xsl:choose></xsl:if>
      \framebox{</xsl:if>
  \includegraphics[<xsl:if test="@rot">angle=<xsl:value-of select="@rot"/>,</xsl:if>
  <xsl:if test="@width"> width=<xsl:value-of select="@width"/><xsl:value-of select="@units"/>,</xsl:if>
  <xsl:if test="@height"> totalheight=<xsl:value-of select="@height"/><xsl:value-of select="@units"/>,</xsl:if>
  ]{<xsl:choose>
    <xsl:when test="starts-with(@data,'graphics/')">
      <xsl:value-of select="substring-after(@data,'graphics/')"/>}
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="@data"/>}
    </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="@border-color">}</xsl:if>
  <xsl:if test="@border-width">}</xsl:if>
  <xsl:if test="not(@captionabove)"><xsl:apply-templates/></xsl:if>
  <xsl:choose>
    <xsl:when test="@pos='float'">
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
      <xsl:if test="@pos='display'">
        \end{center}
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
		  		  
<xsl:template match="html:caption">\caption{<xsl:apply-templates/>}</xsl:template>

</xsl:stylesheet>
