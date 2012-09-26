<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">
<xsl:output method="xml" encoding="UTF-8"/>
<xsl:strip-space elements="*"/>

<xsl:template match="node()|@*">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="active"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="node()|@*" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="active"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="node()|@*" mode="inactive">
	<xsl:copy>
		<xsl:apply-templates select="@*" mode="inactive"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:mtable" mode="active">
  <table>
    <tbody>
		  <xsl:apply-templates select="@*|node()" mode="active"/>
    </tbody>
  </table>
</xsl:template>

<xsl:template match="mml:mtr" mode="active">
  <tr>
		<xsl:apply-templates select="@*|node()" mode="active"/>
  </tr>
</xsl:template>

<xsl:template match="mml:mtd" mode="active">
  <td>
		<xsl:apply-templates select="@*|node()" mode="active"/>
  </td>
</xsl:template>

<xsl:template match="mml:mtable" mode="active">
  <table>
		<xsl:apply-templates select="@*|node()" mode="active"/>
  </table>
</xsl:template>

<xsl:template match="mml:munderover" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:munder" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:mover" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:msubsup" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:msub" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:msup" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:mfrac" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:msqrt" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>

<xsl:template match="mml:mroot" mode="active">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<xsl:apply-templates mode="inactive"/>
	</xsl:copy>
</xsl:template>


<xsl:template match="mml:mi" mode="active">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="mml:mo" mode="active">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="mml:mn" mode="active">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="mml:mtext" mode="active">
  <xsl:value-of select="."/>
</xsl:template>


</xsl:stylesheet>
