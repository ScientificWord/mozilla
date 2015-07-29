<?xml version="1.0"?>
<xsl:stylesheet
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


<!-- The following template is called when we encounter
  an <mo> that may be translated to a LaTeX fence. -->


  <xsl:template name="translate-fencing-mo">
    <xsl:param name="LaTeX-fence-token"/>
    <xsl:variable name="right-mo1-is-fence">
      <xsl:choose>
        <xsl:when test="following-sibling::mml:mo[1]">
          <xsl:call-template name="mo-is-LaTeX-fence">
	        <xsl:with-param name="op-nom" select="normalize-space(following-sibling::mml:mo[1])"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>
      1. LaTeX-fence-token is </xsl:text><xsl:value-of select="$LaTeX-fence-token"/> -->

    <xsl:variable name="right-stretchy">
      <xsl:for-each select="following-sibling::mml:mo[1]">
        <xsl:value-of select="@stretchy"/>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="right-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length($right-stretchy)&gt;0">
          <xsl:value-of select="$right-stretchy"/>
        </xsl:when>
        <xsl:when test="following-sibling::mml:mo[1]">
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(following-sibling::mml:mo[1])"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>
        </xsl:when>
		<xsl:otherwise>
		  <xsl:text>unknown</xsl:text>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="right-form">
      <xsl:for-each select="following-sibling::mml:mo[1]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="right-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($right-form) &gt; 0">
          <xsl:value-of select="$right-form"/>
        </xsl:when>
        <xsl:when test="string-length(normalize-space(following-sibling::mml:mo[1]))=0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(following-sibling::mml:mo[1])"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>



    <xsl:variable name="left-mo1-is-fence">
      <xsl:choose>
        <xsl:when test="preceding-sibling::mml:mo[1]">
          <xsl:call-template name="mo-is-LaTeX-fence">
	        <xsl:with-param name="op-nom" select="normalize-space(preceding-sibling::mml:mo[1])"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>2. left-mo1-is-fence is </xsl:text><xsl:value-of select="$left-mo1-is-fence"/>
 -->

    <xsl:variable name="left-stretchy">
      <xsl:for-each select="preceding-sibling::mml:mo[1]">
        <xsl:value-of select="@stretchy"/>
      </xsl:for-each>
    </xsl:variable>

<!--     <xsl:text>3. left-stretchy is </xsl:text><xsl:value-of select="$left-stretchy"/>
 -->
    <xsl:variable name="left-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length($left-stretchy)&gt;0">
          <xsl:value-of select="$left-stretchy"/>
        </xsl:when>
<!--         <xsl:when test="preceding-sibling::mml:mo[1]">
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(preceding-sibling::mml:mo[1])"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>

        </xsl:when> -->
		<xsl:otherwise>
          <xsl:text>unknown</xsl:text>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>4. left-mo-stretchy is </xsl:text><xsl:value-of select="$left-mo-stretchy"/>
 -->


    <xsl:variable name="left-form">
      <xsl:for-each select="preceding-sibling::mml:mo[1]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
    </xsl:variable>

<!--     <xsl:text>5. left-form is </xsl:text><xsl:value-of select="$left-form"/>
 -->
    <xsl:variable name="left-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($left-form) &gt; 0">
          <xsl:value-of select="$left-form"/>
        </xsl:when>
        <xsl:when test="string-length(normalize-space(preceding-sibling::mml:mo[1]))=0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(preceding-sibling::mml:mo[1])"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>6. left-mo-form is </xsl:text><xsl:value-of select="$left-mo-form"/>
 -->
    <xsl:variable name="curr-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length(@stretchy) &gt; 0">
		  <xsl:value-of select="@stretchy"/>
        </xsl:when>
		<xsl:otherwise>
      <xsl:text>false</xsl:text>

<!--           <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(string())"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template> -->
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>7. curr-mo-stretchy is </xsl:text><xsl:value-of select="$curr-mo-stretchy"/>
 -->
    <xsl:variable name="curr-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length(@form) &gt; 0">
		  <xsl:value-of select="@form"/>
        </xsl:when>
		<xsl:otherwise>
<!--           <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="normalize-space(string())"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template> -->
        <xsl:text>false</xsl:text>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!--     <xsl:text>8. curr-mo-form is </xsl:text><xsl:value-of select="$curr-mo-form"/>
 -->

<!-- Diagnostics
<xsl:text>AAA-</xsl:text>
<xsl:value-of select="$curr-mo-stretchy"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$curr-mo-form"/>
<xsl:text>-BBB-</xsl:text>
<xsl:value-of select="$right-mo1-is-fence"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$right-mo-stretchy"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$right-mo-form"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$right-form"/>
<xsl:text>-CCC</xsl:text>
-->


    <xsl:choose>

<!-- Here we look for a following sibling that could represent
 the closing delimiter of our possible fence. -->

      <xsl:when test="$curr-mo-stretchy!='false'
      and             $curr-mo-form='prefix'
      and             $right-mo1-is-fence!='false'
      and             $right-mo-stretchy!='false'
      and            ($right-mo-form='postfix'
      or              $right-mo-form='ambiguous') ">

