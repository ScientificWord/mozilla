<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

  <xsl:template match="mml:mfrac">
  
<!-- The translation of mfrac is context sensitive.

  In MathML, the size is controlled by nesting the fraction
  in an mstyle element.
  Also, an mfenced ( or equivalent mo's ) may bracket the fraction.
  In LaTeX, an enclosing fence may be part of the fraction command,
  as in \binom or \genfrac{[}{]}{}{}{}{}.

  The design of this script is BAD
   - I don't know how to do any better using xsl.
  
  The problem arises when we translate MathML as follows:

    <mo> <-> <mfrac> <-> <mo>  OR   <mo> <-> <mrow> <-> <mo>
									    		\
										      <mstyle>
										          \
											    <mfrac>

  When the first <mo> is encountered, we look ahead to see if
  it may be absorbed in the LaTeX output a following object.
  For example, the output may be \binom or \genfrac{}{}{}{}{}{}.

  When <mfrac> is encountered, we must look at it's siblings
  (and in some cases, the sibling of <mrow> ancestors) to see
  if the LaTeX to be scripted must include nesting delimiters.

  When the second <mo> is encountered, we look back to see if
  it may have been absorbed in the LaTeX output for a preceding
  object.

  The problem with this scheme is the interdependence between
  the scripts for <mo> and <mfrac>.  The related decisions must
  be made ( and maintained ) in both scripts.
-->

<!-- 4 variables to hold info about a possible fencing <mo>
  to the immediate left of the current <mfrac> -->

    <xsl:variable name="left-mo1-nom">
      <xsl:value-of select="normalize-space(preceding-sibling::*[1][self::mml:mo])"/>
    </xsl:variable>

    <xsl:variable name="has-left-fence-mo">
	  <xsl:choose>
        <xsl:when test="preceding-sibling::*[1][self::mml:mo]">
          <xsl:call-template name="mo-is-LaTeX-fence">
		    <xsl:with-param name="op-nom" select="$left-mo1-nom"/>
          </xsl:call-template>
		</xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="left-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length(preceding-sibling::*[1][self::mml:mo][@stretchy]) &gt; 0">
          <xsl:for-each select="preceding-sibling::*[1][self::mml:mo]">
            <xsl:value-of select="@stretchy"/>
          </xsl:for-each>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$left-mo1-nom"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="left-form">
      <xsl:for-each select="preceding-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="left-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($left-form) &gt; 0">
          <xsl:value-of select="$left-form"/>
        </xsl:when>
        <xsl:when test="string-length(preceding-sibling::*[1][self::mml:mo])=0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$left-mo1-nom"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!-- 4 variables to hold info about a possible fencing <mo>
  to the immediate right of the current <mfrac> -->

    <xsl:variable name="right-mo1-nom">
      <xsl:value-of select="normalize-space(following-sibling::*[1][self::mml:mo])"/>
    </xsl:variable>

    <xsl:variable name="has-right-fence-mo">
	  <xsl:choose>
        <xsl:when test="following-sibling::*[1][self::mml:mo]">
          <xsl:call-template name="mo-is-LaTeX-fence">
		    <xsl:with-param name="op-nom" select="$right-mo1-nom"/>
          </xsl:call-template>
		</xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="right-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length(following-sibling::*[1][self::mml:mo][@stretchy]) &gt; 0">
          <xsl:for-each select="following-sibling::*[1][self::mml:mo]">
            <xsl:value-of select="@stretchy"/>
          </xsl:for-each>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$right-mo1-nom"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="right-form">
      <xsl:for-each select="following-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="right-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($right-form) &gt; 0">
          <xsl:value-of select="$right-form"/>
        </xsl:when>
        <xsl:when test="string-length(following-sibling::*[1][self::mml:mo])=0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$right-mo1-nom"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!-- ************************* -->

    <xsl:variable name="left-ancestor-mo1-nom.tr">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="preceding-sibling::*[1][self::mml:mo]">
	    <exists>
        <xsl:value-of select="normalize-space(string())"/>
	    </exists>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="left-ancestor-mo1-nom" select="$left-ancestor-mo1-nom.tr"/>
    <xsl:variable name="left-ancestor-mo1-nom-exists">
      <xsl:choose>
        <xsl:when test="string-length($left-ancestor-mo1-nom.tr) &gt; 0">
          <xsl:value-of select="normalize-space(string())"/>
        </xsl:when>
    		<xsl:otherwise>
    		</xsl:otherwise>
	    </xsl:choose>
    </xsl:variable>

    <xsl:variable name="ancestor-has-left-fence-mo">
      <xsl:call-template name="mo-is-LaTeX-fence">
	    <xsl:with-param name="op-nom" select="$left-ancestor-mo1-nom-exists"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="left-ancestor-stretchy">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="preceding-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@stretchy"/>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="left-ancestor-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length($left-ancestor-stretchy) &gt; 0">
          <xsl:value-of select="$left-ancestor-stretchy"/>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$left-ancestor-mo1-nom-exists"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="left-ancestor-form">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="preceding-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="left-ancestor-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($left-ancestor-form) &gt; 0">
          <xsl:value-of select="$left-ancestor-form"/>
        </xsl:when>
        <xsl:when test="string-length($left-ancestor-mo1-nom-exists) = 0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$left-ancestor-mo1-nom-exists"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>


    <xsl:variable name="right-ancestor-mo1-nom.tr">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="following-sibling::*[1][self::mml:mo]">
	    <exists>
        <xsl:value-of select="normalize-space(.)"/>
	    </exists>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="right-ancestor-mo1-nom" select="$right-ancestor-mo1-nom.tr"/>
    <xsl:variable name="right-ancestor-mo1-nom-exists">
      <xsl:choose>
        <xsl:when test="string-length($right-ancestor-mo1-nom.tr) &gt; 0">
          <xsl:value-of select="normalize-space(string())"/>
        </xsl:when>
    		<xsl:otherwise>
    		</xsl:otherwise>
	    </xsl:choose>
    </xsl:variable>

    <xsl:variable name="ancestor-has-right-fence-mo">
      <xsl:call-template name="mo-is-LaTeX-fence">
	    <xsl:with-param name="op-nom" select="$right-ancestor-mo1-nom-exists"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="right-ancestor-stretchy">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="following-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@stretchy"/>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="right-ancestor-mo-stretchy">
	  <xsl:choose>
        <xsl:when test="string-length($right-ancestor-stretchy) &gt; 0">
          <xsl:value-of select="$right-ancestor-stretchy"/>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$right-ancestor-mo1-nom-exists"/>
            <xsl:with-param name="attr-nom" select="'stretchy'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="right-ancestor-form">
      <xsl:for-each select="ancestor::*[2][self::mml:mrow]">
      <xsl:for-each select="following-sibling::*[1][self::mml:mo]">
        <xsl:value-of select="@form"/>
      </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="right-ancestor-mo-form">
	  <xsl:choose>
        <xsl:when test="string-length($right-ancestor-form) &gt; 0">
          <xsl:value-of select="$right-ancestor-form"/>
        </xsl:when>
        <xsl:when test="string-length($right-ancestor-mo1-nom-exists) = 0">
          <xsl:text>unknown</xsl:text>
        </xsl:when>
		<xsl:otherwise>
          <xsl:call-template name="get-mo-attr-val">
            <xsl:with-param name="mo-nom"   select="$right-ancestor-mo1-nom-exists"/>
            <xsl:with-param name="attr-nom" select="'form'"/>
          </xsl:call-template>
		</xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>


    <xsl:variable name="opener">
        <xsl:choose>

          <xsl:when test="parent::mml:mfenced[count(./*)&gt;1]">
            <xsl:text>false</xsl:text>
          </xsl:when>

          <xsl:when test="parent::mml:mfenced/@open">
            <xsl:call-template name="mo-is-LaTeX-fence">
    		  <xsl:with-param name="op-nom" select="parent::mml:mfenced/@open"/>
            </xsl:call-template>
		  </xsl:when>
          <xsl:when test="parent::mml:mfenced">
            <xsl:text>(</xsl:text>
		  </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>


    <xsl:variable name="closer">
        <xsl:choose>

          <xsl:when test="parent::mml:mfenced[count(./*)&gt;1]">
            <xsl:text>false</xsl:text>
          </xsl:when>

          <xsl:when test="parent::mml:mfenced/@close">
            <xsl:call-template name="mo-is-LaTeX-fence">
    		  <xsl:with-param name="op-nom" select="parent::mml:mfenced/@close"/>
            </xsl:call-template>
		  </xsl:when>
          <xsl:when test="parent::mml:mfenced">
            <xsl:text>)</xsl:text>
		  </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>


<!-- *************************** -->

      <xsl:variable name="frac-context-size">
        <xsl:choose>
          <xsl:when test="parent::mml:mstyle[@displaystyle='true'][count(./*)=1]">
            <xsl:value-of select="'d'"/>
			    </xsl:when>
          <xsl:when test="parent::mml:mstyle[@displaystyle='false'][count(./*)=1]">
            <xsl:value-of select="'t'"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="'a'"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="frac-context-preceding-delim">
        <xsl:choose>
          <xsl:when test="$has-left-fence-mo='true'
          and             $left-mo-stretchy!='false'
          and            ($left-mo-form='prefix'
          or              $left-mo-form='ambiguous')">
            <xsl:value-of select="$left-mo1-nom"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>


      <xsl:variable name="frac-context-following-delim">
        <xsl:choose>
          <xsl:when test="$has-right-fence-mo='true'
          and             $right-mo-stretchy!='false'
          and            ($right-mo-form='postfix'
          or              $right-mo-form='ambiguous')">
            <xsl:value-of select="$right-mo1-nom"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="frac-context-preceding-delim-nested">
        <xsl:choose>
          <xsl:when test="$left-ancestor-mo1-nom-exists
          and             $ancestor-has-left-fence-mo='true'
          and             $left-ancestor-mo-stretchy!='false'
          and            ($left-ancestor-mo-form='prefix'
          or              $left-ancestor-mo-form='ambiguous')">
            <xsl:value-of select="$left-ancestor-mo1-nom-exists"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="frac-context-following-delim-nested">
        <xsl:choose>
          <xsl:when test="$right-ancestor-mo1-nom-exists
          and             $ancestor-has-right-fence-mo='true'
          and             $right-ancestor-mo-stretchy!='false'
          and            ($right-ancestor-mo-form='postfix'
          or              $right-ancestor-mo-form='ambiguous')">
            <xsl:value-of select="$right-ancestor-mo1-nom-exists"/>
      	</xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>


<!-- check for a parent mfenced that can be absorbed by the LaTeX fraction -->

      <xsl:variable name="frac-context-left-mfence">
        <xsl:choose>
          <xsl:when test="$opener='true'">
            <xsl:value-of select="parent::mml:mfenced/@open"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$opener"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="frac-context-right-mfence">
        <xsl:choose>
          <xsl:when test="$closer='true'">
            <xsl:value-of select="parent::mml:mfenced/@close"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$closer"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

<!-- The following variable uses info in the variables above to define values
     for delimiters that may be incorporated into our LaTeX output -->


    <xsl:variable name="frac-delims-l-delim">
      <xsl:choose>
        <xsl:when test="not($frac-context-preceding-delim='false')
        and             not($frac-context-following-delim='false')">
            <xsl:value-of select="$frac-context-preceding-delim"/>
        </xsl:when>
        <xsl:when test="not($frac-context-preceding-delim-nested='false')
        and             not($frac-context-following-delim-nested='false')
        and             count(preceding-sibling::*)=0
        and             count(following-sibling::*)=0">
            <xsl:value-of select="$frac-context-preceding-delim-nested"/>
        </xsl:when>
        <xsl:when test="not($frac-context-left-mfence='false')
        and             not($frac-context-right-mfence='false')">
            <xsl:value-of select="$frac-context-left-mfence"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="frac-delims-r-delim">
      <xsl:choose>
        <xsl:when test="not($frac-context-preceding-delim='false')
        and             not($frac-context-following-delim='false')">
            <xsl:value-of select="$frac-context-following-delim"/>
        </xsl:when>
        <xsl:when test="not($frac-context-preceding-delim-nested='false')
        and             not($frac-context-following-delim-nested='false')
        and             count(preceding-sibling::*)=0
        and             count(following-sibling::*)=0">
            <xsl:value-of select="$frac-context-following-delim-nested"/>
        </xsl:when>
        <xsl:when test="not($frac-context-left-mfence='false')
        and             not($frac-context-right-mfence='false')">
            <xsl:value-of select="$frac-context-right-mfence"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>



    <xsl:variable name="frac-linethickness">
	  <xsl:choose>
        <xsl:when test="@linethickness">
          <xsl:value-of select="@linethickness"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="get-inherited-attr-value">
            <xsl:with-param name="attr-nom" select="'linethickness'"/>
          </xsl:call-template>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>


<!-- Start generation of LaTeX output -->
<!--
<xsl:text>AAA-</xsl:text>
<xsl:value-of select="$left-ancestor-form"/>
<xsl:text>-BBB-</xsl:text>
<xsl:value-of select="$left-ancestor-mo-form"/>
<xsl:text>-AAA</xsl:text>
-->


    <xsl:choose>

      <xsl:when test="string-length($frac-linethickness)&gt;0">

        <xsl:variable name="rule-depth">
          <xsl:call-template name="get-LaTeX-depth">
            <xsl:with-param name="lthick-attr-val" select="normalize-space($frac-linethickness)"/>
          </xsl:call-template>
        </xsl:variable>
        
        <xsl:choose>

<!-- \genfrac{}{}{0pt}{1}{x}{2} -->

          <xsl:when test="$frac-linethickness='0'">
            <xsl:choose>

              <xsl:when test="($frac-delims-l-delim='(')
                   and        ($frac-delims-r-delim=')')">
                <xsl:choose>
                  <xsl:when test="$frac-context-size='d'">
                    <xsl:text>\dbinom{</xsl:text>
                  </xsl:when>
                  <xsl:when test="$frac-context-size='t'">
                    <xsl:text>\tbinom{</xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>\binom{</xsl:text>
                  </xsl:otherwise>
				</xsl:choose>
			  </xsl:when>

              <xsl:otherwise>
                <xsl:choose>
                  <xsl:when test="not($frac-delims-l-delim='false')
                       and        not($frac-delims-r-delim='false')">
                    <xsl:text xml:space="preserve">\genfrac{</xsl:text>
                    <xsl:call-template name="do-delimiter">
                      <xsl:with-param name="is-left"       select="'true'"/>
                      <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-l-delim)"/>
                    </xsl:call-template>
                    <xsl:text xml:space="preserve">}{</xsl:text>
                    <xsl:call-template name="do-delimiter">
                      <xsl:with-param name="is-left"       select="'false'"/>
                      <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-r-delim)"/>
                    </xsl:call-template>
                    <xsl:text>}{0pt}</xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>\genfrac{}{}{0pt}</xsl:text>
                  </xsl:otherwise>
                </xsl:choose>

                <xsl:choose>
                  <xsl:when test="$frac-context-size='d'">
                    <xsl:text>{0}{</xsl:text>
                  </xsl:when>
                  <xsl:when test="$frac-context-size='t'">
                    <xsl:text>{1}{</xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>{}{</xsl:text>
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="not($frac-delims-l-delim='false')
                   and        not($frac-delims-r-delim='false')">
                <xsl:text xml:space="preserve">\genfrac{</xsl:text>
                <xsl:call-template name="do-delimiter">
                  <xsl:with-param name="is-left"       select="'true'"/>
                  <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-l-delim)"/>
                </xsl:call-template>
                <xsl:text xml:space="preserve">}{</xsl:text>
                <xsl:call-template name="do-delimiter">
                  <xsl:with-param name="is-left"       select="'false'"/>
                  <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-r-delim)"/>
                </xsl:call-template>
                <xsl:text>}{</xsl:text>
                  <xsl:value-of select="$rule-depth"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>\genfrac{}{}{</xsl:text>
                  <xsl:value-of select="$rule-depth"/>
                <xsl:text>}</xsl:text>
              </xsl:otherwise>
            </xsl:choose>

            <xsl:choose>
              <xsl:when test="$frac-context-size='d'">
                <xsl:text>{0}{</xsl:text>
              </xsl:when>
              <xsl:when test="$frac-context-size='t'">
                <xsl:text>{1}{</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>{}{</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>

        </xsl:choose>
      </xsl:when>

<!-- Handle mfracs that have default linethickness -->

      <xsl:otherwise>
        <xsl:choose>

<!-- with delimiters -->
          <xsl:when test="not($frac-delims-l-delim='false')
               and        not($frac-delims-r-delim='false')">
            <xsl:text xml:space="preserve">\genfrac{</xsl:text>
            <xsl:call-template name="do-delimiter">
              <xsl:with-param name="is-left"       select="'true'"/>
              <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-l-delim)"/>
            </xsl:call-template>
            <xsl:text>}{</xsl:text>
            <xsl:call-template name="do-delimiter">
              <xsl:with-param name="is-left"       select="'false'"/>
              <xsl:with-param name="mml-delim-str" select="normalize-space($frac-delims-r-delim)"/>
            </xsl:call-template>
            <xsl:text>}{}</xsl:text>

            <xsl:choose>
              <xsl:when test="$frac-context-size='d'">
                <xsl:text>{0}{</xsl:text>
              </xsl:when>
              <xsl:when test="$frac-context-size='t'">
                <xsl:text>{1}{</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>{}{</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>

<!-- without delimiters -->

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$frac-context-size='d'">
                <xsl:text>\dfrac{</xsl:text>
              </xsl:when>
              <xsl:when test="$frac-context-size='t'">
                <xsl:text>\tfrac{</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>\frac{</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>

        </xsl:choose>
      </xsl:otherwise>

    </xsl:choose>

<!-- Finally, we translate the numerator and denominator -->

    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="1"/>
    </xsl:call-template>
    <xsl:text>}{</xsl:text>
    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="2"/>
    </xsl:call-template>
    <xsl:text>}</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mfrac" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>



<!-- Utility to translate the value of an mfrac linethickness attr
  to a LaTeX dimen.  Needs more work. -->


  <xsl:template name="get-LaTeX-depth">
    <xsl:param name="lthick-attr-val"/>
  
    <xsl:choose>
      <xsl:when test="$lthick-attr-val='thin'">
        <xsl:text>0.5pt</xsl:text>
      </xsl:when>
      <xsl:when test="$lthick-attr-val='medium'">
        <xsl:text>1pt</xsl:text>
      </xsl:when>
      <xsl:when test="$lthick-attr-val='thick'">
        <xsl:text>2pt</xsl:text>
      </xsl:when>

      <xsl:otherwise>
        <xsl:variable name="value">
          <xsl:call-template name="get-number-chars">
            <xsl:with-param name="attrib-cdata" select="$lthick-attr-val"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="unit">
          <xsl:value-of select="substring-after($lthick-attr-val,$value)"/>
        </xsl:variable>

        <xsl:choose>
		  <xsl:when test="string-length($unit)=0">
        	<xsl:value-of select="substring($value * 0.1,1,4)"/>
            <xsl:text>ex</xsl:text>
		  </xsl:when>
		  <xsl:when test="string-length($unit)=1">

            <xsl:choose>
			  <xsl:when test="$unit='%'">
            	<xsl:value-of select="substring($value * 0.001,1,4)"/>
                <xsl:text>ex</xsl:text>
			  </xsl:when>
    		  <xsl:otherwise>
	    	  </xsl:otherwise>
            </xsl:choose>

		  </xsl:when>
		  <xsl:when test="string-length($unit)=2">

            <xsl:variable name="unit-OK">
              <xsl:call-template name="is-TeX-unit">
                <xsl:with-param name="unit-nom" select="$unit"/>
              </xsl:call-template>
            </xsl:variable>

            <xsl:choose>
			  <xsl:when test="$unit-OK='true'">
                <xsl:value-of select="$lthick-attr-val"/>
			  </xsl:when>
    		  <xsl:otherwise>
	    	  </xsl:otherwise>
            </xsl:choose>

		  </xsl:when>
		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


</xsl:stylesheet>

