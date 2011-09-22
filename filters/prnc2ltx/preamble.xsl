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
<!--xsl:variable name="masterpackagelist" select="document('packages.xml',.)"/ -->
  

<xsl:variable name="requiredpackages.tf">
  <xsl:for-each select="//*[@req]">
    <xsl:sort select="@req"/>
	  <xsl:copy-of select="."/>
  </xsl:for-each>
</xsl:variable>

<xsl:variable name="preambletexbuttons.tf">
  <xsl:for-each select="//*[@pre]">
    <xsl:sort select="@ord"/>
	  <xsl:copy-of select="."/>
  </xsl:for-each>
</xsl:variable>

<xsl:variable name="requiredpackages" select ="exsl:node-set($requiredpackages.tf)"/>
<xsl:variable name="preambletexbuttons" select ="exsl:node-set($preambletexbuttons.tf)"/>
 
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
              <!--xsl:variable name="pri" select="$masterpackagelist/packages/package[@name=$pkg]/@pri"/ -->
              <xsl:choose>
                <!--xsl:when test="$pri"><xsl:value-of select="$pri"/></xsl:when -->
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

<xsl:variable name="compiler" select="//html:texprogram/@prog"/>
<xsl:variable name="formattingok" select="//html:texprogram[@formatOK='true']"/>
<xsl:variable name="pagelayoutok" select="//html:texprogram[@pageFormatOK='true']"/>
<xsl:variable name="fontchoiceok" select="//html:texprogram[@fontsOK='true']"/>
<xsl:variable name="lang1" select="//html:babel/@lang1"/>
<xsl:variable name="lang2" select="//html:babel/@lang2"/>

<xsl:template match="html:preamble">
<xsl:text>%% preamble
</xsl:text>
\usepackage{amssymb}
<xsl:if test="$compiler='xelatex'">
\usepackage{xltxtra}
\usepackage{xkeyval}
\TeXXeTstate=1 
\defaultfontfeatures{Scale=MatchLowercase,Mapping=tex-text}
</xsl:if>

<xsl:if test="$compiler!='xelatex'">
\usepackage{textcomp}
</xsl:if>

<xsl:text>\usepackage{xspace}</xsl:text>
<xsl:if test="count(//html:indexitem) &gt; 0"
  >\usepackage{makeidx}</xsl:if>

<xsl:for-each select="$packagelist/*"
  ><xsl:sort select="@pri" data-type="number"/>
\usepackage<xsl:if test="@options"
    >[<xsl:value-of select="@options"/>]</xsl:if
    >{<xsl:value-of select="@package"/>}  %% <xsl:value-of select="@pri"/></xsl:for-each>
  <!--xsl:apply-templates/ -->
  <xsl:for-each select="$preambletexbuttons/*"
  ><xsl:if test="@pre='1'"><xsl:apply-templates mode="tex"/>
</xsl:if></xsl:for-each>

<!-- back to template match="html:preamble"-->
  <xsl:apply-templates/>
  <!-- xsl:call-template name="generateMissingNewTheorems" / -->
  <xsl:call-template name="writeNewTheoremList" />

<!--put this in only if graphicx is used-->
  <xsl:if test="$packagelist//*[@package='graphicx']">
  \graphicspath{{../tcache/}{../gcache/}{../graphics/}}
  </xsl:if>
</xsl:template>

<xsl:template match="html:preambleTeX">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="html:babel">
  <xsl:if test="@lang1">
    \setdefaultlanguage{<xsl:value-of select="@lang1"/>}
  </xsl:if>
  <xsl:if test="@lang2">
    \setotherlanguage{<xsl:value-of select="@lang2"/>}
  </xsl:if>
</xsl:template>

<!-- use docformat information to call the crop package -->
<xsl:template match="html:crop">
  <xsl:if test="$pagelayoutok"
  ><xsl:variable name="unit"><xsl:value-of select="@unit"/></xsl:variable
  >\usepackage[<xsl:value-of select="@type"/><xsl:text>, </xsl:text
  ><xsl:choose
    ><xsl:when test="@paper='other'"
      >width = <xsl:value-of select="@width"/><xsl:value-of select="$unit"/><xsl:text>, </xsl:text
      >height = <xsl:value-of select="@height"/><xsl:value-of select="$unit"/><xsl:text>, </xsl:text
    ></xsl:when>
    <xsl:otherwise
      ><xsl:value-of select="@paper"/><xsl:text>, </xsl:text> 
    </xsl:otherwise> <!-- you can add any crop options you want here, separated with commas --></xsl:choose
  > center]{crop}</xsl:if
></xsl:template>

<!-- use docformat information to call the geometry package -->
<xsl:template match="html:pagelayout[@latex='true']"><xsl:if test="$pagelayoutok"
><xsl:variable name="unit"><xsl:value-of select="@unit"
/></xsl:variable>
\usepackage[ <xsl:apply-templates/>]{geometry}
</xsl:if
></xsl:template>

