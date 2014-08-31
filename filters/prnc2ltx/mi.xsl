<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- 

-->


  <xsl:template match="mml:mi">
  
    <xsl:variable name="LaTeX-symbols">
        <xsl:call-template name="chars-to-LaTeX-Math">
        <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="all-ASCII">
      <xsl:call-template name="is-ASCII">
        <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
      </xsl:call-template>
    </xsl:variable>


    <xsl:variable name="tag">
      <xsl:call-template name="get-tag">
        <xsl:with-param name="raw-LaTeX" select="$LaTeX-symbols"/>
      </xsl:call-template>
    </xsl:variable>



    <xsl:choose>

      <xsl:when test="string-length(normalize-space(string())) = 1">        
        <xsl:choose>
          <xsl:when test="$tag!='false'">

            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:value-of select="$tag"/>
              <xsl:value-of select="$LaTeX-symbols"/>
              <xsl:text>}</xsl:text>
            </xsl:if>

            <xsl:if test="$output-mode='SW-LaTeX'">
                <xsl:choose>
                  <xsl:when test="@mathvariant='script' or @mathvariant='double-struck'">
                    <xsl:variable name="all-caps">
                       <xsl:call-template name="is-all-caps">
                          <xsl:with-param name="unicode-cdata" select="$LaTeX-symbols"/>
                       </xsl:call-template>
                    </xsl:variable>
                
                    <xsl:choose>
                       <xsl:when test="$all-caps='true'">
                          <xsl:value-of select="$tag"/>
                            <xsl:value-of select="$LaTeX-symbols"/>
                          <xsl:text>}</xsl:text>
                       </xsl:when>
                       <xsl:when test="@mathvariant='double-struck' and  $LaTeX-symbols='k'">
                          <xsl:text>\Bbbk </xsl:text>
                       </xsl:when>
                       <xsl:otherwise>
                          <xsl:value-of select="$LaTeX-symbols"/>
                       </xsl:otherwise>
                    </xsl:choose>

                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:value-of select="$tag"/>
                    <xsl:value-of select="$LaTeX-symbols"/>
                    <xsl:text>}</xsl:text>
                  </xsl:otherwise>
                </xsl:choose>
            </xsl:if>

          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true' and @class='msi_unit'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>      
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 2">        
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='lg'">
            <xsl:text xml:space="preserve">\lg </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='ln'">
            <xsl:text xml:space="preserve">\ln </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='Pr'">
            <xsl:text xml:space="preserve">\Pr </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 3">
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='arg'">
            <xsl:text xml:space="preserve">\arg </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='cos'">
            <xsl:text xml:space="preserve">\cos </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='cot'">
            <xsl:text xml:space="preserve">\cot </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='csc'">
            <xsl:text xml:space="preserve">\csc </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='deg'">
            <xsl:text xml:space="preserve">\deg </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='dim'">
            <xsl:text xml:space="preserve">\dim </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='exp'">
            <xsl:text xml:space="preserve">\exp </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='hom'">
            <xsl:text xml:space="preserve">\hom </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='ker'">
            <xsl:text xml:space="preserve">\ker </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='log'">
            <xsl:text xml:space="preserve">\log </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='sec'">
            <xsl:text xml:space="preserve">\sec </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='sin'">
            <xsl:text xml:space="preserve">\sin </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='tan'">
            <xsl:text xml:space="preserve">\tan </xsl:text>
          </xsl:when>
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
          <xsl:when test="normalize-space(string())='sup'">
            <xsl:text xml:space="preserve">\sup </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 4">
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='cosh'">
            <xsl:text xml:space="preserve">\cosh </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='coth'">
            <xsl:text xml:space="preserve">\coth </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='sinh'">
            <xsl:text xml:space="preserve">\sinh </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='tanh'">
            <xsl:text xml:space="preserve">\tanh </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='bmod'">
            <xsl:text xml:space="preserve">\bmod </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 6">
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='arccos'">
            <xsl:text xml:space="preserve">\arccos </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='arcsin'">
            <xsl:text xml:space="preserve">\arcsin </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='arctan'">
            <xsl:text xml:space="preserve">\arctan </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='injlim'">
            <xsl:text xml:space="preserve">\injlim </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='liminf'">
            <xsl:text xml:space="preserve">\liminf </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='limsup'">
            <xsl:text xml:space="preserve">\limsup </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 7">
