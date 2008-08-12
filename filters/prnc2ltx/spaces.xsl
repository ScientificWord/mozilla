<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<xsl:template match="hspace">
	<xsl:choose>
	  <xsl:when test="@type='normal'"> </xsl:when>
	  <xsl:when test="@type='requiredSpace'">\ </xsl:when>
	  <xsl:when test="@type='emSpace'">\quad </xsl:when>
	  <xsl:when test="@type='twoEmSpace'">\qquad </xsl:when>
	  <xsl:when test="@type='nonBreakingSpace'">~</xsl:when>
	  <xsl:when test="@type='thickSpace'">\thickspace </xsl:when>
	  <xsl:when test="@type='thinSpace'">\thinspace </xsl:when>
	  <xsl:when test="@type='italicCorrectionSpace'">\/</xsl:when>
	  <xsl:when test="@type='negativeThinSpace'">\negthinspace </xsl:when>
	  <xsl:when test="@type='zeroSpace'">{}</xsl:when>
	  <xsl:when test="@type='noIndent'">\noindent </xsl:when>
	  <xsl:when test="@type='stretchySpace'">
		  <xsl:choose>
		  	<xsl:when test="@fillWith='dots'">\dotfill </xsl:when>
				<xsl:when test="@fillWith='line'">\hrulefill </xsl:when>
				<xsl:when test="@flex">\hspace<xsl:if test="@atEnd='true'">*</xsl:if>{\stretch{<xsl:value-of select="@flex"/>}}</xsl:when>
			</xsl:choose>
		</xsl:when>
		<xsl:when test="@type='customSpace'">\hspace<xsl:if test="@atEnd='true'">*</xsl:if>{<xsl:value-of select="@dim"/>}</xsl:when>
		<xsl:otherwise/>
	</xsl:choose>
</xsl:template>
		  

		 
<xsl:template match="vspace">
	<xsl:choose>
	  <xsl:when test="@type='smallSkip'">\smallskip </xsl:when>
	  <xsl:when test="@type='mediumSkip'">\medskip </xsl:when>
	  <xsl:when test="@type='bigSkip'">\bigskip </xsl:when>
	  <xsl:when test="@type='strut'">\strut </xsl:when>
	  <xsl:when test="@type='mathStrut'">\mathstrut </xsl:when>
	  <xsl:when test="@type='customSpace'">\vspace<xsl:if test="@atEnd='true'">*</xsl:if>{<xsl:value-of select="@dim"/>}</xsl:when>
		<xsl:otherwise/>
	</xsl:choose>
</xsl:template>

<xsl:template match="msirule">
<xsl:if test="@color"> \textcolor[HTML]{<xsl:value-of select="substring(./@color,2,8)"/>}{</xsl:if>
\rule[<xsl:value-of select="@lift"/>]{<xsl:value-of select="@width"/>}{<xsl:value-of select="@height"/>}
<xsl:if test="@color">}</xsl:if>

</xsl:template>

</xsl:stylesheet>