<xsl:template match="html:page"><xsl:if test="$pagelayoutok">
  paper=<xsl:value-of select="@paper"/>paper, twoside=<xsl:value-of select="@twoside"
  />, <!--landscape=<xsl:value-of select="@landscape"/>, --> </xsl:if></xsl:template>

<xsl:template match="html:page[@paper='screen']"><xsl:if test="$pagelayoutok"
 >  paper=screen, twoside=false, landscape=false,</xsl:if></xsl:template>

<xsl:template match="html:page[@paper='other']"><xsl:if test="$pagelayoutok">
  paperwidth=<xsl:value-of select="@width"
  />, paperheight=<xsl:value-of select="@height"
/>, </xsl:if></xsl:template>

<xsl:template match="html:textregion"><xsl:if test="$pagelayoutok">
  textwidth=<xsl:value-of select="@width"/>, textheight=<xsl:value-of select="@height"
/>, </xsl:if></xsl:template>

<xsl:template match="html:margin"><xsl:if test="$pagelayoutok">
  left=<xsl:value-of select="@left"/>, top=<xsl:value-of select="@top"
/>, </xsl:if></xsl:template>

<xsl:template match="html:header"><xsl:if test="$pagelayoutok">
  headheight=<xsl:value-of select="@height"
  />, headsep=<xsl:value-of select="@sep"
/>, </xsl:if></xsl:template>

<xsl:template match="html:columns[@count='2']"
  >twocolumn=true, columnsep=<xsl:value-of select="@sep"
/>, </xsl:template>

<xsl:template match="html:columns[@count!='2']"
  ></xsl:template>

<xsl:template match="html:marginnote[@hidden='false']"
  ><xsl:if test="$pagelayoutok">marginparwidth=<xsl:value-of select="@width"
  />, marginparsep=<xsl:value-of select="@sep"
/>, </xsl:if></xsl:template>

<xsl:template match="html:marginnote[@hidden!='false']"
  ></xsl:template>


<xsl:template match="html:footer"></xsl:template> <!--%%  footskip=<xsl:value-of select="concat(number(substring(@height,1,string-length(@height)-2))+number(substring(@sep,1,string-length(@sep-2)),substring(@sep,string-length(@sep)-2))"/>%</xsl:template -->
																																	 
<xsl:template match="html:fontchoices">
  <xsl:if test="$fontchoiceok"><xsl:if test="$compiler='xelatex'">
<xsl:apply-templates/></xsl:if></xsl:if></xsl:template> 
   

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
<xsl:template match="html:sectitleformat">
<xsl:if test="@enabled='true'">
<xsl:if test="@newPage='true'">
\newcommand\<xsl:value-of select="@level"/>break{\clearpage}</xsl:if>
\newcommand{\msi<xsl:value-of select="@level"
/>}[1]{<xsl:apply-templates select="html:titleprototype"/>}
\titleformat{\<xsl:value-of select="@level"/>}[<xsl:value-of select="@sectStyle"/>]{<xsl:choose
  ><xsl:when test="@align='l'">\filright</xsl:when
  ><xsl:when test="@align='c'">\center</xsl:when
  ><xsl:otherwise>\filleft</xsl:otherwise
  ></xsl:choose><xsl:apply-templates select="html:toprule"/>}{}{0pt}{\msi<xsl:value-of select="@level"
/>}[{<xsl:apply-templates select="html:bottomrule"/>}]</xsl:if></xsl:template>
						  
<xsl:template match="html:dialogbase">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:titleprototype"><xsl:apply-templates mode="tex"/>
</xsl:template>

<xsl:template match="html:templatebase">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:toprule">
  <xsl:choose>
    <xsl:when test="@role='rule'">\titleline<xsl:if test="@tlwidth='*'"
      >*</xsl:if>[<xsl:value-of select="@tlalign"
      />]{<xsl:if test="@color"
      >\textcolor<xsl:choose
	   	><xsl:when test="substring(./@color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"
   		/></xsl:when
   		><xsl:otherwise>{<xsl:value-of select="./@color"/></xsl:otherwise
   	  ></xsl:choose
      >}{</xsl:if
      ><xsl:choose>
        <xsl:when test="@tlwidth='-'">\titlerule[<xsl:value-of select="@tlheight"/>]}</xsl:when>
        <xsl:when test="@tlwidth='*'">\titlerule[<xsl:value-of select="@tlheight"/>]}</xsl:when>
        <xsl:otherwise>\rule{<xsl:value-of select="@tlwidth"/>}{<xsl:value-of select="@tlheight"/>}}</xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@color">}</xsl:if>
    </xsl:when>
    <xsl:when test="@role='vspace'">\vspace{<xsl:value-of select="@tlheight"/>}</xsl:when>
  </xsl:choose>
</xsl:template>
  
