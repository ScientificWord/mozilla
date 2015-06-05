<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- Script and Limit Schemata -->

<!-- Utility to determine when an msub, msup, msubsup, munder,
     mover, or munderover corresponds to a LaTeX big operator.
     The caller assigns the output to an xsl:variable -->

  <xsl:template name="is-LaTeX-bigop">      
    <xsl:choose>
      <xsl:when test="normalize-space(string(./*[1]))='&#x222B;'  
         or  normalize-space(string(./*[1]))='&#x222C;' 
         or  normalize-space(string(./*[1]))='&#x222D;' 
         or  normalize-space(string(./*[1]))='&#xE378;' 
         or  normalize-space(string(./*[1]))='&#x222B;&#x22EF;&#x222B;' 
         or  normalize-space(string(./*[1]))='&#x222E;' 
         or  normalize-space(string(./*[1]))='&#x2211;' 
         or  normalize-space(string(./*[1]))='&#x220F;' 
         or  normalize-space(string(./*[1]))='&#x22C2;' 
         or  normalize-space(string(./*[1]))='&#x22C0;' 
         or  normalize-space(string(./*[1]))='&#x2295;' 
         or  normalize-space(string(./*[1]))='&#x2299;' 
         or  normalize-space(string(./*[1]))='&#x2294;' 
         or  normalize-space(string(./*[1]))='&#x2210;' 
         or  normalize-space(string(./*[1]))='&#x22C3;' 
         or  normalize-space(string(./*[1]))='&#x22C1;' 
         or  normalize-space(string(./*[1]))='&#x2297;' 
         or  normalize-space(string(./*[1]))='&#x228E;'">
        <xsl:value-of select="*[1]"/>
      </xsl:when>
	    <xsl:otherwise>
        <xsl:text>false</xsl:text>
	    </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



<!-- mtable, used for a multiline script, calls the following -->

  <xsl:template name="substack">
      <xsl:text>\substack{</xsl:text>

    <xsl:for-each select="mml:mtr">
<!-- Translate line contents -->
      <xsl:for-each select="mml:mtd">
        <xsl:apply-templates/>
      </xsl:for-each>
      <xsl:if test="position()!=last()">
<!-- Generate LaTeX line end sequence -->
        <xsl:text xml:space="preserve"> \\ </xsl:text>
      </xsl:if>
    </xsl:for-each>

    <xsl:text>}</xsl:text>
  </xsl:template>


  <xsl:template match="mml:msub">

    <xsl:variable name="is-big-op">
        <!-- msub may contain an embellished BigOp, \int, \iint, etc. -->
        <xsl:value-of select="*[1]//@largeop='true'"/>
        <!-- xsl:call-template name="is-LaTeX-bigop"/ -->
    </xsl:variable>
  

    <xsl:choose>
      <xsl:when test="$is-big-op!='false'">
<!-- the base element is a big operator -->
        <xsl:call-template name="do-embellished-bigop">
<!--  BBM for bug 3094
         <xsl:with-param name="limits-flag" select="'\nolimits '"/>
 -->
          <xsl:with-param name="j1"          select="'_{'"/>
          <xsl:with-param name="j2"          select="''"/>
        </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
        <xsl:apply-templates select="./*[1]"/>
        <xsl:text>_{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="mml:msub" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template match="mml:msup">
  
    <xsl:variable name="is-big-op">
        <!-- msup may contain an embellished BigOp, \int, \iint, etc. -->
        <xsl:value-of select="*[1]//@largeop='true'"/>
        <!-- xsl:call-template name="is-LaTeX-bigop"/ -->
    </xsl:variable>
  
    <xsl:choose>
      <xsl:when test="$is-big-op!='false'">
        <xsl:call-template name="do-embellished-bigop">
<!--           <xsl:with-param name="limits-flag" select="'\nolimits '"/>
 -->          <xsl:with-param name="j1"          select="'^{'"/>
          <xsl:with-param name="j2"          select="''"/>
        </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
        <xsl:apply-templates select="./*[1]"/>
        <xsl:text>^{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
    </xsl:template>

  <xsl:template match="mml:msup" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
    </xsl:template>


  <xsl:template match="mml:msubsup">
  
    <xsl:variable name="is-big-op">
<!-- msubsup may contain an embellished BigOp, \int, \iint, etc. -->
      <!-- xsl:call-template name="is-LaTeX-bigop"/ -->
      <xsl:value-of select="*[1]//@largeop='true'"/>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$is-big-op!='false'">
        <xsl:call-template name="do-embellished-bigop">
<!--           <xsl:with-param name="limits-flag" select="'\nolimits '"/>
 -->          <xsl:with-param name="j1"          select="'_{'"/>
          <xsl:with-param name="j2"          select="'}^{'"/>
        </xsl:call-template>
	  </xsl:when>
	  <xsl:otherwise>
        <xsl:apply-templates select="./*[1]"/>
        <xsl:text>_{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}^{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="3"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="mml:msubsup" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>

</xsl:stylesheet>

