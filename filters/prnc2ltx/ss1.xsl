<!-- Topic.xsl -->
<xsl:stylesheet version="1.1" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
   <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*">
   <!-- Matches everything but the root -->
</xsl:template>

<xsl:template match="book">
   <xsl:apply-templates/>
</xsl:template>

<xsl:template match="chapters">
   <xsl:apply-templates/>
</xsl:template>

<xsl:template match="chapter">
   <book>#<xsl:value-of select="chapterNo"/>, <xsl:value-of select="chapterTopic"/> by <xsl:value-of select="chapterAuthor"/></book>
</xsl:template>



</xsl:stylesheet>