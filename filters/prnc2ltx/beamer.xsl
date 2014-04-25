<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:include href="latex.xsl"/>

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
\begin{stepenumeratewithalert}[&lt;+-| alert@+&gt;]
<xsl:apply-templates/>
\end{enumerate}
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
