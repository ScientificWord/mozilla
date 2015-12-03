<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:include href="latex.xsl"/>

<xsl:template match="html:date">
\date<xsl:apply-templates mode="option"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:date/html:option"></xsl:template>
<xsl:template match="html:date//text()" mode="option"></xsl:template>
<xsl:template match="html:option" mode="option">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:subtitle">
\subtitle<xsl:apply-templates mode="subtitleid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:subtitle/html:subtitleid"></xsl:template>
<xsl:template match="html:subtitle//text()" mode="subtitleid"></xsl:template>
<xsl:template match="html:subtitleid" mode="subtitleid">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:institute">
\institute<xsl:apply-templates mode="instituteid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:institute/html:instituteid"></xsl:template>
<xsl:template match="html:institute//text()" mode="instituteid"></xsl:template>
<xsl:template match="html:instituteid" mode="instituteid">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:subject">\subject{<xsl:apply-templates/>}</xsl:template>
<xsl:template match="html:keywords">\keywords{<xsl:apply-templates/>}</xsl:template>

<xsl:template match="html:author">
\author<xsl:apply-templates mode="authorid"/>{<xsl:apply-templates/>}
</xsl:template>
<xsl:template match="html:author/html:authorid"></xsl:template>
<xsl:template match="html:author//text()" mode="authorid"></xsl:template>
<xsl:template match="html:authorid" mode="authorid">[<xsl:apply-templates/>]</xsl:template>

<xsl:template match="html:beamerframe">
\begin{frame}
<xsl:apply-templates/>
\end{frame}
</xsl:template>

<xsl:template match="html:frametitle">
\frametitle{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:framesubtitle">
\framesubtitle{<xsl:apply-templates/>}<xsl:text/>
</xsl:template>

<xsl:template match="html:stepnumberedlist">
\begin{stepenumerate}
<xsl:apply-templates/>
\end{stepenumerate}
</xsl:template>

<xsl:template match="html:stepbulletlist">
\begin{stepitemize}
<xsl:apply-templates/>
\end{stepitemize}
</xsl:template>

<xsl:template match="html:alertstepnumberedlist">
\begin{stepenumeratewithalert}
<xsl:apply-templates/>
\end{stepenumeratewithalert}
</xsl:template>

<xsl:template match="html:alertstepbulletlist">
\begin{stepitemizewithalert}
<xsl:apply-templates/>
\end{stepitemizewithalert}
</xsl:template>

<xsl:template match="html:stepnumberedListItem">
\item <xsl:apply-templates/>

</xsl:template>

<xsl:template match="html:stepbulletListItem">
\item <xsl:apply-templates/>

</xsl:template>

<xsl:template match="html:alertstepnumberedListItem">
\item <xsl:apply-templates/>

</xsl:template>

<xsl:template match="html:alertstepbulletListItem">
\item <xsl:apply-templates/>

</xsl:template>


</xsl:stylesheet>


