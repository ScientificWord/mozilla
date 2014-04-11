<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


  <xsl:template name="array">
    <xsl:param name="LaTeX-env"/>
  
    <xsl:variable name="column-counts.tr">
      <xsl:call-template name="cell-counter"/>
    </xsl:variable>
    <xsl:variable name="column-counts" select="exsl:node-set($column-counts.tr)"/>

<!--
<xsl:for-each select="$column-counts/ncols">
  <xsl:value-of select="."/>
  <xsl:text>-</xsl:text>
</xsl:for-each>
-->

<!-- xsl:value-of select="$column-counts/ncols[position()=last()]"/ -->

    <xsl:text>\begin{</xsl:text>
    <xsl:value-of select="$LaTeX-env"/>
    <xsl:text>}</xsl:text>
    <xsl:if test="@align='top'">
      <xsl:text xml:space="preserve">[t]</xsl:text>
    </xsl:if>
    <xsl:if test="@align='bottom'">
      <xsl:text xml:space="preserve">[b]</xsl:text>
    </xsl:if>

<!-- now for the cols
frame="solid" 
columnlines="none solid solid solid" columnalign="center right left center center"
{|cr|l|c|c|}
-->

    <xsl:if test="$LaTeX-env='array'">
      <xsl:text>{</xsl:text>
      <xsl:if test="@frame='solid'">
        <xsl:text>|</xsl:text>
      </xsl:if>

      <xsl:call-template name="do-cols">
        <xsl:with-param name="columns-to-do" select="$column-counts/ncols[position()=last()]"/>
        <xsl:with-param name="columnlines"   select="normalize-space(@columnlines)"/>
        <xsl:with-param name="columnalign"   select="normalize-space(@columnalign)"/>
      </xsl:call-template>


      <xsl:if test="@frame='solid'">
        <xsl:text>|</xsl:text>
      </xsl:if>
      <xsl:text>}</xsl:text>
    </xsl:if>


    <!-- JCS <xsl:text xml:space="preserve">\LBe</xsl:text> -->

    <xsl:if test="@frame='solid'">
      <!-- JCS <xsl:text xml:space="preserve">\hline\LBe</xsl:text> -->
      <xsl:text xml:space="preserve">\hline </xsl:text>
    </xsl:if>

    <xsl:for-each select="*">
      <xsl:choose>
        <xsl:when test="self::mml:mtr or self::mml:mlabeledtr">
          <xsl:for-each select="*">
            <xsl:choose>
              <xsl:when test="self::mml:mtd">
				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>\multicolumn{</xsl:text>
                  <xsl:value-of select="@columnspan"/>
                  <xsl:text>}{</xsl:text>
                  <xsl:text>c</xsl:text>
                  <xsl:text>}{</xsl:text>
				</xsl:if>
                <xsl:for-each select="*">
                  <xsl:apply-templates select="."/>
                </xsl:for-each>
				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>}</xsl:text>
				</xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:apply-templates select="."/>
              </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="position() != last()">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
            </xsl:if>
          </xsl:for-each>
        </xsl:when>
        <xsl:when test="self::mml:mtd">
		  <xsl:if test="@columnspan&gt;1">
            <xsl:text>\multicolumn{</xsl:text>
            <xsl:value-of select="@columnspan"/>
            <xsl:text>}{</xsl:text>
            <xsl:text>c</xsl:text>
            <xsl:text>}{</xsl:text>
		  </xsl:if>
          <xsl:for-each select="*">
            <xsl:apply-templates select="."/>
          </xsl:for-each>
		  <xsl:if test="@columnspan&gt;1">
            <xsl:text>}</xsl:text>
		  </xsl:if>
        </xsl:when>
        <xsl:otherwise>
            <xsl:apply-templates select="."/>
        </xsl:otherwise>
      </xsl:choose>

<!-- handle \tag{}, \nonumber, etc. here -->

      <xsl:if test="name()='mml:mlabeledtr'">
	    <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve"> \TCItag{</xsl:text>
	    </xsl:if>
	    <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text xml:space="preserve"> \tag{</xsl:text>
	    </xsl:if>
        <xsl:apply-templates mode="in-text" select="./*[1]"/>
        <xsl:text>}</xsl:text>
      </xsl:if>

      <xsl:call-template name="end-table-row">
        <xsl:with-param name="current-row" select="position()"/>
        <xsl:with-param name="last-row"    select="last()"/>
        <xsl:with-param name="rowlines"    select="normalize-space(@rowlines)"/>
      </xsl:call-template>

    </xsl:for-each>


    <xsl:if test="@frame='solid'">
       <xsl:text>~\\ \hline </xsl:text>
    </xsl:if>

    <xsl:text>\end{</xsl:text>
    <xsl:value-of select="$LaTeX-env"/>
    <xsl:text>}</xsl:text>
  </xsl:template>

</xsl:stylesheet>

