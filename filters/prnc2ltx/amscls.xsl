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

<xsl:template match="html:subjclass">
  \subjclass<xsl:apply-templates mode="subjclassyear"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:subjclass/html:subjclassyear"></xsl:template>
<xsl:template match="html:subjclass//text()" mode="subjclassyear"></xsl:template>
<xsl:template match="html:subjclassyear" mode="subjclassyear">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:urladdr">
  \urladdr<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:urladdr/html:authorid"></xsl:template>
<xsl:template match="html:urladdr//text()" mode="authorid"></xsl:template>

<xsl:template match="html:email">
  \email<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:email/html:authorid"></xsl:template>
<xsl:template match="html:email//text()" mode="authorid"></xsl:template>

<xsl:template match="html:curraddr">
  \curraddr<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:curraddr/html:authorid"></xsl:template>
<xsl:template match="html:curraddr//text()" mode="authorid"></xsl:template>

<xsl:template match="html:address">
  \address<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:address/html:authorid"></xsl:template>
<xsl:template match="html:address//text()" mode="authorid"></xsl:template>

<xsl:template match="html:contrib">
  \contrib<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:contrib/html:authorid"></xsl:template>
<xsl:template match="html:contrib//text()" mode="authorid"></xsl:template>

<xsl:template match="html:author">
  \author<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:author/html:authorid"></xsl:template>
<xsl:template match="html:author//text()" mode="authorid"></xsl:template>

<xsl:template match="html:authorid" mode="authorid">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:translator">
\translator{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:dedicatory">
\dedicatory{<xsl:apply-templates/>}<xsl:text/></xsl:template>

<xsl:template match="html:thanks">
\thanks{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

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