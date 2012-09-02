<?xml version="1.0"?>
<xsl:stylesheet version="1.1"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:html="http://www.w3.org/1999/xhtml">

  <xsl:output method="text" encoding="UTF-8"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="*">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="//*[@req]">
    <xsl:call-template name="parsePackageInfo">
      <xsl:with-param name="pkgString" select="@req"/>
      <xsl:with-param name="optString" select="@opt"/>
      <xsl:with-param name="priString" select="@pri"/>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template name="parsePackageInfo">
    <xsl:param name="pkgString" />
    <xsl:param name="optString" />
    <xsl:param name="priString" />
    <xsl:variable name="ourPkg">
      <xsl:choose>
        <xsl:when test="contains($pkgString,';')">
          <xsl:value-of select="substring-before($pkgString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$pkgString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="ourOptions">
      <xsl:choose>
        <xsl:when test="$optString and contains($optString,';')">
          <xsl:value-of select="substring-before($optString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$optString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="ourPri">
      <xsl:choose>
        <xsl:when test="$priString and contains($priString,';')">
          <xsl:value-of select="substring-before($priString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$priString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:value-of select="name()" /><xsl:text>; </xsl:text>
    <xsl:value-of select="$ourPkg" /><xsl:text>; </xsl:text>
    <xsl:value-of select="$ourOptions" /><xsl:text>; </xsl:text>
    <xsl:value-of select="$ourPri" /><xsl:text>
  </xsl:text>
    <xsl:if test="contains($pkgString,';')">
      <xsl:call-template name="parsePackageInfo">
        <xsl:with-param name="pkgString" select="substring-after($pkgString,';')"/>
        <xsl:with-param name="optString" select="substring-after($optString,';')"/>
        <xsl:with-param name="priString" select="substring-after($priString,';')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="text()"></xsl:template>

</xsl:stylesheet>
