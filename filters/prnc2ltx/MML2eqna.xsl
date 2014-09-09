<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


<!-- mtable can carry a rowing attribute as follows:

  <mml:mtable rowspacing="2.000000ex 3.000000ex 1.500000ex">

  The following recursive template extracts each value
  and puts it in an xsl:variable under a <rs> node.
-->

  <xsl:template name="extract-row-spaces">
    <xsl:param name="rowspacing-list"/>
  
    <xsl:choose>
      <xsl:when test="string-length($rowspacing-list)=0">
        <!-- Nothing to do here. -->
      </xsl:when>
      <xsl:when test="contains($rowspacing-list,' ')">
        <rs>
        <xsl:value-of select="substring-before($rowspacing-list,' ')"/>
        </rs>
        <xsl:call-template name="extract-row-spaces">
          <xsl:with-param name="rowspacing-list" select="normalize-space(substring-after($rowspacing-list,' '))"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <rs>
        <xsl:value-of select="$rowspacing-list"/>
        </rs>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

<!-- We need a general routine to convert MathML vspace into LaTeX.
  For now, I'm just looking for the value I script in LaTeX2MML
  when a LaTeX \bigskip, \medskip, or \smallskip is translated.

  Given a MathML rowspacing attrib value, the following template
  script a LaTeX \skip or \vspace{}
-->

  <xsl:template name="set-eqn-row-space">
    <xsl:param name="current-space"/>
      <xsl:choose>
      <xsl:when test="string-length($current-space)=0">
      </xsl:when>
      <xsl:when test="$current-space='2.000000ex'">
        <xsl:text xml:space="preserve">\medskip </xsl:text>
      </xsl:when>
      <xsl:when test="$current-space='3.000000ex'">
        <xsl:text xml:space="preserve">\bigskip </xsl:text>
      </xsl:when>
      <xsl:when test="$current-space='1.500000ex'">
        <xsl:text xml:space="preserve">\smallskip </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve"> Unexpected eqn line spacer </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- template to script the end of a table row -->

  <xsl:template name="end-eqn-row">
    <xsl:param name="current-row"/>
    <xsl:param name="last-row"/>
  
    <xsl:if test="$current-row &lt; $last-row">
      <xsl:text> \\</xsl:text>
      <xsl:value-of select="$newline"/>
    </xsl:if>
  </xsl:template>


<!-- Utility to check contents of first child of mlabeledtr.
 "(digit..)" is considered to correspond to a LaTeX equation number. -->

  <xsl:template name="check-tag-str">
    <xsl:param name="tag-str"/>
      <xsl:choose>
      <xsl:when test="
        string-length($tag-str)&gt;2
        and substring($tag-str,1,1)='('
        and substring($tag-str,2,1)&gt;='0'
        and substring($tag-str,2,1)&lt;='9'
        and substring($tag-str,string-length($tag-str),1)=')'
      ">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>false</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

