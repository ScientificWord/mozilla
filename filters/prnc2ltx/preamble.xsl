<xsl:stylesheet version="1.1"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

 <xsl:template name="filecontents">
    <xsl:if test="html:preamble">
      <xsl:if test="//html:filecontents">
	<xsl:text>\begin{filecontents}</xsl:text>
	<xsl:value-of select="//html:filecontents"/>
	<xsl:text>\end{filecontents}</xsl:text>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <!-- Package handling. Packages are not inserted directly, but with requirespackage tags and other
  tags with the 'req' and 'opt' attributes. We collect them and remove
  duplicates, and then sort according to the pri attribute -->

  <!-- xsl:variable name="requiredpackages.tf">
    <xsl:for-each select="//*[@req]">
      <xsl:sort select="@req"/>
	    <xsl:copy-of select="."/>
    </xsl:for-each>
  </xsl:variable -->

  <xsl:variable name="rawrequiredpackages.tf">
    <xsl:for-each select="//*[@req]">
      <xsl:call-template name="parsePackageInfo">
        <xsl:with-param name="pkgString" select="@req"/>
        <xsl:with-param name="optString" select="@opt"/>
        <xsl:with-param name="priString" select="@pri"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:variable>

  <xsl:template name="parsePackageInfo">
    <xsl:param name="pkgString" />
    <xsl:param name="optString" />
    <xsl:param name="priString" />
    <xsl:variable name="ourPkg">
      <xsl:choose>
        <xsl:when test="contains($pkgString,';')">
          <xsl:value-of select="substring-before($pkgString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$pkgString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="ourOptions">
      <xsl:choose>
        <xsl:when test="$optString and contains($optString,';')">
          <xsl:value-of select="substring-before($optString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$optString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="ourPri">
      <xsl:choose>
        <xsl:when test="$priString and contains($priString,';')">
          <xsl:value-of select="substring-before($priString,';')"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$priString"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:element name="pkgInfo">
      <xsl:attribute name="pkgname"><xsl:value-of select="$ourPkg"/></xsl:attribute>
      <xsl:if test="$ourOptions and string-length($ourOptions)">
        <xsl:attribute name="options"><xsl:value-of select="$ourOptions"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="$ourPri and string-length($ourPri)">
        <xsl:attribute name="priority"><xsl:value-of select="$ourPri"/></xsl:attribute>
      </xsl:if>
    </xsl:element>
    <xsl:if test="contains($pkgString,';')">
      <xsl:call-template name="parsePackageInfo">
        <xsl:with-param name="pkgString" select="substring-after($pkgString,';')"/>
        <xsl:with-param name="optString" select="substring-after($optString,';')"/>
        <xsl:with-param name="priString" select="substring-after($priString,';')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:variable name="preambletexbuttons.tf">
    <xsl:for-each select="//*[@pre]">
      <xsl:sort select="@pri"/>
      <xsl:copy-of select="."/>
    </xsl:for-each>
  </xsl:variable>

  <xsl:variable name="rawrequiredpackages" select ="exsl:node-set($rawrequiredpackages.tf)"/>

  <xsl:variable name="requiredpackages.tf">
    <xsl:for-each select="$rawrequiredpackages/*">
      <xsl:sort select="@pkgname" />
      <xsl:sort select="@opt" />
      <xsl:sort select="@priority" data-type="number"/>
      <xsl:copy-of select="."/>
    </xsl:for-each>
  </xsl:variable>

  <xsl:variable name="requiredpackages" select ="exsl:node-set($requiredpackages.tf)"/>
  <xsl:variable name="preambletexbuttons" select ="exsl:node-set($preambletexbuttons.tf)"/>

  <xsl:variable name="packagelist.tf">
    <xsl:for-each select="$requiredpackages/*">
      <xsl:variable name="pos" select="position()"/>
	    <xsl:variable name="currentpackage" select="@pkgname"/>
	    <xsl:if test="$pos=1 or not($currentpackage=$requiredpackages/*[$pos - 1]/@pkgname)">
	      <xsl:element name="requiredpackage" >
	        <xsl:attribute name="package"><xsl:value-of select="@pkgname"/></xsl:attribute>
	        <xsl:if test="@options and string-length(@options)"><xsl:attribute name="options"><xsl:value-of select="@options"/></xsl:attribute></xsl:if>
	        <xsl:attribute name="pri">
	  	      <xsl:choose>
	  	        <xsl:when test="@priority">
	  	          <xsl:value-of select="@priority"/>
	  	        </xsl:when>
   	  		    <xsl:otherwise>
                <xsl:choose>
                  <!-- <xsl:when test="$currentpackage='titlesec'">100</xsl:when> -->
                  <xsl:when test="$currentpackage='amsfonts'">010</xsl:when>
                  <xsl:when test="$currentpackage='amsmath'">010</xsl:when>
                  <xsl:when test="$currentpackage='xcolor'">200</xsl:when>
                  <xsl:when test="$currentpackage='wrapfig'">100</xsl:when>
                  <xsl:when test="$currentpackage='minipage'">200</xsl:when>
                  <xsl:when test="$currentpackage='hyperref'">10000</xsl:when>
                  <xsl:when test="$currentpackage='bidi'">10001</xsl:when>
                  <xsl:when test="$currentpackage='tcibkpk'">080</xsl:when>
                  <xsl:when test="$currentpackage='sw20orp1'">300</xsl:when>
                  <xsl:when test="$currentpackage='varioref'">1001</xsl:when>
                  <xsl:when test="$currentpackage='babel'">1000</xsl:when>
                  <xsl:when test="$currentpackage='polyglossia'">1000</xsl:when>
                  <xsl:otherwise>100</xsl:otherwise>
                </xsl:choose>
              </xsl:otherwise>
	  	      </xsl:choose>
	        </xsl:attribute>
        </xsl:element>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>

  <!-- xsl:variable name="packagelist.tf">
    <xsl:for-each select="$requiredpackages/*">
      <xsl:variable name="pos" select="position()"/>
	    <xsl:variable name="currentpackage" select="@req"/>
	    <xsl:if
	      test="$pos=1 or not($currentpackage=$requiredpackages/*[$pos - 1]/@req)">
	      <xsl:element name="requiredpackage" >
	        <xsl:attribute name="package">
	          <xsl:value-of select="@req"/>
	        </xsl:attribute>
	        <xsl:if test="@opt">
	          <xsl:attribute name="options">
	            <xsl:value-of select="@opt"/>
	          </xsl:attribute>
	        </xsl:if>
	        <xsl:attribute name="pri">
	  	      <xsl:choose>
	  	        <xsl:when test="@pri">
	  	          <xsl:value-of select="@pri"/>
	  	        </xsl:when>
   	  		    <xsl:otherwise>
                <xsl:variable name="pkg" select="@req"/>
                <xsl:variable name="pri" select="$masterpackagelist/packages/package[@name=$pkg]/@pri"/>
                <xsl:choose>
                  <xsl:when test="$pri">
                    <xsl:value-of select="$pri"/>
                  </xsl:when>
                  <xsl:otherwise>100</xsl:otherwise>
                </xsl:choose>
              </xsl:otherwise>
	  	      </xsl:choose>
	        </xsl:attribute>
        </xsl:element -->
	          <!-- xsl:copy-of select="."/ -->
      <!-- /xsl:if>
    </xsl:for-each>
  </xsl:variable -->

  <xsl:variable name="packagelist" select ="exsl:node-set($packagelist.tf)"/>

  <xsl:variable name="compiler" select="//html:texprogram/@prog"/>
  <xsl:variable name="formattingok" select="//html:texprogram[@formatOK='true']"/>
  <xsl:variable name="pagelayoutok" select="//html:texprogram[@pageFormatOK='true']"/>
  <xsl:variable name="fontchoiceok" select="//html:texprogram[@fontsOK='true']"/>
  <xsl:variable name="lang1" select="//html:babel/@lang1"/>
  <xsl:variable name="lang2" select="//html:babel/@lang2"/>
  <xsl:variable name="babel" select="//html:babel"/>

  <xsl:variable name="usesBabel">
    <xsl:choose>
      <xsl:when test="//html:babel">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>false</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>



  <xsl:template match="html:preamble">
    <xsl:value-of select="$blankline"/>
    <xsl:text>%% preamble</xsl:text>
    <xsl:value-of select="$blankline"/>
    <xsl:text>\usepackage{amssymb,amsmath,mathtools,xcolor,graphicx,xspace,colortbl,ragged2e,rotating} % </xsl:text>