<!-- If we get here, the current mo is a stretchy fence opener.
  set up variable re fence body.
  The current <mo> may be absorbed by the object to its right -->

        <xsl:variable name="fenced-content.tr">
		  <is-left-absorbed>
		    <xsl:choose>
			  <xsl:when test="
                    following-sibling::*[1][self::mml:mfrac]
                and following-sibling::*[2][self::mml:mo]
			  ">
                <xsl:text>true</xsl:text>
			  </xsl:when>
			  <xsl:when test="
                    following-sibling::*[1][self::mml:mrow]
                and following-sibling::*[1][self::mml:mrow]/*[1][self::mml:mstyle]
                and following-sibling::*[1][self::mml:mrow]/*[1][self::mml:mstyle]/*[1][self::mml:mfrac]
                and following-sibling::*[2][self::mml:mo]
			  ">
                <xsl:text>true</xsl:text>
			  </xsl:when>
			  <xsl:otherwise>
                <xsl:text>false</xsl:text>
			  </xsl:otherwise>
		    </xsl:choose>
          </is-left-absorbed>
        </xsl:variable>
        <xsl:variable name="fenced-content" select="exsl:node-set($fenced-content.tr)"/>

        <xsl:choose>
          <xsl:when test="$fenced-content/is-left-absorbed='true'">
<!-- We don't script anything here - this <mo> will be handled later
 when when the object on its right is scripted. -->
          </xsl:when>
          <xsl:otherwise>
            <xsl:text xml:space="preserve">\left </xsl:text>
            <xsl:value-of select="$LaTeX-fence-token"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>


      <xsl:when test="$curr-mo-stretchy!='false'
      and             $curr-mo-form='postfix'
      and             $left-mo1-is-fence!='false'
      and             $left-mo-stretchy!='false'
      and            ($left-mo-form='prefix'
      or              $left-mo-form='ambiguous') ">

<!-- set up variable re fence body - delimiter may be absorbed -->

        <xsl:variable name="fenced-content.tr">

		  <is-right-absorbed>
		    <xsl:choose>
			  <xsl:when test="
                    preceding-sibling::*[1][self::mml:mfrac]
                and preceding-sibling::*[2][self::mml:mo]
			  ">
                <xsl:text>true</xsl:text>
			  </xsl:when>
			  <xsl:when test="
                    preceding-sibling::*[1][self::mml:mrow]
                and preceding-sibling::*[1][self::mml:mrow]/*[1][self::mml:mstyle]
                and preceding-sibling::*[1][self::mml:mrow]/*[1][self::mml:mstyle]/*[1][self::mml:mfrac]
                and preceding-sibling::*[2][self::mml:mo]
			  ">
                <xsl:text>true</xsl:text>
			  </xsl:when>
			  <xsl:otherwise>
                <xsl:text>false</xsl:text>
			  </xsl:otherwise>
		    </xsl:choose>
          </is-right-absorbed>

        </xsl:variable>
        <xsl:variable name="fenced-content" select="exsl:node-set($fenced-content.tr)"/>

        <xsl:choose>
          <xsl:when test="$fenced-content/is-right-absorbed='true'">
          </xsl:when>
          <xsl:otherwise>
            <xsl:text xml:space="preserve">\right </xsl:text>
            <xsl:value-of select="$LaTeX-fence-token"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>


      <xsl:otherwise>
<!-- Here, the current mo is NOT part of a fence -->
        <xsl:if test="string-length(@lspace) &gt; 0">
          <xsl:call-template name="operator-lrspace-2LaTeX">
            <xsl:with-param name="value" select="substring(@lspace,1,string-length(@lspace)-2)"/>
            <xsl:with-param name="unit"  select="substring(@lspace,string-length(@lspace)-1,2)"/>
          </xsl:call-template>
		</xsl:if>

        <xsl:value-of select="$LaTeX-fence-token"/>

        <xsl:if test="string-length(@rspace) &gt; 0">
          <xsl:call-template name="operator-lrspace-2LaTeX">
            <xsl:with-param name="value" select="substring(@rspace,1,string-length(@rspace)-2)"/>
            <xsl:with-param name="unit"  select="substring(@rspace,string-length(@rspace)-1,2)"/>
          </xsl:call-template>
		</xsl:if>
      </xsl:otherwise>

    </xsl:choose>

  </xsl:template>


<!-- Utility to decide whether or not a unicode
      corresponds to a LaTeX "fence" symbol.  -->

  <xsl:template name="mo-is-LaTeX-fence">
    <xsl:param name="op-nom"/>
	<xsl:choose>
      <xsl:when test="
         $op-nom=')'
      or $op-nom='('
      or $op-nom='{'
      or $op-nom='}'
      or $op-nom='['
      or $op-nom=']'
      or $op-nom='&#x2329;'
      or $op-nom='&#x232A;'
      or $op-nom='&#x3008;'
      or $op-nom='&#x3009;'
      or $op-nom='&#x230A;'
      or $op-nom='&#x230B;'
      or $op-nom='&#x2308;'
      or $op-nom='&#x2309;'
      or $op-nom='|'
      or $op-nom='&#x2016;'
      or $op-nom='/'
      or $op-nom='\'
      or $op-nom='&#x2195;'
      or $op-nom=''
      or $op-nom='&#x21D5;'
      or $op-nom='&#x2191;'
      or $op-nom='&#x21D1;'
      or $op-nom='&#x2193;'
      or $op-nom='&#x21D3;'
      or $op-nom='&#x250A;'">
		<xsl:text>true</xsl:text>
      </xsl:when>
	  <xsl:otherwise>
		<xsl:text>false</xsl:text>
	  </xsl:otherwise>
	</xsl:choose>
  </xsl:template>


</xsl:stylesheet>
