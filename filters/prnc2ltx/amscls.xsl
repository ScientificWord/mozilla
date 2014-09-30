<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:include href="latex.xsl"/>


<xsl:template match="html:msibr">
    <xsl:text>~\\</xsl:text>
    <xsl:value-of select="$newline"/>
</xsl:template>

<xsl:template match="html:author">
 <xsl:value-of select="$newline"/>
 <xsl:text>\author</xsl:text>
 <xsl:if test="./*[1][self::html:authorid]">
    <xsl:apply-templates select="./*[1]" mode="authorid"/>
 </xsl:if>
 <xsl:text>{</xsl:text>
    <xsl:apply-templates/>
 <xsl:text>}</xsl:text>
</xsl:template>
  
<xsl:template match="html:authorid" mode="authorid">
   <xsl:text>[</xsl:text>
   <xsl:apply-templates/>
   <xsl:text>]</xsl:text>
</xsl:template>

<xsl:template match="html:authorid">
</xsl:template>


<xsl:template match="html:address">
 <xsl:value-of select="$newline"/>
 <xsl:text>\address</xsl:text>
 <xsl:if test="./*[1][self::html:authorid]">
    <xsl:apply-templates select="./*[1]" mode="authorid" />
 </xsl:if>
 <xsl:text>{</xsl:text>
    <xsl:apply-templates/>
 <xsl:text>}</xsl:text>
</xsl:template>



<xsl:template match="html:subjclass">
  <xsl:text>\subjclass</xsl:text>
  <xsl:apply-templates mode="subjclassyear"/>
  <xsl:text>{</xsl:text>
  <xsl:apply-templates/>
  <xsl:text>}</xsl:text>
</xsl:template>

<xsl:template match="html:subjclass/html:subjclassyear"></xsl:template>
<xsl:template match="html:subjclass//text()" mode="subjclassyear"></xsl:template>
<xsl:template match="html:subjclassyear" mode="subjclassyear">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:urladdr">
  <xsl:value-of select="$newline"/>
  <xsl:text>\urladdr</xsl:text>
  <xsl:apply-templates mode="authorid"/>
  <xsl:text>{</xsl:text>
  <xsl:apply-templates/>
  <xsl:text>}</xsl:text>
</xsl:template>
<xsl:template match="html:urladdr/html:authorid"></xsl:template>
<xsl:template match="html:urladdr//text()" mode="authorid"></xsl:template>

<xsl:template match="html:email">
  <xsl:value-of select="$newline"/>
  <xsl:text>\email</xsl:text>
  <xsl:apply-templates mode="authorid"/>
  <xsl:text>{</xsl:text>
  <xsl:apply-templates/>
  <xsl:text>}</xsl:text>
</xsl:template>
<xsl:template match="html:email/html:authorid"></xsl:template>
<xsl:template match="html:email//text()" mode="authorid"></xsl:template>

<xsl:template match="html:curraddr">
  <xsl:value-of select="$newline"/>
  <xsl:text>\curraddr</xsl:text>
  <xsl:apply-templates mode="authorid"/>
  <xsl:text>{</xsl:text>
  <xsl:apply-templates/>
  <xsl:text>}</xsl:text>
</xsl:template>
<xsl:template match="html:curraddr/html:authorid"></xsl:template>
<xsl:template match="html:curraddr//text()" mode="authorid"></xsl:template>


<xsl:template match="html:contrib">
  \contrib<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:contrib/html:authorid"></xsl:template>
<xsl:template match="html:contrib//text()" mode="authorid"></xsl:template>

<!-- xsl:template match="html:author/html:authorid"></xsl:template -->



<xsl:template match="html:translator">
\translator{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:dedicatory">
\dedicatory{<xsl:apply-templates/>}<xsl:text/></xsl:template>

<xsl:template match="html:thanks">
\thanks{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<!-- Originally copied from revtex4-1.xsl.  Deleted REVTeX specific items -->

<xsl:template match="html:keywords">
\keywords{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

</xsl:stylesheet>
