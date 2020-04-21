<xsl:stylesheet version="1.1" xmlns:exsl="http://exslt.org/common" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:mml="http://www.w3.org/1998/Math/MathML" xmlns:sw="http://www.sciword.com/namespaces/sciword" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="html:graph">
    <xsl:apply-templates/>
  </xsl:template>
 <xsl:template match="html:imagecaption" mode="doit">  
    <xsl:choose>
      <xsl:when test="..[@pos='floating']">
        <xsl:text>\caption</xsl:text>
        <xsl:apply-templates mode="shortTitle"/>
        <xsl:text>{</xsl:text>
        <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when> 
      <xsl:otherwise>
       <xsl:text>\\</xsl:text>
       <xsl:apply-templates/>
       <xsl:text>\\</xsl:text> 
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="html:imagecaption"/>
  <xsl:template match="html:msiframe">
    <xsl:variable name="framePosType">
      <xsl:choose>
        <xsl:when test="@pos='inline'">ft-inline</xsl:when>
        <xsl:when test="@pos='center'">ft-centered</xsl:when>
        <xsl:when test="@pos='floating'">ft-floating</xsl:when>
        <xsl:when test="@pos='displayed'">ft-centered</xsl:when>
        <xsl:when test="@pos='display'">ft-centered</xsl:when>
        <xsl:when test="@pos='d'">ft-centered</xsl:when>
        <xsl:when test="@ltxfloat">ft-floating</xsl:when>
        <xsl:when test="@pos='left' or @pos='right' or @pos='outside' or @pos='inside'">ft-wrapped</xsl:when>
        <xsl:otherwise>ft-centered</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="width">
      <xsl:choose>
        <!-- <xsl:when test="@frametype='image'"><xsl:call-template name="getObjectWidth"><xsl:with-param name="objNode" select="(./html:object|./html:embed)[1]"/></xsl:call-template></xsl:when>-->
        <!-- ltx_width is being deprecated, but we continue to recognize it here -->
        <xsl:when test="@ltx_width">
          <xsl:value-of select="@ltx_width"/>
        </xsl:when>
        <xsl:when test="@width">
          <xsl:value-of select="@width"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>0</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="units">
      <xsl:choose>
        <xsl:when test="@frametype='image'">
          <xsl:call-template name="unit"/>
        </xsl:when>
        <xsl:when test="@units">
          <xsl:value-of select="@units"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>pt</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="height">
      <xsl:choose>
        <xsl:when test="@frametype='image'">0pt</xsl:when>
        <xsl:when test="@imageHeight">
          <xsl:value-of select="@imageHeight"/>
        </xsl:when>
        <xsl:when test="@height">
          <xsl:value-of select="@height"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>0</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="limitframemetrics">
      <xsl:choose>
        <xsl:when test="@topmargin or @sidemargin or @border or @padding">1</xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="inlineOffset">
      <xsl:choose>
        <xsl:when test="(@pos='inline') and (@inlineOffset) and (number(@inlineOffset)!=0)">
          <xsl:value-of select="-number(@inlineOffset)"/>
          <xsl:value-of select="$units"/>
        </xsl:when>
        <xsl:otherwise/>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="captionloc">
      <xsl:choose>
        <xsl:when test="(html:imagecaption[1]) and (@captionloc='top')">1</xsl:when>
        <xsl:when test="(html:imagecaption[1]) and (@captionloc='bottom')">2</xsl:when>
        <!-- <xsl:otherwise></xsl:otherwise>-->
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="usecolor">
      <xsl:choose>
        <xsl:when test="((@border-color and not (@border-color='#ffffff')) or (@background-color and not (@background-color='#ffffff')))">1</xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="needminipage">
      <xsl:choose>
        <xsl:when test="@frametype='textframe'">1</xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:if test="$inlineOffset and string-length($inlineOffset)">\raisebox{
      <xsl:value-of select="$inlineOffset"/>
      }{</xsl:if>
    <!--
  <xsl:if test="$limitframemetrics=1"><xsl:if test="@sidemargin"><xsl:value-of select="$newline"/><xsl:text>\setlength\columnsep{</xsl:text><xsl:value-of select="@sidemargin"/><xsl:value-of select="$units"/><xsl:text>}</xsl:text></xsl:if><xsl:choose><xsl:when test="@borderw"><xsl:value-of select="$newline"/><xsl:text>\setlength\fboxrule{</xsl:text><xsl:value-of select="@borderw"/><xsl:value-of select="$units"/><xsl:text>}</xsl:text></xsl:when><xsl:otherwise>\setlength\fboxrule{0pt} </xsl:otherwise></xsl:choose><xsl:if test="@padding"><xsl:value-of select="$newline"/><xsl:text>\setlength\fboxsep{</xsl:text><xsl:value-of select="@padding"/><xsl:value-of select="$units"/><xsl:text>}</xsl:text></xsl:if></xsl:if>-->
    <xsl:choose>
      <xsl:when test="$framePosType='ft-wrapped'">
        <xsl:choose>
          <xsl:when test="@frametype='table'">\begin{wraptable}</xsl:when>
          <xsl:otherwise>
            <xsl:if test="@sidemargin &gt; 0">
              <xsl:text>\setlength\columnsep{</xsl:text>
              <xsl:value-of select="@sidemargin"/>
              <xsl:call-template name="unit"/>
              <xsl:text>} </xsl:text>
            </xsl:if>\begin{wrapfigure}</xsl:otherwise>
        </xsl:choose>
        <xsl:if test="@nlines">[
          <xsl:value-of select="@nlines"/>
          ]</xsl:if>
        <xsl:choose>
          <xsl:when test="@pos='I'">{i}</xsl:when>
          <xsl:when test="@pos='O'">{o}</xsl:when>
          <xsl:when test="@pos='L'">{l}</xsl:when>
          <xsl:when test="@pos='R'">{r}</xsl:when>
          <xsl:when test="@pos='inside'">{I}</xsl:when>
          <xsl:when test="@pos='outside'">{O}</xsl:when>
          <xsl:when test="@pos='left'">{L}</xsl:when>
          <xsl:when test="@pos='right'">{R}</xsl:when>
          <xsl:otherwise>{r}</xsl:otherwise>
        </xsl:choose>
        <xsl:if test="@overhang">
          <xsl:text>[</xsl:text>
          <xsl:value-of select="@overhang"/>
          <xsl:value-of select="$units"/>
          <xsl:text>]</xsl:text>
        </xsl:if>
        <xsl:text>{</xsl:text>
        <xsl:choose>
          <xsl:when test="not(@rotation) or (@rotation='rot0')">
            <xsl:value-of select="$width"/>
            <xsl:value-of select="$units"/>
            <!--             \dimexpr </xsl:text><xsl:value-of select="$width"/><xsl:value-of select="$units"/>+2\fboxsep +2\fboxrule + .1in
 -->
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="@overhang">
                <xsl:value-of select="$height"/>
                <xsl:value-of select="$units"/>
              </xsl:when>
              <xsl:otherwise>0pt</xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>}</xsl:when>
      <xsl:when test="$framePosType='ft-floating'">
        <xsl:value-of select="$newline"/>
        <xsl:choose>
          <xsl:when test="@frametype='table'">
            <xsl:text>\begin{table}</xsl:text>
          </xsl:when>
          <xsl:when test="@frametype='textframe'">\begin{minipage}</xsl:when>
          <xsl:otherwise>
            <xsl:text>\begin{figure}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="@ltxfloat">
          <xsl:text>[</xsl:text>
          <xsl:value-of select="@ltxfloat"/>
          <xsl:text>]</xsl:text>
        </xsl:if>
        <xsl:text>\centering </xsl:text>
      </xsl:when>
      <xsl:when test="$framePosType='ft-centered'">
        <xsl:text>\begin{center}</xsl:text>
      </xsl:when>
      <!-- xsl:when test="($framePosType='ft-inline') and (@frametype='image')"><xsl:text>\parbox[b]{</xsl:text><xsl:value-of select="$width"/><xsl:value-of select="$units"/><xsl:text>}{ %</xsl:text><xsl:value-of select="$newline"/><xsl:text>\begin{center}</xsl:text></xsl:when -->
    </xsl:choose>
    <xsl:if test="$captionloc=1">
      <xsl:choose>
        <xsl:when test="$framePosType='ft-floating' or ($framePosType='ft-wrapped')">
          <xsl:apply-templates mode="doit" select="html:imagecaption"/>
          <xsl:if test="./*[@key]">
            <xsl:text>\label{</xsl:text>
            <xsl:value-of select="./*[@key]/@key[1]"/>
            <xsl:text>}</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates mode="doit" select="html:imagecaption"/>
          <xsl:text>\\ </xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@rotation='rot90'">\begin{turn}{-90}</xsl:when>
      <xsl:when test="@rotation='rot270'">\begin{turn}{90}</xsl:when>
      <xsl:otherwise/>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="@borderw">
        <xsl:value-of select="$newline"/>
        <xsl:text>\setlength\fboxrule{</xsl:text>
        <xsl:value-of select="@borderw"/>
        <xsl:value-of select="$units"/>
        <xsl:text>}</xsl:text>
        <xsl:if test="@padding">
          <xsl:text>\setlength\fboxsep{</xsl:text>
          <xsl:value-of select="@padding"/>
          <xsl:value-of select="$units"/>
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>\setlength\fboxrule{0pt} </xsl:otherwise>
    </xsl:choose>
    <xsl:text>\fcolorbox</xsl:text>
    <xsl:if test="@border-color">
      <xsl:choose>
        <xsl:when test="substring(./@border-color,1,1)='#'">
          <xsl:text>[HTML]{</xsl:text>
          <xsl:value-of select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>{</xsl:text>
          <xsl:value-of select="./@border-color"/>
          <xsl:text>}</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:if test="not(@border-color)">
      <xsl:text>[HTML]{FFFFFF}</xsl:text>
    </xsl:if>
    <xsl:if test="@background-color">
      <xsl:text>{</xsl:text>
      <xsl:choose>
        <xsl:when test="substring(./@background-color,1,1)='#'">
          <xsl:value-of select="translate(substring(./@background-color,2,8),'abcdef','ABCDEF')"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="./@background-color"/>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:text>}</xsl:text>
    </xsl:if>
    <xsl:if test="not(@background-color)">
      <xsl:text>{FFFFFF}</xsl:text>
    </xsl:if>
    <xsl:text>{</xsl:text>
    <xsl:if test="@needminipage=1">
      <xsl:text>\setlength\fboxsep{</xsl:text>
      <xsl:value-of select="@padding"/><xsl:value-of select="$units"/>
      <xsl:text>}</xsl:text>
    </xsl:if>
    <xsl:if test="$needminipage=1">\begin{minipage}[t] 
      <xsl:if test="$width &gt;0">
        {
        <xsl:value-of select="$width"/>
        <xsl:value-of select="$units"/>
        }</xsl:if>
      <xsl:if test="not(@frametype='textframe')">
        <xsl:choose>
          <xsl:when test="not(@rotation) or (@rotation='rot0')">{
            <xsl:value-of select="$width"/></xsl:when>
          <xsl:otherwise>{
            <xsl:value-of select="$width"/></xsl:otherwise>
        </xsl:choose>
        <xsl:value-of select="$units"/>} %</xsl:if>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@frametype='image'">
        <xsl:apply-templates mode="contents" select="(html:object|html:embed)[1]"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
    <!-- Now back out putting in \end{environment} or } as necessary -->
    <xsl:if test="$needminipage=1">\end{minipage}    
    </xsl:if>
    } <!-- matches '\fcolorbox{' -->
    <xsl:if test="@rotation='rot90' or @rotation='rot270'">\end{turn}</xsl:if>
    <xsl:if test="$captionloc=2">
      <xsl:choose>
        <xsl:when test="($framePosType='ft-floating') or ($framePosType='ft-wrapped')">
          <xsl:apply-templates mode="doit" select="html:imagecaption"/>
          <xsl:if test="./*[@key]">
            <xsl:text>\label{</xsl:text>
            <xsl:value-of select="./*[@key]/@key[1]"/>
            <xsl:text>}</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates mode="doit" select="html:imagecaption"/>
          <xsl:text>\\ </xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:choose>
      <!-- xsl:when test="$framePosType='ft-inline'"><xsl:text>\end{center}}</xsl:text></xsl:when -->
      <xsl:when test="$framePosType='ft-centered'">\end{center}</xsl:when>
      <xsl:when test="$framePosType='ft-floating'">
        <xsl:choose>
          <xsl:when test="@frametype='table'">
            <xsl:text>\end{table}</xsl:text>
          </xsl:when>
          <xsl:when test="@frametype='textframe'">
            <xsl:text>\end{minipage}</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>\end{figure}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="$framePosType='ft-wrapped'">
        <xsl:choose>
          <xsl:when test="@frametype='table'">\end{wraptable}</xsl:when>
          <xsl:otherwise>\end{wrapfigure}</xsl:otherwise>
        </xsl:choose>
      </xsl:when>
    </xsl:choose>
    <xsl:if test="($inlineOffset and string-length($inlineOffset))">}</xsl:if>
  </xsl:template>
</xsl:stylesheet>