<!-- rwa Find a binary relation to align on if maligngroup not given in an \align or \eqnarray; return the generate-id() value -->

  <xsl:variable name="binrelsstr">
    <xsl:text>=&lt;&gt;&#x2264;&#x227A;&#x2AAF;&#x226A;&#x2282;&#x2286;&#x228F;&#x2291;&#x2208;&#x22A2;&#x2323;&#x2322;</xsl:text>
    <xsl:text>&#x2261;&#x223C;&#x2243;&#x224D;&#x2248;&#x2265;&#x227B;&#x2AB0;&#x226B;&#x2283;&#x2287;&#x2290;&#x2292;</xsl:text>
    <xsl:text>&#x220A;&#x220B;&#x22A3;&#x2223;&#x2225;&#x22A5;&#x2245;&#x22C8;&#x221D;&#x22A7;&#x2250;&#x2A1D;&#x22A9;&#x22AA;</xsl:text>
    <xsl:text>&#x22A8;&#x2257;&#x227F;&#x2273;&#x2A86;&#x2234;&#x2235;&#x2251;&#x225C;&#x227E;&#x2272;&#x2A85;&#x2A95;</xsl:text>
    <xsl:text>&#x2A96;&#x22DE;&#x22DF;&#x227C;&#x2266;&#x2A7D;&#x2276;&#x2253;&#x2252;&#x227D;&#x2267;&#x2A7E;&#x2277;</xsl:text>
    <xsl:text>&#x22B3;&#x22B2;&#x22B5;&#x22B4;&#x226C;&#x25B8;&#x25C2;&#x2256;&#x22DA;&#x22DB;&#x2A8B;&#x2A8C;&#x221D;</xsl:text>
    <xsl:text>&#x22D0;&#x22D1;&#x2AC5;&#x2AC6;&#x224F;&#x224E;&#x22D8;&#x22D9;&#x22D4;&#x223D;&#x22CD;&#x2242;&#x22D6;</xsl:text>
    <xsl:text>&#x22D7;&#x223C;&#x2248;&#x224A;&#x2AB8;&#x2AB7;&#x25B5;&#x2260;&#x2209;&#x2268;&#x2269;&#x2270;&#x2271;</xsl:text>
    <xsl:text>&#x226E;&#x226F;&#x2280;&#x2281;&#x2268;&#x2269;&#x22E7;&#x0338;&#x2A7D;&#x0338;&#x2A7E;&#x2A87;&#x2A88;</xsl:text>
    <xsl:text>&#x0338;&#x2AAF;&#x0338;&#x2AB0;&#x22E8;&#x22E9;&#x22E6;&#x22E7;&#x0338;&#x2266;&#x0338;&#x2267;&#x2AB5;</xsl:text>
    <xsl:text>&#x2AB6;&#x2AB9;&#x2ABA;&#x2A89;&#x2A8A;&#x2241;&#x2247;&#x228A;&#x228B;&#x0338;&#x2AC6;&#x0338;&#x2AC5;</xsl:text>
    <xsl:text>&#x2ACB;&#x2ACC;&#x02ACB;&#x02ACC;&#x0228A;&#x0228B;&#x2288;&#x2289;&#x2226;&#x2224;&#x22AC;&#x22AE;&#x22AD;</xsl:text>
    <xsl:text>&#x22AF;&#x22ED;&#x22EC;&#x22EB;&#x22EA;&#x219A;&#x219B;&#x21CD;&#x21CF;&#x21CE;&#x21AE;&#x2190;&#x21D0;</xsl:text>
    <xsl:text>&#x2192;&#x21D2;&#x2194;&#x21D4;&#x21A6;&#x21A9;&#x21BC;&#x21BD;&#x2191;&#x21D1;&#x2193;&#x21D3;&#x2195;</xsl:text>
    <xsl:text>&#x27F5;&#x27F8;&#x27F6;&#x27F9;&#x27F7;&#x27FA;&#x27FC;&#x21AA;&#x21C0;&#x21C1;&#x2933;&#x21D5;&#x2197;</xsl:text>
    <xsl:text>&#x2198;&#x2199;&#x2196;&#x21BB;&#x21BA;&#x21CC;&#x21CB;&#x21A0;&#x219E;&#x21C7;&#x21C9;&#x21C8;&#x21CA;</xsl:text>
    <xsl:text>&#x21BE;&#x21C2;&#x21BF;&#x21C3;&#x21A3;&#x21A2;&#x21C6;&#x21C4;&#x21B0;&#x21B1;&#x219D;&#x21AD;&#x21AB;</xsl:text>
    <xsl:text>&#x21AC;&#x22B8;&#x23DF;&#x21DB;&#x21DA;&#x21B6;&#x21B7;&#x290F;&#x290E;&#x21D2;&#x21D0;&#x21D4;&#xFD37;&#xFD38;</xsl:text>
  </xsl:variable>

  <xsl:template name="getAlignmentPoint">
    <xsl:for-each select="(mml:mo[contains($binrelsstr, normalize-space(string()))])|(mml:mrow[not(@fence='true')]/mml:mo[contains($binrelsstr, normalize-space(string()))])">
      <!-- xsl:sort select="string-length(substring-before($binrelsstr,string(.)))" / -->
      <xsl:if test="position()=1">
        <xsl:value-of select="generate-id(.)"/>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

