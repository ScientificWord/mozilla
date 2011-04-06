<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">
>  

<xsl:include href="latex.xsl"/>

<xsl:template match="html:preprint">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:volumeyear">
\volumeyear{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:issuenumber">
\issuenumber{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:eid">
\eid{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:received">
\received{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:revised">
\revised{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:accepted">
\accepted{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:published">
\published{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:affiliation">
\affiliation{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:altaffiliation">
\altaffiliation{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:collaboration">
\collaboration{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:noaffiliation">  <!-- use maketitle from latex.css as the example (gp) -->
\noaffiliation
</xsl:template>

<xsl:template match="html:email">
\email{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:homepage">
\homepage{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:pacs">
\pacs{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:keywords">
\keywords{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

</xsl:stylesheet>
