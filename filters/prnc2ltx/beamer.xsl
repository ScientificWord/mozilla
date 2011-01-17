<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>  

<xsl:include href="latex.xsl"/>

<xsl:template match="html:stepnumberedlist">
\begin{enumerate}[<+->]
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>

<xsl:template match="html:stepbulletlist">
\begin{itemize}[<+->]
<xsl:apply-templates/>
\end{itemize}
</xsl:template>

<xsl:template match="html:alertstepnumberedlist">
\begin{enumerate}[<+-| alert@+>]
<xsl:apply-templates/>
\end{enumerate}
</xsl:template>

<xsl:template match="html:alertstepbulletlist">
\begin{itemize}[<+-| alert@+>]
<xsl:apply-templates/>
\end{itemize}
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
