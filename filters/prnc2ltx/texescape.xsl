<!-- texescape.xsl -->
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:regexp="http://exslt.org/regular-expressions">


<xsl:template match="text()">
  <xsl:variable name="sub1">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="."/>
      <xsl:with-param name="substring" select="'\'"/>
      <xsl:with-param name="replacement" select="'\verb.\.'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub2">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub1"/>
      <xsl:with-param name="substring" select="'{'"/>
      <xsl:with-param name="replacement" select="'\{'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub3">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub2"/>
      <xsl:with-param name="substring" select="'}'"/>
      <xsl:with-param name="replacement" select="'\}'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub4">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub3"/>
      <xsl:with-param name="substring" select="'&amp;'"/>
      <xsl:with-param name="replacement" select="'&#x5C;&amp;'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub5">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub4"/>
      <xsl:with-param name="substring" select="'_'"/>
      <xsl:with-param name="replacement" select="'\_'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub6">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub5"/>
      <xsl:with-param name="substring" select="'&#x24;'"/>
      <xsl:with-param name="replacement" select="'\&#x24;'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub7">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub6"/>
      <xsl:with-param name="substring" select="'%'"/>
      <xsl:with-param name="replacement" select="'\%'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub8">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub7"/>
      <xsl:with-param name="substring" select="'~'"/>
      <xsl:with-param name="replacement" select="'\verb.~.'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub9">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub8"/>
      <xsl:with-param name="substring" select="'^'"/>
      <xsl:with-param name="replacement" select="'\verb.^.'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="sub10">
    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub9"/>
      <xsl:with-param name="substring" select="'#'"/>
      <xsl:with-param name="replacement" select="'\#'"/>
    </xsl:call-template>
  </xsl:variable>

    <xsl:call-template name="replace-substring">
      <xsl:with-param name="original" select="$sub10"/>
      <xsl:with-param name="substring" select="'&#xa0;'"/>
      <xsl:with-param name="replacement" select="' '"/>
    </xsl:call-template>
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




