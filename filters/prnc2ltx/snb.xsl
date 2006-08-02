<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml/strict"
                version="1.1">
<xsl:output method="html"/>

<xsl:template match="DOCUMENT">
	<html>
		<xsl:apply-templates/>
	</html>
</xsl:template>

<xsl:template match="PREAMBLE"/>

<xsl:template match="BODY">
    <body><xsl:apply-templates/></body>
</xsl:template>

<xsl:template match="header[@class='section']//words">
	<h1><xsl:apply-templates/></h1>
</xsl:template>

<xsl:template match="header[@class='subsection']//words">
	<h2><xsl:apply-templates/></h2>
</xsl:template>

<xsl:template match="header[@class='subsubsection']//words">
	<h3><xsl:apply-templates/></h3>
</xsl:template>

<xsl:template match="header[@class='paragraph']//words">
	<h4><xsl:apply-templates/></h4>
</xsl:template>

<xsl:template match="header[@class='subparagraph']//words">
	<h5><xsl:apply-templates/></h5>
</xsl:template>

<xsl:template match="theorem[@env-name='theorem']">
	<b>Theorem:</b>
	<i>
		<xsl:apply-templates/>
	</i>
</xsl:template>

<xsl:template match="div[@class='proof']">
	<b>Proof:</b>
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="div[@class='quotation']">
	<q>
		<xsl:apply-templates/>
	</q>
</xsl:template>

<xsl:template match="div[@class='quote']">
	<q>
		<xsl:apply-templates/>
	</q>
</xsl:template>

<xsl:template match="list-env[@class='enumerate']">
	<ol>
		<xsl:apply-templates/>
	</ol>
</xsl:template>

<xsl:template match="list-env[@class='itemize']">
	<ul>
		<xsl:apply-templates/>
	</ul>
</xsl:template>

<xsl:template match="list-env[@class='description']">
	<ul>
		<xsl:apply-templates/>
	</ul>
</xsl:template>

<xsl:template match="list-env[@class='description']/entries/item/tbucket[@nom='label']">
</xsl:template>

<xsl:template match="list-env[@class='description']/entries/item/tbucket[@nom='entry']">
	<li>
		<xsl:apply-templates/>
	</li>
</xsl:template>

<xsl:template match="item">
	<li>
		<xsl:apply-templates/>
	</li>
</xsl:template>

<xsl:template match="verbatim">
	<code>
		<xsl:apply-templates/>
	</code>
</xsl:template>

<xsl:template match="par">
	<p><xsl:apply-templates/></p>
</xsl:template>

<xsl:template match="words">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="math-inline/math">
  <math mode="inline" xmlns="http://www.w3.org/1998/Math/MathML">
  	<xsl:copy-of select="node()"/>
  </math>
</xsl:template>

<xsl:template match="math-display/math">
  <math mode="display" xmlns="http://www.w3.org/1998/Math/MathML">
  	<xsl:copy-of select="node()"/>
  </math>
</xsl:template>

<xsl:template match="eqnarray/math">
  <math mode="display" xmlns="http://www.w3.org/1998/Math/MathML">
  	<xsl:copy-of select="node()"/>
  </math>
</xsl:template>

<xsl:template match="mbucket"/>

<xsl:template match="tbucket">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="span[@tag='textbf']">
	<b><xsl:apply-templates/></b>
</xsl:template>

<xsl:template match="span[@tag='textit']">
	<i><xsl:apply-templates/></i>
</xsl:template>

<xsl:template match="span[@tag='emph']">
	<em><xsl:apply-templates/></em>
</xsl:template>

<xsl:template match="span[@tag='textsl']">
	<b><i><xsl:apply-templates/></i></b>
</xsl:template>

<xsl:template match="span[@tag='textsc']">
	<kbd><xsl:apply-templates/></kbd>
</xsl:template>

<xsl:template match="span[@tag='textsf']">
	<samp><xsl:apply-templates/></samp>
</xsl:template>

<xsl:template match="span[@tag='texttt']">
	<tt><xsl:apply-templates/></tt>
</xsl:template>

<xsl:template match="hyperref"/>

<xsl:template match="note"/>

<xsl:template match="FRAME"/>

</xsl:stylesheet>
