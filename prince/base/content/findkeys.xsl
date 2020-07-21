<?xml version='1.0'?>
<xsl:stylesheet version="1.1" 
	xmlns:html="http://www.w3.org/1999/xhtml" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:mathml="http://www.w3.org/1998/Math/MathML">

	<xsl:output encoding="UTF-8" method="text"/>
	<xsl:template match="/">
	  <xsl:apply-templates/>
	</xsl:template>

	<xsl:variable name="hyphen">--</xsl:variable>
	<xsl:variable name="ignoreIDs">--(removesection)--chapter--paragraph--part--section--subparagraph--subsection--subsubsection--</xsl:variable>
	<xsl:variable name="xrefName">xref</xsl:variable>
	<xsl:variable name="bibkey">bibkey</xsl:variable>

    <xsl:template match="text()|@*"></xsl:template>
	<xsl:template match="*"/>
	<xsl:template match="html:*">
		<xsl:apply-templates select="//*[@Key]|//*[@key][not(local-name()=$xrefName)]|//*[@id]|//mathml:mtable//*[@marker]|//mathml:mtable//*[@customLabel]|//html:bibkey"/>
	</xsl:template>

	<xsl:template match=
		"*[@Key]|//*[@key]|//*[@id]|//mathml:mtable//*[@marker]|//mathml:mtable//*[@customLabel]|//html:bibkey">
		<xsl:choose>
			<xsl:when test="@key and not(local-name()=$xrefName)">
				<xsl:value-of select="@key"/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
			<xsl:when test="@Key">
				<xsl:value-of select="@Key"/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
			<xsl:when test="@marker and not(@key and @key=@marker)">
				<xsl:value-of select="@marker"/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
			<xsl:when test="@id and not(contains($ignoreIDs,concat($hyphen,local-name(),$hyphen))) and not(@key and @key=@id) and not(@marker and @marker=@id)">
				<xsl:value-of select="@id"/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
			<xsl:when test="@customLabel and not(@key and @key=@customLabel) and not(@marker and @marker=@customLabel) and not (@id and @id=@customLabel)">
				<xsl:value-of select="@customLabel"/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
			<xsl:when test="local-name()=$bibkey">
				<xsl:value-of select="."/>
				<xsl:text>\n</xsl:text>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>