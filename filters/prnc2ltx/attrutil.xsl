<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


<!-- Given a unit extracted from the valye of some MahML attribute,
  determine whether or not the unit is OK in LaTeX. -->

  <xsl:template name="is-TeX-unit">
    <xsl:param name="unit-nom"/>
    <xsl:choose>
      <xsl:when test="$unit-nom = 'cm'
      or              $unit-nom = 'em'
      or              $unit-nom = 'ex'
      or              $unit-nom = 'in'
      or              $unit-nom = 'pc'
      or              $unit-nom = 'pt'
      or              $unit-nom = 'mm'">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>false</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- Utility to extract the numeric part of an attribute value like
  "1.0pt", "1", "50%", etc.  -->


  <xsl:template name="get-number-chars">
    <xsl:param name="attrib-cdata"/>
    <xsl:variable name="first-char" select="substring($attrib-cdata,1,1)"/>
    <xsl:if test="$first-char = '0'
    or            $first-char = '1'
    or            $first-char = '2'
    or            $first-char = '3'
    or            $first-char = '4'
    or            $first-char = '5'
    or            $first-char = '6'
    or            $first-char = '7'
    or            $first-char = '8'
    or            $first-char = '9'
    or            $first-char = '+'
    or            $first-char = ','
    or            $first-char = '-'
    or            $first-char = '.'">
      <xsl:value-of select="$first-char"/>
      <xsl:if test="string-length($attrib-cdata)&gt;1">
        <xsl:call-template name="get-number-chars">
          <xsl:with-param name="attrib-cdata" select="substring($attrib-cdata,2)"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:if>
  </xsl:template>


  <xsl:template name="add-dimens">
    <xsl:param name="value1"/>
    <xsl:param name="unit1"/>
    <xsl:param name="value2"/>
    <xsl:param name="unit2"/>
<!-- For now, I'm assuming that unit1 and unit2 are the same. -->
    <xsl:choose>
      <xsl:when test="$unit1=$unit2">
    	<xsl:value-of select="substring($value1+$value2,1,5)"/>
	    <xsl:value-of select="$unit1"/>
      </xsl:when>
      <xsl:otherwise>
	    <xsl:text>Add idmens, mixed units!!</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- The following template serves as an "Operator Dictionary".
  Given an operator name (ie. contents of an mo) and an attribute
  name, it returns the default value of the attribute.
-->

  <xsl:template name="get-mo-attr-val">
    <xsl:param name="mo-nom"/>
    <xsl:param name="attr-nom"/>
    <xsl:choose>
      <xsl:when test="$mo-nom='('
      or              $mo-nom='['
      or              $mo-nom='{'
      or              $mo-nom='&#x2329;'
      or              $mo-nom='&#x3008;'
      or              $mo-nom='&#x230A;'
      or              $mo-nom='&#x2308;'">
        <xsl:choose>
          <xsl:when test="$attr-nom='form'">
		    <xsl:text>prefix</xsl:text>
          </xsl:when>
          <xsl:when test="$attr-nom='stretchy'">
		    <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="$mo-nom=')'
      or              $mo-nom=']'
      or              $mo-nom='}'
      or              $mo-nom='&#x232A;'
      or              $mo-nom='&#x3009;'
      or              $mo-nom='&#x230B;'
      or              $mo-nom='&#x2309;'">
        <xsl:choose>
          <xsl:when test="$attr-nom='form'">
		    <xsl:text>postfix</xsl:text>
          </xsl:when>
          <xsl:when test="$attr-nom='stretchy'">
		    <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="$mo-nom='|'
      or              $mo-nom='&#x2016;'
      or              $mo-nom='/'
      or              $mo-nom='\'
      or              $mo-nom='&#x2195;'
      or              $mo-nom=''
      or              $mo-nom='&#x21D5;'
      or              $mo-nom='&#x2191;'
      or              $mo-nom='&#x21D1;'
      or              $mo-nom='&#x2193;'
      or              $mo-nom='&#x21D3;'">
        <xsl:choose>
          <xsl:when test="$attr-nom='form'">
		    <xsl:text>ambiguous</xsl:text>
          </xsl:when>
          <xsl:when test="$attr-nom='stretchy'">
		    <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