<!--     <xsl:text>
      \newcommand{\mymarginpar}[1]{%
    </xsl:text>
    <xsl:text>
        \marginpar[\RaggedLeft{#1}]{\RaggedRight{#1}}}%
    </xsl:text>
 -->

    <xsl:if test="$compiler!='pdflatex'">
       <xsl:value-of select="$newline"/>
       <xsl:text>\usepackage{xltxtra,xkeyval}</xsl:text>
       <xsl:value-of select="$newline"/>
<!--        <xsl:text>\TeXXeTstate=1</xsl:text>
       <xsl:value-of select="$newline"/> -->
       <xsl:text>\defaultfontfeatures{Scale=MatchLowercase,Mapping=tex-text}</xsl:text>
    </xsl:if>
<!--     <xsl:if test="not($compiler='xelatex')">
      <xsl:value-of select="$newline"/>
      <xsl:text>\usepackage[T1]{fontenc}</xsl:text>
    </xsl:if>
 -->

    <xsl:if test="$compiler='pdflatex'">
      <xsl:value-of select="$newline"/>
      <xsl:text>\usepackage{textcomp}</xsl:text>
    </xsl:if>

    <xsl:if test="count(//html:indexitem) &gt; 0">
       <xsl:value-of select="$newline"/>
       <xsl:text>\usepackage{makeidx}</xsl:text>
    </xsl:if>


 

    <xsl:for-each select="$packagelist/*">
      <xsl:sort select="@pri" data-type="number"/>
      <xsl:choose>
        <xsl:when test="@package='varioref'
                        and exsl:node-set($babel)" />
        <xsl:otherwise>
          <xsl:value-of select="$newline"/>
          <xsl:text>\usepackage</xsl:text>
          <xsl:if test="@options">
             <xsl:text>[</xsl:text>
             <xsl:value-of select="@options"/>
            <xsl:text>]</xsl:text>
          </xsl:if>
          <xsl:text>{</xsl:text>
          <xsl:value-of select="@package"/>
          <xsl:text>}  %% </xsl:text>
          <xsl:value-of select="@pri"/>
          <xsl:text>
          </xsl:text>
        </xsl:otherwise>
      </xsl:choose>
  <!--       <xsl:if test="@package='svg'">
        <xsl:text>\IfFileExists{/dev/null}{%</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>\newcommand{\Inkscape}{/Applications/Inkscape.app/Contents/Resources/bin/inkscape }%</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>}{</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>\newcommand{\Inkscape}{"%programfiles%/Inkscape/inkscape.exe" }%</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>\setsvg{</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>inkscape=\Inkscape -z -C, svgpath=../graphics/</xsl:text>
        <xsl:value-of select="$newline"/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$newline"/>
      </xsl:if> -->
    </xsl:for-each>


    <!-- back to template match="html:preamble"-->
    <!-- xsl:call-template name="generateMissingNewTheorems" / -->
    <!--     <xsl:call-template name="writeNewTheoremList" /> -->

    <!--put this in only if graphicx is used, which, now, it always is-->
    <!-- <xsl:if test="$packagelist//*[@package='graphicx']">  -->
    <xsl:value-of select="$newline"/>
    <xsl:text>\graphicspath{{../graphics/}{../tcache/}{../gcache/}}</xsl:text>
    <xsl:value-of select="$newline"/>
    <xsl:text>\DeclareGraphicsExtensions{.pdf,.eps,.ps,.png,.jpg,.jpeg}</xsl:text>
    <xsl:value-of select="$newline"/>



    <xsl:apply-templates/>
    <xsl:for-each select="$preambletexbuttons/*">
      <xsl:if test="@pre='1'">


        <xsl:apply-templates mode="tex"/>
      </xsl:if>
    </xsl:for-each>

  </xsl:template>


  <xsl:template match="html:preambleTeX">
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="html:dialogbase">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="html:babel">
    <xsl:choose>
      <xsl:when test="@pkg='babel'">
        \usepackage[<xsl:value-of select="$languages"/>]{babel
        <xsl:if test="//*[@req='varioref']">,varioref</xsl:if>}
      </xsl:when>
      <xsl:otherwise>
        \usepackage[<xsl:value-of select="$languages"/>]{polyglossia
        <xsl:if test="//*[@req='varioref']">,varioref</xsl:if>}
        <xsl:if test="@lang1">
          \setdefaultlanguage{<xsl:value-of select="@lang1"/>}
        </xsl:if>
        <xsl:if test="@lang2">
          \setotherlanguage{<xsl:value-of select="@lang2"/>}
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



  <!-- use docformat information to call the crop package -->
  <xsl:template match="html:crop">
    <xsl:if test="$pagelayoutok">
      <xsl:variable name="unit">
        <xsl:value-of select="@unit"/>
      </xsl:variable>
      <xsl:text>\usepackage[</xsl:text>
      <xsl:value-of select="@type"/>
      <xsl:text>,</xsl:text>
      <xsl:choose>
        <xsl:when test="@paper='other'">
          <xsl:text>width =</xsl:text>
          <xsl:value-of select="@width"/>
          <xsl:value-of select="$unit"/>
          <xsl:text>,</xsl:text>
          <xsl:text>height =</xsl:text>
          <xsl:value-of select="@height"/>
          <xsl:value-of select="$unit"/>
          <xsl:text>,</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@paper"/>
          <xsl:text>,</xsl:text>
        </xsl:otherwise>
        <!-- you can add any crop options you want here, separated with commas -->
      </xsl:choose>
      <xsl:text>center]{crop}</xsl:text>
    </xsl:if>
  </xsl:template>

  <!-- use docformat information to call the geometry package -->
  <xsl:template match="html:pagelayout[@latex='true']">
    <xsl:if test="$pagelayoutok">
      <xsl:variable name="unit">
        <xsl:value-of select="@unit"/>
      </xsl:variable>
      <xsl:value-of select="$newline"/>
      <xsl:text>\usepackage[</xsl:text>
      <xsl:apply-templates/>
      <xsl:text>]{geometry}</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:page">
    <xsl:if test="$pagelayoutok">
      <xsl:text>paper=</xsl:text>
      <xsl:value-of select="@paper"/>
      <xsl:text>paper, twoside=</xsl:text>
      <xsl:value-of select="@twoside"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:page[@paper='screen']">
    <xsl:if test="$pagelayoutok">
      <xsl:text>paper=screen, twoside=false, landscape=false,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:page[@paper='other']">
    <xsl:if test="$pagelayoutok">
      <xsl:text>paperwidth=</xsl:text>
      <xsl:value-of select="@width"/>
      <xsl:text>, paperheight=</xsl:text>
      <xsl:value-of select="@height"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:textregion">
    <xsl:if test="$pagelayoutok">
      <xsl:text>textwidth=</xsl:text>
      <xsl:value-of select="@width"/>
      <xsl:text>, textheight=</xsl:text>
      <xsl:value-of select="@height"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:margin">
    <xsl:if test="$pagelayoutok">
      <xsl:text>left=</xsl:text>
      <xsl:value-of select="@left"/>
      <xsl:text>, top=</xsl:text>
      <xsl:value-of select="@top"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:hedd">
    <xsl:if test="$pagelayoutok">
      <xsl:text>headheight=</xsl:text>
      <xsl:value-of select="@height"/>
      <xsl:text>, headsep=</xsl:text>
      <xsl:value-of select="@sep"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:columns[@count='twocolumn']">
    <xsl:text>twocolumn=true, columnsep=</xsl:text>
    <xsl:value-of select="@sep"/>
    <xsl:text>,</xsl:text>
  </xsl:template>

  <xsl:template match="html:columns[@count!='twocolumn']"></xsl:template>

  <xsl:template match="html:marginnote[@hidden='false']">
    <xsl:if test="$pagelayoutok">
      <xsl:text>marginparwidth=</xsl:text>
      <xsl:value-of select="@width"/>
      <xsl:text>, marginparsep=</xsl:text>
      <xsl:value-of select="@sep"/>
      <xsl:text>,</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="html:marginnote[@hidden!='false']"></xsl:template>

  <xsl:template match="html:footer"></xsl:template>
  <!--%%  footskip=<xsl:value-of select="concat(number(substring(@height,1,string-length(@height)-2))+number(substring(@sep,1,string-length(@sep-2)),substring(@sep,string-length(@sep)-2))"/>
  %
</xsl:template -->

<xsl:template match="html:fontchoices">
  <xsl:if test="$fontchoiceok">
    <xsl:if test="$compiler!='pdflatex'">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template match="html:mainfont[@ot='1']">
  \setmainfont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>

<xsl:template match="html:mainfont[@name='Default']"></xsl:template>

<xsl:template match="html:sansfont[@ot='1']"
 >
  \setsansfont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>

<xsl:template match="html:sansfont[@name='']"></xsl:template>

<xsl:template match="html:fixedfont[@ot='1']">
  \setmonofont[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>

<xsl:template match="html:fixedfont[@name='']"></xsl:template>

<xsl:template match="html:x1font">
  \newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>
<xsl:template match="html:x2font">
  \newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>
<xsl:template match="html:x3font">
  \newfontfamily\<xsl:value-of select="@internalname"/>[<xsl:value-of select="@options"/>]{<xsl:value-of select="@name"/>}
</xsl:template>
<!-- section headings redefined. Requires package titlesec -->

<xsl:template match="html:sectitleformat">
  <xsl:if test="@enabled='true'">
    <xsl:if test="@newPage='true'">
      \newcommand\
      <xsl:value-of select="@level"/>
      break{\clearpage}
    </xsl:if>
    \newcommand{\msi<xsl:value-of select="@level"/>}[1]{<xsl:apply-templates select="html:titleprototype"
    />}\titleformat{\<xsl:value-of select="@level"/>}[<xsl:value-of select="@sectStyle"
    />]{<xsl:choose>
      <xsl:when test="@align='r'">\filleft</xsl:when>
      <xsl:when test="@align='c'">\center</xsl:when>
      <xsl:otherwise>\filright</xsl:otherwise>
    </xsl:choose>
    <xsl:apply-templates select="html:toprule"
    />}{}{0pt}{\msi<xsl:value-of select="@level"/>}[{<xsl:apply-templates select="html:bottomrule"/>}]
  </xsl:if>
</xsl:template>

<xsl:template match="html:sectitlenum">
  \the<xsl:value-of select="@level"/>
</xsl:template>

<xsl:template match="html:texparam">
  #<xsl:value-of select="@num"/>
</xsl:template>


<xsl:template match="html:titleprototype">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:templatebase">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="html:toprule">
  <xsl:choose>
    <xsl:when test="@role='rule'">
      \titleline
      <xsl:if test="@tlwidth='*'"
      >*</xsl:if>
      [
      <xsl:value-of select="@tlalign"
      />
      ]{
      <xsl:if test="@color"
      >
        \textcolor
        <xsl:choose
      >
          <xsl:when test="substring(./@color,1,1)='#'">
            [HTML]{
            <xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"
      />
          </xsl:when
      >
          <xsl:otherwise>
            {
            <xsl:value-of select="./@color"/>
          </xsl:otherwise
      >
        </xsl:choose
      >
        }{
      </xsl:if
      >
      <xsl:choose>
        <xsl:when test="@tlwidth='-'">
          \titlerule[
          <xsl:value-of select="@tlheight"/>
          ]}
        </xsl:when>
        <xsl:when test="@tlwidth='*'">
          \titlerule[
          <xsl:value-of select="@tlheight"/>
          ]}
        </xsl:when>
        <xsl:otherwise>
          \rule{
          <xsl:value-of select="@tlwidth"/>
          }{
          <xsl:value-of select="@tlheight"/>
          }}
        </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@color">}</xsl:if>
    </xsl:when>
    <xsl:when test="@role='vspace'">
      \vspace{
      <xsl:value-of select="@tlheight"/>
      }
    </xsl:when>
  </xsl:choose>
</xsl:template>

<xsl:template match="html:bottomrule">
  <xsl:choose>
    <xsl:when test="@role='rule'">
      \titleline
      <xsl:if test="@tlwidth='*'"
      >*</xsl:if>
      [
      <xsl:value-of select="@tlalign"
      />
      ]{
      <xsl:if test="@color"
      >
        \textcolor
        <xsl:choose
      >
          <xsl:when test="substring(./@color,1,1)='#'">
            [HTML]{
            <xsl:value-of select="translate(substring(./@color,2,8),'abcdef','ABCDEF')"
      />
          </xsl:when
      >
          <xsl:otherwise>
            {
            <xsl:value-of select="./@color"/>
          </xsl:otherwise
      >
        </xsl:choose
      >
        }{
      </xsl:if
      >
      <xsl:choose>
        <xsl:when test="@tlwidth='-'">
          \titlerule[
          <xsl:value-of select="@tlheight"/>
          ]}
        </xsl:when>
        <xsl:when test="@tlwidth='*'">
          \titlerule[
          <xsl:value-of select="@tlheight"/>
          ]}
        </xsl:when>
        <xsl:otherwise>
          \rule{
          <xsl:value-of select="@tlwidth"/>
          }{
          <xsl:value-of select="@tlheight"/>
          }}
        </xsl:otherwise>
      </xsl:choose>
      <xsl:if test="@color">}</xsl:if>
    </xsl:when>
    <xsl:when test="@role='vspace'">
      \vspace{
      <xsl:value-of select="@tlheight"/>
      }
    </xsl:when>
  </xsl:choose>
</xsl:template>
<!-- end of section headings -->

<!-- leading -->
<xsl:template match="html:leading"
    >
  <xsl:if test="$formattingok">
    \leading{
    <xsl:value-of select="@val"
  />
    }
  </xsl:if>
</xsl:template>

<!-- magnification?? -->

<!-- section numbering style -->
<xsl:template match="html:numberstyles">

  <xsl:if test="@part">
    <xsl:choose>
      <xsl:when test="@part='none'">\renewcommand\thepart{}</xsl:when>
      <xsl:otherwise>
        \renewcommand\thepart{\
        <xsl:value-of select="@part"/>
        {part}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@chapter">
    <xsl:choose>
      <xsl:when test="@chapter='none'">\renewcommand\thechapter{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\thechapter{\
        <xsl:value-of select="@chapter"/>
        {chapter}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@section">
    <xsl:choose>
      <xsl:when test="@section='none'">\renewcommand\thesection{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\thesection{\
        <xsl:value-of select="@section"/>
        {section}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@subsection">
    <xsl:choose>
      <xsl:when test="@subsection='none'">\renewcommand\thesubsection{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\thesubsection{\thesection.\
        <xsl:value-of select="@subsection"/>
        {subsection}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@subsubsection">
    <xsl:choose>
      <xsl:when test="@subsubsection='none'">\renewcommand\thesubsubsection{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\thesubsubsection{\thesubsection.\
        <xsl:value-of select="@subsubsection"/>
        {subsubsection}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@paragraph">
    <xsl:choose>
      <xsl:when test="@paragraph='none'">\renewcommand\theparagraph{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\paragraph{\thesubbsusection.\
        <xsl:value-of select="@paragraph"/>
        {paragraph}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="@subparagraph">
    <xsl:choose>
      <xsl:when test="@subparagraph='none'">\renewcommand\thesubparagraph{}</xsl:when
      >
      <xsl:otherwise>
        \renewcommand\subparagraph{\theparagraph.\
        <xsl:value-of select="@subparagraph"/>
        {subparagraph}}
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
</xsl:template>

<!-- meta data section. Mostly this info is sent to PDF -->

<xsl:template match="html:meta">
  <xsl:choose>
    <xsl:when test="@name='author'">
      \hypersetup{pdfauthor=
      <xsl:value-of select="@content"/>
      }
    </xsl:when>
    <xsl:when test="@name='copyright'">
      \special{pdf: docinfo &lt;&lt; /Copyright (
      <xsl:value-of select="@content"/>
      )}
    </xsl:when>
    <xsl:when test="@name='disclaimer'">
      \special{pdf: docinfo  &lt;&lt; /Disclaimer (
      <xsl:value-of select="@content"/>
      )}
    </xsl:when>
    <xsl:when test="@name='editor'">
      \special{pdf: docinfo  &lt;&lt; /Editor (
      <xsl:value-of select="@content"/>
      )}
    </xsl:when>
    <xsl:when test="@name='publisher'">
      \special{pdf: docinfo  &lt;&lt; /Publisher (
      <xsl:value-of select="@content"/>
      )}
    </xsl:when>
    <xsl:when test="@name='trademark'">
      \special{pdf: docinfo  &lt;&lt; /Trademark (
      <xsl:value-of select="@content"/>
      )}
    </xsl:when>
  </xsl:choose>
</xsl:template>

<!-- <xsl:template match="html:title">
  \hypersetup{pdftitle=
  <xsl:value-of select="."/>
  }
</xsl:template> -->

<xsl:template name="metadata">
  <xsl:if test="html:sw-meta/@product">
      <xsl:value-of select="$newline"/>
      <xsl:text>%% Produced by </xsl:text><xsl:value-of select="html:sw-meta/@product"/>
  </xsl:if>
  <xsl:if test="html:sw-meta/@version">
      <xsl:value-of select="$newline"/>
      <xsl:text>%% Version </xsl:text><xsl:value-of select="html:sw-meta/@version"/>
  </xsl:if>
  <xsl:if test="html:sw-meta/@created">
      <xsl:value-of select="$newline"/>
      <xsl:text>%% Created </xsl:text><xsl:value-of select="html:sw-meta/@created"/>
  </xsl:if>
  <xsl:if test="html:sw-meta/@lastrevised">
      <xsl:value-of select="$newline"/>
      <xsl:text>%% Last revised </xsl:text><xsl:value-of select="html:sw-meta/@lastrevised"/>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
