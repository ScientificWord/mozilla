<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- mfenced is deprecated in MathML 2.0

mfenced is a schemata for scripting an enclosed list, like (x,y).
Each first level child is an item in the list.
The opening delimiter, closing delimiter, and a list of item
separators are specified in attributes.  Defaults are ( and ) and ,
-->


<!-- WARNING: translations of mfenced and mfrac are inter-related. -->

  <xsl:template match="mml:mfenced">
  
<!-- Variable to examine the content of mfenced.
  If the content is a single mml:mfrac and the delimiters can be built
  into a LaTeX fraction, the mfenced has no direct translation.
  It will be handled by the mml:mfrac template in it's contents.
-->

    <xsl:variable name="mfenced-content.tr">
	  <num-children>
        <xsl:value-of select="count(./*)"/>
      </num-children>

      <left-can-be-absorbed>
        <xsl:choose>
          <xsl:when test="@open">
            <xsl:call-template name="mo-is-LaTeX-fence">
    		  <xsl:with-param name="op-nom" select="@open"/>
            </xsl:call-template>
		  </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="'true'"/>
          </xsl:otherwise>
        </xsl:choose>
      </left-can-be-absorbed>

      <right-can-be-absorbed>
        <xsl:choose>
          <xsl:when test="@close">
            <xsl:call-template name="mo-is-LaTeX-fence">
    		  <xsl:with-param name="op-nom" select="@close"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="'true'"/>
          </xsl:otherwise>
        </xsl:choose>
      </right-can-be-absorbed>
    </xsl:variable>
    <xsl:variable name="mfenced-content" select="exsl:node-set($mfenced-content.tr)"/>


    <xsl:choose>
      <xsl:when test="child::mml:mfrac
      and             $mfenced-content/num-children=1
      and			  $mfenced-content/left-can-be-absorbed='true'
      and			  $mfenced-content/right-can-be-absorbed='true'">
<!-- Here the mfenced is absorbed by the mrac that it contains -->
	    <xsl:apply-templates/>
	  </xsl:when>

	  <xsl:otherwise>

        <xsl:variable name="opener">
          <xsl:choose>
            <xsl:when test="@open">
              <xsl:value-of select="@open"/>
            </xsl:when>
	        <xsl:otherwise>
              <xsl:text>(</xsl:text>
	        </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <xsl:variable name="separator">
          <xsl:choose>
            <xsl:when test="@separators">
              <xsl:value-of select="@separators"/>
            </xsl:when>
	        <xsl:otherwise>
              <xsl:text>,</xsl:text>
	        </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <xsl:variable name="closer">
          <xsl:choose>
            <xsl:when test="@close">
              <xsl:value-of select="@close"/>
            </xsl:when>
	        <xsl:otherwise>
              <xsl:text>)</xsl:text>
	        </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>


        <xsl:call-template name="do-fence-delim">
          <xsl:with-param name="is-left"   select="'true'"/>
          <xsl:with-param name="delimiting-attr-val" select="$opener"/>
        </xsl:call-template>

        <xsl:call-template name="do-fenced-items">
          <xsl:with-param name="item-tally"     select="count(./*)"/>
          <xsl:with-param name="item-number"    select="1"/>
          <xsl:with-param name="separator-list" select="$separator"/>
        </xsl:call-template>

        <xsl:call-template name="do-fence-delim">
          <xsl:with-param name="is-left"   select="'false'"/>
          <xsl:with-param name="delimiting-attr-val" select="$closer"/>
        </xsl:call-template>
	  </xsl:otherwise>
    </xsl:choose>


  </xsl:template>


  <xsl:template match="mml:mfenced" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>



<!-- template to translate an mfenced delimiter string, which could come
  from an attribute value - defaults are ( and ).  This string is equivalent
  to the contents of an <mo>.  I'm scripting a LaTeX fence as output here.
  Consequently, only tokens that can follow LaTeX's \left and \right
  can be generated.
    If the MathML delimiter doesn't correspond to one of LaTeX's fence
  symols, ( or ) is scripted.
