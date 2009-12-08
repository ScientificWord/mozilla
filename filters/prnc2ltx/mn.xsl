<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- In theory, any unicode could occur in the contents of an <mn>.
  Consequently, each character of an <mn>'s contents must be processed.

  I've added script to output tags for various flavors of mn's.
  Calligraphic and Blackboard Bold aren't handled here - digits
  aren't defined in these sets.
-->

  <xsl:template match="mml:mn">
  
#ifdef DEBUG
    <xsl:message><xsl:value-of select="name(.)"/></xsl:message>
#endif

    <xsl:variable name="LaTeX-symbols">
      <xsl:call-template name="chars-to-LaTeX-Math">
        <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="@mathvariant='bold'
      or              ancestor::mml:mstyle[@fontweight='bold']">
        <xsl:text>\mathbf{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='italic'
      or              ancestor::mml:mstyle[@fontstyle='italic']">
        <xsl:text>\mathit{</xsl:text>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontstyle='slanted']">
	    <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\QTR{sl}{</xsl:text>
	    </xsl:if>
	    <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\mathit{</xsl:text>
	    </xsl:if>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontweight='normal']">
        <xsl:text>\mathrm{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='sans-serif'">
        <xsl:text>\mathsf{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='monospace'">
        <xsl:text>\mathtt{</xsl:text>
      </xsl:when>

      <xsl:when test="@mathvariant='fraktur'">
        <xsl:if test="string-length($LaTeX-symbols) = string-length(normalize-space(string()))">
          <xsl:text>\mathfrak{</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:value-of select="$LaTeX-symbols"/>

    <xsl:choose>
      <xsl:when test="@mathvariant='bold'
      or              ancestor::mml:mstyle[@fontweight='bold']">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='italic'
      or              ancestor::mml:mstyle[@fontstyle='italic']">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontstyle='slanted']">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontweight='normal']">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='sans-serif'">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='monospace'">
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='fraktur'">
        <xsl:if test="string-length($LaTeX-symbols) = string-length(normalize-space(string()))">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


<!-- Note that numbers are only implicit objects in LaTeX.
  In ordinary cases, numbers don't contain anything that forces math.
  However, <mn> is used for numbers in math, and generally occur
  in math expressions.  Hence I am switching to math when <mn>
  is translated in a text bucket.
-->

  <xsl:template match="mml:mn" mode="in-text">
  
#ifdef DEBUG
    <xsl:message>Mode=in-text: <xsl:value-of select="name(.)"/></xsl:message>
#endif
    <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>

</xsl:stylesheet>

