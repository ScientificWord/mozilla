<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:html="http://www.w3.org/1999/xhtml" 
    xmlns:mml="http://www.w3.org/1998/Math/MathML">

<xsl:output method="text" encoding="UTF-8"/>

<!-- Start at the beginning and apply templates -->
<xsl:template match="/">
  <xsl:apply-templates/>
</xsl:template>

<!-- 'hide' mode suppresses text generation -->
<xsl:template match="text()|@*" mode="hide"></xsl:template>

<!-- Otherwise text gets generated -->
<xsl:template match="text()|@*"><xsl:value-of select="."/></xsl:template>

<!-- Here is a rule to hide specific tags -->
<xsl:template match="*" mode="hide">
	<xsl:apply-templates mode="hide"/>
</xsl:template>

<!-- When a <math> in encountered write out <math> and switch to math mode -->
<xsl:template match="mml:math">
	<xsl:text>&lt;math&gt;</xsl:text>
	<xsl:apply-templates mode="in-math"/>
	<xsl:text>&lt;/math&gt;</xsl:text>
</xsl:template>

<!-- This is the rule for all nodes contained in <math>...</math> -->
<xsl:template match="*" mode="in-math">
	<xsl:text>&lt;</xsl:text>
	<xsl:value-of select="local-name()"/>
	<xsl:text>&gt;</xsl:text>
	<xsl:apply-templates mode="in-math"/>
	<xsl:text>&lt;/</xsl:text>
	<xsl:value-of select="local-name()"/>
	<xsl:text>&gt;</xsl:text>
</xsl:template>

<!-- Hide the head -->
<xsl:template match="html:head" >
  <xsl:apply-templates mode="hide"/>
</xsl:template>

<!-- Show the body -->
<xsl:template match="html:body" >
  <xsl:apply-templates/>
</xsl:template>

<!-- Here are SW tags that get carried over to the markup -->

<xsl:template match="html:bodyText">
<!--	Don't write out the tag, but output the content. We put in a marker so post-processing can but in line breaks. -->
	<xsl:apply-templates/>
</xsl:template>

<!-- Handle text tags that overlap with markdown -->
<xsl:template match="html:bold">
	<xsl:text> **</xsl:text>
	<xsl:apply-templates/>
	<xsl:text>** </xsl:text>
</xsl:template>

<xsl:template match="html:italics">
	<xsl:text> *</xsl:text>
	<xsl:apply-templates/>
	<xsl:text>* </xsl:text>
</xsl:template>

<xsl:template match="html:emphasized">
	<xsl:text> *</xsl:text>
	<xsl:apply-templates/>
	<xsl:text>* </xsl:text>
</xsl:template>

<!-- Sections and section titles. Move down from the section. -->
<xsl:template match="html:chapter">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:section">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsection">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:subsubsection">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:paragraph">
	<xsl:apply-templates/>
</xsl:template>


<xsl:template match="html:sectiontitle">
	<xsl:choose>
		<xsl:when test="local-name(..)='chapter'">
			<xsl:text># </xsl:text>
		</xsl:when>
		<xsl:when test="local-name(..)='section'">
			<xsl:text>## </xsl:text>
		</xsl:when>
		<xsl:when test="local-name(..)='subsection'">
			<xsl:text>### </xsl:text>
		</xsl:when>
		<xsl:when test="local-name(..)='subsubsection'">
			<xsl:text>#### </xsl:text>
		</xsl:when>
		<xsl:when test="local-name(..)='paragraph'">
			<xsl:text>##### </xsl:text>
		</xsl:when>
	</xsl:choose>
	<xsl:apply-templates/>
</xsl:template>
<!-- To do: lists -->

<!-- Pick up all the remaining cases -->
<xsl:template match="*"> 
	<xsl:apply-templates/>
</xsl:template>
		


</xsl:stylesheet>


