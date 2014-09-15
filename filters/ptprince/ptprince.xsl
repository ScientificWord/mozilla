<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
</xsl:template>

<xsl:template match="html:tr|mml:tr">
  <xsl:variable name="content">
     <xsl:value-of select="."/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="(position()=last()) and ($content='')">
    </xsl:when>
    <xsl:otherwise>
       <xsl:copy>
          <xsl:apply-templates select="@*|node()"/>
       </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="mml:mrow">
  <xsl:choose>
    <xsl:when test="
                    parent::mml:mfrac or 
                    parent::mml:msub or 
                    parent::mml:msup or
                    parent::mml:msubsup or
                    parent::mml:munder or
                    parent::mml:mover or
                    parent::mml:munderover">
      <mml:mrow><xsl:apply-templates/></mml:mrow>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>



</xsl:stylesheet>