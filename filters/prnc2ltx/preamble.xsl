<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<!-- Package handling. Packages are not inserted directly, but with requirespackage tags and other
  tags with the 'req' and 'opt' attributes. We collect them and remove
  duplicates, and then sort according to the pri attribute -->
<xsl:variable name="masterpackagelist" select="document('packages.xml',.)"/>
  

<xsl:variable name="requiredpackages.tf">
  <xsl:for-each select="//*[@req]">
    <xsl:sort select="@req"/>
	<xsl:copy-of select="."/>
  </xsl:for-each>
  <!--xsl:for-each select="//mainfont[@req]">
    <xsl:sort select="@req"/>
	<xsl:copy-of select="."/>
  </xsl:for-each>
  <xsl:for-each select="//mathfont[@req]">
    <xsl:sort select="@req"/>
	<xsl:copy-of select="."/>
  </xsl:for-each>
  <xsl:for-each select="//sansfont[@req]">
    <xsl:sort select="@req"/>
	<xsl:copy-of select="."/>
  </xsl:for-each>
  <xsl:for-each select="//fixedfont[@req]">
    <xsl:sort select="@req"/>
	<xsl:copy-of select="."/>
  </xsl:for-each -->
</xsl:variable>

<xsl:variable name="requiredpackages" select ="exsl:node-set($requiredpackages.tf)"/>
 
<xsl:variable name="packagelist.tf"> 
  <xsl:for-each select="$requiredpackages/*">
    <xsl:variable name="pos" select="position()"/>
	  <xsl:variable name="currentpackage" select="@req"/>
	  <xsl:if
	    test="$pos=1 or not($currentpackage=$requiredpackages/*[$pos - 1]/@req)">
	    <xsl:element name="requiredpackage" >
	      <xsl:attribute name="package"><xsl:value-of select="@req"/></xsl:attribute>
	      <xsl:if test="@opt"><xsl:attribute name="options"><xsl:value-of select="@opt"/></xsl:attribute></xsl:if>
	      <xsl:attribute name="pri">
	  	    <xsl:choose>
	  	      <xsl:when test="@pri">
	  	        <xsl:value-of select="@pri"/>
	  	      </xsl:when>
   	  		  <xsl:otherwise>
              <xsl:variable name="pkg" select="@req"/>
              <xsl:variable name="pri" select="$masterpackagelist/packages/package[@name=$pkg]/@pri"/>
              <xsl:choose>
                <xsl:when test="$pri"><xsl:value-of select="$pri"/></xsl:when>
                <xsl:otherwise>100</xsl:otherwise>
              </xsl:choose>
            </xsl:otherwise>
	  	    </xsl:choose>
	      </xsl:attribute>
      </xsl:element>
	        <!-- xsl:copy-of select="."/ -->
    </xsl:if>
  </xsl:for-each>
</xsl:variable>
 
<xsl:variable name="packagelist" select ="exsl:node-set($packagelist.tf)"/>


<xsl:template match="html:preamble">
<xsl:if test="count(//html:indexitem) &gt; 0"
  >\usepackage{makeidx}
\makeindex</xsl:if>
  <xsl:for-each select="$packagelist/*"
  ><xsl:sort select="@pri" data-type="number"/>
\usepackage<xsl:if test="@options"
    >[<xsl:value-of select="@options"/>]</xsl:if
    >{<xsl:value-of select="@package"/>}  %% <xsl:value-of select="@pri"/></xsl:for-each
  ><xsl:apply-templates
  /></xsl:template>

<xsl:template match="html:preambleTeX">
  <xsl:value-of select="."/>
</xsl:template>

<!-- use docformat information to call the crop package -->
<xsl:template match="html:crop">
  <xsl:variable name="unit"><xsl:value-of select="@unit"/></xsl:variable
  >\usepackage[<xsl:value-of select="@type"/><xsl:text>, </xsl:text
  ><xsl:choose
    ><xsl:when test="@paper='other'"
      >width = <xsl:value-of select="@width"/><xsl:value-of select="$unit"/><xsl:text>, </xsl:text
      >height = <xsl:value-of select="@height"/><xsl:value-of select="$unit"/><xsl:text>, </xsl:text
    ></xsl:when>
    <xsl:otherwise
      ><xsl:value-of select="@paper"/><xsl:text>, </xsl:text> 
    </xsl:otherwise> <!-- you can add any crop options you want here, separated with commas --></xsl:choose
  > center]{crop}</xsl:template>

