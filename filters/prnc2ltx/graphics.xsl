<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:mml="http://www.w3.org/1998/Math/MathML" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:sw="http://www.sciword.com/namespaces/sciword" version="1.1">
  <xsl:template name="unit">
    <xsl:choose>
      <xsl:when test="@units='px'">pt</xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@units"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:variable name="upperCaseAlpha">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
  <xsl:variable name="lowerCaseAlpha">abcdefghijklmnopqrstuvwxyz</xsl:variable>

  <xsl:template name="buildincludegraphics">
    <xsl:variable name="theUnit">
      <xsl:call-template name="unit"/>
    </xsl:variable>
    <xsl:variable name="imageWidth">
      <xsl:call-template name="getImageWidth">
        <xsl:with-param name="objNode" select="."/>
      </xsl:call-template>
    </xsl:variable>
\includegraphics[<xsl:if test="@rotation">angle=<xsl:if test="@rotation='rot90'">
  -90</xsl:if><xsl:if test="@rotation='rot270'">90</xsl:if>,</xsl:if>
  <xsl:if test="number($imageWidth) != 0"> width=<xsl:value-of select="$imageWidth"/>
<xsl:value-of select="$theUnit"/>,</xsl:if>
  <xsl:if test="@imageHeight and (number(@imageHeight) != 0)"> totalheight=<xsl:value-of select="@imageHeight"/>
<xsl:value-of select="$theUnit"/>,</xsl:if>
  <xsl:if test="@naturalWidth and @naturalHeight and (number(@naturalWidth) != 0) and (number(@naturalHeight) != 0)"> natwidth=<xsl:value-of select="@naturalWidth"/>
<xsl:value-of select="$theUnit"/>, natheight=<xsl:value-of select="@naturalHeight"/>
<xsl:value-of select="$theUnit"/>
</xsl:if>]{<xsl:call-template name="getSourceName"/>}
<xsl:if test="@key"> \label{<xsl:value-of select="@key"/>}</xsl:if>
</xsl:template>

