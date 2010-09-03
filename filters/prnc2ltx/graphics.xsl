






<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>

<xsl:template match="html:object">
\includegraphics{../<xsl:value-of select="@data"/>}
</xsl:template>
		  
<xsl:template match="@data">
  @data
</xsl:template>
		  


</xsl:stylesheet>