<!-- use docformat information to call the geometry package -->
<xsl:template match="html:pagelayout[@latex='true']">
<xsl:variable name="unit"><xsl:value-of select="@unit"/></xsl:variable>
\usepackage[ <xsl:apply-templates/>]{geometry}
</xsl:template>

<xsl:template match="html:page">
  paper=<xsl:value-of select="@paper"/>paper, twoside=<xsl:value-of select="@twoside"
  />, <!--landscape=<xsl:value-of select="@landscape"/>, --> </xsl:template>

<xsl:template match="html:page[@paper='screen']">  paper=screen, twoside=false, landscape=false,</xsl:template>

<xsl:template match="html:page[@paper='other']">
  paperwidth=<xsl:value-of select="@width"
  />, paperheight=<xsl:value-of select="@height"
/>, </xsl:template>

<xsl:template match="html:textregion">
  textwidth=<xsl:value-of select="@width"/>, textheight=<xsl:value-of select="@height"
/>, </xsl:template>

<xsl:template match="html:margin">
  left=<xsl:value-of select="@left"/>, top=<xsl:value-of select="@top"
/>, </xsl:template>

<xsl:template match="html:header">
  headheight=<xsl:value-of select="@height"
  />, headsep=<xsl:value-of select="@sep"
/>, </xsl:template>

<xsl:template match="html:columns[@count='2']"
  >twocolumn=true, columnsep=<xsl:value-of select="@sep"
/>, </xsl:template>

<xsl:template match="html:columns[@count!='2']"
  ></xsl:template>

<xsl:template match="html:marginnote[@hidden='false']"
  >marginparwidth=<xsl:value-of select="@width"
  />, marginparsep=<xsl:value-of select="@sep"
/> </xsl:template>

<xsl:template match="html:marginnote[@hidden!='false']"
  ></xsl:template>


<xsl:template match="html:footer"></xsl:template> <!--%%  footskip=<xsl:value-of select="concat(number(substring(@height,1,string-length(@height)-2))+number(substring(@sep,1,string-length(@sep-2)),substring(@sep,string-length(@sep)-2))"/>%</xsl:template -->
																																	 
<xsl:template match="html:fontchoices">
\usepackage{xltxtra}
\TeXXeTstate=1 
\defaultfontfeatures{Scale=MatchLowercase,Mapping=tex-text}
<xsl:apply-templates/></xsl:template>  

<xsl:template match="html:mainfont[@ot='1']">\setmainfont[<xsl:value-of select="@options"
  />]{<xsl:value-of select="@name"
/>}</xsl:template>

<xsl:template match="html:mainfont[@name='Default']"
></xsl:template>

<xsl:template match="html:sansfont[@ot='1']"
 >\setsansfont[<xsl:value-of select="@options"
 />]{<xsl:value-of select="@name"
/>}</xsl:template>

<xsl:template match="html:sansfont[@name='']"></xsl:template>

<xsl:template match="html:fixedfont[@ot='1']">\setmonofont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:fixedfont[@name='']"></xsl:template>
  
<xsl:template match="html:x1font">\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:x2font">\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<xsl:template match="html:x3font">\newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}</xsl:template>

<!-- section headings redefined. Requires package titlesec -->
<xsl:template match="html:sectitleformat" >\newcommand{\msi<xsl:value-of select="@level"/>}[1]{<xsl:apply-templates/>}
\titleformat{\<xsl:value-of select="@level"/>}[<xsl:value-of select="@sectStyle"/>]{<xsl:choose
  ><xsl:when test="@align='l'">\filright</xsl:when
  ><xsl:when test="@align='c'">\center</xsl:when
  ><xsl:otherwise>\filleft</xsl:otherwise
  ></xsl:choose>}{}{0pt}{\msi<xsl:value-of select="@level"
/>}</xsl:template>
						  
<xsl:template match="html:dialogbase">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:titleprototype"><xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:dialogbase">
  <xsl:apply-templates/>
</xsl:template>

<!-- end of section headings -->

<!-- class options   -->
<xsl:template match="html:colist"
  ><xsl:for-each select="@*"
    ><xsl:value-of select="."/>, </xsl:for-each
  ></xsl:template>

<!-- leading -->
<xsl:template match="html:leading"
    >\leading{<xsl:value-of select="@val"
  />}</xsl:template>

    
</xsl:stylesheet>