<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">
<!--
<mml:mtext mathcolor="black"></mml:mtext>
; text accents  4.2.?

\c<uID4.2.13>!\c!REQPARAM(4.2.50,TEXT)
\H<uID4.2.14>!\H!REQPARAM(4.2.50,TEXT)
\k<uID4.2.15>!\k!REQPARAM(4.2.50,TEXT)
\r<uID4.2.16>!\r!REQPARAM(4.2.50,TEXT)
\t<uID4.2.17>!\t!REQPARAM(4.2.50,TEXT)
\d<uID4.2.18>!\d!REQPARAM(4.2.50,TEXT)
\b<uID4.2.19>!\b!REQPARAM(4.2.50,TEXT)
-->

<!--
; functions that "display" with limits above/below

\limfunc<uID8.3.1>!\limfunc!REQPARAM(8.3.2,NONLATEX)_LIMPLACE__FIRSTLIM__SECONDLIM_

;Note - \operatornamewithlimits is obsolete for \operatorname*
\operatornamewithlimits<uID8.3.1>!\operatornamewithlimits!REQPARAM(8.3.2,NONLATEX)_LIMPLACE__FIRSTLIM__SECONDLIM_
\operatorname*<uID8.3.1>!\operatorname*!REQPARAM(8.3.2,NONLATEX)_LIMPLACE__FIRSTLIM__SECONDLIM_

\mathnom*<uID8.3.3>!\mathnom*!REQPARAM(8.3.4,NONLATEX)REQPARAM(8.3.5,NONLATEX)_LIMPLACE__FIRSTLIM__SECONDLIM_

; not in Lamport
\injlim<uID8.4.4>!\injlim!_LIMPLACE__FIRSTLIM__SECONDLIM_

\liminf<uID8.4.6>!\liminf!_LIMPLACE__FIRSTLIM__SECONDLIM_
\limsup<uID8.4.7>!\limsup!_LIMPLACE__FIRSTLIM__SECONDLIM_

; not in Lamport
\projlim<uID8.4.11>!\projlim!_LIMPLACE__FIRSTLIM__SECONDLIM_

; the following are not from Lamport
\varinjlim<uID8.4.13>!\varinjlim!_LIMPLACE__FIRSTLIM__SECONDLIM_
\varliminf<uID8.4.14>!\varliminf!_LIMPLACE__FIRSTLIM__SECONDLIM_
\varlimsup<uID8.4.15>!\varlimsup!_LIMPLACE__FIRSTLIM__SECONDLIM_
\varprojlim<uID8.4.16>!\varprojlim!_LIMPLACE__FIRSTLIM__SECONDLIM_

-->

  
  <xsl:template name="under-struct">
    <xsl:param name="LaTeX-acc"/>
      <xsl:value-of select="$LaTeX-acc"/>
    <xsl:text>{</xsl:text>
    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="1"/>
    </xsl:call-template>
    <xsl:text>}</xsl:text>
  </xsl:template>


  
  <xsl:template match="mml:munder" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template name="check-under-accent">
      <xsl:choose>
<!--
      <xsl:if test="*[2][self::mml:mo][@accent='true']">
          <xsl:text>true</xsl:text>
      </xsl:if>
-->
      <xsl:when test="./*[1][normalize-space(string())='lim']">
        <xsl:choose>
<!-- UnderBar "&#x0332;" -->
          <xsl:when test="./*[2][normalize-space(string())='&#x0332;']">
            <xsl:text>true</xsl:text>
	      </xsl:when>
<!-- rarr "&#x2192;" -->
          <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
            <xsl:text>true</xsl:text>
	      </xsl:when>
<!-- larr "&#x2190;" -->
          <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
            <xsl:text>true</xsl:text>
	      </xsl:when>
		  <xsl:otherwise>
            <xsl:text>false</xsl:text>
		  </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

	  <xsl:otherwise>
        <xsl:text>false</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="check-under-decoration">
      <xsl:choose>
<!--
      <xsl:if test="*[2][self::mml:mo][@stretchy='true']">
          <xsl:text>true</xsl:text>
      </xsl:if>