<!--
\projlim<uID8.4.11>!\projlim!_LIMPLACE__FIRSTLIM__SECONDLIM_
-->
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='projlim'">
            <xsl:text xml:space="preserve">\projlim </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 9">
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='varinjlim'">
            <xsl:text xml:space="preserve">\varinjlim </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='varliminf'">
            <xsl:text xml:space="preserve">\varliminf </xsl:text>
          </xsl:when>
          <xsl:when test="normalize-space(string())='varlimsup'">
            <xsl:text xml:space="preserve">\varlimsup </xsl:text>
          </xsl:when>

          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="string-length(normalize-space(string())) = 10">
<!--
\varprojlim<uID8.4.16>!\varprojlim!_LIMPLACE__FIRSTLIM__SECONDLIM_
-->
        <xsl:if test="$tag!='false'">
          <xsl:value-of select="$tag"/>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="normalize-space(string())='varprojlim'">
            <xsl:text xml:space="preserve">\varprojlim </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="$tag != 'false'">
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:when>
              <xsl:when test="$all-ASCII='true'">
                <xsl:text>\operatorname{</xsl:text>
                  <xsl:value-of select="$LaTeX-symbols"/>
                <xsl:text>}</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$LaTeX-symbols"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="$tag!='false'">
          <xsl:text>}</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="$tag!='false'">
            <xsl:value-of select="$tag"/>
              <xsl:value-of select="$LaTeX-symbols"/>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="$all-ASCII='true'">
            <xsl:text>\operatorname{</xsl:text>
              <xsl:value-of select="$LaTeX-symbols"/>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$LaTeX-symbols"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>

    </xsl:choose>


  </xsl:template>


  <xsl:template name="get-tag">
    <xsl:param name="raw-LaTeX"/>
  
    <xsl:choose>
      <xsl:when test="@mathvariant='bold'
      or              ancestor::mml:mstyle[@fontweight='bold']">
        <xsl:text>\boldsymbol{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='italic'
      or              ancestor::mml:mstyle[@fontstyle='italic']">
        <xsl:text>\mathit{</xsl:text>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontstyle='slanted']">
        <xsl:if test="$output-mode='SW-LaTeX'">
            <xsl:text>\QTR{sl}{</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
            <xsl:text>\mathit{</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="ancestor::mml:mstyle[@fontweight='normal']">
        <xsl:text>\mathrm{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='sans-serif'">
        <xsl:text>\mathsf{</xsl:text>
      </xsl:when>
      <xsl:when test="@mathvariant='monospace'">
        <xsl:text>\mathtt{</xsl:text>
      </xsl:when>

      <xsl:when test="@mathvariant='script'">
        <!-- <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathcal{</xsl:text>
        <!-- </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='fraktur'">
        <!-- <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathfrak{</xsl:text>
        <!-- </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='double-struck'">
        <!-- <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathbb{</xsl:text>
        <!-- </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='bold-italic'">
<!--         <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))">
 -->          <xsl:text>\boldsymbol{</xsl:text>
<!--         </xsl:if>
 -->      </xsl:when>
      <xsl:when test="@mathvariant='bold-fraktur'"> <!-- requires unicode-math package -->
<!--         <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))">
 -->          <xsl:text>\mathbffrak{</xsl:text>
<!--         </xsl:if>
 -->      </xsl:when>
      <xsl:when test="@mathvariant='bold-script'">
<!--         <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))">
 -->          <xsl:text>\mathbfscr{</xsl:text>
<!--         </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='bold-sans-serif'">
<!--         <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathbfsfup{</xsl:text>
<!--         </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='sans-serif-italic'">
        <!-- <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathsfit{</xsl:text>
        <!-- </xsl:if> -->
      </xsl:when>
      <xsl:when test="@mathvariant='sans-serif-bold-italic'">
        <!-- <xsl:if test="string-length($raw-LaTeX) = string-length(normalize-space(string()))"> -->
          <xsl:text>\mathbfsfit{</xsl:text>
        <!-- </xsl:if> -->
      </xsl:when>

      <xsl:otherwise>

<!-- Note that LaTeX ignores size switches like \large in MATH.
  We script them because SW shows them on screen and NoteBook
  uses them when it prints. --> 

        <xsl:variable name="LaTeX-size-switch">
          <xsl:call-template name="get-LaTeX-size-from-attribs"/>
        </xsl:variable>

        <xsl:choose>
          <xsl:when test="string-length($LaTeX-size-switch)&gt;0">
            <xsl:text>{\</xsl:text>
            <xsl:value-of select="$LaTeX-size-switch"/>
            <xsl:text xml:space="preserve"> </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>false</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>

    </xsl:choose>
    </xsl:template>

<!-- JCS
  <xsl:template match="mml:mi" >
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>

-->

</xsl:stylesheet>