<!-- We translate to a LaTeX eqnarray (or one of it's variants)
  if an MML object consists solely of a displayed, multi-line mtable,
  with 1 mtd per row. Variations in the pattern of <maligngroup>'s
  determine which LaTeX environment is scripted.
-->

  <xsl:template name="eqnarray">
    <xsl:param name="n-rows"/>
    <xsl:param name="n-labeledrows"/>
    <xsl:param name="n-aligns"/>
    <xsl:param name="theAlignment"/>
  
    <xsl:variable name="eqn-info.tr">
      <xsl:choose>
<!-- multline or gather -->
        <xsl:when test="$n-aligns=0">

        <LaTeX-env>
          <xsl:choose>
            <xsl:when test="@subtype">
              <xsl:value-of select="@subtype"/>
            </xsl:when>
            <xsl:when test="@type!=''">
              <xsl:value-of select="@type"/>
            </xsl:when>
            <xsl:when test="$theAlignment='alignCentered'">
              <xsl:text>gather</xsl:text>
            </xsl:when>
            <xsl:when test="$theAlignment='alignSingleEqn'">
              <xsl:text>multline</xsl:text>
            </xsl:when>
            <xsl:when test="$n-labeledrows=1">
              <xsl:text>multline</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>gather</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </LaTeX-env>

        <is-starred>
          <xsl:choose>
            <xsl:when test="@subtype">
              <xsl:text>false</xsl:text>
            </xsl:when>
            <xsl:when test="$n-labeledrows=0">
              <xsl:text>true</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>false</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </is-starred>

        </xsl:when>
<!-- align -->
        <xsl:when test="$n-aligns=1">
        <LaTeX-env>
            <xsl:text>align</xsl:text>
        </LaTeX-env>
        <is-starred>
          <xsl:choose>
            <xsl:when test="$n-labeledrows=0">
              <xsl:text>true</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>false</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </is-starred>
        </xsl:when>
<!-- eqnarray -->
        <xsl:when test="$n-aligns=2">
        <LaTeX-env>
            <xsl:text>eqnarray</xsl:text>
        </LaTeX-env>
        <is-starred>
          <xsl:choose>
            <xsl:when test="$n-labeledrows=0">
              <xsl:text>true</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>false</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </is-starred>
        </xsl:when>
<!-- alignat -->
        <xsl:when test="$n-aligns=4">
        <LaTeX-env>
            <xsl:text>alignat</xsl:text>
        </LaTeX-env>
        <is-starred>
          <xsl:choose>
            <xsl:when test="$n-labeledrows=0">
              <xsl:text>true</xsl:text>
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>false</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
        </is-starred>
        </xsl:when>

        <xsl:otherwise>
          <xsl:text>mtable -> ?eqnarray?</xsl:text>
        </xsl:otherwise>
      </xsl:choose>

      <xsl:if test="string-length(@rowspacing)&gt;0">
        <xsl:call-template name="extract-row-spaces">
          <xsl:with-param name="rowspacing-list" select="normalize-space(@rowspacing)"/>
        </xsl:call-template>
      </xsl:if>

    </xsl:variable>
    <xsl:variable name="eqn-info" select="exsl:node-set($eqn-info.tr)"/>

<!-- begin scripting LaTeX output -->

    <xsl:text>\begin{</xsl:text>
    <xsl:value-of select="$eqn-info/LaTeX-env"/>
    <xsl:if test="$eqn-info/is-starred='true'">
      <xsl:text>*</xsl:text>
    </xsl:if>
    <xsl:text>}</xsl:text>
    <xsl:if test="$eqn-info/LaTeX-env='alignat'">
      <xsl:text>{2}</xsl:text>
    </xsl:if>
<!-- jcs    <xsl:text>\LBe</xsl:text> -->

  <!-- loop thru rows -->

    <xsl:for-each select="mml:mtr|mml:mlabeledtr">

  <!-- loop thru cells in each row - only 1 for eqnarrays. -->

      <xsl:for-each select="mml:mtd">
        <xsl:variable name="insertedAlignmark">
          <xsl:if test="($n-aligns=1) or ($n-aligns=2)">
            <xsl:if test="count((./mml:maligngroup)|(./mml:mrow[not(@fence='true')]/mml:maligngroup))=0">
              <xsl:call-template name="getAlignmentPoint" />
            </xsl:if>
          </xsl:if>
        </xsl:variable>
<!--
 I'm iterating the first level children of mml:mtd.
 LTeX2MML scripts mml:maligngroup at the beginning of each mtd.
 This mml:maligngroup doesn't correspond to any LaTeX construct.
-->
        <xsl:for-each select="*">
          <xsl:choose>
            <!-- xsl:when test="position()=1
            and             name()='mml:maligngroup'">
            </xsl:when -->
            <xsl:when test="name()='mml:maligngroup'">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
            </xsl:when>
            <xsl:when test="self::mml:mo and $insertedAlignmark and (generate-id(.)=$insertedAlignmark)">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
              <xsl:apply-templates select="."/>
              <xsl:if test="$n-aligns &gt; 1">
                <xsl:text xml:space="preserve"> &amp; </xsl:text>
              </xsl:if>
            </xsl:when>
            <xsl:when test="name()='mml:mrow'">

              <xsl:for-each select="*">
                <xsl:choose>
                  <xsl:when test="name()='mml:maligngroup'">
                    <xsl:text xml:space="preserve"> &amp; </xsl:text>
                  </xsl:when>
                  <xsl:when test="self::mml:mo and $insertedAlignmark and (generate-id(.)=$insertedAlignmark)">
                    <xsl:text xml:space="preserve"> &amp; </xsl:text>
                    <xsl:apply-templates select="."/>
                    <xsl:if test="$n-aligns &gt; 1">
                      <xsl:text xml:space="preserve"> &amp; </xsl:text>
                    </xsl:if>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:apply-templates select="."/>
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:for-each>

            </xsl:when>
            <xsl:otherwise>
              <xsl:apply-templates select="."/>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:for-each>

      </xsl:for-each>

      <xsl:choose>
        <xsl:when test="./mml:mtd/*[1][self::mml:mrow][@id!='']">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="child::mml:mrow/@id"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
        <xsl:when test="@key and string-length(normalize-space(@key))">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="@key"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
        <xsl:when test="@marker and string-length(normalize-space(@marker))">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="@marker"/>
          <xsl:text>}</xsl:text>
        </xsl:when>
      </xsl:choose>

  <!-- handle \tag{} -->

      <xsl:choose>
        <xsl:when test="name()='mml:mlabeledtr'">

