<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:include href="latex.xsl"/>

<xsl:template match="html:preprint">
\preprint{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:volumeyear">
\volumeyear{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:volumenumber">
\volumenumber{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:volumename">
\volumename{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:issuenumber">
\issuenumber{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:eid">
\eid{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:startpage">
\startpage{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:endpage">
\endpage{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:received">
\received<xsl:if test="@xalt and string-length(@xalt)">[<xsl:value-of select="@xalt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:revised">
\revised<xsl:if test="@xalt and string-length(@xalt)">[<xsl:value-of select="@xalt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:accepted">
\accepted<xsl:if test="@xalt and string-length(@xalt)">[<xsl:value-of select="@xalt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:published">
\published<xsl:if test="@xalt and string-length(@xalt)">[<xsl:value-of select="@xalt"/>]</xsl:if>{<xsl:apply-templates/>}<xsl:text/>
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

<xsl:template match="html:firstname">
\firstname{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:surname">
\surname{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

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

<xsl:template match="html:widetext">
\begin{widetext}<xsl:apply-templates/>\end{widetext}</xsl:template>

</xsl:stylesheet>