<!-- sum-like bigops -->

      <xsl:when test="$mo-nom='&#x2211;'
      or              $mo-nom='&#x220F;'
      or              $mo-nom='&#x22C2;'
      or              $mo-nom='&#x22C0;'
      or              $mo-nom='&#x2295;'
      or              $mo-nom='&#x2299;'
      or              $mo-nom='&#x2294;'
      or              $mo-nom='&#x2210;'
      or              $mo-nom='&#x22C3;'
      or              $mo-nom='&#x22C1;'
      or              $mo-nom='&#x2297;'
      or              $mo-nom='&#x228E;'
      ">
        <xsl:choose>
          <xsl:when test="$attr-nom='movablelimits'">
		    <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

<!-- int-like bigops -->

      <xsl:when test="normalize-space(string($mo-nom))='&#x222B;'
      or              normalize-space(string($mo-nom))='&#x222C;'
      or              normalize-space(string($mo-nom))='&#x222D;'
      or              normalize-space(string($mo-nom))='&#xE378;'
      or              normalize-space(string($mo-nom))='&#x222B;&#x22EF;&#x222B;'
      or              normalize-space(string($mo-nom))='&#x222E;' ">
        <xsl:choose>
          <xsl:when test="$attr-nom='movablelimits'">
		    <xsl:text>false</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>


	  <xsl:otherwise>
	  </xsl:otherwise>
    </xsl:choose>

  </xsl:template>



<!-- Utility to extract the last value from a , separated list
     of inherited attribute values. -->

  <xsl:template name="get-last-list-entry">
    <xsl:param name="list"/>
    <xsl:choose>
      <xsl:when test="contains($list,',')">
        <xsl:call-template name="get-last-list-entry">
          <xsl:with-param name="list" select="normalize-space(substring-after($list,','))"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
	    <xsl:value-of select="$list"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- Utility to retrieve the value of an attribute inherited from mstyle

3.2.1 Attributes common to token elements
Several attributes related to text formatting are provided on all token elements
 except <mspace/>, but on no other elements except <mstyle>. These are: 

 Name        values                            default  
 fontsize    number v-unit                     inherited 
 fontweight  normal | bold                     inherited 
 fontstyle   normal | italic                   normal (except on <mi>) 
 fontfamily  string | css-fontfamily           inherited 
 color       #rgb | #rrggbb | html-color-name  inherited 
-->

  <xsl:template name="get-inherited-attr-value">
    <xsl:param name="attr-nom"/>
    <xsl:variable name="value-list">
      <xsl:choose>
	    <xsl:when test="$attr-nom='fontsize'">
          <xsl:for-each select="ancestor::mml:mstyle[@fontsize]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@fontsize"/>
          </xsl:for-each>
	    </xsl:when>
	    <xsl:when test="$attr-nom='fontweight'">
          <xsl:for-each select="ancestor::mml:mstyle[@fontweight]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@fontweight"/>
          </xsl:for-each>
	    </xsl:when>
	    <xsl:when test="$attr-nom='fontstyle'">
          <xsl:for-each select="ancestor::mml:mstyle[@fontstyle]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@fontstyle"/>
          </xsl:for-each>
	    </xsl:when>
	    <xsl:when test="$attr-nom='fontfamily'">
          <xsl:for-each select="ancestor::mml:mstyle[@fontfamily]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@fontfamily"/>
          </xsl:for-each>
	    </xsl:when>
	    <xsl:when test="$attr-nom='color'">
          <xsl:for-each select="ancestor::mml:mstyle[@color]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@color"/>
          </xsl:for-each>
	    </xsl:when>

	    <xsl:when test="$attr-nom='linethickness'">
          <xsl:for-each select="ancestor::mml:mstyle[@linethickness]">
            <xsl:text>,</xsl:text>
            <xsl:value-of select="@linethickness"/>
          </xsl:for-each>
	    </xsl:when>

<!-- clauses for everything else that inherits -->

	    <xsl:otherwise>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length($value-list)&gt;0">
      <xsl:call-template name="get-last-list-entry">
        <xsl:with-param name="list" select="normalize-space($value-list)"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>


</xsl:stylesheet>

