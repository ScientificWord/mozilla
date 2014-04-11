<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- recursive template to generate LaTeX cols param from mml attributes -->

  <xsl:template name="do-cols">
    <xsl:param name="columns-to-do"/>
    <xsl:param name="columnlines"/>
    <xsl:param name="columnalign"/>
  
<!-- xsl:value-of select="$columns-to-do"/ -->

<!-- script l or r or c	-->
    <xsl:choose>
      <xsl:when test="starts-with($columnalign,'left')">
        <xsl:text xml:space="preserve">l</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($columnalign,'right')">
        <xsl:text xml:space="preserve">r</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve">c</xsl:text>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:if test="$columns-to-do > 1">
      <xsl:choose>
        <xsl:when test="starts-with($columnlines,'solid')">
          <xsl:text xml:space="preserve">|</xsl:text>
        </xsl:when>
        <xsl:when test="starts-with($columnlines,'dashed')">
          <xsl:text xml:space="preserve">|</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text xml:space="preserve"></xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <xsl:if test="$columns-to-do > 1">
      <xsl:choose>
        <xsl:when test="contains($columnalign,' ') and contains($columnlines,' ')">
          <xsl:call-template name="do-cols">
            <xsl:with-param name="columns-to-do" select="$columns-to-do - 1"/>
            <xsl:with-param name="columnlines"   select="substring-after($columnlines,' ')"/>
            <xsl:with-param name="columnalign"   select="substring-after($columnalign,' ')"/>
          </xsl:call-template>
        </xsl:when>

        <xsl:when test="contains($columnalign,' ')">
          <xsl:call-template name="do-cols">
            <xsl:with-param name="columns-to-do" select="$columns-to-do - 1"/>
            <xsl:with-param name="columnlines"   select="$columnlines"/>
            <xsl:with-param name="columnalign"   select="substring-after($columnalign,' ')"/>
          </xsl:call-template>
        </xsl:when>

        <xsl:when test="contains($columnlines,' ')">
          <xsl:call-template name="do-cols">
            <xsl:with-param name="columns-to-do" select="$columns-to-do - 1"/>
            <xsl:with-param name="columnlines"   select="substring-after($columnlines,' ')"/>
            <xsl:with-param name="columnalign"   select="$columnalign"/>
          </xsl:call-template>
        </xsl:when>

        <xsl:otherwise>
          <xsl:call-template name="do-cols">
            <xsl:with-param name="columns-to-do" select="$columns-to-do - 1"/>
            <xsl:with-param name="columnlines"   select="$columnlines"/>
            <xsl:with-param name="columnalign"   select="$columnalign"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

  </xsl:template>


<!-- recursive template to extract info from mtable rowlines attribute -->

  <xsl:template name="script-hline">
    <xsl:param name="current-row"/>
    <xsl:param name="rowlines"/>
  
    <xsl:choose>
      <xsl:when test="$current-row = 1">
        <xsl:if test="starts-with($rowlines,'solid')">
          <xsl:text xml:space="preserve"> \hline</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="contains($rowlines,' ')">
        <xsl:call-template name="script-hline">
          <xsl:with-param name="current-row" select="$current-row - 1"/>
          <xsl:with-param name="rowlines"    select="substring-after($rowlines,' ')"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="starts-with($rowlines,'solid')">
          <xsl:text xml:space="preserve"> \hline</xsl:text>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


<!-- template to script the end of a table row -->

  <xsl:template name="end-table-row">
    <xsl:param name="current-row"/>
    <xsl:param name="last-row"/>
    <xsl:param name="rowlines"/>
  
    <xsl:if test="$current-row &lt; $last-row">
      <xsl:text> \\</xsl:text>
      <xsl:value-of select="$newline"/>

      <xsl:if test="string-length($rowlines)&gt;0">
        <xsl:call-template name="script-hline">
          <xsl:with-param name="current-row" select="$current-row"/>
          <xsl:with-param name="rowlines"    select="$rowlines"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:if>

    <!-- JCS <xsl:text xml:space="preserve">\LBe</xsl:text> -->
  </xsl:template>


  <xsl:template name="cell-counter">
  <!--
    <xsl:for-each select="mml:mtr|mml:mlabeledtr">
      <xsl:sort select="count(*)"/>
	  <ncols>
        <xsl:value-of select="count(*)"/>
	  </ncols>
    </xsl:for-each>
-->

    <xsl:variable name="column-counts.tr">
      <xsl:for-each select="*">
      <n-columns>
        <xsl:choose>
          <xsl:when test="self::mml:mtr or self::mml:mlabeledtr">
            <xsl:value-of select="count(*)"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="1"/>
          </xsl:otherwise>
        </xsl:choose>
      </n-columns>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="column-counts" select="exsl:node-set($column-counts.tr)"/>

    <xsl:for-each select="$column-counts/*">
      <xsl:sort select="."/>
      <ncols>
        <xsl:value-of select="."/>
   	  </ncols>
    </xsl:for-each>

  </xsl:template>


  <xsl:template name="tabular">
    <xsl:param name="columns-to-do"/>
    <xsl:param name="columnlines" select="normalize-space(@columnlines)"/>
    <xsl:param name="columnalign" select="normalize-space(@columnalign)"/>
    <xsl:param name="rowlines"    select="normalize-space(@rowlines)"/>
  
    <xsl:variable name="column-counts.tr">
      <xsl:call-template name="cell-counter"/>
    </xsl:variable>
    <xsl:variable name="column-counts" select="exsl:node-set($column-counts.tr)"/>

<!-- xsl:value-of select="$column-counts/ncols[position()=last()]"/ -->


