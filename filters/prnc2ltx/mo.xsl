<xsl:stylesheet 
      xmlns:xsl="http://wwwnormalize-space(string())w3normalize-space(string())org/1999/XSL/Transform"
      xmlns:exsl="http://exsltnormalize-space(string())org/common"
      xmlns:mml="http://wwwnormalize-space(string())w3normalize-space(string())org/1998/Math/MathML"
      version="1normalize-space(string())1">

<!-- Algorithm
  if lspace exists, script LaTeX space as required

    Translate operators that have special markup in LaTeX
      1) Big Ops like \int, \sum, etcnormalize-space(string())
	  2) Operators that delimit fences
      3) Named operator like \det, \max, \Pr, etcnormalize-space(string())
    otherwise
    {
      if ( num-chars > 1 
	  &&   @form='prefix'
	  &&   all-ASCII='true' )
		  output \operatorname*{unicodes-2-LaTeX-math}
		else
		  output unicodes-2-LaTeX-math
    }

  if rspace exists, script LaTeX space as required
-->

  <xsl:template match="mml:mo">
	  <xsl:choose>
	  	<xsl:when test="@msimathname='true'">
				<xsl:choose>
					<xsl:when test="normalize-space(string())='arccos'">\arccos </xsl:when>
			    <xsl:when test="normalize-space(string())='cos'">\cos </xsl:when>
			    <xsl:when test="normalize-space(string())='csc'">\csc </xsl:when>
			    <xsl:when test="normalize-space(string())='exp'">\exp </xsl:when>
			    <xsl:when test="normalize-space(string())='ker'">\ker </xsl:when>
			    <xsl:when test="normalize-space(string())='limsup'">\limsup </xsl:when>
			    <xsl:when test="normalize-space(string())='min'">\min </xsl:when>
			    <xsl:when test="normalize-space(string())='sinh'">\sinh </xsl:when>
			    <xsl:when test="normalize-space(string())='arcsin'">\arcsin </xsl:when>
			    <xsl:when test="normalize-space(string())='cosh'">\cosh </xsl:when>
			    <xsl:when test="normalize-space(string())='deg'">\deg </xsl:when>
			    <xsl:when test="normalize-space(string())='gcd'">\gcd </xsl:when>
			    <xsl:when test="normalize-space(string())='lg'">\lg </xsl:when>
			    <xsl:when test="normalize-space(string())='ln'">\ln </xsl:when>
			    <xsl:when test="normalize-space(string())='Pr'">\Pr </xsl:when>
			    <xsl:when test="normalize-space(string())='sup'">\sup </xsl:when>
			    <xsl:when test="normalize-space(string())='arctan'">\arctan </xsl:when>
			    <xsl:when test="normalize-space(string())='cot'">\cot </xsl:when>
			    <xsl:when test="normalize-space(string())='det'">\det </xsl:when>
			    <xsl:when test="normalize-space(string())='hom'">\hom </xsl:when>
			    <xsl:when test="normalize-space(string())='lim'">\lim </xsl:when>
			    <xsl:when test="normalize-space(string())='log'">\log </xsl:when>
			    <xsl:when test="normalize-space(string())='sec'">\sec </xsl:when>
			    <xsl:when test="normalize-space(string())='tan'">\tan </xsl:when>
			    <xsl:when test="normalize-space(string())='arg'">\arg </xsl:when>
			    <xsl:when test="normalize-space(string())='coth'">\coth </xsl:when>
			    <xsl:when test="normalize-space(string())='dim'">\dim </xsl:when>
			    <xsl:when test="normalize-space(string())='inf'">\inf </xsl:when>
			    <xsl:when test="normalize-space(string())='liminf'">\liminf </xsl:when>
			    <xsl:when test="normalize-space(string())='max'">\max </xsl:when>
			    <xsl:when test="normalize-space(string())='sin'">\sin </xsl:when>
			    <xsl:when test="normalize-space(string())='tanh'">\tanh </xsl:when>
			    <xsl:otherwise>\ensuremath{\operatorname*{<xsl:apply-templates/>}}</xsl:otherwise>	
				</xsl:choose>
	  	</xsl:when>
      <xsl:otherwise>
        
  
    <xsl:if test="string-length(@lspace)&gt;0">

      <xsl:variable name="ls-value">
        <xsl:call-template name="get-number-chars">
          <xsl:with-param name="attrib-cdata" select="@lspace"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="ls-unit">
        <xsl:value-of select="substring-after(@lspace,$ls-value)"/>
      </xsl:variable>

      <xsl:call-template name="operator-lrspace-2LaTeX">
        <xsl:with-param name="value" select="$ls-value"/>
        <xsl:with-param name="unit"  select="$ls-unit"/>
      </xsl:call-template>
	</xsl:if>

    <xsl:if test="@largeop='true'">
  
    <xsl:choose>

