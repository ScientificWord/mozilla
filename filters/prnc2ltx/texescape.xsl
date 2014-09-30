<!-- texescape.xsl -->
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:regexp="http://exslt.org/regular-expressions">

<xsl:template mode="texcomment" match="text()">
  <xsl:call-template name="replace-substring">
    <xsl:with-param name="original" select="."/>
    <xsl:with-param name="substring" select="'&#x0A;'"/>
    <xsl:with-param name="replacement" select="'&#x0A;% '"/>
  </xsl:call-template>
</xsl:template>

<xsl:template mode="tex" match="text()">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="text()">
  <xsl:variable name="sub1">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="."/>
      <xsl:with-param name="substring" select="'\'"/>
      <xsl:with-param name="replacement" select="'##dollar##\backslash##dollar##'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub2">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub1"/>
      <xsl:with-param name="substring" select="'&#x24;'"/>
      <xsl:with-param name="replacement" select="'\&#x24;'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub3">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub2"/>
      <xsl:with-param name="substring" select="'##dollar##'"/>
      <xsl:with-param name="replacement" select="'&#x24;'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub4">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub3"/>
      <xsl:with-param name="substring" select="'{'"/>
      <xsl:with-param name="replacement" select="'\{'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub5">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub4"/>
      <xsl:with-param name="substring" select="'}'"/>
      <xsl:with-param name="replacement" select="'\}'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub6">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub5"/>
      <xsl:with-param name="substring" select="'&amp;'"/>
      <xsl:with-param name="replacement" select="'&#x5C;&amp;'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub7">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub6"/>
      <xsl:with-param name="substring" select="'_'"/>
      <xsl:with-param name="replacement" select="'\_'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub8">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub7"/>
      <xsl:with-param name="substring" select="'%'"/>
      <xsl:with-param name="replacement" select="'\%'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub9">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub8"/>
      <xsl:with-param name="substring" select="'~'"/>
      <xsl:with-param name="replacement" select="'\symbol{126}'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub10">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub9"/>
      <xsl:with-param name="substring" select="'^'"/>
      <xsl:with-param name="replacement" select="'\symbol{94}'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub11">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub10"/>
      <xsl:with-param name="substring" select="'#'"/>
      <xsl:with-param name="replacement" select="'\#'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub12">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub11"/>
      <xsl:with-param name="substring" select="'&#x2013;'"/>
      <xsl:with-param name="replacement" select="'--'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub13">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub12"/>
      <xsl:with-param name="substring" select="'&#x2014;'"/>
      <xsl:with-param name="replacement" select="'---'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub14">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub13"/>
      <xsl:with-param name="substring" select="'&#x201C;'"/>
      <xsl:with-param name="replacement" select="'``'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub15">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub14"/>
      <xsl:with-param name="substring" select="'&#x201D;'"/>
      <xsl:with-param name="replacement" select='"&#x27;&#x27;"'/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub16">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub15"/>
      <xsl:with-param name="substring" select="'&#x2192;'"/>
      <xsl:with-param name="replacement" select="'\textrightarrow'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub17">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub16"/>
      <xsl:with-param name="substring" select="'&#x2190;'"/>
      <xsl:with-param name="replacement" select="'\textleftarrow'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub18">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub17"/>
      <xsl:with-param name="substring" select="'&#x2191;'"/>
      <xsl:with-param name="replacement" select="'\textuparrow'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub19">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub18"/>
      <xsl:with-param name="substring" select="'&#x2193;'"/>
      <xsl:with-param name="replacement" select="'\textdownarrow'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub20">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub19"/>
      <xsl:with-param name="substring" select="'&#x2026;'"/>
      <xsl:with-param name="replacement" select="'...'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub21">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub20"/>
      <xsl:with-param name="substring" select="'&#xA0;'"/>
      <xsl:with-param name="replacement" select="'~'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub22">
    <xsl:call-template name="replace-substring">
	    <xsl:with-param name="original" select="$sub21"/>
      <xsl:with-param name="substring" select="'&#x20AC;'"/>
      <xsl:with-param name="replacement" select="'\euro'"/>
    </xsl:call-template>
  </xsl:variable>
  
  <xsl:value-of select="$sub22"/>
</xsl:template>

<xsl:template name="replace-substring">
  <xsl:param name="original" />
  <xsl:param name="substring" />
  <xsl:param name="replacement" />
  <xsl:choose>
    <xsl:when test="contains($original, $substring)">
      <xsl:value-of 
        select="substring-before($original, $substring)" />
      <xsl:value-of select="$replacement" />
      <xsl:call-template name="replace-substring">
        <xsl:with-param name="original" 
          select="substring-after($original, $substring)" />
        <xsl:with-param 
          name="substring" select="$substring" />
        <xsl:with-param 
          name="replacement" select="$replacement" />
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$original" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>  


</xsl:stylesheet>