-->

  <xsl:template name="do-delimiter">
    <xsl:param name="is-left"/>
    <xsl:param name="mml-delim-str"/>
  
	<xsl:choose>
      <xsl:when test="$mml-delim-str='('">
        <xsl:text>(</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str=')'">
        <xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='['">
        <xsl:text>[</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str=']'">
        <xsl:text>]</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='{'">
        <xsl:text>\{</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='}'">
        <xsl:text>\}</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2329;'
      or              $mml-delim-str='&#x3008;'">
        <xsl:text xml:space="preserve">\langle </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x232A;'
      or              $mml-delim-str='&#x3009;'">
        <xsl:text xml:space="preserve">\rangle </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x230A;'">
        <xsl:text xml:space="preserve">\lfloor </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x230B;'">
        <xsl:text xml:space="preserve">\rfloor </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2308;'">
        <xsl:text xml:space="preserve">\lceil </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2309;'">
        <xsl:text xml:space="preserve">\rceil </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='|'">
        <xsl:text xml:space="preserve">\vert </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2016;'">
        <xsl:text xml:space="preserve">\Vert </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='/'">
        <xsl:text>/</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='\'">
        <xsl:text xml:space="preserve">\backslash </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2195;'">
        <xsl:text xml:space="preserve">\updownarrow </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x250A;'">
        <xsl:text>.</xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x21D5;'">
        <xsl:text xml:space="preserve">\Updownarrow </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2191;'">
        <xsl:text xml:space="preserve">\uparrow </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x21D1;'">
        <xsl:text xml:space="preserve">\Uparrow </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x2193;'">
        <xsl:text xml:space="preserve">\downarrow </xsl:text>
      </xsl:when>
      <xsl:when test="$mml-delim-str='&#x21D3;'">
        <xsl:text xml:space="preserve">\Downarrow </xsl:text>
      </xsl:when>

      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="$is-left='true'">
            <xsl:text>(</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>)</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


<!-- template to script a LaTeX fence delimiting sequence
  ie. \left[ or \right\} etc.
-->

  <xsl:template name="do-fence-delim">
    <xsl:param name="is-left"/>
    <xsl:param name="delimiting-attr-val"/>
  
<!-- Note that the call to "do-delimiter" that follows
      always scripts a LaTeX fence symbol.  -->

    <xsl:choose>
      <xsl:when test="$is-left='true'">
        <xsl:text xml:space="preserve">\left</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve">\right </xsl:text>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:call-template name="do-delimiter">
      <xsl:with-param name="is-left"       select="$is-left"/>
      <xsl:with-param name="mml-delim-str" select="normalize-space($delimiting-attr-val)"/>
    </xsl:call-template>

  </xsl:template>


<!-- recursive template to handle mfenced list of children  -->

  <xsl:template name="do-fenced-items">
    <xsl:param name="item-tally"/>
    <xsl:param name="item-number"/>
    <xsl:param name="separator-list"/>
  
    <xsl:apply-templates select="*[position()=$item-number]"/>

    <xsl:if test="$item-number &lt; $item-tally">

<!-- If we get here, there are more items in our fenced list -->

<!--   Script the separator.  A separator-list is always passed in.
       If there are too few entries, the last entry is repeated. -->

      <xsl:choose>
        <xsl:when test="string-length(normalize-space($separator-list))&gt;1">
          <xsl:call-template name="chars-to-LaTeX-Math">
            <xsl:with-param name="unicode-cdata" select="substring-before($separator-list,substring($separator-list,2))"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="chars-to-LaTeX-Math">
            <xsl:with-param name="unicode-cdata" select="$separator-list"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>

<!-- Set up a recursive call to handle the remaining items -->

      <xsl:choose>
        <xsl:when test="string-length(normalize-space($separator-list))&gt;1">
          <xsl:call-template name="do-fenced-items">
            <xsl:with-param name="item-tally"     select="$item-tally"/>
            <xsl:with-param name="item-number"    select="$item-number + 1"/>
            <xsl:with-param name="separator-list" select="normalize-space(substring($separator-list,2))"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="do-fenced-items">
            <xsl:with-param name="item-tally"     select="$item-tally"/>
            <xsl:with-param name="item-number"    select="$item-number + 1"/>
            <xsl:with-param name="separator-list" select="$separator-list"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

  </xsl:template>

</xsl:stylesheet>