-->
      <xsl:when test="./*[2][normalize-space(string())='&#x0332;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#xF613;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x23DF;']">
        <xsl:text>true</xsl:text>
      </xsl:when>

	  <xsl:otherwise>
        <xsl:text>false</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="mml:munder">
  
    <xsl:variable name="munder-structure.tr">
      <is-accent>
        <xsl:call-template name="check-under-accent"/>
      </is-accent>
      <is-decoration>
        <xsl:call-template name="check-under-decoration"/>
      </is-decoration>

      <big-op-char>
        <xsl:call-template name="is-LaTeX-bigop"/>
      </big-op-char>

      <is-LaTeX-op-limits>
        <xsl:choose>
          <xsl:when test="*[1][self::mml:mo][@form='prefix'][
                  normalize-space(string())='det'
              or  normalize-space(string())='gcd'
              or  normalize-space(string())='inf'
              or  normalize-space(string())='lim'
              or  normalize-space(string())='max'
              or  normalize-space(string())='min'
              or  normalize-space(string())='Pr'
              or  normalize-space(string())='sup'  ]">
            <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
            <xsl:text>false</xsl:text>
		  </xsl:otherwise>
        </xsl:choose>
      </is-LaTeX-op-limits>

      <is-user-op-limits>
        <xsl:choose>
          <xsl:when test="*[1][self::mml:mo][@form='prefix']">
            <xsl:text>true</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
            <xsl:text>false</xsl:text>
		  </xsl:otherwise>
        </xsl:choose>
      </is-user-op-limits>

      <movablelimits>
        <xsl:if test="./*[1][mml:mo]">
        <xsl:choose>
          <xsl:when test="string-length(*[1][@movablelimits]) &gt; 0">
            <xsl:for-each select="*[1][self::mml:mo]">
              <xsl:value-of select="@movablelimits"/>
            </xsl:for-each>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="get-mo-attr-val">
              <xsl:with-param name="mo-nom"   select="*[1][string()]"/>
              <xsl:with-param name="attr-nom" select="'movablelimits'"/>
            </xsl:call-template>
          </xsl:otherwise>
		</xsl:choose>
        </xsl:if>
      </movablelimits>

    </xsl:variable>
    <xsl:variable name="munder-structure" select="exsl:node-set($munder-structure.tr)"/>

    <xsl:variable name="limits">
      <xsl:if test="./*[1][mml:mo]">
      <xsl:choose>
        <xsl:when test="$munder-structure/movablelimits='false'">
          <xsl:text>\limits </xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      </xsl:if>
    </xsl:variable>

    <xsl:variable name="LaTeX-symbols">
      <xsl:call-template name="chars-to-LaTeX-Math">
        <xsl:with-param name="unicode-cdata" select="normalize-space(*[1][string()])"/>
      </xsl:call-template>
    </xsl:variable>


    <xsl:choose>

<!-- the base element is a big operator -->

      <xsl:when test="$munder-structure/big-op-char!='false'">
        <xsl:call-template name="do-embellished-bigop">
          <xsl:with-param name="limits-flag" select="$limits"/>
          <xsl:with-param name="j1"          select="'_{'"/>
          <xsl:with-param name="j2"          select="''"/>
        </xsl:call-template>
      </xsl:when>

<!-- accent under -->

      <xsl:when test="$munder-structure/is-accent='true'">
        <xsl:choose>
          <xsl:when test="./*[1][normalize-space(normalize-space(string()))='lim']">
            <xsl:choose>
<!-- UnderBar "&#x0332;" -->
              <xsl:when test="./*[2][normalize-space(string())='&#x0332;']">
			    <xsl:text mml:space="preserve">\varliminf </xsl:text>
	          </xsl:when>
<!-- rarr "&#x2192;" -->
              <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
			    <xsl:text mml:space="preserve">\varinjlim </xsl:text>
	          </xsl:when>
<!-- larr "&#x2190;" -->
              <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
			    <xsl:text mml:space="preserve">\varprojlim </xsl:text>
	          </xsl:when>
		      <xsl:otherwise>
			    <!--xsl:text mml:space="preserve">Unexpected under accent</xsl:text-->
		      </xsl:otherwise>
            </xsl:choose>
          </xsl:when>

		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
	  </xsl:when>

<!-- decorations below -->

      <xsl:when test="$munder-structure/is-decoration='true'">
        <xsl:choose>
          <xsl:when test="./*[2][normalize-space(string())='&#x0332;']">
            <xsl:call-template name="under-struct">
              <xsl:with-param name="LaTeX-acc" select="'\underline'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
            <xsl:call-template name="under-struct">
              <xsl:with-param name="LaTeX-acc" select="'\underleftarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
            <xsl:call-template name="under-struct">
              <xsl:with-param name="LaTeX-acc" select="'\underrightarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
            <xsl:call-template name="under-struct">
              <xsl:with-param name="LaTeX-acc" select="'\underleftrightarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x23DF;']">
            <xsl:call-template name="under-struct">
              <xsl:with-param name="LaTeX-acc" select="'\underbrace'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
		      </xsl:otherwise>
        </xsl:choose>
	  </xsl:when>

<!-- \lim_{} with qualifier below -->

      <xsl:when test="$munder-structure/is-LaTeX-op-limits='true'">
        <xsl:text>\</xsl:text>
        <xsl:value-of select="*[1]"/>
        <xsl:if test="$munder-structure/movablelimits='false'">
          <xsl:text>\limits</xsl:text>
        </xsl:if>
        <xsl:text>_{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:when>

<!-- \limfunc{mylimfunc}_{} with qualifier below -->
<!-- \operatorname*{mylimfunc}_{} with qualifier below -->

      <xsl:when test="$munder-structure/is-user-op-limits='true'">

        <xsl:choose>
          <xsl:when test="string-length($LaTeX-symbols) != string-length(normalize-space(*[1][string()]))">
            <xsl:value-of select="$LaTeX-symbols"/>
            <xsl:text>???</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>\operatorname*{</xsl:text>
              <xsl:value-of select="$LaTeX-symbols"/>
            <xsl:text>}</xsl:text>
            <xsl:if test="$munder-structure/movablelimits='false'">
              <xsl:text>\limits</xsl:text>
            </xsl:if>
            <xsl:text>_{</xsl:text>
            <xsl:call-template name="do-positional-arg">
              <xsl:with-param name="arg-num" select="2"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
	  </xsl:when>

	  <xsl:otherwise>
        <xsl:text>\underset{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="1"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>

    </xsl:choose>

  </xsl:template>

</xsl:stylesheet>


