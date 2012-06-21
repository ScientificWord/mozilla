<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">
>  

<xsl:template match="html:QTR[@type='QQQuline']">
\uline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQuuline']">
\uuline{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQuwave']">
\uwave{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQsout']">
\sout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:QTR[@type='QQQxout']">
\xout{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uline">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uuline">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:uwave">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:sout">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:xout">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>


</xsl:stylesheet>