<!-- Start of Big op's -->

      <xsl:when test="normalize-space(string())='&#x222B;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'int'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x222C;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'iint'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x222D;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'iiint'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2A0C;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'iiiint'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x222B;&#x22EF;&#x222B;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'idotsint'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x222E;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'oint'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2211;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'sum'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x220F;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'prod'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x22C2;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigcap'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x22C0;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigwedge'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2295;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigoplus'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2299;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigodot'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2294;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigsqcup'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2210;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'coprod'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x22C3;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigcup'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x22C1;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigvee'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2297;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'bigotimes'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x228E;'">
        <xsl:call-template name="bigop">
          <xsl:with-param name="LaTeX-nom" select="'biguplus'"/>
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
    </xsl:if>

    <xsl:if test="not(@largeop='true')">
    <xsl:choose>
<!-- Start of fencing <mo>'s -->
      <xsl:when test="normalize-space(string())='&#x250A;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'.'"/>
        </xsl:call-template>
      </xsl:when>

      <xsl:when test="normalize-space(string())='('">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'('"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())=')'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="')'"/>
        </xsl:call-template>
      </xsl:when>

      <xsl:when test="normalize-space(string())='['">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'['"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())=']'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="']'"/>
        </xsl:call-template>
      </xsl:when>

      <xsl:when test="normalize-space(string())='{'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='}'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\}'"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\langle       \right\rangle      --> 
      <xsl:when test="normalize-space(string())='&#x2329;'
      or              normalize-space(string())='&#x3008;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\langle '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x232A;'
      or              normalize-space(string())='&#x3009;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\rangle '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\lfloor       \right\rfloor 		-->
      <xsl:when test="normalize-space(string())='&#x230A;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\lfloor '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x230B;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\rfloor '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\lceil        \right\rceil		-->
      <xsl:when test="normalize-space(string())='&#x2308;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\lceil '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="normalize-space(string())='&#x2309;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\rceil '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\vert         \right\vert 		-->
      <xsl:when test="normalize-space(string())='|'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\vert '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\Vert         \right\Vert 		-->
      <xsl:when test="normalize-space(string())='&#x2016;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\Vert '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left/             \right/			-->
      <xsl:when test="normalize-space(string())='/'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'/'"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\backslash    \right\backslash 	-->
      <xsl:when test="normalize-space(string())='\'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\backslash '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\updownarrow  \right\updownarrow	-->
      <xsl:when test="normalize-space(string())='&#x2195;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\updownarrow '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left.             \right.  -->
      <xsl:when test="normalize-space(string())='.' and  @fence='true'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'.'"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\Updownarrow  \right\Updownarrow -->
      <xsl:when test="normalize-space(string())='&#x21D5;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\Updownarrow '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\uparrow      \right\uparrow 	-->
      <xsl:when test="normalize-space(string())='&#x2191;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\uparrow '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\Uparrow      \right\Uparrow 	-->
      <xsl:when test="normalize-space(string())='&#x21D1;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\Uparrow '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\downarrow    \right\downarrow 	-->
      <xsl:when test="normalize-space(string())='&#x2193;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\downarrow '"/>
        </xsl:call-template>
      </xsl:when>
<!-- \left\Downarrow    \right\Downarrow 	-->
      <xsl:when test="normalize-space(string())='&#x21D3;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'\Downarrow '"/>
        </xsl:call-template>
      </xsl:when>
<!--       invisible brace match -->
<!--       <xsl:when test="normalize-space(string())='&#x250A;'">
        <xsl:call-template name="translate-fencing-mo">
          <xsl:with-param name="LaTeX-fence-token" select="'&#x002E;'"/>
        </xsl:call-template>
      </xsl:when> -->

<!-- !ENTITY dd or DifferentialD "d" 
  At load time, &dd; may be mapped to "d"normalize-space(string())
  We don't need a special translation here - default gives same outputnormalize-space(string())

      <xsl:when test="normalize-space(string())='d'">
        <xsl:if test="@lspace='0normalize-space(string())333333em'">
          <xsl:text>\,</xsl:text>
        </xsl:if>
        <xsl:text>d</xsl:text>
      </xsl:when>
-->

