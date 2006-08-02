<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:msxsl="urn:schemas-microsoft-com:xslt"
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
      <xsl:text xml:space="preserve"> \\</xsl:text>
    </xsl:if>
    <!--jcs <xsl:text xml:space="preserve">\LBe</xsl:text> -->
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


<!-- We translate to a LaTeX eqnarray (or one of it's variants)
  if an MML object consists solely of a displayed, multi-line mtable,
  with 1 mtd per row. Variations in the pattern of <maligngroup>'s
  determine which LaTeX environment is scripted.
-->

  <xsl:template name="eqnarray">
    <xsl:param name="n-rows"/>
    <xsl:param name="n-labeledrows"/>
    <xsl:param name="n-aligns"/>

    <xsl:variable name="eqn-info.tr">
      <xsl:choose>
<!-- multline or gather -->
        <xsl:when test="$n-aligns=0">

        <LaTeX-env>
          <xsl:choose>
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
    <xsl:variable name="eqn-info" select="$eqn-info.tr"/>

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
<!--
 I'm iterating the first level children of mml:mtd.
 LTeX2MML scripts mml:maligngroup at the beginning of each mtd.
 This mml:maligngroup doesn't correspond to any LaTeX construct.
-->
        <xsl:for-each select="*">
		  <xsl:choose>
		    <xsl:when test="position()=1
		    and             name()='mml:maligngroup'">
			</xsl:when>
		    <xsl:when test="name()='mml:maligngroup'">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
			</xsl:when>
		    <xsl:when test="name()='mml:mrow'">

              <xsl:for-each select="*">
		        <xsl:choose>
      		      <xsl:when test="name()='mml:maligngroup'">
                    <xsl:text xml:space="preserve"> &amp; </xsl:text>
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

		<xsl:if test="./*[1][self::mml:mrow][@id!='']">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="child::mml:mrow/@id"/>
          <xsl:text>}</xsl:text>
		</xsl:if>
      </xsl:for-each>

  <!-- handle \tag{} -->

      <xsl:if test="name()='mml:mlabeledtr'">

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
        <xsl:variable name="tag-info" select="$tag-info.tr"/>

        <xsl:variable name="tag-is-digits.tr">
		<is-eqn-number>
          <xsl:call-template name="check-tag-str">
            <xsl:with-param name="tag-str" select="normalize-space($tag-info/tag-text)"/>
          </xsl:call-template>
		</is-eqn-number>
        </xsl:variable>
        <xsl:variable name="tag-is-digits" select="$tag-is-digits.tr"/>

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

      </xsl:if>

  <!-- handle \nonumber -->

      <xsl:if test="name()='mml:mtr'
      and           $eqn-info/is-starred='false'
      and           $eqn-info/LaTeX-env!='multline'">
        <xsl:text xml:space="preserve"> \nonumber </xsl:text>
      </xsl:if>

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