<xsl:template name="convertHourMinSecThirtiethTimeToSeconds">
  <xsl:param name="HMSTime" select="0"/>
  <xsl:param name="currPiece" select="0"/>
  <xsl:param name="prevValue" select="0"/>
  <xsl:variable name="ourValue">
    <xsl:value-of select="substring-before($HMSTime,':')" />
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="number($currPiece) &gt; 2">
      <xsl:value-of select="number($prevValue) + (number($ourValue) div 30.0)"/>
    </xsl:when>
    <xsl:when test="contains($HMSTime,':')">
      <xsl:call-template name="convertHourMinSecThirtiethTimeToSeconds">
        <xsl:with-param name="HMSTime" select="substring-after($HMSTime,':')"/>
        <xsl:with-param name="currPiece" select="number($currPiece) + 1" />
        <xsl:with-param name="prevValue">
          <xsl:choose>
            <xsl:when test="number($currPiece) = 2">
              <xsl:value-of select="number($ourValue) + number($prevValue)" />
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="60 * (number($ourValue) + number($prevValue))" />
            </xsl:otherwise>
          </xsl:choose>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="number($ourValue) + number($prevValue)" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

  <xsl:template name="getMovieOptions">
    <xsl:variable name="controller">
      <xsl:choose>
        <xsl:when test="local-name()='object'">
          <xsl:value-of select="./*[local-name()='param'][@name='controller']/@value" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@controller" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="autoplay">
      <xsl:choose>
        <xsl:when test="local-name()='object'">
          <xsl:value-of select="./*[local-name()='param'][@name='autoplay']/@value" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@autoplay" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="loop">
      <xsl:choose>
        <xsl:when test="local-name()='object'">
          <xsl:value-of select="./*[local-name()='param'][@name='loop']/@value" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@loop" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="starttime">
      <xsl:choose>
        <xsl:when test="local-name()='object'">
          <xsl:value-of select="./*[local-name()='param'][@name='starttime']/@value" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@starttime" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="endtime">
      <xsl:choose>
        <xsl:when test="local-name()='object'">
          <xsl:value-of select="./*[local-name()='param'][@name='endtime']/@value" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@endtime" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:text xml:space="preserve">poster, url, </xsl:text>
    <xsl:if test="@type and string-length(@type)">
      <xsl:text>mimetype=</xsl:text><xsl:value-of select="@type"/><xsl:text xml:space="preserve">, </xsl:text>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="$controller and not($controller='false')"><xsl:text xml:space="preserve">controls, </xsl:text></xsl:when>
      <xsl:otherwise><xsl:text>mouse=true, </xsl:text></xsl:otherwise>
    </xsl:choose>
    <xsl:if test="$autoplay and not($autoplay='false')"><xsl:text xml:space="preserve">autoplay, </xsl:text></xsl:if>
    <xsl:choose>
      <xsl:when test="not($loop) or not(string-length($loop))"></xsl:when>
      <xsl:when test="translate($loop,$upperCaseAlpha,$lowerCaseAlpha)='palindrome'"><xsl:text xml:space="preserve">palindrome, </xsl:text></xsl:when>
      <xsl:when test="translate($loop,$upperCaseAlpha,$lowerCaseAlpha)='true'"><xsl:text xml:space="preserve">repeat, </xsl:text></xsl:when>
      <xsl:otherwise></xsl:otherwise>
    </xsl:choose>
    <xsl:if test="$starttime and string-length($starttime)">
      <xsl:text xml:space="preserve">startat=time:</xsl:text>
      <xsl:call-template name="convertHourMinSecThirtiethTimeToSeconds"><xsl:with-param name="HMSTime" select="$starttime"/></xsl:call-template>
      <xsl:text xml:space="preserve">, </xsl:text>
    </xsl:if>
    <xsl:if test="$endtime and string-length($endtime)">
      <xsl:text xml:space="preserve">endat=time:</xsl:text>
      <xsl:call-template name="convertHourMinSecThirtiethTimeToSeconds"><xsl:with-param name="HMSTime" select="$endtime"/></xsl:call-template>
      <xsl:text xml:space="preserve">, </xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="buildincludemovie"><xsl:variable name="theUnit"><xsl:call-template name="unit"/></xsl:variable><xsl:variable name="imageWidth"><xsl:call-template name="getImageWidth"><xsl:with-param name="objNode" select="."/></xsl:call-template></xsl:variable>
  <xsl:variable name="movieOptions"><xsl:call-template name="getMovieOptions"/></xsl:variable>
  \includemovie<xsl:if test="$movieOptions and (string-length(normalize-space($movieOptions)) &gt; 0)">[<xsl:value-of select="$movieOptions"/>]</xsl:if>
  {<xsl:if test="number($imageWidth) != 0"><xsl:value-of select="$imageWidth"/>
<xsl:value-of select="$theUnit"/></xsl:if>}{<xsl:if test="@imageHeight and (number(@imageHeight) != 0)"><xsl:value-of select="@imageHeight"/>
<xsl:value-of select="$theUnit"/></xsl:if>}
{<xsl:call-template name="getSourceName"><xsl:with-param name="needExtension" select="1" /><xsl:with-param name="fullPath" select="1" /></xsl:call-template>}
  </xsl:template>

  <xsl:template name="getSourceName">
    <xsl:param name="needExtension" select="0" />
    <xsl:param name="fullPath" select="0" />
    <xsl:variable name="rawName">
      <xsl:choose>
        <xsl:when test="@typesetSource">
          <xsl:value-of select="@typesetSource"/>
        </xsl:when>
        <xsl:when test="@src">
          <xsl:value-of select="@src"/>
        </xsl:when>
        <xsl:when test="@data">
          <xsl:value-of select="@data"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="''"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="correctedName">
      <xsl:choose>
        <xsl:when test="contains($rawName,'%') and @originalSrcUrl and string-length(@originalSrcUrl)"
        ><xsl:value-of select="@originalSrcUrl"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$rawName"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="fileName">
      <xsl:choose>
        <xsl:when test="starts-with($correctedName, 'graphics/')">
          <xsl:if test="$fullPath = 1"><!-- <xsl:text>../graphics/</xsl:text> --></xsl:if>
          <xsl:value-of select="substring-after($correctedName, 'graphics/')"/>
        </xsl:when>
        <xsl:when test="starts-with($correctedName, 'gcache/')">
          <xsl:if test="$fullPath = 1"><!-- <xsl:text>../gcache/</xsl:text> --></xsl:if>
          <xsl:value-of select="substring-after($correctedName, 'gcache/')"/>
        </xsl:when>
        <xsl:when test="starts-with($correctedName, 'tcache/')">
          <xsl:if test="$fullPath = 1"><!-- <xsl:text>../tcache/</xsl:text> --></xsl:if>
          <xsl:value-of select="substring-after($correctedName, 'tcache/')"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$correctedName"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="afterLastDot">
      <xsl:call-template name="charsAfterLastOccurence">
        <xsl:with-param name="theString" select="$fileName"/>
        <xsl:with-param name="theSubstring" select="'.'"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$needExtension = 1"><xsl:value-of select="$fileName"/></xsl:when>
      <xsl:otherwise><xsl:value-of select="substring($fileName, 1, string-length($fileName) - number($afterLastDot) - 1)"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name="charsAfterLastOccurence">
    <xsl:param name="theString"/>
    <xsl:param name="theSubstring"/>
    <xsl:choose>
      <xsl:when test="contains($theString,$theSubstring)">
        <xsl:call-template name="charsAfterLastOccurence">
          <xsl:with-param name="theString" select="substring-after($theString,$theSubstring)"/>
          <xsl:with-param name="theSubstring" select="$theSubstring"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="string-length($theString)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="html:object|html:embed" mode="contents">
    <xsl:variable name="theUnit">
      <xsl:call-template name="unit"/>
    </xsl:variable>
    <xsl:if test="@border">
    \setlength\fboxrule{<xsl:value-of select="@border"/>
      <xsl:value-of select="$theUnit"/>}
      <xsl:if test="@padding"> \setlength\fboxsep{<xsl:value-of select="@padding"/>
        <xsl:value-of select="$theUnit"/>}</xsl:if>
      <xsl:if test="@border-color">{\color    
        <xsl:choose><xsl:when test="substring(./@border-color,1,1)='#'">[HTML]{<xsl:value-of select="translate(substring(./@border-color,2,8),'abcdef','ABCDEF')"/>}</xsl:when>
          <xsl:otherwise>{black}</xsl:otherwise></xsl:choose>
      </xsl:if>
      \framebox{
    </xsl:if>
    <xsl:choose>
      <xsl:when test="@isVideo='true'">
        <xsl:call-template name="buildincludemovie"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="buildincludegraphics"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="@border"><xsl:if test="@border-color">}</xsl:if>}</xsl:if>
  </xsl:template>

    <xsl:template match="html:object|html:embed">
      <xsl:if test="@msisnap or not(@msigraph='true')">
        <xsl:choose>
          <xsl:when test="@pos='inline'">
            <xsl:apply-templates select="." mode="contents"/>
          </xsl:when>
          <xsl:when test="@pos='display'">
            <xsl:value-of select="$newline"/>
            <xsl:text>\begin{center}</xsl:text>
            <xsl:apply-templates select="." mode="contents"/>
            <xsl:text>\end{center}</xsl:text>
          </xsl:when>
          <xsl:when test="@pos='float'">
            <xsl:choose>
              <xsl:when test="@placement='f'">
                 <xsl:value-of select="$newline"/>
                 <xsl:text>\begin{figure}\begin{center}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$newline"/>
                <xsl:text>\begin{wrapfigure}{</xsl:text>
                <xsl:choose>
                   <xsl:when test="not(substring(@placement,1,1))">
                     <xsl:text>O</xsl:text>
                   </xsl:when>
                   <xsl:otherwise>
                     <xsl:value-of select="substring(@placement,1,1)"/>
                   </xsl:otherwise>
                </xsl:choose>
                <xsl:text>}</xsl:text>
                <xsl:if test="@overhang &gt; 0">
                  <xsl:text>[</xsl:text>
                  <xsl:value-of select="@overhang"/>
                  <xsl:call-template name="unit"/>
                  <xsl:text>]</xsl:text>
                </xsl:if>
                <xsl:text>{0pt}</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="@sidemargin &gt; 0">
              <xsl:text>columnsep=</xsl:text>
              <xsl:value-of select="@sidemargin"/>
              <xsl:call-template name="unit"/>
              <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:if test="@topmargin &gt; 0">
              <xsl:text>\intextsep=</xsl:text>
              <xsl:value-of select="@topmargin"/>
              <xsl:call-template name="unit"/>
              <xsl:text> </xsl:text>
            </xsl:if>
          <!-- xsl:if test="@captionabove"><xsl:apply-templates/> </xsl:if -->
          <xsl:apply-templates select="." mode="contents"/>
          <xsl:choose>
            <xsl:when test="@placement='f'">
               <xsl:text>\end{center}\end{figure}</xsl:text>
            </xsl:when>
            <xsl:otherwise>
               <xsl:text>\end{wrapfigure}</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="contents"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>


  <xsl:template match="html:caption">\caption{<xsl:apply-templates/>}</xsl:template>
  <xsl:template match="html:imagecaption" mode="caption">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template name="getImageWidth">
    <xsl:param name="objNode"/>
    <xsl:param name="noZero" select="false"/>
    <xsl:choose>
      <xsl:when test="$objNode/@imageWidth and (number($objNode/@imageWidth) != 0)">
        <xsl:value-of select="number($objNode/@imageWidth)"/>
      </xsl:when>
      <xsl:when test="$objNode/@imageHeight and (number($objNode/@imageHeight) != 0) and $objNode/@naturalHeight and (number($objNode/@naturalHeight) != 0) and $objNode/@naturalWidth">
        <xsl:value-of select="(number($objNode/@naturalWidth) * number($objNode/@imageHeight)) div number($objNode/@naturalHeight)"/>
      </xsl:when>
      <xsl:when test="$objNode/@width and (number($objNode/@width) != 0)">
        <xsl:value-of select="number($objNode/@width)"/>
      </xsl:when>
      <xsl:when test="$objNode/@height and (number($objNode/@height) != 0) and $objNode/@naturalHeight and (number($objNode/@naturalHeight) != 0) and $objNode/@naturalWidth">
        <xsl:value-of select="(number($objNode/@naturalWidth) * number($objNode/@height)) div number($objNode/@naturalHeight)"/>
      </xsl:when>
      <xsl:when test="$noZero and $objNode/@naturalWidth">
        <xsl:value-of select="number($objNode/@naturalWidth)"/>
      </xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template name="getObjectWidth">
    <xsl:param name="objNode"/>
    <xsl:variable name="baseWidth">
      <xsl:call-template name="getImageWidth">
        <xsl:with-param name="objNode" select="$objNode"/>
        <xsl:with-param name="noZero" select="true"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="padding">
      <xsl:choose>
        <xsl:when test="$objNode/@padding">
          <xsl:value-of select="2*number($objNode/@padding)"/>
        </xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="border">
      <xsl:choose>
        <xsl:when test="$objNode/@border">
          <xsl:value-of select="2*number($objNode/@border)"/>
        </xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:value-of select="number($baseWidth) + number($padding) + number($border)"/>
  </xsl:template>
  <!-- plotwrapper section. Plotwrappers do not have a placement information (they will frequently be inside an msiframe, which takes
    care of that). They do not have margins (that's done by the frame's padding). They do have borders, background color, and padding. They
    are also given a size. -->
  <xsl:template match="html:plotwrapper">
    <xsl:choose>
      <xsl:when test="@borderw">\setlength\fboxrule{<xsl:value-of select="@borderw"/><xsl:value-of select="@units"/>} </xsl:when>
      <xsl:otherwise>\setlength\fboxrule{0pt} </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="@padding">\setlength\fboxsep{<xsl:value-of select="@padding"/><xsl:value-of select="@units"/>} </xsl:if>
    <xsl:if test="@border-color or @background-color">
      <xsl:text>\fcolorbox</xsl:text>
      <xsl:if test="@border-color">
        <xsl:choose>
          <xsl:when test="substring(./@border-color,1,1)='#'">
             <xsl:text>[HTML]</xsl:text>
             <xsl:text>{</xsl:text>
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
         <xsl:text>{white}</xsl:text>
      </xsl:if>
      <xsl:if test="@background-color">
        <xsl:choose>
          <xsl:when test="substring(./@background-color,1,1)='#'">
             <xsl:text>{</xsl:text>
             <xsl:value-of select="translate(substring(./@background-color,2,8),'abcdef','ABCDEF')"/>
             <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:otherwise>{<xsl:value-of select="./@background-color"/>}</xsl:otherwise>
        </xsl:choose>
      </xsl:if>
      <xsl:if test="not(@background-color)">
         <xsl:text>{white}</xsl:text>
      </xsl:if>
      <xsl:text>{</xsl:text>
      <xsl:apply-templates/>
      <xsl:text>}</xsl:text>
    </xsl:if>    
    <xsl:if test="not (@border-color or @background-color)">
      <xsl:apply-templates/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