<!-- a few special cases -->

      <xsl:when test="normalize-space(string())='det'">
        <xsl:text xml:space="preserve">\det </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='gcd'">
        <xsl:text xml:space="preserve">\gcd </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='inf'">
        <xsl:text xml:space="preserve">\inf </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='lim'">
        <xsl:text xml:space="preserve">\lim </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='max'">
        <xsl:text xml:space="preserve">\max </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='min'">
        <xsl:text xml:space="preserve">\min </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='Pr'">
        <xsl:text xml:space="preserve">\Pr </xsl:text>
      </xsl:when>
      <xsl:when test="normalize-space(string())='sup'">
        <xsl:text xml:space="preserve">\sup </xsl:text>
      </xsl:when>

      <xsl:when test="normalize-space(string())='mod'">
        <xsl:text>\operatorname{mod}</xsl:text>
      </xsl:when>

      <xsl:otherwise>

        <xsl:variable name="LaTeX-symbols">
          <xsl:call-template name="chars-to-LaTeX-Math">
            <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
          </xsl:call-template>
        </xsl:variable>

   
        <xsl:choose>
          <xsl:when test="string-length(normalize-space(string()))&gt;1">

            <xsl:variable name="all-ASCII">
              <xsl:call-template name="is-ASCII">
                <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
              </xsl:call-template>
            </xsl:variable>

            <xsl:variable name="n-letters">
              <xsl:call-template name="count-letters">
                <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
                <xsl:with-param name="letters-found" select="0"/>
              </xsl:call-template>
            </xsl:variable>

            <xsl:choose>
              <xsl:when test="$all-ASCII='true' and $n-letters&gt;0 and @form='prefix'">

                <xsl:variable name="LaTeX-contents">
                  <xsl:call-template name="do-chars-in-TEXT">
                    <xsl:with-param name="unicode-cdata" select="string()"/>
                  </xsl:call-template>
                </xsl:variable>

                <xsl:text>\operatorname*{</xsl:text>
                  <xsl:value-of select="$LaTeX-contents"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>

          </xsl:when>
          <xsl:otherwise>
            <xsl:if test="string-length(normalize-space(string()))=1">
              <xsl:text> </xsl:text>
            </xsl:if>
            <xsl:value-of select="$LaTeX-symbols"/>
          </xsl:otherwise>
        </xsl:choose>

      </xsl:otherwise>

    </xsl:choose>
    </xsl:if>

    <xsl:if test="string-length(@rspace)&gt;0">

      <xsl:variable name="rs-value">
        <xsl:call-template name="get-number-chars">
          <xsl:with-param name="attrib-cdata" select="@rspace"/>
        </xsl:call-template>
      </xsl:variable>
      <xsl:variable name="rs-unit">
        <xsl:value-of select="substring-after(@rspace,$rs-value)"/>
      </xsl:variable>

      <xsl:call-template name="operator-lrspace-2LaTeX">
        <xsl:with-param name="value" select="$rs-value"/>
        <xsl:with-param name="unit"  select="$rs-unit"/>
      </xsl:call-template>
	</xsl:if>      
  </xsl:otherwise>
	  </xsl:choose>


  </xsl:template>




<!-- mo's contain math objectsnormalize-space(string()) If we encounter them when a LaTeX
  text bucket is being scripted, the translation must enclosed in $'snormalize-space(string())
-->
  
  <xsl:template match="mml:mo" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  
  <xsl:template name="bigop">
    <xsl:param name="LaTeX-nom"/>
    <xsl:choose>
      <xsl:when test="((@largeop='true') or (@stretchy='true'))">
        <xsl:choose>
<!-- BBM for bug 3094
          <xsl:when test="ancestor-or-self::*/@displaystyle='true'">
            <xsl:text xml:space="preserve">\mathop{</xsl:text>
            <xsl:text>\displaystyle\</xsl:text>
            <xsl:text>\</xsl:text><xsl:value-of select="$LaTeX-nom"/>
            <xsl:text xml:space="preserve">}</xsl:text>
          </xsl:when>
 -->
         <xsl:otherwise>
            <xsl:text>\</xsl:text>
            <xsl:value-of select="$LaTeX-nom"/>
            <xsl:text> </xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
<!-- The following when clause has been commented out. Does anyone know why? -BBM -->
      <xsl:when test="@largeop='false' and @stretchy='false'">
        <xsl:text xml:space="preserve">\t</xsl:text>
        <xsl:value-of select="$LaTeX-nom"/>
        <xsl:text xml:space="preserve"> </xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve">\</xsl:text>
        <xsl:value-of select="$LaTeX-nom"/>
        <xsl:text xml:space="preserve"> </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



<!-- Occassionally we need to know if the contents of an element
     (mo,mi,mn) are all ASCII charsnormalize-space(string())  LaTeX commands like \operatorname{}
     can only be scripted for elements containing ASCIInormalize-space(string()) BBM herenormalize-space(string())normalize-space(string())normalize-space(string()) This doesn't look rightnormalize-space(string()) Test will
     be true if there are *any* ASCII charactersnormalize-space(string()) Under XeTeX I'm not sure Jack's assertion is true,
     so I'm making this always return true-->

  <xsl:template name="is-ASCII">
    <xsl:param name="unicode-cdata"/>
      <!--xsl:choose>
      <xsl:when test="regexp:test(normalize-space(string($unicode-cdata)),'[\x01-\xFF]+','g')"-->
         <xsl:text>true</xsl:text>
      <!--/xsl:when>
      <xsl:otherwise>
         <xsl:text>false</xsl:text>
      </xsl:otherwise>
	</xsl:choose -->
	
  </xsl:template>


  <xsl:template name="count-letters">
    <xsl:param name="unicode-cdata"/>
    <xsl:param name="letters-found"/>
  
    <xsl:variable name="first-char" select="substring($unicode-cdata,1,1)"/>

    <xsl:choose>
      <xsl:when test="
         $first-char = 'A'
      or $first-char = 'B'
      or $first-char = 'C'
      or $first-char = 'D'
      or $first-char = 'E'
      or $first-char = 'F'
      or $first-char = 'G'
      or $first-char = 'H'
      or $first-char = 'I'
      or $first-char = 'J'
      or $first-char = 'K'
      or $first-char = 'L'
      or $first-char = 'M'
      or $first-char = 'N'
      or $first-char = 'O'
      or $first-char = 'P'
      or $first-char = 'Q'
      or $first-char = 'R'
      or $first-char = 'S'
      or $first-char = 'T'
      or $first-char = 'U'
      or $first-char = 'V'
      or $first-char = 'W'
      or $first-char = 'X'
      or $first-char = 'Y'
      or $first-char = 'Z'
      or $first-char = 'a'
      or $first-char = 'b'
      or $first-char = 'c'
      or $first-char = 'd'
      or $first-char = 'e'
      or $first-char = 'f'
      or $first-char = 'g'
      or $first-char = 'h'
      or $first-char = 'i'
      or $first-char = 'j'
      or $first-char = 'k'
      or $first-char = 'l'
      or $first-char = 'm'
      or $first-char = 'n'
      or $first-char = 'o'
      or $first-char = 'p'
      or $first-char = 'q'
      or $first-char = 'r'
      or $first-char = 's'
      or $first-char = 't'
      or $first-char = 'u'
      or $first-char = 'v'
      or $first-char = 'w'
      or $first-char = 'x'
      or $first-char = 'y'
      or $first-char = 'z'">
        <xsl:choose>
          <xsl:when test="string-length($unicode-cdata)&gt;1">
            <xsl:call-template name="count-letters">
              <xsl:with-param name="unicode-cdata" select="substring($unicode-cdata,2)"/>
              <xsl:with-param name="letters-found" select="$letters-found+1"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$letters-found+1"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
        <xsl:value-of select="$letters-found"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template name="is-all-caps">
    <xsl:param name="unicode-cdata"/>
      <!-- xsl:choose>
      <xsl:when test="regexp:test(normalize-space(string($unicode-cdata)),'[^A-Z]+','g')" -->
         <xsl:text>false</xsl:text>
      <!--/xsl:when>
      <xsl:otherwise>
         <xsl:text>true</xsl:text>
      </xsl:otherwise>
	</xsl:choose -->
    <!--xsl:variable name="first-char" select="substring($unicode-cdata,1,1)"/>

    <xsl:choose>
      <xsl:when test="
         $first-char = 'A'
      or $first-char = 'B'
      or $first-char = 'C'
      or $first-char = 'D'
      or $first-char = 'E'
      or $first-char = 'F'
      or $first-char = 'G'
      or $first-char = 'H'
      or $first-char = 'I'
      or $first-char = 'J'
      or $first-char = 'K'
      or $first-char = 'L'
      or $first-char = 'M'
      or $first-char = 'N'
      or $first-char = 'O'
      or $first-char = 'P'
      or $first-char = 'Q'
      or $first-char = 'R'
      or $first-char = 'S'
      or $first-char = 'T'
      or $first-char = 'U'
      or $first-char = 'V'
      or $first-char = 'W'
      or $first-char = 'X'
      or $first-char = 'Y'
      or $first-char = 'Z'">
        <xsl:choose>
          <xsl:when test="string-length($unicode-cdata)&gt;1">
            <xsl:call-template name="is-all-caps">
              <xsl:with-param name="unicode-cdata" select="substring($unicode-cdata,2)"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>true</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
        <xsl:text>false</xsl:text>
      </xsl:otherwise>
    </xsl:choose-->

  </xsl:template>


</xsl:stylesheet>