<!-- NOTE: we need to set the class on an mlabeledtr to decide
     whether it carries a user defined tag or an auto eqn number. -->

  <!-- the tag in the first child - it may be a user defined tag
       or just the number that LaTeX's theequation counter generates
       automatically.  Translate the tag into an xsl:variable and
       decide what to do. -->

          <xsl:variable name="tag-info.tr">
          <tag-text>
            <xsl:apply-templates select="./*[1]" mode="in-text"/>
          </tag-text>
          </xsl:variable>
          <xsl:variable name="tag-info" select="exsl:node-set($tag-info.tr)"/>

          <xsl:variable name="tag-is-digits.tr">
          <is-eqn-number>
            <xsl:call-template name="check-tag-str">
              <xsl:with-param name="tag-str" select="normalize-space($tag-info/tag-text)"/>
            </xsl:call-template>
          </is-eqn-number>
          </xsl:variable>
          <xsl:variable name="tag-is-digits" select="exsl:node-set($tag-is-digits.tr)"/>

          <xsl:if test="$tag-is-digits/is-eqn-number='false'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text xml:space="preserve"> \TCItag{</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text xml:space="preserve"> \tag{</xsl:text>
            </xsl:if>

            <xsl:choose>
              <xsl:when test="substring($tag-info/tag-text,1,1)='('">
                <xsl:value-of select="substring($tag-info/tag-text,2,string-length($tag-info/tag-text)-2)"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$tag-info/tag-text"/>
              </xsl:otherwise>
            </xsl:choose>
            <xsl:text>}</xsl:text>
          </xsl:if>

        </xsl:when>

        <xsl:when test="self::mml:mtr and ancestor::html:msidisplay
                   and           $eqn-info/is-starred='false'
                   and           $eqn-info/LaTeX-env!='multline'">
          <xsl:choose>
            <xsl:when test="@customLabel">
              <xsl:text xml:space="preserve"> \tag{</xsl:text>
              <xsl:value-of select="@customLabel"/>
              <xsl:text>}</xsl:text>
            </xsl:when>
            <xsl:when test="@numbering='none'">
              <xsl:text xml:space="preserve"> \nonumber </xsl:text>
            </xsl:when>
          </xsl:choose>
        </xsl:when>

        <xsl:when test="self::mml:mtr and ancestor::html:msidisplay
                   and     $eqn-info/LaTeX-env='multiline'
                   and     $eqn-info/is-starred='false'
                   and     @customLabel
                   and     not(preceding-sibling::*[@customLabel])">
         <xsl:text xml:space="preserve"> \tag{</xsl:text>
         <xsl:value-of select="@customLabel"/>
         <xsl:text>}</xsl:text>
       </xsl:when>
  <!-- handle \nonumber -->

        <xsl:when test="name()='mml:mtr'
        and           $eqn-info/is-starred='false'
        and           $eqn-info/LaTeX-env!='multline'">
          <xsl:text xml:space="preserve"> \nonumber </xsl:text>
        </xsl:when>

      </xsl:choose>

  <!-- handle vspace for current row -->

      <xsl:variable name="curr-pos" select="position()"/>
      <xsl:call-template name="set-eqn-row-space">
        <xsl:with-param name="current-space" select="$eqn-info/rs[$curr-pos]"/>
      </xsl:call-template>


      <xsl:call-template name="end-eqn-row">
        <xsl:with-param name="current-row" select="position()"/>
        <xsl:with-param name="last-row"    select="last()"/>
      </xsl:call-template>

    </xsl:for-each>

<!-- end environment -->

    <xsl:text>\end{</xsl:text>
    <xsl:value-of select="$eqn-info/LaTeX-env"/>
    <xsl:if test="$eqn-info/is-starred='true'">
      <xsl:text>*</xsl:text>
    </xsl:if>
    <xsl:text>}</xsl:text>
  </xsl:template>

</xsl:stylesheet>