<xsl:template match="html:bottomrule">
  <xsl:choose>
    <xsl:when test="@role='rule'">\titleline<xsl:if test="@tlwidth='*'"
      >*</xsl:if>[<xsl:value-of select="@tlalign"
      />]{<xsl:if test="@color"
      >\textcolor<xsl:choose
	   	><xsl:when test="substring(./@color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"
   		/></xsl:when
   		><xsl:otherwise>{<xsl:value-of select="./@color"/></xsl:otherwise
   	  ></xsl:choose
      >}{</xsl:if
      ><xsl:choose>
        <xsl:when test="@tlwidth='-'">\titlerule[<xsl:value-of select="@tlheight"/>]}</xsl:when>
        <xsl:when test="@tlwidth='*'">\titlerule[<xsl:value-of select="@tlheight"/>]}</xsl:when>
        <xsl:otherwise>\rule{<xsl:value-of select="@tlwidth"/>}{<xsl:value-of select="@tlheight"/>}}</xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@color">}</xsl:if>
    </xsl:when>
    <xsl:when test="@role='vspace'">\vspace{<xsl:value-of select="@tlheight"/>}</xsl:when>
  </xsl:choose>
</xsl:template>
<!-- end of section headings -->

<!-- leading -->
<xsl:template match="html:leading"
    ><xsl:if test="$formattingok">\leading{<xsl:value-of select="@val"
  />}</xsl:if></xsl:template>
  
<!-- magnification?? -->



<!-- section numbering style -->
<xsl:template match="html:numberstyles">

  <xsl:if test="@part">
  	<xsl:choose>
    	<xsl:when test="@part='none'">
    	  \renewcommand\thepart{}</xsl:when>
    	<xsl:otherwise>
    	  \renewcommand\thepart{\<xsl:value-of select="@part"/>{part}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@chapter">
  	<xsl:choose>
    	<xsl:when test="@chapter='none'">
    	  \renewcommand\thechapter{}</xsl:when
    	><xsl:otherwise>
    	  \renewcommand\thechapter{\<xsl:value-of select="@chapter"/>{chapter}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@section">
  	<xsl:choose>
    	<xsl:when test="@section='none'">
    	\renewcommand\thesection{}</xsl:when
    	><xsl:otherwise>
    	\renewcommand\thesection{\<xsl:value-of select="@section"/>{section}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@subsection">
  	<xsl:choose>
    	<xsl:when test="@subsection='none'">
    	\renewcommand\thesubsection{}</xsl:when
    	><xsl:otherwise>
    	\renewcommand\thesubsection{\thesection.\<xsl:value-of select="@subsection"/>{subsection}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@subsubsection">
  	<xsl:choose>
    	<xsl:when test="@subsubsection='none'">
    	\renewcommand\thesubsubsection{}</xsl:when
    	><xsl:otherwise>
    	\renewcommand\thesubsubsection{\thesubsection.\<xsl:value-of select="@subsubsection"/>{subsubsection}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@paragraph">
  	<xsl:choose>
    	<xsl:when test="@paragraph='none'">
    	\renewcommand\theparagraph{}</xsl:when
    	><xsl:otherwise>
    	\renewcommand\paragraph{\thesubbsusection.\<xsl:value-of select="@paragraph"/>{paragraph}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
  <xsl:if test="@subparagraph">
  	<xsl:choose>
    	<xsl:when test="@subparagraph='none'">
    	\renewcommand\thesubparagraph{}</xsl:when
    	><xsl:otherwise>
    	\renewcommand\subparagraph{\theparagraph.\<xsl:value-of select="@subparagraph"/>{subparagraph}}</xsl:otherwise> 
	</xsl:choose></xsl:if>
</xsl:template>


<!-- meta data section. Mostly this info is sent to PDF -->

<xsl:template match="html:meta">
  <xsl:choose>
    <xsl:when test="@name='author'">\hypersetup{pdfauthor=<xsl:value-of select="@content"/>}
    </xsl:when>
    <xsl:when test="@name='copyright'">\special{pdf: docinfo &lt;&lt; /Copyright (<xsl:value-of select="@content"/>)}</xsl:when>
    <xsl:when test="@name='disclaimer'">\special{pdf: docinfo  &lt;&lt; /Disclaimer (<xsl:value-of select="@content"/>)}</xsl:when>
    <xsl:when test="@name='editor'">\special{pdf: docinfo  &lt;&lt; /Editor (<xsl:value-of select="@content"/>)}</xsl:when>
    <xsl:when test="@name='publisher'">\special{pdf: docinfo  &lt;&lt; /Publisher (<xsl:value-of select="@content"/>)}</xsl:when>
    <xsl:when test="@name='trademark'">\special{pdf: docinfo  &lt;&lt; /Trademark (<xsl:value-of select="@content"/>)}</xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:title">
  \hypersetup{pdftitle=<xsl:value-of select="."/>} 
</xsl:template>

</xsl:stylesheet>

