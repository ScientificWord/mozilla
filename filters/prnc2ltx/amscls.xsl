<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:template match="html:author">
 \author{<xsl:apply-templates mode="frontmatter"/>
   <xsl:if test="../html:address">~\\</xsl:if>
   <xsl:apply-templates select="../html:address" mode="frontmatter" />
 }</xsl:template>  
 <!-- for the sake of the above template, -->
 <xsl:template match="html:msibr" mode="frontmatter">~\\
</xsl:template>

<xsl:include href="latex.xsl"/>

<!-- Originally copied from revtex4-1.xsl.  Deleted REVTeX specific items -->

<xsl:template match="html:email">
\email<xsl:if test="@alt and string-length(@alt)">[<xsl:value-of select="@alt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:homepage">
\homepage<xsl:if test="@alt and string-length(@alt)">[<xsl:value-of select="@alt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:pacs">
\pacs{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:keywords">
\keywords{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

</xsl:stylesheet>