<!--
<mml:mtable 
align="top" 
frame="solid" 
rowlines="solid none solid" 
columnlines="none solid solid solid" 
columnalign="center right left center center">

\begin{tabular}[t]{|cr|l|c||c|}
\hline
$\alpha $ & $\beta $ & $\gamma $ & $\delta $ & $\epsilon $ \\ \hline
$\varepsilon $ & $\zeta $ & $\eta $ & $\theta $ & $\vartheta $ \\ 
$\iota $ & $\kappa $ & $\lambda $ & $\mu $ & $\nu $ \\ \hline
$\xi $ & $\pi $ & $\varpi $ & $\rho $ & $\sigma $%
\end{tabular}%
-->
    <xsl:text xml:space="preserve">\begin{tabular}</xsl:text>
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

    <xsl:text xml:space="preserve">{</xsl:text>
    <xsl:if test="@frame='solid'">
      <xsl:text xml:space="preserve">|</xsl:text>
    </xsl:if>

    <xsl:call-template name="do-cols">
      <xsl:with-param name="columns-to-do" select="$column-counts/ncols[position()=last()]"/>
      <xsl:with-param name="columnlines" select="$columnlines"/>
      <xsl:with-param name="columnalign" select="$columnalign"/>
    </xsl:call-template>


    <xsl:if test="@frame='solid'">
      <xsl:text xml:space="preserve">|</xsl:text>
    </xsl:if>
    <xsl:text xml:space="preserve">}</xsl:text>


    <!-- JCS <xsl:text xml:space="preserve">\LBe</xsl:text> -->

    <xsl:if test="@frame='solid'">
      <xsl:text xml:space="preserve">\hline <!-- JCS \LBe --></xsl:text>
    </xsl:if>

<!-- Loop thru first level children - rows -->

    <xsl:for-each select="*">
      <xsl:choose>
        <xsl:when test="self::mml:mtr or self::mml:mlabeledtr">
          <xsl:for-each select="*">
            <xsl:choose>
              <xsl:when test="self::mml:mtd">

<!-- xsl:apply-templates mode="in-text"/ -->

                <xsl:variable name="LaTeX-contents.tr">
				  <raw-LaTeX>
                    <xsl:apply-templates mode="in-text"/>
				  </raw-LaTeX>
                </xsl:variable>
                <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>\multicolumn{</xsl:text>
                  <xsl:value-of select="@columnspan"/>
                  <xsl:text>}{</xsl:text>
                  <xsl:text>c</xsl:text>
                  <xsl:text>}{</xsl:text>
				</xsl:if>

                <xsl:call-template name="remove-dollar-dollar">
                  <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
                </xsl:call-template>

				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>}</xsl:text>
				</xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:variable name="LaTeX-contents.tr">
				  <raw-LaTeX>
                    <xsl:apply-templates mode="in-text"/>
				  </raw-LaTeX>
                </xsl:variable>
                <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>
                <xsl:call-template name="remove-dollar-dollar">
                  <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="position() != last()">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
            </xsl:if>
          </xsl:for-each>
        </xsl:when>
        <xsl:when test="self::mml:mtd">

<!-- xsl:apply-templates mode="in-text"/ -->
          <xsl:variable name="LaTeX-contents.tr">
		    <raw-LaTeX>
              <xsl:apply-templates mode="in-text"/>
		    </raw-LaTeX>
          </xsl:variable>
          <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

		  <xsl:if test="@columnspan&gt;1">
            <xsl:text>\multicolumn{</xsl:text>
            <xsl:value-of select="@columnspan"/>
            <xsl:text>}{</xsl:text>
            <xsl:text>c</xsl:text>
            <xsl:text>}{</xsl:text>
		  </xsl:if>

          <xsl:call-template name="remove-dollar-dollar">
            <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
          </xsl:call-template>

		  <xsl:if test="@columnspan&gt;1">
          <xsl:text>}</xsl:text>
		  </xsl:if>
        </xsl:when>
        <xsl:otherwise>
          <xsl:variable name="LaTeX-contents.tr">
		    <raw-LaTeX>
              <xsl:apply-templates mode="in-text"/>
		    </raw-LaTeX>
          </xsl:variable>
          <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>
          <xsl:call-template name="remove-dollar-dollar">
            <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>


<!-- handle \tag{}, \nonumber, etc. here -->

      <xsl:if test="name()='mml:mlabeledtr'">
        <xsl:variable name="LaTeX-contents.tr">
		  <raw-LaTeX>
            <xsl:apply-templates mode="in-text" select="*[position()=1]"/>
		  </raw-LaTeX>
        </xsl:variable>
        <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

	    <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve"> \TCItag{</xsl:text>
	    </xsl:if>
	    <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text xml:space="preserve"> \tag{</xsl:text>
	    </xsl:if>
        <xsl:call-template name="remove-dollar-dollar">
          <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
        </xsl:call-template>
        <xsl:text xml:space="preserve">}</xsl:text>
      </xsl:if>

      <xsl:call-template name="end-table-row">
        <xsl:with-param name="current-row" select="position()"/>
        <xsl:with-param name="last-row"    select="last()"/>
        <xsl:with-param name="rowlines"    select="$rowlines"/>
      </xsl:call-template>

    </xsl:for-each>


    <xsl:if test="@frame='solid'">
      <xsl:text xml:space="preserve">\\ \hline <!-- JCS \LBe --></xsl:text>
    </xsl:if>

     <xsl:text xml:space="preserve">\end{tabular}</xsl:text>
   </xsl:template>

</xsl:stylesheet>
