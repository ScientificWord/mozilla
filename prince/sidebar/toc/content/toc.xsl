<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:html="http://www.w3.org/1999/xhtml" >
<xsl:output method="xml" encoding="UTF-8"/>

<xsl:template match="/">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="*"><!--map all nodes to blank unless they match a rule below--> </xsl:template>
<xsl:template match="html:*"><xsl:apply-templates/></xsl:template>

<xsl:template match="html:body">

  <tree xmlns = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul"
  	id="toc-tree" flex="1" seltype="single"	class="tree" contextmenu="toc-context-menu">
    <treecols>
      <treecol id="tocColumn" label="Name" primary="true" flex="1" />
    </treecols>
	<treechildren>
		<xsl:apply-templates/>	
	</treechildren>
  </tree>

</xsl:template>

<xsl:template match="##sectiontags##">
	<treeitem xmlns = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
		<xsl:attribute name="container">true</xsl:attribute>
		<xsl:attribute name="open">true</xsl:attribute>
	  <treerow>
		  <treecell>
			  <xsl:attribute name="label">
				  <xsl:value-of select="*[1]"/>
			  </xsl:attribute>
				<xsl:attribute name="value">
				  <xsl:value-of select="@id"/>
				</xsl:attribute>		
			</treecell>
		</treerow>
		<xsl:if test="##otherstest####sectiontags##">
			<treechildren>
				<xsl:apply-templates/>
		  </treechildren>
		</xsl:if>
	</treeitem>
</xsl:template>

<!-- BBM  The following two templates need to be modified to pick up a caption --> 

<xsl:template match="##LOF##"> 
	<treeitem xmlns = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
		<treerow>
			<treecell>
				<xsl:attribute name="label">Figure: <xsl:value-of select = "@title"/></xsl:attribute>
				<xsl:attribute name="value">
					<xsl:value-of select="@id"/>
				</xsl:attribute>		
			</treecell>
		</treerow>
	</treeitem>
</xsl:template>;

<xsl:template match="##LOT##"> 
	<treeitem xmlns = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul">
		<treerow>
			<treecell>
				<xsl:attribute name="label">Table</xsl:attribute>
				<xsl:attribute name="value">
					<xsl:value-of select="@id"/>
				</xsl:attribute>		
			</treecell>
		</treerow>
	</treeitem>
</xsl:template>;

</xsl:stylesheet>


