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
    <xsl:value-of select="name()" />
    <xsl:text>;</xsl:text>
    <xsl:value-of select="@req" />
    <xsl:text>;</xsl:text>
    <xsl:value-of select="@opt" />
    <xsl:text>;</xsl:text>
    <xsl:value-of select="@pri" />
    <xsl:text></xsl:text>
  </xsl:template>

  <xsl:template match="text()"></xsl:template>

</xsl:stylesheet>
